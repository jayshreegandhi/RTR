#include<windows.h>
#include<stdio.h>

#define _USE_MATH_DEFINES
#include<math.h>

#include<d3d11.h>
#include<d3dcompiler.h>

#pragma warning(disable : 4838)
#include"XNAMath/xnamath.h"

#include "Sphere.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "Sphere.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ToggleFullScreen(void);
HRESULT initialize(void);
HRESULT resize(int, int);
void display(void);
void update(void);
void uninitialize(void);

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

ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Position_Sphere = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Normal_Sphere = NULL;
ID3D11Buffer *gpID3D11Buffer_IndexBuffer_Sphere = NULL;

ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

ID3D11RasterizerState *gpID3D11RasterizerState = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

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

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_texture[746];
unsigned short sphere_elements[2280];
int gNumVertices = 0;
int gNumElements = 0;

float LightAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightPosition[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

float MaterialAmbient[24][4];
float MaterialDiffuse[24][4];
float MaterialSpecular[24][4];
float MaterialShininess[24];

float rotationAngleX = 0.0f;
float rotationAngleY = 0.0f;
float rotationAngleZ = 0.0f;

float angleZeroRadian;
float angleOneRadian;
float angleTwoRadian;

int keyPress = 0; 

void setMaterialSphere(void);

int gWidth;
int gHeight;


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
		gWidth = LOWORD(lParam);
		gHeight = HIWORD(lParam);

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

	case WM_CHAR:
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

		case 'X':
		case 'x':
			keyPress = 1;
			rotationAngleX = 0.0f;
			break;

		case 'Y':
		case 'y':
			keyPress = 2;
			rotationAngleY = 0.0f;
			break;

		case 'Z':
		case 'z':
			keyPress = 3;
			rotationAngleZ = 0.0f;
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
		"{" \
		"	float4 position : SV_POSITION;" \
		"	float3 tNorm : NORMAL0;" \
		"	float3 lightDirection : NORMAL1;" \
		"	float3 viewerVector : NORMAL2;" \
		"};" \
		"vertex_output main(float4 pos : POSITION, float4 normal : NORMAL)" \
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
		"{" \
		"	float4 position : SV_POSITION;" \
		"	float3 tNorm : NORMAL0;" \
		"	float3 lightDirection : NORMAL1;" \
		"	float3 viewerVector : NORMAL2;" \
		"};" \
		"float4 main(float4 pos : SV_POSITION, vertex_output input) : SV_TARGET" \
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
		"		phong_ads_light = ambient + diffuse + specular ;" \
		"	}" \
		"	else" \
		"	{" \
		"		phong_ads_light = float4(1.0, 1.0, 1.0, 1.0);" \
		"	}" \
		"	float4 color = phong_ads_light;" \
		"	return(color);" \
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
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	//position
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	//normal
	inputElementDesc[1].SemanticName = "NORMAL";
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

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

	getSphereVertexData(sphere_vertices,
		sphere_normals,
		sphere_texture,
		sphere_elements);

	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	//create vertex buffer
	//sphere
	//position
	D3D11_BUFFER_DESC bufferDesc_VertexBuffer_Poition_Sphere;
	ZeroMemory(&bufferDesc_VertexBuffer_Poition_Sphere, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_VertexBuffer_Poition_Sphere.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_VertexBuffer_Poition_Sphere.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_vertices);
	bufferDesc_VertexBuffer_Poition_Sphere.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_VertexBuffer_Poition_Sphere.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_VertexBuffer_Poition_Sphere,
		NULL,
		&gpID3D11Buffer_VertexBuffer_Position_Sphere);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() failed for vertex buffer position sphere.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for vertex buffer position sphere.\n");
		fclose(gpFile);
	}

	//copy vertices in above buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Position_Sphere, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_vertices, sizeof(sphere_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Position_Sphere, NULL);

	//normal
	D3D11_BUFFER_DESC bufferDesc_VertexBuffer_Normal_Sphere;
	ZeroMemory(&bufferDesc_VertexBuffer_Normal_Sphere, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_VertexBuffer_Normal_Sphere.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_VertexBuffer_Normal_Sphere.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_normals);
	bufferDesc_VertexBuffer_Normal_Sphere.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc_VertexBuffer_Normal_Sphere.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_VertexBuffer_Normal_Sphere,
		NULL,
		&gpID3D11Buffer_VertexBuffer_Normal_Sphere);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() failed for vertex buffer normal sphere.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for vertex buffer normal sphere.\n");
		fclose(gpFile);
	}

	//copy vertices in above buffer
	//D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Normal_Sphere, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_normals, sizeof(sphere_normals));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Normal_Sphere, NULL);

	//create index buffer
	//elements
	D3D11_BUFFER_DESC bufferDesc_IndexBuffer_Element_Sphere;
	ZeroMemory(&bufferDesc_IndexBuffer_Element_Sphere, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_IndexBuffer_Element_Sphere.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc_IndexBuffer_Element_Sphere.ByteWidth = sizeof(short) * gNumElements;
	bufferDesc_IndexBuffer_Element_Sphere.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc_IndexBuffer_Element_Sphere.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_IndexBuffer_Element_Sphere,
		NULL,
		&gpID3D11Buffer_IndexBuffer_Sphere);
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() failed for index buffer element sphere.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateBuffer() succeeded for index buffer element sphere.\n");
		fclose(gpFile);
	}

	//copy vertices in above buffer
	//D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_IndexBuffer_Sphere, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_elements, gNumElements * sizeof(short));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer_Sphere, NULL);

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
		fprintf_s(gpFile, "ID3D11Device::CreateRasterizerState succeeded for culling.\n");
		fclose(gpFile);
	}

	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	//clear color
	gClearColor[0] = 0.25f;
	gClearColor[1] = 0.25f;
	gClearColor[2] = 0.25f;
	gClearColor[3] = 1.0f;

	setMaterialSphere();

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

	if (height == 0)
	{
		height = 1;
	}

	//set viewport
	D3D11_VIEWPORT d3dViewport;
	ZeroMemory(&d3dViewport, sizeof(D3D11_VIEWPORT));
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

	int wid = gWidth / 4;
	int ht = gHeight / 6;

	//set viewport
	D3D11_VIEWPORT d3dViewport;
	ZeroMemory(&d3dViewport, sizeof(D3D11_VIEWPORT));
	d3dViewport.Width = (float)wid;
	d3dViewport.Height = (float)ht;
	d3dViewport.MinDepth = 0.0f;
	d3dViewport.MaxDepth = 1.0f;
	
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 6; y++)
		{
			d3dViewport.TopLeftX = (float)(x * wid);
			d3dViewport.TopLeftY = (float)(y * ht);
			gpID3D11DeviceContext->RSSetViewports(1, &d3dViewport);
			gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)wid / (float)ht, 0.1f, 100.0f);

			UINT stride = sizeof(float) * 3;
			UINT offset = 0;
			gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Position_Sphere, &stride, &offset);

			stride = sizeof(float) * 3;
			offset = 0;
			gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Normal_Sphere, &stride, &offset);

			gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer_Sphere, DXGI_FORMAT_R16_UINT, 0);

			gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			XMMATRIX worldMatrix = XMMatrixIdentity();
			XMMATRIX viewMatrix = XMMatrixIdentity();
			XMMATRIX projectionMatrix = XMMatrixIdentity();
			XMMATRIX translationMatrix = XMMatrixIdentity();

			worldMatrix = XMMatrixIdentity();
			viewMatrix = XMMatrixIdentity();
			projectionMatrix = XMMatrixIdentity();
			translationMatrix = XMMatrixIdentity();

			translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);

			worldMatrix = translationMatrix;
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

				if (keyPress == 1)
				{
					angleZeroRadian = (float)(M_PI * rotationAngleX / 180.0f);
					LightPosition[0] = 0.0f;
					LightPosition[1] = ((float)cos(angleZeroRadian) / 2.0f - (float)sin(angleZeroRadian) / 2.0f);
					LightPosition[2] = -((float)sin(angleZeroRadian) / 2.0f + (float)cos(angleZeroRadian) / 2.0f);
					LightPosition[3] = 1.0f;
					constantBuffer.Light_Position = XMVectorSet(LightPosition[0], LightPosition[1], LightPosition[2], LightPosition[3]);
				}
				else if (keyPress == 2)
				{
					angleOneRadian = (float)(M_PI * rotationAngleY / 180.0f);
					LightPosition[0] = ((float)cos(angleOneRadian) / 2.0f + (float)sin(angleOneRadian) / 2.0f);
					LightPosition[1] = 0.0f;
					LightPosition[2] = (-(float)sin(angleOneRadian) / 2.0f + (float)cos(angleOneRadian) / 2.0f);
					LightPosition[3] = 1.0f;
					constantBuffer.Light_Position = XMVectorSet(LightPosition[0], LightPosition[1], LightPosition[2], LightPosition[3]);
				}
				else if (keyPress == 3)
				{
					angleTwoRadian = (float)(M_PI * rotationAngleZ / 180.0f);
					LightPosition[0] = ((float)cos(angleTwoRadian) / 2.0f - (float)sin(angleTwoRadian) / 2.0f);
					LightPosition[1] = ((float)sin(angleTwoRadian) / 2.0f + (float)cos(angleTwoRadian) / 2.0f);
					LightPosition[2] = -2.0f;
					LightPosition[3] = 1.0f;
					constantBuffer.Light_Position = XMVectorSet(LightPosition[0], LightPosition[1], LightPosition[2], LightPosition[3]);
				}
				else
				{
					constantBuffer.Light_Position = XMVectorSet(LightPosition[0], LightPosition[1], LightPosition[2], LightPosition[3]);
				}

				if (x == 0)
				{
					if (y == 0)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[0][0], MaterialAmbient[0][1], MaterialAmbient[0][2], MaterialAmbient[0][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[0][0], MaterialDiffuse[0][1], MaterialDiffuse[0][2], MaterialDiffuse[0][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[0][0], MaterialSpecular[0][1], MaterialSpecular[0][2], MaterialSpecular[0][3]);
						constantBuffer.Material_Shininess = MaterialShininess[0];
					}
					else if (y == 1)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[1][0], MaterialAmbient[1][1], MaterialAmbient[1][2], MaterialAmbient[1][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[1][0], MaterialDiffuse[1][1], MaterialDiffuse[1][2], MaterialDiffuse[1][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[1][0], MaterialSpecular[1][1], MaterialSpecular[1][2], MaterialSpecular[1][3]);
						constantBuffer.Material_Shininess = MaterialShininess[1];
					}
					else if (y == 2)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[2][0], MaterialAmbient[2][1], MaterialAmbient[2][2], MaterialAmbient[2][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[2][0], MaterialDiffuse[2][1], MaterialDiffuse[2][2], MaterialDiffuse[2][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[2][0], MaterialSpecular[2][1], MaterialSpecular[2][2], MaterialSpecular[2][3]);
						constantBuffer.Material_Shininess = MaterialShininess[2];
					}
					else if (y == 3)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[3][0], MaterialAmbient[3][1], MaterialAmbient[3][2], MaterialAmbient[3][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[3][0], MaterialDiffuse[3][1], MaterialDiffuse[3][2], MaterialDiffuse[3][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[3][0], MaterialSpecular[3][1], MaterialSpecular[3][2], MaterialSpecular[3][3]);
						constantBuffer.Material_Shininess = MaterialShininess[3];
					}
					else if (y == 4)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[4][0], MaterialAmbient[4][1], MaterialAmbient[4][2], MaterialAmbient[4][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[4][0], MaterialDiffuse[4][1], MaterialDiffuse[4][2], MaterialDiffuse[4][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[4][0], MaterialSpecular[4][1], MaterialSpecular[4][2], MaterialSpecular[4][3]);
						constantBuffer.Material_Shininess = MaterialShininess[4];
					}
					else if (y == 5)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[5][0], MaterialAmbient[5][1], MaterialAmbient[5][2], MaterialAmbient[5][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[5][0], MaterialDiffuse[5][1], MaterialDiffuse[5][2], MaterialDiffuse[5][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[5][0], MaterialSpecular[5][1], MaterialSpecular[5][2], MaterialSpecular[5][3]);
						constantBuffer.Material_Shininess = MaterialShininess[5];
					}
				}
				else if (x == 1)
				{
					if (y == 0)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[6][0], MaterialAmbient[6][1], MaterialAmbient[6][2], MaterialAmbient[6][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[6][0], MaterialDiffuse[6][1], MaterialDiffuse[6][2], MaterialDiffuse[6][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[6][0], MaterialSpecular[6][1], MaterialSpecular[6][2], MaterialSpecular[6][3]);
						constantBuffer.Material_Shininess = MaterialShininess[6];
					}
					else if (y == 1)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[7][0], MaterialAmbient[7][1], MaterialAmbient[7][2], MaterialAmbient[7][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[7][0], MaterialDiffuse[7][1], MaterialDiffuse[7][2], MaterialDiffuse[7][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[7][0], MaterialSpecular[7][1], MaterialSpecular[7][2], MaterialSpecular[7][3]);
						constantBuffer.Material_Shininess = MaterialShininess[7];
					}
					else if (y == 2)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[8][0], MaterialAmbient[8][1], MaterialAmbient[8][2], MaterialAmbient[8][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[8][0], MaterialDiffuse[8][1], MaterialDiffuse[8][2], MaterialDiffuse[8][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[8][0], MaterialSpecular[8][1], MaterialSpecular[8][2], MaterialSpecular[8][3]);
						constantBuffer.Material_Shininess = MaterialShininess[8];
					}
					else if (y == 3)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[9][0], MaterialAmbient[9][1], MaterialAmbient[9][2], MaterialAmbient[9][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[9][0], MaterialDiffuse[9][1], MaterialDiffuse[9][2], MaterialDiffuse[9][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[9][0], MaterialSpecular[9][1], MaterialSpecular[9][2], MaterialSpecular[9][3]);
						constantBuffer.Material_Shininess = MaterialShininess[9];
					}
					else if (y == 4)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[10][0], MaterialAmbient[10][1], MaterialAmbient[10][2], MaterialAmbient[10][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[10][0], MaterialDiffuse[10][1], MaterialDiffuse[10][2], MaterialDiffuse[10][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[10][0], MaterialSpecular[10][1], MaterialSpecular[10][2], MaterialSpecular[10][3]);
						constantBuffer.Material_Shininess = MaterialShininess[10];
					}
					else if (y == 5)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[11][0], MaterialAmbient[11][1], MaterialAmbient[11][2], MaterialAmbient[11][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[11][0], MaterialDiffuse[11][1], MaterialDiffuse[11][2], MaterialDiffuse[11][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[11][0], MaterialSpecular[11][1], MaterialSpecular[11][2], MaterialSpecular[11][3]);
						constantBuffer.Material_Shininess = MaterialShininess[11];
					}
				}
				else if (x == 2)
				{
					if (y == 0)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[12][0], MaterialAmbient[12][1], MaterialAmbient[12][2], MaterialAmbient[12][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[12][0], MaterialDiffuse[12][1], MaterialDiffuse[12][2], MaterialDiffuse[12][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[12][0], MaterialSpecular[12][1], MaterialSpecular[12][2], MaterialSpecular[12][3]);
						constantBuffer.Material_Shininess = MaterialShininess[12];
					}
					else if (y == 1)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[13][0], MaterialAmbient[13][1], MaterialAmbient[13][2], MaterialAmbient[13][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[13][0], MaterialDiffuse[13][1], MaterialDiffuse[13][2], MaterialDiffuse[13][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[13][0], MaterialSpecular[13][1], MaterialSpecular[13][2], MaterialSpecular[13][3]);
						constantBuffer.Material_Shininess = MaterialShininess[13];
					}
					else if (y == 2)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[14][0], MaterialAmbient[14][1], MaterialAmbient[14][2], MaterialAmbient[14][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[14][0], MaterialDiffuse[14][1], MaterialDiffuse[14][2], MaterialDiffuse[14][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[14][0], MaterialSpecular[14][1], MaterialSpecular[14][2], MaterialSpecular[14][3]);
						constantBuffer.Material_Shininess = MaterialShininess[14];
					}
					else if (y == 3)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[15][0], MaterialAmbient[15][1], MaterialAmbient[15][2], MaterialAmbient[15][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[15][0], MaterialDiffuse[15][1], MaterialDiffuse[15][2], MaterialDiffuse[15][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[15][0], MaterialSpecular[15][1], MaterialSpecular[15][2], MaterialSpecular[15][3]);
						constantBuffer.Material_Shininess = MaterialShininess[15];
					}
					else if (y == 4)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[16][0], MaterialAmbient[16][1], MaterialAmbient[16][2], MaterialAmbient[16][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[16][0], MaterialDiffuse[16][1], MaterialDiffuse[16][2], MaterialDiffuse[16][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[16][0], MaterialSpecular[16][1], MaterialSpecular[16][2], MaterialSpecular[16][3]);
						constantBuffer.Material_Shininess = MaterialShininess[16];
					}
					else if (y == 5)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[17][0], MaterialAmbient[17][1], MaterialAmbient[17][2], MaterialAmbient[17][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[17][0], MaterialDiffuse[17][1], MaterialDiffuse[17][2], MaterialDiffuse[17][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[17][0], MaterialSpecular[17][1], MaterialSpecular[17][2], MaterialSpecular[17][3]);
						constantBuffer.Material_Shininess = MaterialShininess[17];
					}
				}
				else if (x == 3)
				{
					if (y == 0)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[18][0], MaterialAmbient[18][1], MaterialAmbient[18][2], MaterialAmbient[18][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[18][0], MaterialDiffuse[18][1], MaterialDiffuse[18][2], MaterialDiffuse[18][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[18][0], MaterialSpecular[18][1], MaterialSpecular[18][2], MaterialSpecular[18][3]);
						constantBuffer.Material_Shininess = MaterialShininess[18];
					}
					else if (y == 1)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[19][0], MaterialAmbient[19][1], MaterialAmbient[19][2], MaterialAmbient[19][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[19][0], MaterialDiffuse[19][1], MaterialDiffuse[19][2], MaterialDiffuse[19][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[19][0], MaterialSpecular[19][1], MaterialSpecular[19][2], MaterialSpecular[19][3]);
						constantBuffer.Material_Shininess = MaterialShininess[19];
					}
					else if (y == 2)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[20][0], MaterialAmbient[20][1], MaterialAmbient[20][2], MaterialAmbient[20][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[20][0], MaterialDiffuse[20][1], MaterialDiffuse[20][2], MaterialDiffuse[20][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[20][0], MaterialSpecular[20][1], MaterialSpecular[20][2], MaterialSpecular[20][3]);
						constantBuffer.Material_Shininess = MaterialShininess[20];
					}
					else if (y == 3)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[21][0], MaterialAmbient[21][1], MaterialAmbient[21][2], MaterialAmbient[21][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[21][0], MaterialDiffuse[21][1], MaterialDiffuse[21][2], MaterialDiffuse[21][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[21][0], MaterialSpecular[21][1], MaterialSpecular[21][2], MaterialSpecular[21][3]);
						constantBuffer.Material_Shininess = MaterialShininess[21];
					}
					else if (y == 4)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[22][0], MaterialAmbient[22][1], MaterialAmbient[22][2], MaterialAmbient[22][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[22][0], MaterialDiffuse[22][1], MaterialDiffuse[22][2], MaterialDiffuse[22][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[22][0], MaterialSpecular[22][1], MaterialSpecular[22][2], MaterialSpecular[22][3]);
						constantBuffer.Material_Shininess = MaterialShininess[22];
					}
					else if (y == 5)
					{
						constantBuffer.Ka = XMVectorSet(MaterialAmbient[23][0], MaterialAmbient[23][1], MaterialAmbient[23][2], MaterialAmbient[23][3]);
						constantBuffer.Kd = XMVectorSet(MaterialDiffuse[23][0], MaterialDiffuse[23][1], MaterialDiffuse[23][2], MaterialDiffuse[23][3]);
						constantBuffer.Ks = XMVectorSet(MaterialSpecular[23][0], MaterialSpecular[23][1], MaterialSpecular[23][2], MaterialSpecular[23][3]);
						constantBuffer.Material_Shininess = MaterialShininess[23];
					}
				}
			}
			else
			{
				constantBuffer.LKeyPressed = 0;
			}

			gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

			gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);
		}
	}
	gpIDXGISwapChain->Present(0, 0);
}

void update(void)
{
	rotationAngleX = rotationAngleX + 1.0f;
	if (rotationAngleX >= 360.0f)
	{
		rotationAngleX = 0.0f;
	}

	rotationAngleY = rotationAngleY + 1.0f;
	if (rotationAngleY >= 360.0f)
	{
		rotationAngleY = 0.0f;
	}

	rotationAngleZ = rotationAngleZ + 1.0f;
	if (rotationAngleZ >= 360.0f)
	{
		rotationAngleZ = 0.0f;
	}
}

void uninitialize(void)
{
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

	if (gpID3D11Buffer_IndexBuffer_Sphere)
	{
		gpID3D11Buffer_IndexBuffer_Sphere->Release();
		gpID3D11Buffer_IndexBuffer_Sphere = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Normal_Sphere)
	{
		gpID3D11Buffer_VertexBuffer_Normal_Sphere->Release();
		gpID3D11Buffer_VertexBuffer_Normal_Sphere = NULL;
	}

	if (gpID3D11Buffer_VertexBuffer_Position_Sphere)
	{
		gpID3D11Buffer_VertexBuffer_Position_Sphere->Release();
		gpID3D11Buffer_VertexBuffer_Position_Sphere = NULL;
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

void setMaterialSphere(void)
{
	//material 1 : Emerald
	MaterialAmbient[0][0] = 0.0215f;
	MaterialAmbient[0][1] = 0.1745f;
	MaterialAmbient[0][2] = 0.0215f;
	MaterialAmbient[0][3] = 1.0f;

	MaterialDiffuse[0][0] = 0.07568f;
	MaterialDiffuse[0][1] = 0.61424f;
	MaterialDiffuse[0][2] = 0.07568f;
	MaterialDiffuse[0][3] = 1.0f;

	MaterialSpecular[0][0] = 0.633f;
	MaterialSpecular[0][1] = 0.727811f;
	MaterialSpecular[0][2] = 0.633f;
	MaterialSpecular[0][3] = 1.0f;

	MaterialShininess[0] = 0.6f * 128.0f;

	//material 2: Jade
	MaterialAmbient[1][0] = 0.135f;
	MaterialAmbient[1][1] = 0.225f;
	MaterialAmbient[1][2] = 0.1575f;
	MaterialAmbient[1][3] = 1.0f;

	MaterialDiffuse[1][0] = 0.54f;
	MaterialDiffuse[1][1] = 0.89f;
	MaterialDiffuse[1][2] = 0.63f;
	MaterialDiffuse[1][3] = 1.0f;

	MaterialSpecular[1][0] = 0.316228f;
	MaterialSpecular[1][1] = 0.316228f;
	MaterialSpecular[1][2] = 0.316228f;
	MaterialSpecular[1][3] = 1.0f;

	MaterialShininess[1] = 0.1f * 128.0f;

	//material 3 : Obsidian
	MaterialAmbient[2][0] = 0.05375f;
	MaterialAmbient[2][1] = 0.05f;
	MaterialAmbient[2][2] = 0.06625f;
	MaterialAmbient[2][3] = 1.0f;

	MaterialDiffuse[2][0] = 0.18275f;
	MaterialDiffuse[2][1] = 0.17f;
	MaterialDiffuse[2][2] = 0.22525f;
	MaterialDiffuse[2][3] = 1.0f;

	MaterialSpecular[2][0] = 0.332741f;
	MaterialSpecular[2][1] = 0.328634f;
	MaterialSpecular[2][2] = 0.346435f;
	MaterialSpecular[2][3] = 1.0f;

	MaterialShininess[2] = 0.3f * 128.0f;


	//material 4 : Pearl
	MaterialAmbient[3][0] = 0.25f;
	MaterialAmbient[3][1] = 0.20725f;
	MaterialAmbient[3][2] = 0.20725f;
	MaterialAmbient[3][3] = 1.0f;

	MaterialDiffuse[3][0] = 1.0f;
	MaterialDiffuse[3][1] = 0.829f;
	MaterialDiffuse[3][2] = 0.829f;
	MaterialDiffuse[3][3] = 1.0f;

	MaterialSpecular[3][0] = 0.296648f;
	MaterialSpecular[3][1] = 0.296648f;
	MaterialSpecular[3][2] = 0.296648f;
	MaterialSpecular[3][3] = 1.0f;

	MaterialShininess[3] = 0.088f * 128.0f;

	//material 5 : Ruby

	MaterialAmbient[4][0] = 0.1745f;
	MaterialAmbient[4][1] = 0.01175f;
	MaterialAmbient[4][2] = 0.01175f;
	MaterialAmbient[4][3] = 1.0f;

	MaterialDiffuse[4][0] = 0.61424f;
	MaterialDiffuse[4][1] = 0.04136f;
	MaterialDiffuse[4][2] = 0.04136f;
	MaterialDiffuse[4][3] = 1.0f;

	MaterialSpecular[4][0] = 0.727811f;
	MaterialSpecular[4][1] = 0.626959f;
	MaterialSpecular[4][2] = 0.626959f;
	MaterialSpecular[4][3] = 1.0f;

	MaterialShininess[4] = 0.6f * 128.0f;

	//material 6 : Turquoise

	MaterialAmbient[5][0] = 0.1f;
	MaterialAmbient[5][1] = 0.18725f;
	MaterialAmbient[5][2] = 0.1745f;
	MaterialAmbient[5][3] = 1.0f;

	MaterialDiffuse[5][0] = 0.396f;
	MaterialDiffuse[5][1] = 0.74151f;
	MaterialDiffuse[5][2] = 0.69102f;
	MaterialDiffuse[5][3] = 1.0f;

	MaterialSpecular[5][0] = 0.297254f;
	MaterialSpecular[5][1] = 0.30829f;
	MaterialSpecular[5][2] = 0.306678f;
	MaterialSpecular[5][3] = 1.0f;

	MaterialShininess[5] = 0.1f * 128.0f;

	//---------------COLUMN 2 : METAL -----------------------------
	//material 1 : Brass
	MaterialAmbient[6][0] = 0.329412f;
	MaterialAmbient[6][1] = 0.223529f;
	MaterialAmbient[6][2] = 0.027451f;
	MaterialAmbient[6][3] = 1.0f;

	MaterialDiffuse[6][0] = 0.780392f;
	MaterialDiffuse[6][1] = 0.568627f;
	MaterialDiffuse[6][2] = 0.113725f;
	MaterialDiffuse[6][3] = 1.0f;

	MaterialSpecular[6][0] = 0.992157f;
	MaterialSpecular[6][1] = 0.941176f;
	MaterialSpecular[6][2] = 0.807843f;
	MaterialSpecular[6][3] = 1.0f;

	MaterialShininess[6] = 0.21794872f * 128.0f;

	//material  2 : Bronze
	MaterialAmbient[7][0] = 0.2125f;
	MaterialAmbient[7][1] = 0.1275f;
	MaterialAmbient[7][2] = 0.054f;
	MaterialAmbient[7][3] = 1.0f;

	MaterialDiffuse[7][0] = 0.714f;
	MaterialDiffuse[7][1] = 0.4284f;
	MaterialDiffuse[7][2] = 0.18144f;
	MaterialDiffuse[7][3] = 1.0f;

	MaterialSpecular[7][0] = 0.393548f;
	MaterialSpecular[7][1] = 0.271906f;
	MaterialSpecular[7][2] = 0.166721f;
	MaterialSpecular[7][3] = 1.0f;

	MaterialShininess[7] = 0.2f * 128.0f;

	//material 3 : Chrome 
	MaterialAmbient[8][0] = 0.25f;
	MaterialAmbient[8][1] = 0.25f;
	MaterialAmbient[8][2] = 0.25f;
	MaterialAmbient[8][3] = 1.0f;

	MaterialDiffuse[8][0] = 0.4f;
	MaterialDiffuse[8][1] = 0.4f;
	MaterialDiffuse[8][2] = 0.4f;
	MaterialDiffuse[8][3] = 1.0f;

	MaterialSpecular[8][0] = 0.774597f;
	MaterialSpecular[8][1] = 0.774597f;
	MaterialSpecular[8][2] = 0.774597f;
	MaterialSpecular[8][3] = 1.0f;

	MaterialShininess[8] = 0.6f * 128.0f;

	//material 4 : Copper
	MaterialAmbient[9][0] = 0.19125f;
	MaterialAmbient[9][1] = 0.0735f;
	MaterialAmbient[9][2] = 0.0225f;
	MaterialAmbient[9][3] = 1.0f;

	MaterialDiffuse[9][0] = 0.7038f;
	MaterialDiffuse[9][1] = 0.27048f;
	MaterialDiffuse[9][2] = 0.0828f;
	MaterialDiffuse[9][3] = 1.0f;

	MaterialSpecular[9][0] = 0.256777f;
	MaterialSpecular[9][1] = 0.1376222f;
	MaterialSpecular[9][2] = 0.086014f;
	MaterialSpecular[9][3] = 1.0f;

	MaterialShininess[9] = 0.1f * 128.0f;

	//material 5 : Gold
	MaterialAmbient[10][0] = 0.24725f;
	MaterialAmbient[10][1] = 0.1995f;
	MaterialAmbient[10][2] = 0.0745f;
	MaterialAmbient[10][3] = 1.0f;

	MaterialDiffuse[10][0] = 0.75164f;
	MaterialDiffuse[10][1] = 0.60648f;
	MaterialDiffuse[10][2] = 0.22648f;
	MaterialDiffuse[10][3] = 1.0f;

	MaterialSpecular[10][0] = 0.628281f;
	MaterialSpecular[10][1] = 0.555802f;
	MaterialSpecular[10][2] = 0.366065f;
	MaterialSpecular[10][3] = 1.0f;

	MaterialShininess[10] = 0.4f * 128.0f;

	//material 6 : Silver
	MaterialAmbient[11][0] = 0.19225f;
	MaterialAmbient[11][1] = 0.19225f;
	MaterialAmbient[11][2] = 0.19225f;
	MaterialAmbient[11][3] = 1.0f;

	MaterialDiffuse[11][0] = 0.50754f;
	MaterialDiffuse[11][1] = 0.50754f;
	MaterialDiffuse[11][2] = 0.50754f;
	MaterialDiffuse[11][3] = 1.0f;

	MaterialSpecular[11][0] = 0.508273f;
	MaterialSpecular[11][1] = 0.508273f;
	MaterialSpecular[11][2] = 0.508273f;
	MaterialSpecular[11][3] = 1.0f;

	MaterialShininess[11] = 0.4f * 128.0f;

	//---------------COLUMN 3 : PLASTIC -----------------------------
	//material 1 : Black

	MaterialAmbient[12][0] = 0.0f;
	MaterialAmbient[12][1] = 0.0f;
	MaterialAmbient[12][2] = 0.0f;
	MaterialAmbient[12][3] = 1.0f;

	MaterialDiffuse[12][0] = 0.0f;
	MaterialDiffuse[12][1] = 0.0f;
	MaterialDiffuse[12][2] = 0.0f;
	MaterialDiffuse[12][3] = 1.0f;

	MaterialSpecular[12][0] = 0.50f;
	MaterialSpecular[12][1] = 0.50f;
	MaterialSpecular[12][2] = 0.50f;
	MaterialSpecular[12][3] = 1.0f;

	MaterialShininess[12] = 0.25f * 128.0f;

	//material  2 : Cyan
	MaterialAmbient[13][0] = 0.0f;
	MaterialAmbient[13][1] = 0.1f;
	MaterialAmbient[13][2] = 0.06f;
	MaterialAmbient[13][3] = 1.0f;

	MaterialDiffuse[13][0] = 0.0f;
	MaterialDiffuse[13][1] = 0.50980329f;
	MaterialDiffuse[13][2] = 0.50980329f;
	MaterialDiffuse[13][3] = 1.0f;

	MaterialSpecular[13][0] = 0.50196078f;
	MaterialSpecular[13][1] = 0.50196078f;
	MaterialSpecular[13][2] = 0.50196078f;
	MaterialSpecular[13][3] = 1.0f;

	MaterialShininess[13] = 0.25f * 128.0f;

	//material 3 : Green
	MaterialAmbient[14][0] = 0.0f;
	MaterialAmbient[14][1] = 0.0f;
	MaterialAmbient[14][2] = 0.0f;
	MaterialAmbient[14][3] = 1.0f;

	MaterialDiffuse[14][0] = 0.1f;
	MaterialDiffuse[14][1] = 0.35f;
	MaterialDiffuse[14][2] = 0.1f;
	MaterialDiffuse[14][3] = 1.0f;


	MaterialSpecular[14][0] = 0.45f;
	MaterialSpecular[14][1] = 0.55f;
	MaterialSpecular[14][2] = 0.45f;
	MaterialSpecular[14][3] = 1.0f;

	MaterialShininess[14] = 0.25f * 128.0f;

	//material 4 : red
	MaterialAmbient[15][0] = 0.0f;
	MaterialAmbient[15][1] = 0.0f;
	MaterialAmbient[15][2] = 0.0f;
	MaterialAmbient[15][3] = 1.0f;

	MaterialDiffuse[15][0] = 0.5f;
	MaterialDiffuse[15][1] = 0.0f;
	MaterialDiffuse[15][2] = 0.0f;
	MaterialDiffuse[15][3] = 1.0f;

	MaterialSpecular[15][0] = 0.7f;
	MaterialSpecular[15][1] = 0.6f;
	MaterialSpecular[15][2] = 0.6f;
	MaterialSpecular[15][3] = 1.0f;

	MaterialShininess[15] = 0.25f * 128.0f;

	//material 5 : white
	MaterialAmbient[16][0] = 0.0f;
	MaterialAmbient[16][1] = 0.0f;
	MaterialAmbient[16][2] = 0.0f;
	MaterialAmbient[16][3] = 1.0f;

	MaterialDiffuse[16][0] = 0.55f;
	MaterialDiffuse[16][1] = 0.55f;
	MaterialDiffuse[16][2] = 0.55f;
	MaterialDiffuse[16][3] = 1.0f;

	MaterialSpecular[16][0] = 0.70f;
	MaterialSpecular[16][1] = 0.70f;
	MaterialSpecular[16][2] = 0.70f;
	MaterialSpecular[16][3] = 1.0f;

	MaterialShininess[16] = 0.4f * 128.0f;

	//material 6 : yellow
	MaterialAmbient[17][0] = 0.0f;
	MaterialAmbient[17][1] = 0.0f;
	MaterialAmbient[17][2] = 0.0f;
	MaterialAmbient[17][3] = 1.0f;

	MaterialDiffuse[17][0] = 0.5f;
	MaterialDiffuse[17][1] = 0.5f;
	MaterialDiffuse[17][2] = 0.0f;
	MaterialDiffuse[17][3] = 1.0f;

	MaterialSpecular[17][0] = 0.60f;
	MaterialSpecular[17][1] = 0.60f;
	MaterialSpecular[17][2] = 0.50f;
	MaterialSpecular[17][3] = 1.0f;

	MaterialShininess[17] = 0.35f * 128.0f;


	//---------------COLUMN 4 : RUBBER -----------------------------
	//material 1 : Black
	MaterialAmbient[18][0] = 0.02f;
	MaterialAmbient[18][1] = 0.02f;
	MaterialAmbient[18][2] = 0.02f;
	MaterialAmbient[18][3] = 1.0f;

	MaterialDiffuse[18][0] = 0.01f;
	MaterialDiffuse[18][1] = 0.01f;
	MaterialDiffuse[18][2] = 0.01f;
	MaterialDiffuse[18][3] = 1.0f;


	MaterialSpecular[18][0] = 0.4f;
	MaterialSpecular[18][1] = 0.4f;
	MaterialSpecular[18][2] = 0.4f;
	MaterialSpecular[18][3] = 1.0f;

	MaterialShininess[18] = 0.078125f * 128.0f;

	//material  2 : Cyan
	MaterialAmbient[19][0] = 0.0f;
	MaterialAmbient[19][1] = 0.05f;
	MaterialAmbient[19][2] = 0.05f;
	MaterialAmbient[19][3] = 1.0f;

	MaterialDiffuse[19][0] = 0.4f;
	MaterialDiffuse[19][1] = 0.5f;
	MaterialDiffuse[19][2] = 0.5f;
	MaterialDiffuse[19][3] = 1.0f;

	MaterialSpecular[19][0] = 0.04f;
	MaterialSpecular[19][1] = 0.7f;
	MaterialSpecular[19][2] = 0.7f;
	MaterialSpecular[19][3] = 1.0f;

	MaterialShininess[19] = 0.078125f * 128.0f;

	//material 3 : Green
	MaterialAmbient[20][0] = 0.0f;
	MaterialAmbient[20][1] = 0.05f;
	MaterialAmbient[20][2] = 0.0f;
	MaterialAmbient[20][3] = 1.0f;

	MaterialDiffuse[20][0] = 0.4f;
	MaterialDiffuse[20][1] = 0.5f;
	MaterialDiffuse[20][2] = 0.4f;
	MaterialDiffuse[20][3] = 1.0f;

	MaterialSpecular[20][0] = 0.04f;
	MaterialSpecular[20][1] = 0.7f;
	MaterialSpecular[20][2] = 0.04f;
	MaterialSpecular[20][3] = 1.0f;

	MaterialShininess[20] = 0.078125f * 128.0f;

	//material 4 : red
	MaterialAmbient[21][0] = 0.05f;
	MaterialAmbient[21][1] = 0.0f;
	MaterialAmbient[21][2] = 0.0f;
	MaterialAmbient[21][3] = 1.0f;

	MaterialDiffuse[21][0] = 0.5f;
	MaterialDiffuse[21][1] = 0.4f;
	MaterialDiffuse[21][2] = 0.4f;
	MaterialDiffuse[21][3] = 1.0f;

	MaterialSpecular[21][0] = 0.7f;
	MaterialSpecular[21][1] = 0.04f;
	MaterialSpecular[21][2] = 0.04f;
	MaterialSpecular[21][3] = 1.0f;

	MaterialShininess[21] = 0.078125f * 128.0f;

	//material 5 : white
	MaterialAmbient[22][0] = 0.05f;
	MaterialAmbient[22][1] = 0.05f;
	MaterialAmbient[22][2] = 0.05f;
	MaterialAmbient[22][3] = 1.0f;

	MaterialDiffuse[22][0] = 0.5f;
	MaterialDiffuse[22][1] = 0.5f;
	MaterialDiffuse[22][2] = 0.5f;
	MaterialDiffuse[22][3] = 1.0f;

	MaterialSpecular[22][0] = 0.7f;
	MaterialSpecular[22][1] = 0.7f;
	MaterialSpecular[22][2] = 0.7f;
	MaterialSpecular[22][3] = 1.0f;

	MaterialShininess[22] = 0.078125f * 128.0f;

	//material 6 : yellow
	MaterialAmbient[23][0] = 0.05f;
	MaterialAmbient[23][1] = 0.05f;
	MaterialAmbient[23][2] = 0.0f;
	MaterialAmbient[23][3] = 1.0f;

	MaterialDiffuse[23][0] = 0.5f;
	MaterialDiffuse[23][1] = 0.5f;
	MaterialDiffuse[23][2] = 0.4f;
	MaterialDiffuse[23][3] = 1.0f;

	MaterialSpecular[23][0] = 0.7f;
	MaterialSpecular[23][1] = 0.7f;
	MaterialSpecular[23][2] = 0.04f;
	MaterialSpecular[23][3] = 1.0f;

	MaterialShininess[23] = 0.078125f * 128.0f;

}

