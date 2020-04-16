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
	GLuint tessellationControlShaderObject;
	GLuint tessellationEvaluationShaderObject;
    GLuint fragmentShaderObject;

    GLuint vao;
    GLuint vbo;

    GLuint mvpUniform;
	GLuint numberOfSegmentsUniform;
	GLuint numberOfStripsUniform;
	GLuint lineColorUniform;

	unsigned int numberOfLineSegments;

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

    numberOfLineSegments = 1;

    //****************1. VERTEX SHADER****************
    //define vertex shader object
	//create vertex shader object
	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	//Write vertex shader code
	const GLchar *vertexShaderSourceCode =
		"#version 410" \
		"\n" \
		"in vec4 vPosition;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"}";

	//specify above source code to vertex shader object
	glShaderSource(vertexShaderObject,//to whom?
		1,//how many strings
		(const GLchar **)&vertexShaderSourceCode,//address of string
		NULL);// NULL specifes that there is only one string with fixed length

	//Compile the vertex shader
	glCompileShader(vertexShaderObject);


	//Error checking for compilation:
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(vertexShaderObject, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus);//in what?

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

				glGetShaderInfoLog(vertexShaderObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

				fprintf(gpFile, "\nVertex Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

                [self release];
                [NSApp terminate:self];
			}
		}
	}

	//************************** 2. TESSELLATION CONTROL SHADER ********************************
	//define TC shader object
	//create TC shader object
	tessellationControlShaderObject = glCreateShader(GL_TESS_CONTROL_SHADER);

	//write fragment shader code
	const GLchar *tessellationControlShaderSourceCode =
		"#version 410"
		"\n"
		"layout(vertices=4)out;"
		"uniform int u_numberOfSegments;"
		"uniform int u_numberOfStrips;"
		"void main(void)"
		"{"
		"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;"
		"	gl_TessLevelOuter[0] = float(u_numberOfStrips);"
		"	gl_TessLevelOuter[1] = float(u_numberOfSegments);"
		"}";

	//specify the above source code to fragment shader object
	glShaderSource(tessellationControlShaderObject,
				   1,
				   (const GLchar **)&tessellationControlShaderSourceCode,
				   NULL);

	//compile the fragment shader
	glCompileShader(tessellationControlShaderObject);

	//Error checking for compilation
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(tessellationControlShaderObject, // whose?
				  GL_COMPILE_STATUS,			   //what to get?
				  &iShaderCompileStatus);		   //in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(tessellationControlShaderObject,
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

				glGetShaderInfoLog(tessellationControlShaderObject, //whose?
								   iInfoLogLength,					//length?
								   &written,						//might have not used all, give that much only which have been used in what?
								   szInfoLog);						//store in what?

				fprintf(gpFile, "\nTC Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

				[self release];
                [NSApp terminate:self];
			}
		}
	}

	//************************** 3. TESSELLATION EVALUATION SHADER ********************************
	//define TC shader object
	//create TC shader object
	tessellationEvaluationShaderObject = glCreateShader(GL_TESS_EVALUATION_SHADER);

	//write fragment shader code
	const GLchar *tessellationEvaluationShaderSourceCode =
		"#version 410"
		"\n"
		"layout(isolines)in;"
		"uniform mat4 u_mvp_matrix;"
		"void main(void)"
		"{"
		"	float u = gl_TessCoord.x;"
		"	vec3 p0 = gl_in[0].gl_Position.xyz;"
		"	vec3 p1 = gl_in[1].gl_Position.xyz;"
		"	vec3 p2 = gl_in[2].gl_Position.xyz;"
		"	vec3 p3 = gl_in[3].gl_Position.xyz;"
		"	float u1 = (1.0 - u);"
		"	float u2 = u * u;"
		"	float b3 = u2 * u;"
		"	float b2 = 3.0 * u2 * u1;"
		"	float b1 = 3.0 * u * u1 * u1;"
		"	float b0 = u1 * u1 * u1;"
		"	vec3 p = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;"
		"	gl_Position = u_mvp_matrix * vec4(p, 1.0);"
		"}";

	//specify the above source code to fragment shader object
	glShaderSource(tessellationEvaluationShaderObject,
				   1,
				   (const GLchar **)&tessellationEvaluationShaderSourceCode,
				   NULL);

	//compile the fragment shader
	glCompileShader(tessellationEvaluationShaderObject);

	//Error checking for compilation
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	//Step 1 : Call glGetShaderiv() to get comiple status of particular shader
	glGetShaderiv(tessellationEvaluationShaderObject, // whose?
				  GL_COMPILE_STATUS,				  //what to get?
				  &iShaderCompileStatus);			  //in what?

	//Step 2 : Check shader compile status for GL_FALSE
	if (iShaderCompileStatus == GL_FALSE)
	{
		//Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
		glGetShaderiv(tessellationEvaluationShaderObject,
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

				glGetShaderInfoLog(tessellationEvaluationShaderObject, //whose?
								   iInfoLogLength,					   //length?
								   &written,						   //might have not used all, give that much only which have been used in what?
								   szInfoLog);						   //store in what?

				fprintf(gpFile, "\nTE Shader Compilation Log : %s\n", szInfoLog);

				//free the memory
				free(szInfoLog);

			    [self release];
                [NSApp terminate:self];
			}
		}
	}

	//************************** 4. FRAGMENT SHADER ********************************
	//define fragment shader object
	//create fragment shader object
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	//write fragment shader code
	const GLchar *fragmentShaderSourceCode =
		"#version 410" \
		"\n" \
        "uniform vec4 u_lineColor;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{" \
		"fragColor = u_lineColor;" \
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
	glGetShaderiv(fragmentShaderObject, // whose?
		GL_COMPILE_STATUS,//what to get?
		&iShaderCompileStatus);//in what?

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

				glGetShaderInfoLog(fragmentShaderObject,//whose?
					iInfoLogLength,//length?
					&written,//might have not used all, give that much only which have been used in what?
					szInfoLog);//store in what?

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

	//Attach TC shader to shader program
	glAttachShader(shaderProgramObject,			 //to whom?
				   tessellationControlShaderObject); //what to attach?

	//Attach TC shader to shader program
	glAttachShader(shaderProgramObject,				//to whom?
				   tessellationEvaluationShaderObject); //what to attach?

	//Attach fragment shader to shader program
	glAttachShader(shaderProgramObject,
		fragmentShaderObject);

	//Pre-Linking binding to vertex attribute
	glBindAttribLocation(shaderProgramObject,
		AMC_ATTRIBUTE_POSITION,
		"vPosition");

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
	mvpUniform = glGetUniformLocation(shaderProgramObject,
		"u_mvp_matrix");

	numberOfSegmentsUniform = glGetUniformLocation(shaderProgramObject,
													"u_numberOfSegments");

	numberOfStripsUniform = glGetUniformLocation(shaderProgramObject,
												  "u_numberOfStrips");

	lineColorUniform = glGetUniformLocation(shaderProgramObject,
											 "u_lineColor");

	//above is the preparation of data transfer from CPU to GPU
	//i.e glBindAttribLocation() & glGetUniformLocation()

	//array initialization (glBegin() and glEnd())
	const GLfloat vertices[] = {
		-1.0f, -1.0f,
		-0.5f, 1.0f,
		0.5f, -1.0f,
		1.0f, -1.0f};

	//create vao (vertex array object)
	glGenVertexArrays(1, &vao);

	//Bind vao
	glBindVertexArray(vao);

	//generate vertex buffers
	glGenBuffers(1, &vbo);

	//bind buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//transfer vertex data(CPU) to GPU buffer
	glBufferData(GL_ARRAY_BUFFER,
				 8 * sizeof(float),
				 vertices,
				 GL_STATIC_DRAW);

	//attach or map attribute pointer to vbo's buffer
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  NULL);

	//enable vertex attribute array
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	//unbind vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//unbind vao
	glBindVertexArray(0);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glEnable(GL_CULL_FACE);

    glLineWidth(5.0f);
    
    //set bk color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    perspectiveProjectionMatrix = vmath::mat4::identity();

    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext]CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];

    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
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

	//matrices
	vmath::mat4 modelViewMatrix;
	vmath::mat4 modelViewProjectionMatrix;
	vmath::mat4 translationMatrix;

	//make identity
	modelViewMatrix = vmath::mat4::identity();
	modelViewProjectionMatrix = vmath::mat4::identity();
	translationMatrix = vmath::mat4::identity();

	//do necessary transformation
	translationMatrix = vmath::translate(0.5f, 0.5f, -5.0f);

	//do necessary matrix multiplication
	//this was internally done by gluOrtho() in ffp
	modelViewMatrix = modelViewMatrix * translationMatrix;
	modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

	//send necessary matrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform,				   //which uniform?
					   1,						   //how many matrices
					   GL_FALSE,				   //have to transpose?
					   modelViewProjectionMatrix); //actual matrix

	glUniform1i(numberOfSegmentsUniform, numberOfLineSegments);
	glUniform1i(numberOfStripsUniform, 1);
	if (numberOfLineSegments == 1)
	{
		glUniform4fv(lineColorUniform, 1, vmath::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	}
	else if (numberOfLineSegments < 50)
	{
		glUniform4fv(lineColorUniform, 1, vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else if (numberOfLineSegments == 50)
	{
		glUniform4fv(lineColorUniform, 1, vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}

	//bind with vao
	glBindVertexArray(vao);

	//similarly bind with textures if any

	//now draw the necessary scene
	glDrawArrays(GL_PATCHES,
		0,
		4);

	//unbind vao
	glBindVertexArray(0);

	//unbinding program
	glUseProgram(0);

    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
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

		//down
		case 'd':
        case 'D':
			numberOfLineSegments--;
			if (numberOfLineSegments <= 0)
			{
				numberOfLineSegments = 1;
			}
			break;

		//up
		case 'u':
        case 'U':
			numberOfLineSegments++;
			if (numberOfLineSegments >= 50)
			{
				numberOfLineSegments = 50;
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
    if (vbo)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
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
