#include<iostream> 
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

#include<SOIL/SOIL.h>

using namespace std;

bool bFullscreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
int gWindowWidth = 800;
int gWindowHeight = 600;
FILE *gpFile = NULL;

//function prototypes
void CreateWindow(void);
void ToggleFullscreen(void);
void uninitialize(void);

GLXContext gGLXContext;

void initialize(void);
void resize(int,int);
void display(void);
void update(void);

bool bDone = false;

bool loadTexture(GLuint *, const char *);
void DrawPyramid(void);
void DrawCube(void);

GLfloat pyramidSpinningAngle = 0.0f;
GLuint texture_stone;

GLfloat cubeRotationAngle = 0.0f;
GLuint texture_kundali;

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
		GLX_DEPTH_SIZE,24,
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

bool loadTexture(GLuint *texture, const char* path)
{
	bool bResult = false;
	int imageWidth;
	int imageHeight;
	unsigned char* imageData = NULL;

	imageData = SOIL_load_image(path,
		&imageWidth,
		&imageHeight,
		0,
		SOIL_LOAD_RGB);

	if(imageData == NULL)
	{
		bResult = false;
		return(bResult);
	}
	else
	{
		bResult = true;
	}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D,
		3,
		imageWidth,
		imageHeight,
		GL_BGR_EXT,
		GL_UNSIGNED_BYTE,
		imageData);

	SOIL_free_image_data(imageData);

	return(bResult);
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

	

	glShadeModel(GL_SMOOTH);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	loadTexture(&texture_stone,"Stone.bmp");

	loadTexture(&texture_kundali,"Kundali.bmp");

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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
	
	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -5.0f);
	glRotatef(pyramidSpinningAngle, 0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D,texture_stone);
	DrawPyramid();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_2D);
	
	glLoadIdentity();
	glTranslatef(1.5f, 0.0f, -5.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(cubeRotationAngle,1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_kundali);
	DrawCube();
	glDisable(GL_TEXTURE_2D);
	
	glXSwapBuffers(gpDisplay, gWindow);
}

void update(void)
{
	pyramidSpinningAngle = pyramidSpinningAngle + 1.0f;
	if (pyramidSpinningAngle >= 360.0f)
	{
		pyramidSpinningAngle = 0.0f;
	}

	cubeRotationAngle = cubeRotationAngle - 1.0f;
	if (cubeRotationAngle <= -360.0f)
	{
		cubeRotationAngle = 0.0f;
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

	if (texture_stone)
	{
		glDeleteTextures(1,&texture_stone);
	}


	if (texture_kundali)
	{
		glDeleteTextures(1, &texture_kundali);
	}


	if (gpFile)
	{
		fprintf(gpFile, "Log file closed successfully\n");
		fclose(gpFile);
		gpFile = NULL;
	}

}

void DrawPyramid(void)
{
	glBegin(GL_TRIANGLES);

	//front face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//right face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//back face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);


	//left face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd();
}

void DrawCube(void)
{
	glBegin(GL_QUADS);
	//top face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);//rt
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);//lt
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);//lb
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);//rb
	glEnd();

	glBegin(GL_QUADS);
	//bottom face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);//rt
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);//lt
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);//lb
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);//rb
	glEnd();

	glBegin(GL_QUADS);
	//front face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	//back face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	//right face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	//left face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glEnd();
}