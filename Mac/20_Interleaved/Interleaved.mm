#import <Foundation/Foundation.h> //stdio.h
#import <Cocoa/Cocoa.h> // windowing framework

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGl/gl3ext.h>

#import "vmath.h"

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0,
};

// C Style global function declaration
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *, void *);

//global variables
FILE *gpFile = NULL;

//interface declarations
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView : NSOpenGLView
@end

//Entry point function
int main(int argc, const char* argv[])
{
    //code
    NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc]init];
    NSApp = [NSApplication sharedApplication];

    [NSApp setDelegate:[[AppDelegate alloc]init]];

    [NSApp run];

    [pPool release];

    return(0);
}

@implementation AppDelegate
{
@private
    NSWindow *window;
    GLView *glView;
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //code
    //log file
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *appDirName = [mainBundle bundlePath];
    NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/Log.txt",parentDirPath];
    const char *pszLogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    gpFile = fopen(pszLogFileNameWithPath,"w");
    if(gpFile == NULL)
    {
        printf("Can not create log file. \nExiting...\n");
        [self release];
        [NSApp terminate:self];
    }
    fprintf(gpFile, "Program is started successfully\n");

    //window
    NSRect win_rect;
    win_rect = NSMakeRect(0.0, 0.0, 800.0, 600.0);

    //create simple window
    window = [[NSWindow alloc] initWithContentRect:win_rect
                                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                backing:NSBackingStoreBuffered
                                defer:NO];

    [window setTitle:@"macOS OpenGl Window"];
    [window center];

    glView =[[GLView alloc] initWithFrame:win_rect];
    [window setContentView: glView];
    [window setDelegate: self];
    [window makeKeyAndOrderFront:self];
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    fprintf(gpFile, "Program Is terminated successfully\n");

    if(gpFile)
    {
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void)windowWillClose:(NSNotification *)notification
{
    //code
    [NSApp terminate:self];
}

-(void)dealloc
{
    //code
    [glView release];
    [window release];
    [super dealloc];
}
@end

@implementation GLView
{
@private
    CVDisplayLinkRef displayLink;

    GLuint shaderProgramObject;
    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;

	GLuint vao_cube;
	GLuint vbo_cube;
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
	GLuint samplerUniform;

	GLuint texture_marble;
	bool gbLight;
	GLfloat angleCube;

	GLfloat LightAmbient[4];
	GLfloat LightDiffuse[4];
	GLfloat LightSpecular[4];
	GLfloat LightPosition[4];

	GLfloat MaterialAmbient[4];
	GLfloat MaterialDiffuse[4];
	GLfloat MaterialSpecular[4];
	GLfloat MaterialShininess;

    vmath::mat4 perspectiveProjectionMatrix;
}

-(id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];

    if(self)
    {
        [[self window]setContentView: self];

        NSOpenGLPixelFormatAttribute attrs[] =
        {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer,
            0
        };

        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc]initWithAttributes:attrs] autorelease];

        if(pixelFormat == nil)
        {
            fprintf(gpFile, "No Valid OpenGL pixel format is available.Exiting...\n");
            [self release];
            [NSApp terminate:self];
        }

        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil] autorelease];

        [self setPixelFormat:pixelFormat];
        [self setOpenGLContext:glContext];
    }
    return(self);
}

-(CVReturn)getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    //code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];

    [self drawView];
    [pool release];

    return(kCVReturnSuccess);
}

-(void)prepareOpenGL
{
    //code
	[super prepareOpenGL];
    //OpenGL info

    fprintf(gpFile, "OpenGL Version : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    [[self openGLContext]makeCurrentContext];

    GLint swapInt = 1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

	LightAmbient[0] = 0.25f;
	LightAmbient[1] = 0.25f;
	LightAmbient[2] = 0.25f;
	LightAmbient[3] = 0.0f;

	LightDiffuse[0] = 1.0f;
	LightDiffuse[1] = 1.0f;
	LightDiffuse[2] = 1.0f;
	LightDiffuse[3] = 1.0f;

	LightSpecular[0] = 1.0f;
	LightSpecular[1] = 1.0f;
	LightSpecular[2] = 1.0f;
	LightSpecular[3] = 1.0f;

	LightPosition[0] = 100.0f;
	LightPosition[1] = 100.0f;
	LightPosition[2] = 100.0f;
	LightPosition[3] = 1.0f;

	MaterialAmbient[0] = 0.0f;
	MaterialAmbient[1] = 0.0f;
	MaterialAmbient[2] = 0.0f;
	MaterialAmbient[3] = 1.0f;

	MaterialDiffuse[0] = 1.0f;
	MaterialDiffuse[1] = 1.0f;
	MaterialDiffuse[2] = 1.0f;
	MaterialDiffuse[3] = 1.0f;

	MaterialSpecular[0] = 1.0f;
	MaterialSpecular[1] = 1.0f;
	MaterialSpecular[2] = 1.0f;
	MaterialSpecular[3] = 1.0f;

	MaterialShininess = 128.0f;

    //****************1. VERTEX SHADER****************
    //define vertex shader object
	//create vertex shader object
	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write vertex shader code
	const GLchar *vertexShaderSourceCode =
		"#version 410"
		"\n"
		"in vec4 vPosition;"
		"in vec4 vColor;"
		"in vec2 vTexCoord;"
		"in vec3 vNormal;"
		"uniform int u_lKeyPressed;"
		"uniform mat4 u_view_matrix;"
		"uniform mat4 u_model_matrix;"
		"uniform mat4 u_projection_matrix;"
		"uniform vec4 u_light_position;"
		"out vec3 tNormal;"
		"out vec4 out_color;"
		"out vec2 out_texcoord;"
		"out vec3 light_direction;"
		"out vec3 viewer_vector;"
		"void main(void)"
		"{"
		"	vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;"
		"	tNormal = mat3(u_view_matrix * u_model_matrix) * vNormal;"
		"	light_direction = vec3(u_light_position - eyeCoords);"
		"	viewer_vector = vec3(-eyeCoords.xyz);"
		"	gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;"
		"	out_texcoord = vTexCoord;"
		"	out_color = vColor;"
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

				[self release];
                [NSApp terminate:self];
			}
		}
	}

	//************************** 2. FRAGMENT SHADER ********************************
	//define fragment shader object
	//create fragment shader object
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 410"
		"\n"
		"in vec3 tNormal;"
		"in vec2 out_texcoord;"
		"in vec4 out_color;"
		"in vec3 light_direction;"
		"in vec3 viewer_vector;"
		"uniform int u_lKeyPressed;"
		"uniform sampler2D u_sampler;"
		"uniform vec3 u_la;"
		"uniform vec3 u_ld;"
		"uniform vec3 u_ls;"
		"uniform vec3 u_ka;"
		"uniform vec3 u_kd;"
		"uniform vec3 u_ks;"
		"uniform float u_material_shininess;"
		"vec3 phong_ads_light;"
		"out vec4 fragColor;"
		"void main(void)"
		"{"
		"if(u_lKeyPressed == 1)"
		"{"
		"vec3 normalized_tNormal = normalize(tNormal);"
		"vec3 normalized_light_direction = normalize(light_direction);"
		"float tNorm_Dot_LightDirection = max(dot(normalized_light_direction, normalized_tNormal), 0.0);"
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_tNormal);"
		"vec3 normalized_viewer_vector = normalize(viewer_vector);"
		"vec3 ambient = u_la * u_ka;"
		"vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;"
		"vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,normalized_viewer_vector), 0.0), u_material_shininess);"
		"phong_ads_light = ambient + diffuse + specular;"
		"}"
		"else"
		"{"
		"phong_ads_light = vec3(1.0, 1.0, 1.0);"
		"}"
		"fragColor = texture(u_sampler, out_texcoord) * out_color * vec4(phong_ads_light ,1.0);"
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

				[self release];
                [NSApp terminate:self];
			}
		}
	}

	//create shader program object
	shaderProgramObject = glCreateProgram();

	//Attach vertex shader to shader program
	glAttachShader(shaderProgramObject,//to whom?
		vertexShaderObject);//what to attach?

	//Attach fragment shader to shader program
	glAttachShader(shaderProgramObject,
		fragmentShaderObject);

	//Pre-Linking binding to vertex attribute
	glBindAttribLocation(shaderProgramObject,
		AMC_ATTRIBUTE_POSITION,
		"vPosition");

	glBindAttribLocation(shaderProgramObject,
		AMC_ATTRIBUTE_COLOR,
		"vColor");

	glBindAttribLocation(shaderProgramObject,
		 AMC_ATTRIBUTE_NORMAL,
		 "vNormal");

	glBindAttribLocation(shaderProgramObject,
		AMC_ATTRIBUTE_TEXCOORD0,
		"vTexCoord");

	//Link the shader program
	glLinkProgram(shaderProgramObject);//link to whom?

	//Error checking for linking
	GLint iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetProgramiv(shaderProgramObject, // whose?
		GL_LINK_STATUS,//what to get?
		&iProgramLinkStatus);//in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iProgramLinkStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetProgramiv(shaderProgramObject,
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

				glGetShaderInfoLog(shaderProgramObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nShader Program Linking Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

                [self release];
                [NSApp terminate:self];
			}
		}
	}

	//Post-Linking reteriving uniform location
	viewUniform = glGetUniformLocation(shaderProgramObject,
									   "u_view_matrix");

	modelUniform = glGetUniformLocation(shaderProgramObject,
										"u_model_matrix");

	projectionUniform = glGetUniformLocation(shaderProgramObject,
											 "u_projection_matrix");

	samplerUniform = glGetUniformLocation(shaderProgramObject,
										  "u_sampler");

	lKeyPressedUniform = glGetUniformLocation(shaderProgramObject,
											  "u_lKeyPressed");

	laUniform = glGetUniformLocation(shaderProgramObject,
									 "u_la");

	ldUniform = glGetUniformLocation(shaderProgramObject,
									 "u_ld");

	lsUniform = glGetUniformLocation(shaderProgramObject,
									 "u_ls");

	lightPositionUniform = glGetUniformLocation(shaderProgramObject,
												"u_light_position");

	kaUniform = glGetUniformLocation(shaderProgramObject,
									 "u_ka");

	kdUniform = glGetUniformLocation(shaderProgramObject,
									 "u_kd");

	ksUniform = glGetUniformLocation(shaderProgramObject,
									 "u_ks");

	materialShininessUniform = glGetUniformLocation(shaderProgramObject,
													"u_material_shininess");

	//above is the preparation of data transfer from CPU to GPU
	//i.e glBindAttribLocation() & glGetUniformLocation()

	//array initialization (glBegin() and glEnd())
	const GLfloat cubeVCNT[] = {
		1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

		1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

		1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

		1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	//------------------------------CUBE-------------------------------
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
						  (void *)0);

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

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //set bk color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);
	texture_marble = [self loadTextureFromBMPFile:"marble.bmp"];

    perspectiveProjectionMatrix = vmath::mat4::identity();

    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];

    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

-(GLuint)loadTextureFromBMPFile:(const char *)texFileName
{
	NSBundle *mainBundle = [NSBundle mainBundle];
	NSString *appDirName = [mainBundle bundlePath];
	NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
	NSString *textureFileNameWithPath = [NSString stringWithFormat:@"%@/%s", parentDirPath, texFileName];

	NSImage *bmpImage=[[NSImage alloc] initWithContentsOfFile:textureFileNameWithPath];
	if(!bmpImage)
	{
		fprintf(gpFile, "Can't find texture\n");
		return(0);
	}

	CGImageRef cgImage = [bmpImage CGImageForProposedRect:nil context:nil hints:nil];

	int w = (int)CGImageGetWidth(cgImage);
	int h = (int)CGImageGetHeight(cgImage);

	CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));

	void* pixels = (void *)CFDataGetBytePtr(imageData);

	GLuint bmpTexture;
	glGenTextures(1, &bmpTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, bmpTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
					0,
					GL_RGBA,
					w,
					h,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					pixels);

	glGenerateMipmap(GL_TEXTURE_2D);
	CFRelease(imageData);
	return(bmpTexture);
}

-(void)reshape
{
    //code
	[super reshape];

    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    NSRect rect = [self bounds];

    GLfloat width = rect.size.width;
    GLfloat height = rect.size.height;

    if(height == 0)
        height = 1;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(void)drawRect:(NSRect)dirtyRect
{
    //code
    [self drawView];
}

-(void)drawView
{
    [[self openGLContext]makeCurrentContext];

    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Binding Opengl code to shader program object
	glUseProgram(shaderProgramObject);

	vmath::mat4 viewMatrix;
	vmath::mat4 modelMatrix;
	vmath::mat4 ProjectionMatrix;
	vmath::mat4 translationMatrix;
	vmath::mat4 scaleMatrix;
	vmath::mat4 rotationMatrix;

	//make identity
	viewMatrix = vmath::mat4::identity();
	modelMatrix = vmath::mat4::identity();
	ProjectionMatrix = vmath::mat4::identity();
	translationMatrix = vmath::mat4::identity();
	rotationMatrix = vmath::mat4::identity();

	//do necessary transformation
	translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
	scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);
	rotationMatrix = vmath::rotate(angleCube, angleCube, angleCube);
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
	//bind with vao
	glBindVertexArray(vao_cube);

	//Before binding to vao, work with texture
	//similarly bind with textures if any
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, texture_marble);
	glUniform1i(samplerUniform, 0);

	//bind with vao
	glBindVertexArray(vao_cube);

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

    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);

	angleCube = angleCube - 1.0f;

	if (angleCube <= -360.0f)
	{
		angleCube = 0.0f;
	}
}

-(BOOL)acceptsFirstResponder
{
    //code
    [[self window]makeFirstResponder:self];
    return(YES);
}

-(void)keyDown:(NSEvent *)theEvent
{
    //code
    int key = (int)[[theEvent characters]characterAtIndex:0];

    switch(key)
    {
        case 27:
            [self release];
            [NSApp terminate:self];
            break;

        case 'F':
        case 'f':
            [[self window]toggleFullScreen:self];
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

        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *)theEvent
{
    //code
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    //code
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    //code
}

-(void)dealloc
{
    //code
	if (texture_marble)
	{
		glDeleteTextures(1, &texture_marble);
		texture_marble = 0;
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

	if (shaderProgramObject)
	{
		GLsizei shaderCount;
		GLsizei shaderNumber;

		glUseProgram(shaderProgramObject);

		//ask the program how many shaders are attached to you?
		glGetProgramiv(shaderProgramObject,
			GL_ATTACHED_SHADERS,
			&shaderCount);

		GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

		if (pShaders)
		{
			//fprintf(gpFile, "\npshaders sucessful\n");

			//get shaders
			glGetAttachedShaders(shaderProgramObject,
				shaderCount,
				&shaderCount,
				pShaders);

			for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
			{
				//detach
				glDetachShader(shaderProgramObject,
					pShaders[shaderNumber]);

				//delete
				glDeleteShader(pShaders[shaderNumber]);

				//explicit 0
				pShaders[shaderNumber] = 0;
			}

			free(pShaders);
		}

		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;

		glUseProgram(0);
	}

    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);

    [super dealloc];
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow, const CVTimeStamp *pOutputTime, CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *)pDisplayLinkContext getFrameForTime:pOutputTime];
    return(result);
}