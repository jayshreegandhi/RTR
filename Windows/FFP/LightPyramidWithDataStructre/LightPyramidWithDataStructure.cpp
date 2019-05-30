#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<stdio.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

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

void DrawTriangle(void);

GLfloat triangleSpinningAngle = 0.0f;

bool bLight = false;

struct Light
{
	GLfloat Ambient[4];
	GLfloat Diffuse[4];
	GLfloat Specular[4];
	GLfloat Position[4];
};

Light lights[2];


/*
GLfloat LightAmbientZero[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseZero[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightSpecularZero[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat LightPositionZero[] = { -2.0f,0.0f,0.0f,1.0f };

GLfloat LightAmbientOne[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat LightDiffuseOne[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightSpecularOne[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat LightPositionOne[] = { 2.0f,0.0f,0.0f,1.0f };
*/

GLfloat MaterialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat MaterialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialPosition[] = { 100.0f,100.0f,100.0f,1.0f };
GLfloat MaterialShininess[] = { 50.0f };
//GLfloat MaterialShininess[] = { 128.0f };


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
		resize(LOWORD(lParam), HIWORD(lParam));
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

		case 'L':
		case 'l':
			if (bLight == false)
			{
				bLight = true;
				glEnable(GL_LIGHTING);
			}
			else
			{
				bLight = false;
				glDisable(GL_LIGHTING);
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
		gbFullScreen = true;
	}
}

int initialize(void)
{
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

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


	glShadeModel(GL_SMOOTH);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//red light
	lights[0].Ambient[0] = 0.0f;
	lights[0].Ambient[1] = 0.0f;
	lights[0].Ambient[2] = 0.0f;
	lights[0].Ambient[3] = 1.0f;

	lights[0].Diffuse[0] = 1.0f;
	lights[0].Diffuse[1] = 0.0f;
	lights[0].Diffuse[2] = 0.0f;
	lights[0].Diffuse[3] = 1.0f;

	lights[0].Specular[0] = 1.0f;
	lights[0].Specular[1] = 0.0f;
	lights[0].Specular[2] = 0.0f;
	lights[0].Specular[3] = 1.0f;

	lights[0].Position[0] = -2.0f;
	lights[0].Position[1] = 0.0f;
	lights[0].Position[2] = 0.0f;
	lights[0].Position[3] = 1.0f;

	//blue light
	lights[1].Ambient[0] = 0.0f;
	lights[1].Ambient[1] = 0.0f;
	lights[1].Ambient[2] = 0.0f;
	lights[1].Ambient[3] = 0.0f;

	lights[1].Diffuse[0] = 0.0f;
	lights[1].Diffuse[1] = 0.0f;
	lights[1].Diffuse[2] = 1.0f;
	lights[1].Diffuse[3] = 1.0f;

	lights[1].Specular[0] = 0.0f;
	lights[1].Specular[1] = 0.0f;
	lights[1].Specular[2] = 1.0f;
	lights[1].Specular[3] = 1.0f;

	lights[1].Position[0] = 2.0f;
	lights[1].Position[1] = 0.0f;
	lights[1].Position[2] = 0.0f;
	lights[1].Position[3] = 1.0f;


	glLightfv(GL_LIGHT0, GL_AMBIENT, lights[0].Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lights[0].Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lights[0].Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, lights[0].Position);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, lights[1].Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lights[1].Diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lights[1].Specular);
	glLightfv(GL_LIGHT1, GL_POSITION, lights[1].Position);
	glEnable(GL_LIGHT1);
	

	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
	//glMaterialfv(GL_FRONT, GL_POSITION, MaterialPosition);
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(triangleSpinningAngle, 0.0f, 1.0f, 0.0f);
	DrawTriangle();

	SwapBuffers(ghdc);

}

void update(void)
{
	triangleSpinningAngle = triangleSpinningAngle + 1.0f;

	if (triangleSpinningAngle >= 360.0f)
	{
		triangleSpinningAngle = 0.0f;
	}

}


void uninitialize(void)
{
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

void DrawTriangle(void)
{
	glBegin(GL_TRIANGLES);

	//front face
	glNormal3f(0.0f, 0.447214f, 0.894427f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glNormal3f(0.0f, 0.447214f, 0.894427f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glNormal3f(0.0f, 0.447214f, 0.894427f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//right face
	glNormal3f(0.894427f, 0.447214f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glNormal3f(0.894427f, 0.447214f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glNormal3f(0.894427f, 0.447214f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//back face
	glNormal3f(0.0f, 0.447214f, -0.894427f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glNormal3f(0.0f, 0.447214f, -0.894427f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glNormal3f(0.0f, 0.447214f, -0.894427f);
	glVertex3f(1.0f, -1.0f, -1.0f);


	//left face
	glNormal3f(-0.894427f, 0.447214f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glNormal3f(-0.894427f, 0.447214f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glNormal3f(-0.894427f, 0.447214f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd();
}
