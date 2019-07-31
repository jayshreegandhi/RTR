#include<iostream> 
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#define _USE_MATH_DEFINES 1
#include<math.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

using namespace std;

#define PI 3.1415

bool bFullscreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
int gWindowWidth = 800;
int gWindowHeight = 600;
FILE *gpFile = NULL;

//---

GLXContext gGLXContext;
bool bDone = false;

//function prototypes
void CreateWindow(void);
void ToggleFullscreen(void);
void uninitialize(void);

void initialize(void);
void resize(int,int);
void display(void);
void update(void);

void DrawTriangle(void);
void DrawInCircle(void);
void DrawLine(void);

float a, b, c, perimeter, semiPerimeter, radius;
float ax = 0.0f;
float ay = 1.0f;
float az = 0.0f;

float bx = -1.0f;
float by = -1.0f;
float bz = 0.0f;

float cx = 1.0f;
float cy = -1.0f;
float cz = 0.0f;

float ox, oy;
float angle = 0.0f;

float translationLine = 2.0f;

float traslationTriangle = 2.0f;
float rotationTriangle = 0.0f;
	
float translationCircle = 2.0f;
float rotationCircle = 0.0f;


int main(void)
{

	int winWidth = gWindowWidth;
	int winHeight = gWindowHeight;

	gpFile = fopen("Log.txt", "w"); 
	if (!(gpFile))
	{
		printf("Log file can not be created\n");
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file created successfully...\n");
	}

	CreateWindow();

	initialize();

	XEvent event;
	KeySym keysym;

	while(bDone == false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);

			switch(event.type)
			{
				case MapNotify:
					break;

				case KeyPress:
					keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
					switch(keysym)
					{
						case XK_Escape:
							bDone = true;
							break;

						case XK_F:
						case XK_f:
							if(bFullscreen == false)
							{
								ToggleFullscreen();
								bFullscreen = true;
							}
							else
							{
								ToggleFullscreen();
								bFullscreen = false;
							}
							break;

						default:
							break;
					}
					break;

				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1:
							break;

						case 2:
							break;

						case 3:
							break;

						default:
							break;
					}
					break;

				case MotionNotify:
					break;

				case ConfigureNotify:
					winWidth = event.xconfigure.width;
					winHeight = event.xconfigure.height;

					resize(winWidth,winHeight);
					break;

				case Expose:
					break;

				case DestroyNotify:
					break;

				case 33:
					bDone = true;
					break;

				default:
					break;
			}
		}

		update();
		display();
	}

	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	//int defaultDepth;
	int styleMask;

	//though not necessary in our opengl practices , it(static) is done by many SDKs ex: Imagination,Unreal 
	static int frameBufferAttribute[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		//GLX_DEPTH_SIZE,24,
		None
	};

	
	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL)
	{
		fprintf(gpFile, "\nERROR : Unable to open XDisplay.Exitting now..\n");
		printf("\nERROR : Unable to open XDisplay.\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	defaultScreen = XDefaultScreen(gpDisplay);
	//defaultDepth = DefaultDepth(gpDisplay,defaultScreen);
	
	//Bridging API gives you visual in opengl code
	gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttribute);
	
	if(gpXVisualInfo == NULL)
	{
		fprintf(gpFile, "\nERROR : Unable to get XVisualInfo .Exitting now..\n");
		printf("\nERROR : Unable to get XVisualInfo.\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	winAttribs.border_pixel = 0;
	winAttribs.border_pixmap = 0;
	winAttribs.colormap = XCreateColormap(gpDisplay, 
			RootWindow(gpDisplay, gpXVisualInfo->screen), 
			gpXVisualInfo->visual, 
			AllocNone);
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
	winAttribs.background_pixmap = 0;
	winAttribs.event_mask = ExposureMask | 
		VisibilityChangeMask |
		ButtonPressMask | 
		KeyPressMask |
		PointerMotionMask |
		StructureNotifyMask;

	styleMask = CWBorderPixel |
		CWBackPixel |
		CWEventMask |
		CWColormap;

	gWindow = XCreateWindow(gpDisplay,
			RootWindow(gpDisplay, gpXVisualInfo->screen),
			0,
			0,
			gWindowWidth,
			gWindowHeight,
			0,
			gpXVisualInfo->depth,
			InputOutput,
			gpXVisualInfo->visual,
			styleMask,
			&winAttribs);

	if(!gWindow)
	{	
		fprintf(gpFile, "\nERROR : Failed to create main window.Exitting now..\n");
		printf("\nERROR : Failed to create main window.\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay, gWindow, "My First XWindows Window - Jayshree Gandhi");

	Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

	XMapWindow(gpDisplay, gWindow);
}


void ToggleFullscreen(void)
{

	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);

	memset(&xev, 0, sizeof(xev));

	xev.type = ClientMessage;
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = bFullscreen ? 0 : 1;


	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1] = fullscreen;

	XSendEvent(gpDisplay, 
			RootWindow(gpDisplay, gpXVisualInfo->screen),
			False,
			StructureNotifyMask,
			&xev);
}



void initialize(void)
{
	//get the rendering context
	gGLXContext = glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);

	if(gGLXContext == NULL)
	{
		//return(-1);

		fprintf(gpFile, "\nERROR : glXCreateContext() Failed\n");
		printf("\nERROR : glXCreateContext() Failed\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	//usual opengl initialization code:

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	resize(gWindowWidth,gWindowHeight);

	
}

void resize(int width, int height)
{
	//usual resize code
	if(height == 0)
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
	//usual display code - last line only in double buffering
	//but in single buffering, whole code is same

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, translationLine, -4.0f);
	DrawLine();

	glLoadIdentity();
	glTranslatef(traslationTriangle, -traslationTriangle, -4.0f);
	glRotatef(rotationTriangle, 0.0f, 1.0f, 0.0f);
	DrawTriangle();

	glLoadIdentity();
	glTranslatef(-translationCircle, -translationCircle, -4.0f);
	glRotatef(rotationCircle, 0.0f, 1.0f, 0.0f);
	DrawInCircle();
	
	
	glXSwapBuffers(gpDisplay, gWindow);
}

void update(void)
{
	translationLine = translationLine - 0.005f;

	if (translationLine <= 0.0f)
	{
		translationLine = 0.0f;
	}
	

	traslationTriangle = traslationTriangle - 0.005f;
	if (traslationTriangle <= 0.0f)
	{
		traslationTriangle = 0.0f;
	}

	rotationTriangle = rotationTriangle + 1.0f;
	if (rotationTriangle >= 360.0f)
	{
		rotationTriangle = 0.0f;
	}

	translationCircle = translationCircle - 0.005f;
	if (translationCircle <= 0.0f)
	{
		translationCircle = 0.0f;
	}

	rotationCircle = rotationCircle + 1.0f;
	if (rotationCircle >= 360.0f)
	{
		rotationCircle = 0.0f;
	}
}

void uninitialize(void)
{
	GLXContext currentGLXContext = glXGetCurrentContext();
	if(currentGLXContext != NULL && currentGLXContext == gGLXContext)
	{
		glXMakeCurrent(gpDisplay, 0,0);
	}

	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
	}
	
	if(gWindow)
	{
		XDestroyWindow(gpDisplay, gWindow);
		gWindow = 0;
	}

	if(gColormap)
	{
		XFreeColormap(gpDisplay, gColormap);
		gColormap = 0;
	}

	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}

	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
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
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(ax, ay, az);
	glVertex3f(bx, by, bz);
	glVertex3f(cx, cy, cz);
	glEnd();

}
//Drawing the incircle for the traingle :
//				ax,ay,az
//					/\
//				   /  \
//				a /    \ c
//				 /      \
//				/________\
//		bx,by,bz    b     cx,cy,cz

void DrawInCircle(void)
{
	a = sqrtf((float)pow((bx - ax), 2) + (float)pow((by - ay), 2));
	b = sqrtf((float)pow((cx - bx), 2) + (float)pow((cy - by), 2));
	c = sqrtf((float)pow((cx - ax), 2) + (float)pow((cy - ay), 2));

	perimeter = a + b + c;

	semiPerimeter = perimeter / 2.0f;

	ox = (a * cx + b * ax + c * bx) / perimeter;
	oy = (a * cy + b * ay + c * by) / perimeter;

	radius = sqrtf(semiPerimeter*(semiPerimeter - a)*(semiPerimeter - b)*(semiPerimeter - c)) / semiPerimeter;


	glPointSize(2.0f);
	glBegin(GL_POINTS);

	for (GLfloat angle = 0.0f; angle <= 2.0f * PI; angle = angle + 0.00001f)
	{
		glVertex3f(radius*(GLfloat)cos(angle) + ox, radius*(GLfloat)sin(angle) + oy, 0.0f);
	}
	glEnd();

	angle = angle = 0.1f;
}

void DrawLine(void)
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);
	glEnd();
}
