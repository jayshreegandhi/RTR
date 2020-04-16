#include<windows.h>
#include<stdio.h>

#include<d3d11.h>
#include<d3dcompiler.h>

#pragma warning(disable : 4838)
#include"XNAMath/xnamath.h"

#include"WICTextureLoader.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ToggleFullScreen(void);
HRESULT initialize(void);
HRESULT resize(int, int);
void display(void);
void update(void);
void uninitialize(void);
HRESULT LoadD3DTexture(const wchar_t *, ID3D11ShaderResourceView **);

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghWnd = NULL;

bool gbFullScreen = false;
bool gbActiveWindow = false;

FILE *gpFile = NULL;
char gszLogFileName[] = "Log.txt";

float gClearColor[4];
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;

ID3D11Buffer *gpID3D11Buffer_VertexBuffer = NULL;

ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

ID3D11RasterizerState *gpID3D11RasterizerState = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

ID3D11ShaderResourceView *gpID3D11ShaderResourceView_Texture_Cube = NULL;
ID3D11SamplerState *gpID3D11SamplerState_Texture_Cube = NULL;

float angleCube = 0.0f;

struct CBUFFER
{
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;

	XMVECTOR La;
	XMVECTOR Ld;
	XMVECTOR Ls;
	XMVECTOR Light_Position;

	XMVECTOR Ka;
	XMVECTOR Kd;
	XMVECTOR Ks;

	float Material_Shininess;
	unsigned int LKeyPressed;
};

bool gbLight = false;

float LightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightPosition[4] = { 100.0f, 100.0f, -100.0f, 1.0f };

float MaterialAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float MaterialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess = 128.0f;
XMMATRIX gPerspectiveProjectionMatrix;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("Direct3D11");
	bool bDone = false;


	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can not be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "Log file created successfully...\n");
		fclose(gpFile);
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("D3D11 Window - Jayshree"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghWnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//initialize()
	HRESULT hr;
	hr = initialize();
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "initialize() failed. Exitting now...\n");
		fclose(gpFile);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "initialize() succeeded.\n");
		fclose(gpFile);
	}

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			//Play game here
			if (gbActiveWindow == true)
			{
				//code
				//here call update
				update();
			}
			display();
		}
	}

	//clean up
	uninitialize();

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;

	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;

	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "resize() failed.\n");
				fclose(gpFile);
				return(hr);
			}
			else
			{
				fopen_s(&gpFile, gszLogFileName, "a+");
				fprintf_s(gpFile, "resize() succeeded.\n");
				fclose(gpFile);
			}
		}
		break;

	case WM_ERASEBKGND:
		return(0);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'f':
		case 'F':
			if (gbFullScreen == false)
			{
				ToggleFullScreen();
				gbFullScreen = true;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = false;
			}
			break;

		case 'l':
		case 'L':
			if (gbLight == false)
			{
				gbLight = true;
			}
			else
			{
				gbLight = false;
			}
			break;
		}
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghWnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghWnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghWnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghWnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

HRESULT initialize(void)
{

	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquried = D3D_FEATURE_LEVEL_10_0;

	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;

	//code
	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void *)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghWnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;


	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,						//Adapter
			d3dDriverType,				//Driver Type
			NULL,						//Software
			createDeviceFlags,			//Flags
			&d3dFeatureLevel_required,	//Feature levels
			numFeatureLevels,			//Num feature levels
			D3D11_SDK_VERSION,			//SDK Version
			&dxgiSwapChainDesc,			//Swap Chain desc
			&gpIDXGISwapChain,			//Swap Chain
			&gpID3D11Device,			//Device
			&d3dFeatureLevel_acquried,	//Feature level
			&gpID3D11DeviceContext		//Device Context
		);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() succeeded.\n");

		fprintf_s(gpFile, "The choosen driver is of: \n");
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, "Hardware type. \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, "Warp type. \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, "Reference type. \n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown type. \n");
		}

		fprintf_s(gpFile, "The supported highest feature level is: \n");
		if (d3dFeatureLevel_acquried == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0 \n");
		}
		else if (d3dFeatureLevel_acquried == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10.1 \n");
		}
		else if (d3dFeatureLevel_acquried == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10.0 \n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown.\n");
		}

		fclose(gpFile);
	}

	//initialize shaders, input layout, constant buffers, etc.

	//**********************VERTEX SHADER**********************************************
	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"	float4x4 worldMatrix;" \
		"	float4x4 viewMatrix;" \
		"	float4x4 projectionMatrix;" \
		"	float4 la;" \
		"	float4 ld;" \
		"	float4 ls;" \
		"	float4 lightPosition;" \
		"	float4 ka;" \
		"	float4 kd;" \
		"	float4 ks;" \
		"	float material_shininess;" \
		"	uint lKeyPressed;" \
		"}" \
		"struct vertex_output" \
		"{"
		"	float4 position : SV_POSITION;" \
		"	float4 color : COLOR;" \
		"	float3 tNorm : NORMAL0;" \
		"	float3 lightDirection : NORMAL1;" \
		"	float3 viewerVector : NORMAL2;" \
		"	float2 texcoord : TEXCOORD;" \
		"};" \
		"vertex_output main(float4 pos : POSITION, float4 color : COLOR, float4 normal : NORMAL, float2 texcoord : TEXCOORD)" \
		"{" \
		"	vertex_output output;" \
		"	float4 eyeCoordinates = mul(worldMatrix,pos);" \
		"	eyeCoordinates = mul(viewMatrix,eyeCoordinates);" \
		"	output.tNorm = (float3)mul((float3x3)worldMatrix,(float3)normal);" \
		"	output.lightDirection = (float3)(lightPosition - eyeCoordinates);" \
		"	output.viewerVector = -eyeCoordinates.xyz;" \
		"	float4 position = mul(worldMatrix, pos);" \
		"	position = mul(viewMatrix, position);" \
		"	position = mul(projectionMatrix, position);" \
		"	output.position = position;" \
		"	output.color = color;" \
		"	output.texcoord = texcoord;" \
		"	return(output);" \
		"}";

	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() failed for vertex shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeeded for vertex shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11VertexShader);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateVertexShader() succeeded.\n");
		fclose(gpFile);
		//return(hr);
	}

	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, NULL, 0);

	//*************************PIXEL SHADER************************************
	const char *pixelShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"	float4x4 worldMatrix;" \
		"	float4x4 viewMatrix;" \
		"	float4x4 projectionMatrix;" \
		"	float4 la;" \
		"	float4 ld;" \
		"	float4 ls;" \
		"	float4 lightPosition;" \
		"	float4 ka;" \
		"	float4 kd;" \
		"	float4 ks;" \
		"	float material_shininess;" \
		"	uint lKeyPressed;" \
		"}" \
		"struct vertex_output" \
		"{"
		"	float4 position : SV_POSITION;" \
		"	float4 color : COLOR;" \
		"	float3 tNorm : NORMAL0;" \
		"	float3 lightDirection : NORMAL1;" \
		"	float3 viewerVector : NORMAL2;" \
		"	float2 texcoord : TEXCOORD;" \
		"};" \
		"Texture2D myTexture2D;" \
		"SamplerState mySamplerState;" \
		"float4 main(vertex_output input) : SV_TARGET" \
		"{" \
		"	float4 phong_ads_light;" \
		"	if(lKeyPressed == 1)" \
		"	{" \
		"		float3 normalized_tNorm = normalize(input.tNorm);" \
		"		float3 normalized_lightDirection = normalize(input.lightDirection);" \
		"		float tNorm_dot_lightDirection = max(dot(normalized_tNorm, normalized_lightDirection), 0.0);" \
		"		float3 reflectionVector = reflect(-normalized_lightDirection, normalized_tNorm);" \
		"		float3 normalized_viewerVector = normalize(input.viewerVector);" \
		"		float4 ambient = la * ka;" \
		"		float4 diffuse = ld * kd * tNorm_dot_lightDirection;" \
		"		float4 specular = ls * ks * pow(max(dot(reflectionVector, normalized_viewerVector), 0.0), material_shininess);" \
		"		phong_ads_light = input.color * myTexture2D.Sample(mySamplerState, input.texcoord) * (ambient  + diffuse + specular);" \
		"	}" \
		"	else" \
		"	{" \
		"		phong_ads_light = input.color * myTexture2D.Sample(mySamplerState, input.texcoord);" \
		"	}" \
		"	return(phong_ads_light);" \
		"}";

	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() failed for pixel shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3DCompile() succeeded for pixel shader.\n");
		fclose(gpFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(),
		pID3DBlob_PixelShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11PixelShader);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreatePixelShader() succeeded.\n");
		fclose(gpFile);
		//return(hr);
	}

	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, NULL, 0);

	//create and set input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[4];
	//position
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	//color
	inputElementDesc[1].SemanticName = "COLOR";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	//normal
	inputElementDesc[2].SemanticName = "NORMAL";
	inputElementDesc[2].SemanticIndex = 0;
	inputElementDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[2].InputSlot = 2;
	inputElementDesc[2].AlignedByteOffset = 0;
	inputElementDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[2].InstanceDataStepRate = 0;

	//texcoord
	inputElementDesc[3].SemanticName = "TEXCOORD";
	inputElementDesc[3].SemanticIndex = 0;
	inputElementDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[3].InputSlot = 3;
	inputElementDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputElementDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[3].InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateInputLayout() succeeded.\n");
		fclose(gpFile);
		//return(hr);
	}

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);

	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;

	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = NULL;

	float cubeVCNT[] =
	{
		//top
		//triangle one x,y,z,r,g,b,n1,n2,n3,u,v
		-1.0f, 1.0f, 1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,0.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,1.0f, 0.0f,
		//triangle two
		-1.0f, 1.0f, -1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		1.0f, 1.0f, -1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,1.0f, 1.0f,

		//bottom
		//triangle one
		1.0f, -1.0f, -1.0f,0.0f, 1.0f, 0.0f,0.0f, -1.0f, 0.0f,0.0f, 0.0f,
		1.0f, -1.0f, 1.0f,0.0f, 1.0f, 0.0f,0.0f, -1.0f, 0.0f,0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,0.0f, 1.0f, 0.0f,0.0f, -1.0f, 0.0f,1.0f, 0.0f,
		//triangle two
		-1.0f, -1.0f, -1.0f,0.0f, 1.0f, 0.0f,0.0f, -1.0f, 0.0f,1.0f, 0.0f,
		1.0f, -1.0f, 1.0f,0.0f, 1.0f, 0.0f,0.0f, -1.0f, 0.0f,0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,0.0f, 1.0f, 0.0f,0.0f, -1.0f, 0.0f,1.0f, 1.0f,

		//front
		//triangle one
		-1.0f, 1.0f, -1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f, -1.0f,0.0f, 0.0f,
		1.0f, 1.0f, -1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f, -1.0f,0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f, -1.0f,1.0f, 0.0f,
		//triangle two
		-1.0f, -1.0f, -1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f, -1.0f,1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f, -1.0f,0.0f, 1.0f,
		1.0f, -1.0f, -1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f, -1.0f,1.0f, 1.0f,

		//back
		//triangle one
		1.0f, -1.0f, 1.0f,0.0f, 1.0f, 1.0f,0.0f, 0.0f, 1.0f,0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,0.0f, 1.0f, 1.0f,0.0f, 0.0f, 1.0f,0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,0.0f, 1.0f, 1.0f,0.0f, 0.0f, 1.0f,1.0f, 0.0f,
		//triangle two
		-1.0f, -1.0f, 1.0f,0.0f, 1.0f, 1.0f,0.0f, 0.0f, 1.0f,1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,0.0f, 1.0f, 1.0f,0.0f, 0.0f, 1.0f,0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,0.0f, 1.0f, 1.0f,0.0f, 0.0f, 1.0f,1.0f, 1.0f,

		//left
		//triangle one
		-1.0f, 1.0f, 1.0f,1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,1.0f, 0.0f,
		//triangle two
		-1.0f, -1.0f, 1.0f,1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f,1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f,1.0f, 1.0f,

		//right
		//triangle one
		1.0f, -1.0f, -1.0f,1.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,0.0f, 0.0f,
		1.0f, 1.0f, -1.0f,1.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f,
		1.0f, -1.0f, 1.0f,1.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,1.0f, 0.0f,
		//triangle two
		1.0f, -1.0f, 1.0f,1.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,1.0f, 0.0f,
		1.0f, 1.0f, -1.0f,1.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,1.0f, 1.0f, 0.0f,1.0f, 0.0f, 0.0f,1.0f, 1.0f

	};

	//create vertex buffer
	//cube
	
	D3D11_BUFFER_DESC bufferDesc_VertexBuffer;
	ZeroMemory(&bufferDesc_VertexBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_VertexBuffer.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_VertexBuffer.ByteWidth = sizeof(float) * ARRAYSIZE(cubeVCNT);
	bufferDesc_VertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_VertexBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_VertexBuffer,
		NULL,
		&gpID3D11Buffer_VertexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() failed for vertex buffer cube.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for vertex buffer cube.\n");
		fclose(gpFile);
	}

	//copy vertices in above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, cubeVCNT, sizeof(cubeVCNT));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer, NULL);

	//constant buffer;
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer,
		NULL,
		&gpID3D11Buffer_ConstantBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() failed for constant buffer.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for constant buffer.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);
	
	//Raterization state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void *)&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	hr = gpID3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RasterizerState);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRasterizerState() failed for culling.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRasterizerState() succeeded for culling.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	//load textures
	//cube
	hr = LoadD3DTexture(L"marble.bmp", &gpID3D11ShaderResourceView_Texture_Cube);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::LoadD3DTexture() failed for cube texture.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::LoadD3DTexture() succeeded for pyramid texture.\n");
		fclose(gpFile);
	}

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Cube);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateSamplerState() failed for cube texture.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateSamplerState() succeeded for cube texture.\n");
		fclose(gpFile);
	}

	//clear color
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 1.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() succeeded.\n");
		fclose(gpFile);
	}

	return(S_OK);
}

HRESULT LoadD3DTexture(const wchar_t *textureFileName, ID3D11ShaderResourceView **ppID3D11ShaderResourceView)
{
	HRESULT hr;

	hr = DirectX::CreateWICTextureFromFile(gpID3D11Device, gpID3D11DeviceContext, textureFileName, NULL, ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateWICTextureFromFile() failed for texture resource.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "CreateWICTextureFromFile() succeeded for texture resource.\n");
		fclose(gpFile);
	}

	return(hr);
}


HRESULT resize(int width, int height)
{
	HRESULT hr = S_OK;

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2D_BackBuffer);

	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() succeeded.\n");
		fclose(gpFile);
	}

	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	//depth stencil view
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer;
	gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;


	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDepthStencilView() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateDepthStencilView() succeeded.\n");
		fclose(gpFile);
	}

	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	//set viewport
	D3D11_VIEWPORT d3dViewport;
	d3dViewport.TopLeftX = 0;
	d3dViewport.TopLeftY = 0;
	d3dViewport.Width = (float)width;
	d3dViewport.Height = (float)height;
	d3dViewport.MinDepth = 0.0f;
	d3dViewport.MaxDepth = 1.0f;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewport);

	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	return(hr);
}

void display(void)
{
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//cube
	//position
	UINT stride = sizeof(float) * 11;
	UINT offset = sizeof(float) * 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);

	//color
	offset = sizeof(float) * 3;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);

	//normal
	offset = sizeof(float) * 6;
	gpID3D11DeviceContext->IASetVertexBuffers(2, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);

	//texture
	offset = sizeof(float) * 9;
	gpID3D11DeviceContext->IASetVertexBuffers(3, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);

	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Texture_Cube);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Cube);

	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX projectionMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();

	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 6.0f);

	XMMATRIX r1 = XMMatrixRotationX(angleCube);
	XMMATRIX r2 = XMMatrixRotationY(angleCube);
	XMMATRIX r3 = XMMatrixRotationZ(angleCube);
	rotationMatrix = r1 * r2 * r3;

	XMMATRIX scaleMatrix = XMMatrixScaling(0.75f, 0.75f, 0.75f);

	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
	projectionMatrix = gPerspectiveProjectionMatrix;

	CBUFFER constantBuffer;
	constantBuffer.WorldMatrix = worldMatrix;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = projectionMatrix;
	if (gbLight == true)
	{
		constantBuffer.LKeyPressed = 1;

		constantBuffer.La = XMVectorSet(LightAmbient[0], LightAmbient[1], LightAmbient[2], LightAmbient[3]);
		constantBuffer.Ld = XMVectorSet(LightDiffuse[0], LightDiffuse[1], LightDiffuse[2], LightDiffuse[3]);
		constantBuffer.Ls = XMVectorSet(LightSpecular[0], LightSpecular[1], LightSpecular[2], LightSpecular[3]);
		constantBuffer.Light_Position = XMVectorSet(LightPosition[0], LightPosition[1], LightPosition[2], LightPosition[3]);

		constantBuffer.Ka = XMVectorSet(MaterialAmbient[0], MaterialAmbient[1], MaterialAmbient[2], MaterialAmbient[3]);
		constantBuffer.Kd = XMVectorSet(MaterialDiffuse[0], MaterialDiffuse[1], MaterialDiffuse[2], MaterialDiffuse[3]);
		constantBuffer.Ks = XMVectorSet(MaterialSpecular[0], MaterialSpecular[1], MaterialSpecular[2], MaterialSpecular[3]);

		constantBuffer.Material_Shininess = MaterialShininess;
	}
	else
	{
		constantBuffer.LKeyPressed = 0;
	}

	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	gpID3D11DeviceContext->Draw(6, 0);
	gpID3D11DeviceContext->Draw(6, 6);
	gpID3D11DeviceContext->Draw(6, 12);
	gpID3D11DeviceContext->Draw(6, 18);
	gpID3D11DeviceContext->Draw(6, 24);
	gpID3D11DeviceContext->Draw(6, 30);

	gpIDXGISwapChain->Present(0, 0);
}

void update(void)
{

	angleCube = angleCube - 0.002f;
	if (angleCube <= 0.0f)
	{
		angleCube = angleCube + 360.0f;
	}

}


void uninitialize(void)
{

	if (gpID3D11SamplerState_Texture_Cube)
	{
		gpID3D11SamplerState_Texture_Cube->Release();
		gpID3D11SamplerState_Texture_Cube = NULL;
	}

	if (gpID3D11ShaderResourceView_Texture_Cube)
	{
		gpID3D11ShaderResourceView_Texture_Cube->Release();
		gpID3D11ShaderResourceView_Texture_Cube = NULL;
	}

	if (gpID3D11RasterizerState)
	{
		gpID3D11RasterizerState->Release();
		gpID3D11RasterizerState = NULL;
	}

	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer)
	{
		gpID3D11Buffer_VertexBuffer->Release();
		gpID3D11Buffer_VertexBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}

	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}

	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}

	if (gpFile)
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "uninitialize() succeeded.\n");
		fprintf_s(gpFile, "Log file closed successfully.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
