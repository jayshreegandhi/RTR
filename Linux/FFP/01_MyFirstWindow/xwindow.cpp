#include<iostream> 
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

using namespace std;

bool bFullscreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
int gWindowWidth = 800;
int gWindowHeight = 600;

//function prototypes
void CreateWindow(void);
void ToggleFullscreen(void);
void uninitialize(void);

int main(void)
{

	int winWidth = gWindowWidth;
	int winHeight = gWindowHeight;

	CreateWindow();

	XEvent event;
	KeySym keysym;

	while(1)
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
						uninitialize();
						exit(0);
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
				break;

			case Expose:
				break;

			case DestroyNotify:
				break;

			case 33:
				uninitialize();
				exit(0);
				break;

			default:
				break;
		}
	}

	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL)
	{
		printf("\nERROR : Unable to open XDisplay.\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	defaultScreen = XDefaultScreen(gpDisplay);
	defaultDepth = DefaultDepth(gpDisplay,defaultScreen);
	gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));
	if(gpXVisualInfo == NULL)
	{
		printf("\nERROR : Unable to allocate memory for Visual Info.\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo);
	if(gpXVisualInfo == NULL)
	{	
		printf("\nERROR : Unable to get a visual.\nExitting now..\n");
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

void uninitialize(void)
{
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
}


