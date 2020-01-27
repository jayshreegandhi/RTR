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

#include<SOIL/SOIL.h>

#include"vmath.h"

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

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint vao_cube;// vertex array object
GLuint vbo_cube;
GLuint texture_marble;

GLfloat angleCube = 0.0f;

GLuint modelUniform; //model view uniform
GLuint viewUniform;
GLuint projectionUniform;

GLuint laUniform;
GLuint ldUniform;
GLuint lsUniform;
GLuint lightPositionUniform;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;

GLuint samplerUniform;
GLuint materialShininessUniform;
GLuint lKeyPressedUniform;

mat4 perspectiveProjectionMatrix;

bool gbLight = false;

float LightAmbient[4] = { 0.25f, 0.25f, 0.25f, 0.0f };
float LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float LightPosition[4] = { 100.0f, 100.0f, 100.0f, 1.0f };

float MaterialAmbient[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float MaterialDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float MaterialShininess = 128.0f;


//function prototypes
void CreateWindow(void);
void ToggleFullscreen(void);
void uninitialize(void);

void initialize(void);
void resize(int,int);
void display(void);
void update(void);
bool loadTexture(GLuint *, const char *);


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
	
	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			imageWidth,
			imageHeight,
			0,
			GL_BGR,
			GL_UNSIGNED_BYTE,
			imageData);

	
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	SOIL_free_image_data(imageData);

	return(bResult);
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

	//***************** 1. VERTEX SHADER ************************************ 
	//define vertex shader object
	//create vertex shader object
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write vertex shader code
	const GLchar *vertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"in vec2 vTexCoord;" \
		"in vec3 vNormal;" \
		"uniform int u_lKeyPressed;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform vec4 u_light_position;" \
		"out vec3 tNormal;" \
		"out vec4 out_color;" \
		"out vec2 out_texcoord;" \
		"out vec3 light_direction;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		"	vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" \
		"	tNormal = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"	light_direction = vec3(u_light_position - eyeCoords);" \
		"	viewer_vector = vec3(-eyeCoords.xyz);" \
		"	gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"	out_texcoord = vTexCoord;" \
		"	out_color = vColor;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(gVertexShaderObject,//to whom?
		1,//how many strings
		(const GLchar **)&vertexShaderSourceCode,//address of string
		NULL);// NULL specifes that there is only one string with fixed length

	//Compile the vertex shader
	glCompileShader(gVertexShaderObject);

	
	//Error checking for compilation:
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(gVertexShaderObject, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length 
		glGetShaderiv(gVertexShaderObject,
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

				glGetShaderInfoLog(gVertexShaderObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nVertex Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

				uninitialize();
				exit(0);
			}
		}
	}


	//************************** 2. FRAGMENT SHADER ********************************
	//define fragment shader object
	//create fragment shader object
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec3 tNormal;" \
		"in vec2 out_texcoord;" \
		"in vec4 out_color;" \
		"in vec3 light_direction;" \
		"in vec3 viewer_vector;" \
		"uniform int u_lKeyPressed;" \
		"uniform sampler2D u_sampler;" \
		"uniform vec3 u_la;" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_ls;" \
		"uniform vec3 u_ka;" \
		"uniform vec3 u_kd;" \
		"uniform vec3 u_ks;" \
		"uniform float u_material_shininess;" \
		"vec3 phong_ads_light;" \
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
		"fragColor = texture(u_sampler, out_texcoord) * out_color * vec4(phong_ads_light ,1.0);" \
		"}";


	//specify the above source code to fragment shader object
	glShaderSource(gFragmentShaderObject,
		1,
		(const GLchar **)&fragmentShaderSourceCode,
		NULL);

	//compile the fragment shader
	glCompileShader(gFragmentShaderObject);

	
	//Error checking for compilation
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(gFragmentShaderObject, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length 
		glGetShaderiv(gFragmentShaderObject,
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

				glGetShaderInfoLog(gFragmentShaderObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nFragment Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

				uninitialize();
				exit(0);
			}
		}
	}

	//create shader program object
	gShaderProgramObject = glCreateProgram();

	//Attach vertex shader to shader program
	glAttachShader(gShaderProgramObject,//to whom?
		gVertexShaderObject);//what to attach?

	//Attach fragment shader to shader program
	glAttachShader(gShaderProgramObject,
		gFragmentShaderObject);
		
	//Pre-Linking binding to vertex attribute
	glBindAttribLocation(gShaderProgramObject,
		AMC_ATTRIBUTE_POSITION,
		"vPosition");

	glBindAttribLocation(gShaderProgramObject,
		AMC_ATTRIBUTE_COLOR,
		"vColor");

	glBindAttribLocation(gShaderProgramObject,
		AMC_ATTRIBUTE_TEXCOORD0,
		"vTexCoord");

	glBindAttribLocation(gShaderProgramObject,
		AMC_ATTRIBUTE_NORMAL,
		"vNormal");

	//Link the shader program
	glLinkProgram(gShaderProgramObject);//link to whom?

	
	//Error checking for linking
	GLint iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetProgramiv() to get comiple status of particular shader
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

	samplerUniform = glGetUniformLocation(gShaderProgramObject,
		"u_sampler");

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
	
	const GLfloat cubeVCNT[] = {
		1.0f, 1.0f, -1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,0.0f,0.0f,
		-1.0f, 1.0f, -1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,1.0f,0.0f,
		-1.0f, 1.0f, 1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,1.0f,1.0f,
		1.0f, 1.0f, 1.0f,1.0f, 0.0f, 0.0f,0.0f, 1.0f, 0.0f,0.0f,1.0f,
		
		1.0f, -1.0f, -1.0f,0.0f,1.0f,0.0f,0.0f, -1.0f, 0.0f,0.0f,0.0f,
		-1.0f, -1.0f, -1.0f,0.0f,1.0f,0.0f,0.0f, -1.0f, 0.0f,1.0f,0.0f,
		-1.0f, -1.0f, 1.0f,0.0f,1.0f,0.0f,0.0f, -1.0f, 0.0f,1.0f,1.0f,
		1.0f, -1.0f, 1.0f,0.0f,1.0f,0.0f,0.0f, -1.0f, 0.0f,0.0f,1.0f,
		
		1.0f, 1.0f, 1.0f,0.0f,0.0f,1.0f,0.0f, 0.0f, 1.0f,0.0f,0.0f,
		-1.0f, 1.0f, 1.0f,0.0f,0.0f,1.0f,0.0f, 0.0f, 1.0f,1.0f,0.0f,
		-1.0f, -1.0f, 1.0f,0.0f,0.0f,1.0f,0.0f, 0.0f, 1.0f,1.0f,1.0f,
		1.0f, -1.0f, 1.0f,0.0f,0.0f,1.0f,0.0f, 0.0f, 1.0f,0.0f,1.0f,
		
		1.0f, 1.0f, -1.0f,0.0f,1.0f,1.0f,0.0f, 0.0f, -1.0f,0.0f,0.0f,
		-1.0f, 1.0f, -1.0f,0.0f,1.0f,1.0f,0.0f, 0.0f, -1.0f,1.0f,0.0f,
		-1.0f, -1.0f, -1.0f,0.0f,1.0f,1.0f,0.0f, 0.0f, -1.0f,1.0f,1.0f,
		1.0f, -1.0f, -1.0f,0.0f,1.0f,1.0f,0.0f, 0.0f, -1.0f,0.0f,1.0f,
		
		1.0f, 1.0f, -1.0f,1.0f,0.0f,1.0f,1.0f, 0.0f, 0.0f,0.0f,0.0f,
		1.0f, 1.0f, 1.0f,1.0f,0.0f,1.0f,1.0f, 0.0f, 0.0f,1.0f,0.0f,
		1.0f, -1.0f, 1.0f,1.0f,0.0f,1.0f,1.0f, 0.0f, 0.0f,1.0f,1.0f,
		1.0f, -1.0f, -1.0f,1.0f,0.0f,1.0f,1.0f, 0.0f, 0.0f,0.0f,1.0f,
		
		-1.0f, 1.0f, -1.0f,1.0f,1.0f,0.0f,-1.0f, 0.0f, 0.0f,0.0f,0.0f,
		-1.0f, 1.0f, 1.0f,1.0f,1.0f,0.0f,-1.0f, 0.0f, 0.0f,1.0f,0.0f,
		-1.0f, -1.0f, 1.0f,1.0f,1.0f,0.0f,-1.0f, 0.0f, 0.0f,1.0f,1.0f,
		-1.0f, -1.0f, -1.0f,1.0f,1.0f,0.0f,-1.0f, 0.0f, 0.0f,0.0f,1.0f };

    
	//------------------------------Cube-------------------------------
	//create vao rectangle(vertex array object)
	glGenVertexArrays(1, &vao_cube);

	//Bind vao
	glBindVertexArray(vao_cube);

	//generate vertex buffers
	glGenBuffers(1, &vbo_cube);

	//bind buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);

	//transfer vertex data(CPU) to GPU buffer
	glBufferData(GL_ARRAY_BUFFER,
		24 * 11 * sizeof(float),
		cubeVCNT,
		GL_STATIC_DRAW);

	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION,
		3,
		GL_FLOAT,
		GL_FALSE,
		11 * sizeof(float),
		(void *) 0);

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);


	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR,
		3,
		GL_FLOAT,
		GL_FALSE,
		11 * sizeof(float),
		(void *)(3 * sizeof(float)));

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);


	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL,
		3,
		GL_FLOAT,
		GL_FALSE,
		11 * sizeof(float),
		(void *)(6 * sizeof(float)));

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0,
		2,
		GL_FLOAT,
		GL_FALSE,
		11 * sizeof(float),
		(void *)(9 * sizeof(float)));

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);

	//unbind vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//unbind vao
	glBindVertexArray(0);

	//usual opengl initialization code:
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	//glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	loadTexture(&texture_marble,"marble.bmp");

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
	glUseProgram(gShaderProgramObject);
	
	//matrices
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 ProjectionMatrix;
	mat4 translationMatrix;
	mat4 scaleMatrix;
	mat4 rotationMatrix;

	//make identity
	viewMatrix = mat4::identity();
	modelMatrix = mat4::identity();
	ProjectionMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	//do necessary transformation
	translationMatrix = translate(0.0f, 0.0f, -6.0f);
	scaleMatrix = scale(0.75f, 0.75f, 0.75f);
	rotationMatrix = rotate(angleCube, angleCube, angleCube);

	//do necessary matrix multiplication
	//this was internally done by gluOrtho() in ffp
	modelMatrix = translationMatrix * scaleMatrix * rotationMatrix;
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
		glUniform3fv(laUniform, 1, LightAmbient);
		glUniform3fv(ldUniform, 1, LightDiffuse);
		glUniform3fv(lsUniform, 1, LightSpecular);
		glUniform3fv(kaUniform, 1, MaterialAmbient);
		glUniform3fv(kdUniform, 1, MaterialDiffuse);
		glUniform3fv(ksUniform, 1, MaterialSpecular);
		glUniform1f(materialShininessUniform, MaterialShininess);
		glUniform4fv(lightPositionUniform, 1, LightPosition);
	}
	else
	{
		glUniform1i(lKeyPressedUniform, 0);
	}

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, texture_marble);
	glUniform1i(samplerUniform, 0);

	//bind with vao
	glBindVertexArray(vao_cube);

	//similarly bind with textures if any

	//now draw the necessary scene
	glDrawArrays(GL_TRIANGLE_FAN,
		0,
		4);
	glDrawArrays(GL_TRIANGLE_FAN,
		4,
		4);
	glDrawArrays(GL_TRIANGLE_FAN,
		8,
		4);
	glDrawArrays(GL_TRIANGLE_FAN,
		12,
		4);
	glDrawArrays(GL_TRIANGLE_FAN,
		16,
		4);
	glDrawArrays(GL_TRIANGLE_FAN,
		20,
		4);

	//unbind vao
	glBindVertexArray(0);
	//unbinding program
	glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow);
}

void update(void)
{
	angleCube = angleCube - 1.0f;

	if (angleCube <= -360.0f)
	{
		angleCube = 0.0f;
	}
}

void uninitialize(void)
{
	if (texture_marble)
	{
		glDeleteTextures(1, &texture_marble);
	}

    if (vbo_cube)
	{
		glDeleteBuffers(1, &vbo_cube);
		vbo_cube = 0;
	}

	if (vao_cube)
	{
		glDeleteVertexArrays(1, &vao_cube);
		vao_cube = 0;
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

