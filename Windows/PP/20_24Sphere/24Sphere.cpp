#include<windows.h>
#include<GL/glew.h> //Wrangler For PP , add additional headers and lib path 
#include<gl/GL.h>
#include<stdio.h>
#include"vmath.h"
#include"Sphere.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"Sphere.lib")

//global namespace

using namespace vmath;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool gbFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghWnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;

void ToggleFullScreen(void);
int initialize(void);
void resize(int, int);
void display(void);
void update(void);
void uninitialize(void);

GLuint gShaderProgramObject;
GLuint vao_sphere;// vertex array object

GLuint vbo_position_sphere; // vertex buffer object
GLuint vbo_normal_sphere;
GLuint vbo_element_sphere;

//uniforms
GLuint viewUniform;
GLuint modelUniform;
GLuint projectionUniform;

GLuint laUniform;
GLuint ldUniform;
GLuint lsUniform;
GLuint lightPositionUniform;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lKeyPressedUniform;

mat4 perspectiveProjectionMatrix;

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

GLfloat rotationAngleX = 0.0f;
GLfloat rotationAngleY = 0.0f;
GLfloat rotationAngleZ = 0.0f;
GLint keyPress = 0;

void setMaterialSphere(void);

GLint gWidth;
GLint gHeight;
void setViewports(int, int, int, int);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MY OGL WINDOW");
	bool bDone = false;
	int iRet = 0;

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file can not be created"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file created successfully...\n");
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
		TEXT("My Double buffer Window - Jayshree"),
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

	iRet = initialize();
	if (iRet == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat() Failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2)
	{
		fprintf(gpFile, "SetPixelFormat() Failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext() Failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4)
	{
		fprintf(gpFile, "wglMakeCurrent() Failed\n");
		DestroyWindow(hwnd);
	}
	else
	{
		fprintf(gpFile, "Initialization succeded\n");
	}

	ToggleFullScreen();
	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

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

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

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

		resize(gWidth, gHeight);
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
		gbFullScreen = true;
	}
}

int initialize(void)
{

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;
	GLenum result;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghWnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return(-1);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return(-2);
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return(-3);
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return(-4);
	}

	//On the extensions requred for PP
	result = glewInit();
	if (result != GLEW_OK)
	{
		fprintf(gpFile, "\nglewInit() failed...\n");
		uninitialize();
		DestroyWindow(ghWnd);
	}

	GLuint vertexShaderObject;
	GLuint fragmentShaderObject;

	//***************** 1. VERTEX SHADER ************************************ 
	//define vertex shader object
	//create vertex shader object
	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write vertex shader code
	const GLchar *vertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform int u_lKeyPressed;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform vec4 u_light_position;" \
		"out vec3 tNormal;" \
		"out vec3 light_direction;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		"vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" \
		"tNormal = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction = vec3(u_light_position - eyeCoords);" \
		"viewer_vector = vec3(-eyeCoords.xyz);" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(vertexShaderObject,//to whom?
		1,//how many strings
		(const GLchar **)&vertexShaderSourceCode,//address of string
		NULL);// NULL specifes that there is only one string with fixed length

	//Compile the vertex shader
	glCompileShader(vertexShaderObject);


	//Error checking for compilation:
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(vertexShaderObject, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(vertexShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength > 0)
		{
			//allocate memory to pointer
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(vertexShaderObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nVertex Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

				uninitialize();
				DestroyWindow(ghWnd);
				exit(0);
			}
		}
	}


	//************************** 2. FRAGMENT SHADER ********************************
	//define fragment shader object
	//create fragment shader object
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec3 tNormal;" \
		"in vec3 light_direction;" \
		"in vec3 viewer_vector;" \
		"uniform int u_lKeyPressed;" \
		"uniform vec3 u_la;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_ls;" \
		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \
		"uniform float u_material_shininess;" \
		"out vec3 phong_ads_light;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{" \
		"if(u_lKeyPressed == 1)" \
		"{" \
		"vec3 normalized_tNormal = normalize(tNormal);" \
		"vec3 normalized_light_direction = normalize(light_direction);" \
		"float tNorm_Dot_LightDirection = max(dot(normalized_light_direction, normalized_tNormal), 0.0);" \
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_tNormal);" \
		"vec3 normalized_viewer_vector = normalize(viewer_vector);" \
		"vec3 ambient = u_la * u_ka;" \
		"vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;" \
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,normalized_viewer_vector), 0.0), u_material_shininess);" \
		"phong_ads_light = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"fragColor = vec4(phong_ads_light,1.0);" \
		"}";

	//specify the above source code to fragment shader object
	glShaderSource(fragmentShaderObject,
		1,
		(const GLchar **)&fragmentShaderSourceCode,
		NULL);

	//compile the fragment shader
	glCompileShader(fragmentShaderObject);


	//Error checking for compilation
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(fragmentShaderObject, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(fragmentShaderObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength > 0)
		{
			//allocate memory to pointer
			szInfoLog = (GLchar *)malloc(iInfoLogLength);

			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(fragmentShaderObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nFragment Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

				uninitialize();
				DestroyWindow(ghWnd);
				exit(0);
			}
		}
	}


	//create shader program object
	gShaderProgramObject = glCreateProgram();

	//Attach vertex shader to shader program
	glAttachShader(gShaderProgramObject,//to whom?
		vertexShaderObject);//what to attach?

	//Attach fragment shader to shader program
	glAttachShader(gShaderProgramObject,
		fragmentShaderObject);

	//Pre-Linking binding to vertex attribute
	glBindAttribLocation(gShaderProgramObject,
		AMC_ATTRIBUTE_POSITION,
		"vPosition");

	glBindAttribLocation(gShaderProgramObject,
		AMC_ATTRIBUTE_NORMAL,
		"vNormal");

	//Link the shader program
	glLinkProgram(gShaderProgramObject);//link to whom?

	//Error checking for linking
	GLint iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetProgramiv(gShaderProgramObject, // whose?
		GL_LINK_STATUS,//what to get?
		&iProgramLinkStatus);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iProgramLinkStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetProgramiv(gShaderProgramObject,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength > 0)
		{
			//allocate memory to pointer
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;

				glGetProgramInfoLog(gShaderProgramObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nShader Program Linking Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

				uninitialize();
				DestroyWindow(ghWnd);
				exit(0);
			}
		}
	}


	//Post-Linking reteriving uniform location
	viewUniform = glGetUniformLocation(gShaderProgramObject,
		"u_view_matrix");

	modelUniform = glGetUniformLocation(gShaderProgramObject,
		"u_model_matrix");

	projectionUniform = glGetUniformLocation(gShaderProgramObject,
		"u_projection_matrix");

	lKeyPressedUniform = glGetUniformLocation(gShaderProgramObject,
		"u_lKeyPressed");

	laUniform = glGetUniformLocation(gShaderProgramObject,
		"u_la");

	ldUniform = glGetUniformLocation(gShaderProgramObject,
		"u_ld");

	lsUniform = glGetUniformLocation(gShaderProgramObject,
		"u_ls");

	lightPositionUniform = glGetUniformLocation(gShaderProgramObject,
		"u_light_position");

	kaUniform = glGetUniformLocation(gShaderProgramObject,
		"u_ka");

	kdUniform = glGetUniformLocation(gShaderProgramObject,
		"u_kd");

	ksUniform = glGetUniformLocation(gShaderProgramObject,
		"u_ks");

	materialShininessUniform = glGetUniformLocation(gShaderProgramObject,
		"u_material_shininess");

	//above is the preparation of data transfer from CPU to GPU 
	//i.e glBindAttribLocation() & glGetUniformLocation()

	//array initialization (glBegin() and glEnd())
	getSphereVertexData(sphere_vertices,
		sphere_normals,
		sphere_texture,
		sphere_elements);

	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	//------------------------------sphere-------------------------------
	//create vao rectangle(vertex array object)
	glGenVertexArrays(1, &vao_sphere);

	//Bind vao
	glBindVertexArray(vao_sphere);

	//generate vertex buffers
	glGenBuffers(1, &vbo_position_sphere);

	//bind buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);

	//transfer vertex data(CPU) to GPU buffer
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_vertices),
		sphere_vertices,
		GL_STATIC_DRAW);

	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		NULL);

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	//unbind vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//--------------------normal------------------------
	//generate vertex buffers
	glGenBuffers(1, &vbo_normal_sphere);

	//bind buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);

	//transfer vertex data(CPU) to GPU buffer
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(sphere_normals),
		sphere_normals,
		GL_STATIC_DRAW);

	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		NULL);

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	//unbind vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//--------------------element------------------------
	//generate vertex buffers
	glGenBuffers(1, &vbo_element_sphere);

	//bind buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);

	//transfer vertex data(CPU) to GPU buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(sphere_elements),
		sphere_elements,
		GL_STATIC_DRAW);

	//unbind vbo
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//unbind vao
	glBindVertexArray(0);
	

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	//make identity
	perspectiveProjectionMatrix = mat4::identity();

	setMaterialSphere();

	resize(WIN_WIDTH, WIN_HEIGHT);

	return(0);
}


void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int wid = gWidth / 4;
	int ht = gHeight / 6;

	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 6; y++)
		{
			setViewports(x * wid, y*ht, wid, ht);

			glUseProgram(gShaderProgramObject);

			mat4 viewMatrix;
			mat4 modelMatrix;
			mat4 ProjectionMatrix;
			mat4 translationMatrix;

			viewMatrix = mat4::identity();;
			modelMatrix = mat4::identity();;
			ProjectionMatrix = mat4::identity();
			translationMatrix = mat4::identity();

			translationMatrix = translate(0.0f, 0.0f, -2.0f);

			modelMatrix = translationMatrix;
			ProjectionMatrix = perspectiveProjectionMatrix;

			glUniformMatrix4fv(viewUniform,//which uniform?
				1,//how many matrices
				GL_FALSE,//have to transpose?
				viewMatrix);//actual matrix

			glUniformMatrix4fv(modelUniform,//which uniform?
				1,//how many matrices
				GL_FALSE,//have to transpose?
				modelMatrix);//actual matrix

			glUniformMatrix4fv(projectionUniform,//which uniform?
				1,//how many matrices
				GL_FALSE,//have to transpose?
				ProjectionMatrix);//actual matrix

			if (gbLight == true)
			{
				glUniform1i(lKeyPressedUniform, 1);
				glUniform3fv(laUniform, 1, LightAmbient);
				glUniform3fv(ldUniform, 1, LightDiffuse);
				glUniform3fv(lsUniform, 1, LightSpecular);

				if (keyPress == 1)
				{
					LightPosition[0] = 0.0f;
					LightPosition[1] = cosf(rotationAngleX);
					LightPosition[2] = sinf(rotationAngleX);
					glUniform4fv(lightPositionUniform, 1, LightPosition);
				}
				else if (keyPress == 2)
				{
					LightPosition[0] = cosf(rotationAngleY);
					LightPosition[1] = 0.0f;
					LightPosition[2] = sinf(rotationAngleY);
					glUniform4fv(lightPositionUniform, 1, LightPosition);
				}
				else if (keyPress == 3)
				{
					LightPosition[0] = cosf(rotationAngleZ);
					LightPosition[1] = sinf(rotationAngleZ);
					LightPosition[2] = 0.0f;
					glUniform4fv(lightPositionUniform, 1, LightPosition);
				}

				if (x == 0)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[0]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[0]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[0]);
						glUniform1f(materialShininessUniform, MaterialShininess[0]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[1]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[1]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[1]);
						glUniform1f(materialShininessUniform, MaterialShininess[1]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[2]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[2]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[2]);
						glUniform1f(materialShininessUniform, MaterialShininess[2]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[3]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[3]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[3]);
						glUniform1f(materialShininessUniform, MaterialShininess[3]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[4]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[4]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[4]);
						glUniform1f(materialShininessUniform, MaterialShininess[4]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[5]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[5]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[5]);
						glUniform1f(materialShininessUniform, MaterialShininess[5]);
					}
				}
				else if (x == 1)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[6]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[6]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[6]);
						glUniform1f(materialShininessUniform, MaterialShininess[6]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[7]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[7]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[7]);
						glUniform1f(materialShininessUniform, MaterialShininess[7]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[8]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[8]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[8]);
						glUniform1f(materialShininessUniform, MaterialShininess[8]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[9]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[9]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[9]);
						glUniform1f(materialShininessUniform, MaterialShininess[9]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[10]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[10]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[10]);
						glUniform1f(materialShininessUniform, MaterialShininess[10]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[11]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[11]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[11]);
						glUniform1f(materialShininessUniform, MaterialShininess[11]);
					}
				}
				else if (x == 2)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[12]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[12]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[12]);
						glUniform1f(materialShininessUniform, MaterialShininess[12]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[13]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[13]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[13]);
						glUniform1f(materialShininessUniform, MaterialShininess[13]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[14]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[14]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[14]);
						glUniform1f(materialShininessUniform, MaterialShininess[14]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[15]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[15]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[15]);
						glUniform1f(materialShininessUniform, MaterialShininess[15]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[16]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[16]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[16]);
						glUniform1f(materialShininessUniform, MaterialShininess[16]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[17]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[17]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[17]);
						glUniform1f(materialShininessUniform, MaterialShininess[17]);
					}
				}
				else if (x == 3)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[18]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[18]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[18]);
						glUniform1f(materialShininessUniform, MaterialShininess[18]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[19]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[19]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[19]);
						glUniform1f(materialShininessUniform, MaterialShininess[19]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[20]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[20]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[20]);
						glUniform1f(materialShininessUniform, MaterialShininess[20]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[21]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[21]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[21]);
						glUniform1f(materialShininessUniform, MaterialShininess[21]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[22]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[22]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[22]);
						glUniform1f(materialShininessUniform, MaterialShininess[22]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[23]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[23]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[23]);
						glUniform1f(materialShininessUniform, MaterialShininess[23]);
					}
				}
			}
			else
			{
				glUniform1i(lKeyPressedUniform, 0);
			}

			glBindVertexArray(vao_sphere);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);

			glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

			glBindVertexArray(0);

			glUseProgram(0);
		}
	}

	SwapBuffers(ghdc);
}

void update(void)
{
	rotationAngleX = rotationAngleX + 0.05f;
	if (rotationAngleX >= 360.0f)
	{
		rotationAngleX = 0.0f;
	}

	rotationAngleY = rotationAngleY + 0.05f;
	if (rotationAngleY >= 360.0f)
	{
		rotationAngleY = 0.0f;
	}

	rotationAngleZ = rotationAngleZ + 0.05f;
	if (rotationAngleZ >= 360.0f)
	{
		rotationAngleZ = 0.0f;
	}
}

void uninitialize(void)
{
	
	if (vbo_element_sphere)
	{
		glDeleteBuffers(1, &vbo_element_sphere);
		vbo_element_sphere = 0;
	}

	if (vbo_normal_sphere)
	{
		glDeleteBuffers(1, &vbo_normal_sphere);
		vbo_normal_sphere = 0;
	}

	if (vbo_position_sphere)
	{
		glDeleteBuffers(1, &vbo_position_sphere);
		vbo_position_sphere = 0;
	}

	if (vao_sphere)
	{
		glDeleteVertexArrays(1, &vao_sphere);
		vao_sphere = 0;
	}
	

	if (gShaderProgramObject)
	{
		GLsizei shaderCount;
		GLsizei shaderNumber;

		glUseProgram(gShaderProgramObject);

		//ask the program how many shaders are attached to you?
		glGetProgramiv(gShaderProgramObject,
			GL_ATTACHED_SHADERS,
			&shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			//fprintf(gpFile, "\npshaders sucessful\n");

			//get shaders
			glGetAttachedShaders(gShaderProgramObject,
				shaderCount,
				&shaderCount,
				pShaders);

			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				//detach
				glDetachShader(gShaderProgramObject,
					pShaders[shaderNumber]);

				//delete
				glDeleteShader(pShaders[shaderNumber]);

				//explicit 0
				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(gShaderProgramObject);
		gShaderProgramObject = 0;

		glUseProgram(0);
	}

	if (gbFullScreen == true)
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghWnd, &wpPrev);

		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghWnd, ghdc);
		ghdc = NULL;
	}


	if (gpFile)
	{
		fprintf(gpFile, "Log file closed successfully\n");
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

void setViewports(int x, int y, int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(x, y, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}
