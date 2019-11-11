#include<iostream> 
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glx.h>

#include"vmath.h"
#include"Sphere.h"

using namespace std;
using namespace vmath;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};


Window gWindow;
Colormap gColormap;
XVisualInfo gVisualInfo;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;

bool bFullscreen = false;
int gWindowWidth = 800;
int gWindowHeight = 600;
FILE *gpFile = NULL;

typedef GLXContext (*glXCreateContextAttribsARBProc) (Display *, GLXFBConfig,
GLXContext, Bool, const int *);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;

GLXFBConfig gGLXFBConfig;
static GLXContext gGLXContext;
bool bDone = false;

GLuint gShaderProgramObject_perFrag;
GLuint gShaderProgramObject_perVert;

GLuint vao_sphere;

GLuint vbo_position_sphere; // vertex buffer object
GLuint vbo_normal_sphere;
GLuint vbo_element_sphere;

//uniforms
GLuint viewUniform_pf;
GLuint modelUniform_pf;
GLuint projectionUniform_pf;

GLuint laUniform_pf;
GLuint ldUniform_pf;
GLuint lsUniform_pf;
GLuint lightPositionUniform_pf;

GLuint kaUniform_pf;
GLuint kdUniform_pf;
GLuint ksUniform_pf;
GLuint materialShininessUniform_pf;

GLuint lKeyPressedUniform_pf;

//uniforms
GLuint viewUniform_pv;
GLuint modelUniform_pv;
GLuint projectionUniform_pv;

GLuint laUniform_pv;
GLuint ldUniform_pv;
GLuint lsUniform_pv;
GLuint lightPositionUniform_pv;

GLuint kaUniform_pv;
GLuint kdUniform_pv;
GLuint ksUniform_pv;
GLuint materialShininessUniform_pv;

GLuint lKeyPressedUniform_pv;

mat4 perspectiveProjectionMatrix;

bool gbLight = false;
bool isFKeyPressed = false;
bool isVKeyPressed = false;

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

//function prototypes
void CreateWindow(void);
void ToggleFullscreen(void);
void uninitialize(void);

void initialize(void);
void resize(int,int);
void display(void);
void update(void);

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

						case XK_S:
						case XK_s:
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
                        
                        case XK_L:
                        case XK_l:
                            if (gbLight == false)
                            {
                                gbLight = true;
                            }
                            else
                            {
                                gbLight = false;
                            }
                            break;

                        case XK_f:
                        case XK_F:
                            if (isFKeyPressed == false)
                            {
                                isFKeyPressed = true;
                                isVKeyPressed = false;
                            }
                            else
                            {
                                isFKeyPressed = false;
                                //isVKeyPressed = true;
                            }
                            break;

                        case XK_v:
                        case XK_V:
                            if (isVKeyPressed == false)
                            {
                                isVKeyPressed = true;
                                isFKeyPressed = false;
                            }
                            else
                            {
                                isVKeyPressed = false;
                                //isFKeyPressed = true;
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
	int styleMask;

	GLXFBConfig *pGLXFBConfig = NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo = NULL;
	int iNumberOfFBConfigs = 0;

	//though not necessary in our opengl practices , it(static) is done by many SDKs ex: Imagination,Unreal 
	static int frameBufferAttribute[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE,24,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER,True,
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

	//reterive all FBConfigs driver has
	pGLXFBConfig = glXChooseFBConfig(gpDisplay, defaultScreen, frameBufferAttribute, &iNumberOfFBConfigs);

	printf("\nThere are %d matching FBConfigs\n", iNumberOfFBConfigs);

	int bestFrameBufferConfig = -1;
	int bestNumberOfSamples = -1;
	int worstFrameBufferConfig = -1;
	int worstNumberOfSamples = 999;

	int sampleBuffers = 0;
	int samples = 0;

	for(int i = 0; i < iNumberOfFBConfigs; i++)
	{
		
		//for each obtained FBCongig, get temp visual info
		pTempXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfig[i]);
		if(pTempXVisualInfo)
		{
		
			//get numbers of sample buffers from respective FBConfig
			glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
			
			//get number of samples from respective FBConfig
			glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLES, &samples);
			
			//More the number of samples and sample buffers, more the eligible FBConfig is.So do the comparision
			//get best from all
			if(bestFrameBufferConfig < 0 || (sampleBuffers && (samples > bestNumberOfSamples)))
			{
			
				bestFrameBufferConfig = i;
				bestNumberOfSamples = samples;
			} 

			//get worst from all
			if(worstFrameBufferConfig < 0 || (!sampleBuffers) || (samples < worstNumberOfSamples))
			{
				worstFrameBufferConfig = i;
				worstNumberOfSamples = samples;
			}
		}
		
		//free pTempXVisualInfo
		XFree(pTempXVisualInfo);
	}
	
	//Assign the best found
	bestGLXFBConfig = pGLXFBConfig[bestFrameBufferConfig];
	
	//assign the same to global one
	gGLXFBConfig = bestGLXFBConfig;
	
	//Free the obtained glXFBConfig;
	XFree(pGLXFBConfig);
	
	//accordingly get the best visual
	gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, bestGLXFBConfig);
	
	/*
	//Bridging API gives you visual in opengl code
	gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttribute);
	

	if(gpXVisualInfo == NULL)
	{
		printf("\n18");
		fprintf(gpFile, "\nERROR : Unable to get XVisualInfo .Exitting now..\n");
		printf("\nERROR : Unable to get XVisualInfo.\nExitting now..\n");
		uninitialize();
		exit(1);
	}
	*/

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

	XStoreName(gpDisplay, gWindow, "My First XWindows PP Window - Jayshree Gandhi");

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
	GLenum result;

	//get the rendering context
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");
	
	if(!glXCreateContextAttribsARB)
	{
		//return(-1);
		fprintf(gpFile, "\nERROR : glXGetProcAddressARB() Failed\n");
		printf("\nRashtrageet!");
		printf("\nERROR : glXGetProcAddressARB() Failed\nExitting now..\n");
		uninitialize();
		exit(0);
	}
	
	//Context attribute array
	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	//now get the context
	gGLXContext = glXCreateContextAttribsARB(gpDisplay,
		gGLXFBConfig,
		0,
		True,
		attribs);

	if(!gGLXContext)
	{
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			None
		};
		printf("\ni8");
		gGLXContext = glXCreateContextAttribsARB(gpDisplay,
			gGLXFBConfig,
			0,
			True,
			attribs);
	}
	
	if(!glXIsDirect(gpDisplay, gGLXContext))
	{
		fprintf(gpFile, "\nThe obtained context is not hardware context\n");
		printf("\nThe obtained context is not hardware context\n");
		
	}
	else
	{	fprintf(gpFile, "\nThe obtained context is a hardware context\n");
		printf("\nThe obtained context is a hardware context\n");
		
	}
	
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	//On the extentions 
	result = glewInit();
	if(result != GLEW_OK)
	{
		fprintf(gpFile, "\nERROR : glewInit() Failed\n");
		printf("\nERROR : glewInit() Failed\nExitting now..\n");
		uninitialize();
		exit(1);
	}

	GLuint vertexShaderObject_perFrag;
	GLuint vertexShaderObject_perVert;

	GLuint fragmentShaderObject_perFrag;	
	GLuint fragmentShaderObject_perVert;

	//***************** 1. VERTEX SHADER ************************************ 
	//define vertex shader object
	//create vertex shader object
	vertexShaderObject_perFrag = glCreateShader(GL_VERTEX_SHADER);
	vertexShaderObject_perVert = glCreateShader(GL_VERTEX_SHADER);

	//Write vertex shader code
	const GLchar *vertexShaderSourceCode_perFrag =
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


	//Write vertex shader code
	const GLchar *vertexShaderSourceCode_perVert =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_lKeyPressed;" \
		"uniform vec3 u_la;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_ls;" \
		"uniform vec4 u_light_position;" \
		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \
		"uniform float u_material_shininess;" \
		"out vec3 phong_ads_light;" \
		"void main(void)" \
		"{" \
		"if(u_lKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" \
		"vec3 tNormal = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \
		"vec3 light_direction = normalize(vec3(u_light_position - eyeCoords));" \
		"float tNorm_Dot_LightDirection = max(dot(light_direction, tNormal), 0.0);" \
		"vec3 reflection_vector = reflect(-light_direction, tNormal);" \
		"vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));" \
		"vec3 ambient = u_la * u_ka;" \
		"vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;" \
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,viewer_vector), 0.0), u_material_shininess);" \
		"phong_ads_light = ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(vertexShaderObject_perFrag,//to whom?
		1,//how many strings
		(const GLchar **)&vertexShaderSourceCode_perFrag,//address of string
		NULL);// NULL specifes that there is only one string with fixed length

	glShaderSource(vertexShaderObject_perVert,//to whom?
		1,//how many strings
		(const GLchar **)&vertexShaderSourceCode_perVert,//address of string
		NULL);// NULL specifes that there is only one string with fixed length

	//Compile the vertex shader
	glCompileShader(vertexShaderObject_perFrag);

	glCompileShader(vertexShaderObject_perVert);

	//Error checking for compilation:
	GLint iShaderCompileStatus_perFrag = 0;
	GLint iInfoLogLength_perFrag = 0;
	GLchar *szInfoLog_perFrag = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(vertexShaderObject_perFrag, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus_perFrag);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus_perFrag == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(vertexShaderObject_perFrag,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength_perFrag);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength_perFrag > 0)
		{
			//allocate memory to pointer
			szInfoLog_perFrag = (GLchar *)malloc(iInfoLogLength_perFrag);
			if (szInfoLog_perFrag != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(vertexShaderObject_perFrag,//whose?
					iInfoLogLength_perFrag,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog_perFrag);//store in what?

				fprintf(gpFile, "\nVertex Shader Compilation Log (Per Frag): %s\n", szInfoLog_perFrag);

				//free the memory
				free(szInfoLog_perFrag);

				uninitialize();
				exit(0);
			}
		}
	}

	//Error checking for compilation:
	GLint iShaderCompileStatus_perVert = 0;
	GLint iInfoLogLength_perVert = 0;
	GLchar *szInfoLog_perVert = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(vertexShaderObject_perVert, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus_perVert);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus_perVert == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(vertexShaderObject_perVert,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength_perVert);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength_perVert > 0)
		{
			//allocate memory to pointer
			szInfoLog_perVert = (GLchar *)malloc(iInfoLogLength_perVert);
			if (szInfoLog_perVert != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(vertexShaderObject_perVert,//whose?
					iInfoLogLength_perVert,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog_perVert);//store in what?

				fprintf(gpFile, "\nVertex Shader Compilation Log (Per Vertex): %s\n", szInfoLog_perVert);

				//free the memory
				free(szInfoLog_perVert);

				uninitialize();
				exit(0);
			}
		}
	}


	//************************** 2. FRAGMENT SHADER ********************************
	//define fragment shader object
	//create fragment shader object
	fragmentShaderObject_perFrag = glCreateShader(GL_FRAGMENT_SHADER);

	fragmentShaderObject_perVert = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode_perFrag =
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

	const GLchar *fragmentShaderSourceCode_perVert =
		"#version 450 core" \
		"\n" \
		"uniform int u_lKeyPressed;" \
		"in vec3 phong_ads_light;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{" \
		"fragColor = vec4(phong_ads_light,1.0);" \
		"}";

	//specify the above source code to fragment shader object
	glShaderSource(fragmentShaderObject_perFrag,
		1,
		(const GLchar **)&fragmentShaderSourceCode_perFrag,
		NULL);

	glShaderSource(fragmentShaderObject_perVert,
		1,
		(const GLchar **)&fragmentShaderSourceCode_perVert,
		NULL);

	//compile the fragment shader
	glCompileShader(fragmentShaderObject_perFrag);

	glCompileShader(fragmentShaderObject_perVert);

	//Error checking for compilation
	iShaderCompileStatus_perFrag = 0;
	iInfoLogLength_perFrag = 0;
	szInfoLog_perFrag = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(fragmentShaderObject_perFrag, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus_perFrag);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus_perFrag == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(fragmentShaderObject_perFrag,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength_perFrag);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength_perFrag > 0)
		{
			//allocate memory to pointer
			szInfoLog_perFrag = (GLchar *)malloc(iInfoLogLength_perFrag);

			if (szInfoLog_perFrag != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(fragmentShaderObject_perFrag,//whose?
					iInfoLogLength_perFrag,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog_perFrag);//store in what?

				fprintf(gpFile, "\nFragment Shader Compilation Log (Per Frag): %s\n", szInfoLog_perFrag);

				//free the memory
				free(szInfoLog_perFrag);

				uninitialize();
				exit(0);
			}
		}
	}

	//Error checking for compilation
	iShaderCompileStatus_perVert = 0;
	iInfoLogLength_perVert = 0;
	szInfoLog_perVert = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(fragmentShaderObject_perVert, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus_perVert);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus_perVert == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(fragmentShaderObject_perVert,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength_perVert);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength_perVert > 0)
		{
			//allocate memory to pointer
			szInfoLog_perVert = (GLchar *)malloc(iInfoLogLength_perVert);

			if (szInfoLog_perVert != NULL)
			{
				GLsizei written;

				glGetShaderInfoLog(fragmentShaderObject_perVert,//whose?
					iInfoLogLength_perVert,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog_perVert);//store in what?

				fprintf(gpFile, "\nFragment Shader Compilation Log (Per Vert): %s\n", szInfoLog_perFrag);

				//free the memory
				free(szInfoLog_perVert);

				uninitialize();
				exit(0);
			}
		}
	}

	//create shader program object
	gShaderProgramObject_perFrag = glCreateProgram();

	gShaderProgramObject_perVert = glCreateProgram();

	//Attach vertex shader to shader program
	glAttachShader(gShaderProgramObject_perFrag,//to whom?
		vertexShaderObject_perFrag);//what to attach?

	//Attach fragment shader to shader program
	glAttachShader(gShaderProgramObject_perFrag,
		fragmentShaderObject_perFrag);

	glAttachShader(gShaderProgramObject_perVert,//to whom?
		vertexShaderObject_perVert);//what to attach?

	//Attach fragment shader to shader program
	glAttachShader(gShaderProgramObject_perVert,
		fragmentShaderObject_perVert);

	//Pre-Linking binding to vertex attribute
	glBindAttribLocation(gShaderProgramObject_perFrag,
		AMC_ATTRIBUTE_POSITION,
		"vPosition");

	glBindAttribLocation(gShaderProgramObject_perFrag,
		AMC_ATTRIBUTE_NORMAL,
		"vNormal");

	//Pre-Linking binding to vertex attribute
	glBindAttribLocation(gShaderProgramObject_perVert,
		AMC_ATTRIBUTE_POSITION,
		"vPosition");

	glBindAttribLocation(gShaderProgramObject_perVert,
		AMC_ATTRIBUTE_NORMAL,
		"vNormal");

	//Link the shader program
	glLinkProgram(gShaderProgramObject_perFrag);//link to whom?

	glLinkProgram(gShaderProgramObject_perVert);//link to whom?

	//Error checking for linking
	GLint iProgramLinkStatus_perFrag = 0;
	iInfoLogLength_perFrag = 0;
	szInfoLog_perFrag = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetProgramiv(gShaderProgramObject_perFrag, // whose?
		GL_LINK_STATUS,//what to get?
		&iProgramLinkStatus_perFrag);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iProgramLinkStatus_perFrag == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetProgramiv(gShaderProgramObject_perFrag,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength_perFrag);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength_perFrag > 0)
		{
			//allocate memory to pointer
			szInfoLog_perFrag = (GLchar *)malloc(iInfoLogLength_perFrag);
			if (szInfoLog_perFrag != NULL)
			{
				GLsizei written;

				glGetProgramInfoLog(gShaderProgramObject_perFrag,//whose?
					iInfoLogLength_perFrag,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog_perFrag);//store in what?

				fprintf(gpFile, "\nShader Program Linking Log (Per Frag): %s\n", szInfoLog_perFrag);

				//free the memory
				free(szInfoLog_perFrag);

				uninitialize();
				exit(0);
			}
		}
	}

	//Error checking for linking
	GLint iProgramLinkStatus_perVert = 0;
	iInfoLogLength_perVert = 0;
	szInfoLog_perVert = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetProgramiv(gShaderProgramObject_perVert, // whose?
		GL_LINK_STATUS,//what to get?
		&iProgramLinkStatus_perVert);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iProgramLinkStatus_perVert == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetProgramiv(gShaderProgramObject_perVert,
			GL_INFO_LOG_LENGTH,
			&iInfoLogLength_perVert);

		//Step 4 : if info log length > 0 , call glGetShaderInfoLog()
		if (iInfoLogLength_perVert > 0)
		{
			//allocate memory to pointer
			szInfoLog_perVert = (GLchar *)malloc(iInfoLogLength_perVert);
			if (szInfoLog_perVert != NULL)
			{
				GLsizei written;

				glGetProgramInfoLog(gShaderProgramObject_perVert,//whose?
					iInfoLogLength_perVert,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog_perVert);//store in what?

				fprintf(gpFile, "\nShader Program Linking Log (Per Vert) : %s\n", szInfoLog_perFrag);

				//free the memory
				free(szInfoLog_perVert);

				uninitialize();
				exit(0);
			}
		}
	}


	//Post-Linking reteriving uniform location
	viewUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_view_matrix");

	modelUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_model_matrix");

	projectionUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_projection_matrix");

	lKeyPressedUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_lKeyPressed");

	laUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_la");

	ldUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_ld");

	lsUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_ls");

	lightPositionUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_light_position");

	kaUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_ka");

	kdUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_kd");

	ksUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_ks");

	materialShininessUniform_pf = glGetUniformLocation(gShaderProgramObject_perFrag,
		"u_material_shininess");

	//Post-Linking reteriving uniform location
	viewUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_view_matrix");

	modelUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_model_matrix");

	projectionUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_projection_matrix");

	lKeyPressedUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_lKeyPressed");

	laUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_la");

	ldUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_ld");

	lsUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_ls");

	lightPositionUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_light_position");

	kaUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_ka");

	kdUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_kd");

	ksUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
		"u_ks");

	materialShininessUniform_pv = glGetUniformLocation(gShaderProgramObject_perVert,
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

	//usual opengl initialization code:
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	//glDisable(GL_CULL_FACE);

	//make identity
	perspectiveProjectionMatrix = mat4::identity();


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
	
	perspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

void display(void)
{
	//usual display code - last line only in double buffering
	//but in single buffering, whole code is same

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//Binding Opengl code to shader program object
	if (isFKeyPressed == true)
	{
		glUseProgram(gShaderProgramObject_perFrag);

		//matrices
		mat4 viewMatrix;
		mat4 modelMatrix;
		mat4 ProjectionMatrix;
		mat4 translationMatrix;

		//make identity
		viewMatrix = mat4::identity();;
		modelMatrix = mat4::identity();;
		ProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		//do necessary transformation
		translationMatrix = translate(0.0f, 0.0f, -2.0f);
		//do necessary matrix multiplication
		//this was internally done by gluOrtho() in ffp
		modelMatrix = translationMatrix;
		ProjectionMatrix = perspectiveProjectionMatrix;

		//send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(viewUniform_pf,//which uniform?
			1,//how many matrices
			GL_FALSE,//have to transpose?
			viewMatrix);//actual matrix

		glUniformMatrix4fv(modelUniform_pf,//which uniform?
			1,//how many matrices
			GL_FALSE,//have to transpose?
			modelMatrix);//actual matrix

		glUniformMatrix4fv(projectionUniform_pf,//which uniform?
			1,//how many matrices
			GL_FALSE,//have to transpose?
			ProjectionMatrix);//actual matrix

		if (gbLight == true)
		{
			glUniform1i(lKeyPressedUniform_pf, 1);
			glUniform3fv(laUniform_pf, 1, LightAmbient);
			glUniform3fv(ldUniform_pf, 1, LightDiffuse);
			glUniform3fv(lsUniform_pf, 1, LightSpecular);
			glUniform3fv(kaUniform_pf, 1, MaterialAmbient);
			glUniform3fv(kdUniform_pf, 1, MaterialDiffuse);
			glUniform3fv(ksUniform_pf, 1, MaterialSpecular);
			glUniform1f(materialShininessUniform_pf, MaterialShininess);
			glUniform4fv(lightPositionUniform_pf, 1, LightPosition);
		}
		else
		{
			glUniform1i(lKeyPressedUniform_pf, 0);
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
	}

	if (isVKeyPressed == true)
	{
		glUseProgram(gShaderProgramObject_perVert);

		//matrices
		mat4 viewMatrix;
		mat4 modelMatrix;
		mat4 ProjectionMatrix;
		mat4 translationMatrix;

		//make identity
		viewMatrix = mat4::identity();;
		modelMatrix = mat4::identity();;
		ProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		//do necessary transformation
		translationMatrix = translate(0.0f, 0.0f, -2.0f);
		//do necessary matrix multiplication
		//this was internally done by gluOrtho() in ffp
		modelMatrix = translationMatrix;
		ProjectionMatrix = perspectiveProjectionMatrix;

		//send necessary matrices to shader in respective uniforms
		glUniformMatrix4fv(viewUniform_pv,//which uniform?
			1,//how many matrices
			GL_FALSE,//have to transpose?
			viewMatrix);//actual matrix

		glUniformMatrix4fv(modelUniform_pv,//which uniform?
			1,//how many matrices
			GL_FALSE,//have to transpose?
			modelMatrix);//actual matrix

		glUniformMatrix4fv(projectionUniform_pv,//which uniform?
			1,//how many matrices
			GL_FALSE,//have to transpose?
			ProjectionMatrix);//actual matrix

		if (gbLight == true)
		{
			glUniform1i(lKeyPressedUniform_pv, 1);
			glUniform3fv(laUniform_pv, 1, LightAmbient);
			glUniform3fv(ldUniform_pv, 1, LightDiffuse);
			glUniform3fv(lsUniform_pv, 1, LightSpecular);
			glUniform3fv(kaUniform_pv, 1, MaterialAmbient);
			glUniform3fv(kdUniform_pv, 1, MaterialDiffuse);
			glUniform3fv(ksUniform_pv, 1, MaterialSpecular);
			glUniform1f(materialShininessUniform_pv, MaterialShininess);
			glUniform4fv(lightPositionUniform_pv, 1, LightPosition);
		}
		else
		{
			glUniform1i(lKeyPressedUniform_pv, 0);
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
	}

	glXSwapBuffers(gpDisplay, gWindow);
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

		if (gShaderProgramObject_perFrag)
	{
		GLsizei shaderCount;
		GLsizei shaderNumber;

		glUseProgram(gShaderProgramObject_perFrag);

		//ask the program how many shaders are attached to you?
		glGetProgramiv(gShaderProgramObject_perFrag,
			GL_ATTACHED_SHADERS,
			&shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			//fprintf(gpFile, "\npshaders sucessful\n");

			//get shaders
			glGetAttachedShaders(gShaderProgramObject_perFrag,
				shaderCount,
				&shaderCount,
				pShaders);

			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				//detach
				glDetachShader(gShaderProgramObject_perFrag,
					pShaders[shaderNumber]);

				//delete
				glDeleteShader(pShaders[shaderNumber]);

				//explicit 0
				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(gShaderProgramObject_perFrag);
		gShaderProgramObject_perFrag = 0;

		glUseProgram(0);
	}

	if (gShaderProgramObject_perVert)
	{
		GLsizei shaderCount;
		GLsizei shaderNumber;

		glUseProgram(gShaderProgramObject_perVert);

		//ask the program how many shaders are attached to you?
		glGetProgramiv(gShaderProgramObject_perVert,
			GL_ATTACHED_SHADERS,
			&shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			//fprintf(gpFile, "\npshaders sucessful\n");

			//get shaders
			glGetAttachedShaders(gShaderProgramObject_perVert,
				shaderCount,
				&shaderCount,
				pShaders);

			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				//detach
				glDetachShader(gShaderProgramObject_perVert,
					pShaders[shaderNumber]);

				//delete
				glDeleteShader(pShaders[shaderNumber]);

				//explicit 0
				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(gShaderProgramObject_perVert);
		gShaderProgramObject_perVert = 0;

		glUseProgram(0);
	}

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

