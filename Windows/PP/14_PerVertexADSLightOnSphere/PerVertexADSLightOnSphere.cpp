#include <windows.h>
#include <GL/glew.h> //Wrangler For PP , add additional headers and lib path
#include <gl/GL.h>
#include <stdio.h>
#include "vmath.h"
#include "Sphere.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Sphere.lib")

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
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
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
GLuint vao_sphere; // vertex array object

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

float LightAmbient[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float LightDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float LightSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float LightPosition[4] = {100.0f, 100.0f, 100.0f, 1.0f};

float MaterialAmbient[4] = {0.0f, 0.0f, 0.0f, 0.0f};
float MaterialDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float MaterialSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float MaterialShininess = 128.0f;

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

	return ((int)msg.wParam);
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
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
		return (0);
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
		}
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghWnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = {sizeof(MONITORINFO)};

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
		return (-1);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return (-2);
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return (-3);
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return (-4);
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
		"#version 450 core"
		"\n"
		"in vec4 vPosition;"
		"in vec3 vNormal;"
		"uniform mat4 u_view_matrix;"
		"uniform mat4 u_model_matrix;"
		"uniform mat4 u_projection_matrix;"
		"uniform int u_lKeyPressed;"
		"uniform vec3 u_la;"
		"uniform vec3 u_ld;"
		"uniform vec3 u_ls;"
		"uniform vec4 u_light_position;"
		"uniform vec3 u_ka;"
		"uniform vec3 u_kd;"
		"uniform vec3 u_ks;"
		"uniform float u_material_shininess;"
		"out vec3 phong_ads_light;"
		"void main(void)"
		"{"
		"if(u_lKeyPressed == 1)"
		"{"
		"vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;"
		"vec3 tNormal = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);"
		"vec3 light_direction = normalize(vec3(u_light_position - eyeCoords));"
		"float tNorm_Dot_LightDirection = max(dot(light_direction, tNormal), 0.0);"
		"vec3 reflection_vector = reflect(-light_direction, tNormal);"
		"vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));"
		"vec3 ambient = u_la * u_ka;"
		"vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;"
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,viewer_vector), 0.0), u_material_shininess);"
		"phong_ads_light = ambient + diffuse + specular;"
		"}"
		"else"
		"{"
		"phong_ads_light = vec3(1.0, 1.0, 1.0);"
		"}"
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;"
		"}";

	//specify above source code to vertex shader object
	glShaderSource(vertexShaderObject,						 //to whom?
				   1,										 //how many strings
				   (const GLchar **)&vertexShaderSourceCode, //address of string
				   NULL);									 // NULL specifes that there is only one string with fixed length

	//Compile the vertex shader
	glCompileShader(vertexShaderObject);

	//Error checking for compilation:
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(vertexShaderObject,	 // whose?
				  GL_COMPILE_STATUS,	  //what to get?
				  &iShaderCompileStatus); //in what?

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

				glGetShaderInfoLog(vertexShaderObject, //whose?
								   iInfoLogLength,	 //length?
								   &written,		   //might have not used all, give that much only which have been used in what?
								   szInfoLog);		   //store in what?

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
		"#version 450 core"
		"\n"
		"uniform int u_lKeyPressed;"
		"in vec3 phong_ads_light;"
		"out vec4 fragColor;"
		"void main(void)"
		"{"
		"fragColor = vec4(phong_ads_light,1.0);"
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
	glGetShaderiv(fragmentShaderObject,   // whose?
				  GL_COMPILE_STATUS,	  //what to get?
				  &iShaderCompileStatus); //in what?

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

				glGetShaderInfoLog(fragmentShaderObject, //whose?
								   iInfoLogLength,		 //length?
								   &written,			 //might have not used all, give that much only which have been used in what?
								   szInfoLog);			 //store in what?

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
	glAttachShader(gShaderProgramObject, //to whom?
				   vertexShaderObject);  //what to attach?

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
	glLinkProgram(gShaderProgramObject); //link to whom?

	//Error checking for linking
	GLint iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetProgramiv(gShaderProgramObject, // whose?
				   GL_LINK_STATUS,		 //what to get?
				   &iProgramLinkStatus); //in what?

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

				glGetProgramInfoLog(gShaderProgramObject, //whose?
									iInfoLogLength,		  //length?
									&written,			  //might have not used all, give that much only which have been used in what?
									szInfoLog);			  //store in what?

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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//make identity
	perspectiveProjectionMatrix = mat4::identity();

	resize(WIN_WIDTH, WIN_HEIGHT);

	return (0);
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

	//Binding Opengl code to shader program object
	glUseProgram(gShaderProgramObject);

	//matrices
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 ProjectionMatrix;
	mat4 translationMatrix;

	//make identity
	viewMatrix = mat4::identity();
	modelMatrix = mat4::identity();
	ProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	//do necessary transformation
	translationMatrix = translate(0.0f, 0.0f, -2.0f);
	//do necessary matrix multiplication
	//this was internally done by gluOrtho() in ffp
	modelMatrix = translationMatrix;
	ProjectionMatrix = perspectiveProjectionMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(viewUniform, //which uniform?
					   1,			//how many matrices
					   GL_FALSE,	//have to transpose?
					   viewMatrix); //actual matrix

	glUniformMatrix4fv(modelUniform, //which uniform?
					   1,			 //how many matrices
					   GL_FALSE,	 //have to transpose?
					   modelMatrix); //actual matrix

	glUniformMatrix4fv(projectionUniform, //which uniform?
					   1,				  //how many matrices
					   GL_FALSE,		  //have to transpose?
					   ProjectionMatrix); //actual matrix

	if (gbLight == true)
	{
		glUniform1i(lKeyPressedUniform, 1);
		glUniform3fv(laUniform, LightAmbient);
		glUniform3fv(ldUniform, LightDiffuse);
		glUniform3fv(lsUniform, LightSpecular);
		glUniform3fv(kaUniform, MaterialAmbient);
		glUniform3fv(kdUniform, MaterialDiffuse);
		glUniform3fv(ksUniform, MaterialSpecular);
		glUniform1f(materialShininessUniform, MaterialShininess);
		glUniform4fv(lightPositionUniform, LightPosition);
	}
	else
	{
		glUniform1i(lKeyPressedUniform, 0);
	}
	//bind with vao
	glBindVertexArray(vao_sphere);

	//similarly bind with textures if any
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);

	//now draw the necessary scene
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	//unbind vao
	glBindVertexArray(0);

	//unbinding program
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
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
