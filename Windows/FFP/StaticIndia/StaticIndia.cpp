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
void uninitialize(void);

void drawD(void);
void drawFirstI(void);
void drawSecondI(void);
void drawN(void);
void drawA(void);

void drawReferenceLine(void);

float saffronR = 1.0f;
float saffronG = 0.6f;
float saffronB = 0.2f;

float whiteR = 1.0f;
float whiteG = 1.0f;
float whiteB = 1.0f;

float greenR = 0.070f;
float greenG = 0.533f;
float greenB = 0.027f;



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

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
			ToggleFullScreen();
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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);
	drawFirstI();
	//drawReferenceLine();
	
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);
	drawN();

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);
	drawD();
	
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);
	drawSecondI();

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);
	drawA();

	SwapBuffers(ghdc);
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

void drawFirstI(void)
{
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	//		   |||
	//		   |||
	//		   |||
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-2.5f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-2.5f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-2.65f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-2.65f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-2.8f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-2.8f, -1.0f, 0.0f);
	glEnd();
}

void drawN(void)
{
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	//
	//				|
	//				|
	//				|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	//
	//			 \	|
	//            \ |
	//             \|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-1.9f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	//
	//		   |
	//		   |
	//		   |
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-2.0f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-2.0f, -1.0f, 0.0f);
	//
	//		   |\
	//		   | \
	//		   |  \
	//
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-2.0f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.1f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	//
	//		   |
	//		   ||
	//		   |||
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-1.92f, 0.7f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.92f, -1.0f, 0.0f);

	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-1.84f, 0.55f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.84f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	//
	//				  |
	//				 ||
	//				|||
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-1.08f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.08f, -0.7f, 0.0f);

	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-1.15f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-1.15f, -0.55f, 0.0f);
	
	glEnd();
}

void drawD(void)
{
	glLineWidth(5.0f);
	
	glBegin(GL_LINES);
	//		_______
	//		|     |
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-0.5f, 1.0f, 0.0f);
	glVertex3f(0.5f, 1.0f, 0.0f);
	//		|
	//		|
	//		|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-0.5f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-0.5f, -1.0f, 0.0f);
	//				|
	//				|
	//				|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(0.5f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(0.5f, -1.0f, 0.0f);
	//      |______|
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-0.5f, -1.0f, 0.0f);
	glVertex3f(0.5f, -1.0f, 0.0f);
	glEnd();


	glBegin(GL_LINES);
	//		_______
	//		|     |
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-0.5f / 1.3f, 1.0f / 1.3f, 0.0f);
	glVertex3f(0.5f / 1.3f, 1.0f / 1.3f, 0.0f);
	//		|
	//		|
	//		|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-0.5f / 1.3f, 1.0f / 1.3f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-0.5f / 1.3f, -1.0f / 1.3f, 0.0f);
	//				|
	//				|
	//				|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(0.5f / 1.3f, 1.0f / 1.3f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(0.5f / 1.3f, -1.0f / 1.3f, 0.0f);
	//      |______|
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-0.5f / 1.3f, -1.0f / 1.3f, 0.0f);
	glVertex3f(0.5f / 1.3f, -1.0f / 1.3f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	//		_______
	//		|     |
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-0.5f/2.0f, 1.0f/2.0f, 0.0f);
	glVertex3f(0.5f/2.0f, 1.0f/2.0f, 0.0f);
	//		|
	//		|
	//		|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(-0.5f/2.0f, 1.0f/2.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-0.5f/2.0f, -1.0f/2.0f, 0.0f);
	//				|
	//				|
	//				|
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(0.5f/2.0f, 1.0f/2.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(0.5f/2.0f, -1.0f/2.0f, 0.0f);
	//      |______|
	glColor3f(greenR, greenG, greenB);
	glVertex3f(-0.5f / 2.0f, -1.0f / 2.0f, 0.0f);
	glVertex3f(0.5f / 2.0f, -1.0f / 2.0f, 0.0f);
	glEnd();

}

void drawSecondI(void)
{
	glLineWidth(5.0f);
	
	glBegin(GL_LINES);
	//		|||
	//		|||
	//		|||
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(1.15f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(1.15f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(1.3f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(1.3f, -1.0f, 0.0f);
	glEnd();

}

void drawA(void)
{
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	//			/\
	//		   /  \
	//		  /    \
	//
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.3f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(1.8f, -1.0f, 0.0f);
	
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.3f, 1.0f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(2.8f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	//			/\
	//		   //\\
	//		  //  \\
	//
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.3f, 0.7f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(1.9f, -1.0f, 0.0f);

	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.3f, 0.7f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(2.7f, -1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	//			/\
	//		   //\\
	//		  ///\\\
	//
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.3f, 0.4f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(2.0f, -1.0f, 0.0f);

	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.3f, 0.4f, 0.0f);
	glColor3f(greenR, greenG, greenB);
	glVertex3f(2.6f, -1.0f, 0.0f);
	glEnd();
	
	// FLAG
	glBegin(GL_LINES);
	glColor3f(saffronR, saffronG, saffronB);
	glVertex3f(2.19f, -0.2f, 0.0f);
	glVertex3f(2.42f, -0.2f, 0.0f);
	
	glColor3f(whiteR, whiteG, whiteB);
	glVertex3f(2.18f, -0.23f, 0.0f);
	glVertex3f(2.42f, -0.23f, 0.0f);

	glColor3f(greenR, greenG, greenB);
	glVertex3f(2.18f, -0.25f, 0.0f);
	glVertex3f(2.43f, -0.25f, 0.0f);
	
	glEnd();
}
