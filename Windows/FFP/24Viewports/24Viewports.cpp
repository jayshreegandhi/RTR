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
bool gbMakeFull = false;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghWnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *gpFile = NULL;
int gWidth;
int gHeight;
int gX = 0;
int gY = 0;

void ToggleFullScreen(void);
int initialize(void);
void resize(int, int);
void display(void);
void uninitialize(void);

void setViewports(int, int, int, int);
int gkeyPressedX = 0;
int gKeyPreddedY = 0;

int size[4];

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

	ShowWindow(hwnd, iCmdShow);
	ToggleFullScreen();
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
		gWidth = LOWORD(lParam);
		gHeight = HIWORD(lParam);

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
		case 'F':
		case 'f':
			if (gbMakeFull == false)
			{
				gbMakeFull = true;
			}
			else
			{
				gbMakeFull = false;
			}
			break;

		case '1':
			gkeyPressedX = 0;
			break;

		case '2':
			gkeyPressedX = 1;
			break;

		case '3':
			gkeyPressedX = 2;
			break;

		case '4':
			gkeyPressedX = 3;
			break;

		case '5':
			gkeyPressedX = 4;
			break;

		case '6':
			gkeyPressedX = 5;
			break;

		case 'Q':
		case 'q':
			gKeyPreddedY = 0;
			break;

		case 'W':
		case 'w':
			gKeyPreddedY = 1;
			break;

		case 'E':
		case 'e':
			gKeyPreddedY = 2;
			break;

		case 'R':
		case 'r':
			gKeyPreddedY = 3;
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

void setViewports(int x, int y, int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(x, y, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	/*glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -4.0f);
*/


	/*for (int i = 0; i < 2; i++)
	{
		size[0][0] = gX;
		size[0][1] = gY;
		size[0][2] = gWidth / 2;
		size[0][3] = gHeight;

		size[1][0] = gWidth / 2;
		size[1][1] = gY;
		size[1][2] = gWidth / 2;
		size[1][3] = gHeight;

		setViewports(size[i][0], size[i][1], size[i][2], size[i][3]);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(0.0f, 1.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(-1.0f, -1.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(1.0f, -1.0f);

		glEnd();
	}*/
	if (gbMakeFull) {
		
		size[0] = size[0] - (gWidth / 400);
		if (0 > size[0]) {
			size[0] = 0;
		}
		size[1] = size[1] - (gHeight / 600);
		if (0 > size[1]) {
			size[1] = 0;
		}

		size[2] = size[2] + (gWidth / 400);
		if (size[2] > gWidth) {
			size[2] = gWidth;
		}
		size[3] = size[3] + (gHeight / 600);
		if (size[3] > gHeight) {
			size[3] = gHeight;
		}
		fprintf(gpFile, "size[23][0] = %d , size[23][1] = %d ,size[23][2] = %d , size[23][2] = %d \n", size[0], size[1], size[2], size[3]);
		setViewports(size[0], size[1], size[2], size[3]);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(0.0f, 0.0f, -4.0f);


		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(0.0f, 1.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(-1.0f, -1.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(1.0f, -1.0f);

		glEnd();

	}
	else {

		int width = gWidth / 6;
		int height = gHeight / 4;

		for (int x = 0; x < 6; x++)
		{
			if (gbMakeFull && 5 == x) {
				continue;

			}
			for (int y = 0; y < 4; y++)
			{
				if (x == gkeyPressedX && y == gKeyPreddedY) {
					size[0] = x * width;
					size[1] = y * height;
					size[2] = width;
					size[3] = height;
				}
				setViewports(x * width, y*height, width, height);

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				glTranslatef(0.0f, 0.0f, -4.0f);


				glBegin(GL_TRIANGLES);
				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex2f(0.0f, 1.0f);

				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex2f(-1.0f, -1.0f);

				glColor3f(0.0f, 0.0f, 1.0f);
				glVertex2f(1.0f, -1.0f);

				glEnd();
			}
		}
	}
	
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
