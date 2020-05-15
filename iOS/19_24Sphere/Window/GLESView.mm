//
//  GLESView.mm
//  Window - ortho
//
//  Created by sugat mankar on 21/02/20.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <math.h>

#import "vmath.h"
#import "sphere.h"

#import "GLESView.h"

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0,
};


@implementation GLESView
{
    EAGLContext *eaglContext;

    GLuint defaultFramebuffer;
    GLuint colorRenderbuffer;
    GLuint depthRenderbuffer;

    id displayLink;
    NSInteger animationFrameInterval;
    BOOL isAnimating;

    GLuint shaderProgramObject;
    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;

    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;

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

	bool gbLight;

	GLint keyPress;
	GLint gWidth;
	GLint gHeight;

    GLfloat LightAmbient[4];
    GLfloat LightDiffuse[4];
    GLfloat LightSpecular[4];
    GLfloat LightPosition[4];
    
    GLfloat MaterialAmbient[24][4];
    GLfloat MaterialDiffuse[24][4];
    GLfloat MaterialSpecular[24][4];
    GLfloat MaterialShininess[24];
    
    GLfloat rotationAngleX;
    GLfloat rotationAngleY;
    GLfloat rotationAngleZ;
    
    GLfloat angleZeroRadian;
    GLfloat angleOneRadian;
    GLfloat angleTwoRadian;

    float sphere_vertices[1146];
    float sphere_normals[1146];
    float sphere_texture[746];
    short sphere_elements[2280];
    int gNumVertices;
    int gNumElements;
    
    vmath::mat4 perspectiveProjectionMatrix;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if(self)
    {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)super.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,kEAGLColorFormatRGBA8,kEAGLDrawablePropertyColorFormat,nil];

        eaglContext = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if(eaglContext == nil)
        {
            [self release];
            return(nil);
        }
        [EAGLContext setCurrentContext:eaglContext];

        glGenFramebuffers(1, &defaultFramebuffer);
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

        GLint backingWidth;
        GLint backingHeight;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);

        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("Failed to create framebuffer object %x\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));

            glDeleteFramebuffers(1, &defaultFramebuffer);
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            glDeleteRenderbuffers(1, &depthRenderbuffer);

            return(nil);
        }

        printf("Renderer : %s | GL version : %s | GLSL version : %s\n",glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

        isAnimating = NO;
        animationFrameInterval = 60;

        LightAmbient[0] = 0.0f;
        LightAmbient[1] = 0.0f;
        LightAmbient[2] = 0.0f;
        LightAmbient[3] = 0.0f;

        LightDiffuse[0] = 1.0f;
        LightDiffuse[1] = 1.0f;
        LightDiffuse[2] = 1.0f;
        LightDiffuse[3] = 1.0f;

        LightSpecular[0] = 1.0f;
        LightSpecular[1] = 1.0f;
        LightSpecular[2] = 1.0f;
        LightSpecular[3] = 1.0f;

        LightPosition[0] = 0.0f;
        LightPosition[1] = 0.0f;
        LightPosition[2] = 0.0f;
        LightPosition[3] = 1.0f;

        //****************1. VERTEX SHADER****************
        //define vertex shader object
        //create vertex shader object
        vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

        //Write vertex shader code
        const GLchar *vertexShaderSourceCode =
        "#version 300 es" \
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

                    printf("\nVertex Shader Compilation Log : %s\n",szInfoLog);

                    //free the memory
                    free(szInfoLog);
                    [self release];
                }
            }
        }


        //************************** 2. FRAGMENT SHADER ********************************
        //define fragment shader object
        //create fragment shader object
        fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

        //write fragment shader code
        const GLchar *fragmentShaderSourceCode =
        "#version 300 es" \
        "\n" \
        "precision highp float;" \
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
		"fragColor = vec4(phong_ads_light,1.0);" \
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

                    printf("\nFragment Shader Compilation Log : %s\n", szInfoLog);

                    //free the memory
                    free(szInfoLog);

                    [self release];
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
                             AMC_ATTRIBUTE_NORMAL,
                             "vNormal");

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

                    printf("\nShader Program Linking Log : %s\n", szInfoLog);

                    //free the memory
                    free(szInfoLog);

                    [self release];
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

        Sphere();
        getSphereVertexData(sphere_vertices, sphere_normals, sphere_texture, sphere_elements);

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

        gbLight = false;
        keyPress = 0;

        [self setMaterialSphere];
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        //glEnable(GL_CULL_FACE);

        //set bk color
        glClearColor(0.25f, 0.25f, 0.25f, 0.25f);

        perspectiveProjectionMatrix = vmath::mat4::identity();

        //GESTURES
        UITapGestureRecognizer *singleTapGestureRecognizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onSingleTap:)];
        [singleTapGestureRecognizer setNumberOfTapsRequired:1];
        [singleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [singleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:singleTapGestureRecognizer];

        UITapGestureRecognizer *doubleTapGestureRecognizer = [[UITapGestureRecognizer alloc]initWithTarget:self action:@selector(onDoubleTap:)];
        [doubleTapGestureRecognizer setNumberOfTapsRequired:2];
        [doubleTapGestureRecognizer setNumberOfTouchesRequired:1];
        [doubleTapGestureRecognizer setDelegate:self];
        [self addGestureRecognizer:doubleTapGestureRecognizer];

        [singleTapGestureRecognizer requireGestureRecognizerToFail:doubleTapGestureRecognizer];

        UISwipeGestureRecognizer *swipeGestureRecognizer = [[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(onSwipe:)];
        [self addGestureRecognizer:swipeGestureRecognizer];

        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]initWithTarget:self action:@selector(onLongPress:)];
        [self addGestureRecognizer:longPressGestureRecognizer];

    }
    return(self);
}

/*- (void)drawRect:(CGRect)rect
{
}*/

+(Class)layerClass
{
    return([CAEAGLLayer class]);
}

- (void)drawView:(id)sender
{
    [EAGLContext setCurrentContext:eaglContext];

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);

    int wid = gWidth / 4;
	int ht = gHeight / 6;

    for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 6; y++)
		{
            int xcor = x * wid;
            int ycor = y * ht;
			[self setViewports:xcor :ycor :wid :ht];

			glUseProgram(shaderProgramObject);

			vmath::mat4 viewMatrix;
			vmath::mat4 modelMatrix;
			vmath::mat4 projectionMatrix;
			vmath::mat4 translationMatrix;

			viewMatrix = vmath::mat4::identity();;
			modelMatrix = vmath::mat4::identity();;
            projectionMatrix = vmath::mat4::identity();
			translationMatrix = vmath::mat4::identity();

			translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

			modelMatrix = translationMatrix;
			projectionMatrix = perspectiveProjectionMatrix;

			glUniformMatrix4fv(viewUniform,//which uniform?
				1,//how many matrices
				GL_FALSE,//have to transpose?
				viewMatrix);//actual matrix

			glUniformMatrix4fv(modelUniform,//which uniform?
				1,//how many matrices
				GL_FALSE,//have to transpose?
				modelMatrix);//actual matrix

			glUniformMatrix4fv(projectionUniform,//which uniform?
				1,//how many matrices
				GL_FALSE,//have to transpose?
				projectionMatrix);//actual matrix

			if (gbLight == true)
			{
				glUniform1i(lKeyPressedUniform, 1);
				glUniform3fv(laUniform, 1, LightAmbient);
				glUniform3fv(ldUniform, 1, LightDiffuse);
				glUniform3fv(lsUniform, 1, LightSpecular);

				if (keyPress == 1)
				{
					angleZeroRadian = (GLfloat)(M_PI * rotationAngleX / 180.0f);
					LightPosition[0] = 0.0f;
					LightPosition[1] = 100.f * ((GLfloat)cos(angleZeroRadian)/2.0f - (GLfloat)sin(angleZeroRadian)/2.0f);
					LightPosition[2] = 100.f * ((GLfloat)sin(angleZeroRadian)/2.0f + (GLfloat)cos(angleZeroRadian)/2.0f);
					LightPosition[3] = 1.0f;
					glUniform4fv(lightPositionUniform, 1, LightPosition);
				}
				else if (keyPress == 2)
				{
					angleOneRadian = (GLfloat)(M_PI * rotationAngleY / 180.0f);
					LightPosition[0] = 100.f * ((GLfloat)cos(angleOneRadian)/2.0f + (GLfloat)sin(angleOneRadian)/2.0f);
					LightPosition[1] = 0.0f;
					LightPosition[2] = 100.f * (-(GLfloat)sin(angleOneRadian)/2.0f + (GLfloat)cos(angleOneRadian)/2.0f);
					LightPosition[3] = 1.0f;

					glUniform4fv(lightPositionUniform, 1, LightPosition);
				}
				else if (keyPress == 3)
				{
					angleTwoRadian = (GLfloat)(M_PI * rotationAngleZ / 180.0f);
					LightPosition[0] = 100.f * ((GLfloat)cos(angleTwoRadian)/2.0f - (GLfloat)sin(angleTwoRadian)/2.0f);
					LightPosition[1] = 100.f * ((GLfloat)sin(angleTwoRadian)/2.0f + (GLfloat)cos(angleTwoRadian)/2.0f);
					LightPosition[2] = 0.0f;
					LightPosition[3] = 1.0f;
					glUniform4fv(lightPositionUniform, 1, LightPosition);
				}

				if (x == 0)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[0]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[0]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[0]);
						glUniform1f(materialShininessUniform, MaterialShininess[0]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[1]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[1]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[1]);
						glUniform1f(materialShininessUniform, MaterialShininess[1]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[2]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[2]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[2]);
						glUniform1f(materialShininessUniform, MaterialShininess[2]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[3]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[3]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[3]);
						glUniform1f(materialShininessUniform, MaterialShininess[3]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[4]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[4]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[4]);
						glUniform1f(materialShininessUniform, MaterialShininess[4]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[5]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[5]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[5]);
						glUniform1f(materialShininessUniform, MaterialShininess[5]);
					}
				}
				else if (x == 1)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[6]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[6]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[6]);
						glUniform1f(materialShininessUniform, MaterialShininess[6]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[7]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[7]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[7]);
						glUniform1f(materialShininessUniform, MaterialShininess[7]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[8]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[8]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[8]);
						glUniform1f(materialShininessUniform, MaterialShininess[8]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[9]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[9]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[9]);
						glUniform1f(materialShininessUniform, MaterialShininess[9]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[10]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[10]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[10]);
						glUniform1f(materialShininessUniform, MaterialShininess[10]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[11]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[11]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[11]);
						glUniform1f(materialShininessUniform, MaterialShininess[11]);
					}
				}
				else if (x == 2)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[12]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[12]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[12]);
						glUniform1f(materialShininessUniform, MaterialShininess[12]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[13]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[13]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[13]);
						glUniform1f(materialShininessUniform, MaterialShininess[13]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[14]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[14]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[14]);
						glUniform1f(materialShininessUniform, MaterialShininess[14]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[15]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[15]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[15]);
						glUniform1f(materialShininessUniform, MaterialShininess[15]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[16]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[16]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[16]);
						glUniform1f(materialShininessUniform, MaterialShininess[16]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[17]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[17]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[17]);
						glUniform1f(materialShininessUniform, MaterialShininess[17]);
					}
				}
				else if (x == 3)
				{
					if (y == 5)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[18]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[18]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[18]);
						glUniform1f(materialShininessUniform, MaterialShininess[18]);
					}
					else if (y == 4)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[19]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[19]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[19]);
						glUniform1f(materialShininessUniform, MaterialShininess[19]);
					}
					else if (y == 3)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[20]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[20]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[20]);
						glUniform1f(materialShininessUniform, MaterialShininess[20]);
					}
					else if (y == 2)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[21]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[21]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[21]);
						glUniform1f(materialShininessUniform, MaterialShininess[21]);
					}
					else if (y == 1)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[22]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[22]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[22]);
						glUniform1f(materialShininessUniform, MaterialShininess[22]);
					}
					else if (y == 0)
					{
						glUniform3fv(kaUniform, 1, MaterialAmbient[23]);
						glUniform3fv(kdUniform, 1, MaterialDiffuse[23]);
						glUniform3fv(ksUniform, 1, MaterialSpecular[23]);
						glUniform1f(materialShininessUniform, MaterialShininess[23]);
					}
				}
			}
			else
			{
				glUniform1i(lKeyPressedUniform, 0);
			}

			glBindVertexArray(vao_sphere);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);

			glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

			glBindVertexArray(0);

			glUseProgram(0);
		}
	}

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];

    rotationAngleX = rotationAngleX + 0.5f;
	if (rotationAngleX >= 360.0f)
	{
		rotationAngleX = 0.0f;
	}

	rotationAngleY = rotationAngleY + 0.5f;
	if (rotationAngleY >= 360.0f)
	{
		rotationAngleY = 0.0f;
	}

	rotationAngleZ = rotationAngleZ + 0.5f;
	if (rotationAngleZ >= 360.0f)
	{
		rotationAngleZ = 0.0f;
	}

}

- (void)layoutSubviews
{
    GLint width = 0;
    GLint height = 0;

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

    gWidth = width;
    gHeight = height;
    
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);


    glViewport(0, 0, width, height);

    GLfloat fwidth = (GLfloat)width;
    GLfloat fheight = (GLfloat)height;

    perspectiveProjectionMatrix = vmath::perspective(45.0f, fwidth / fheight, 0.1f, 100.0f);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
         printf("Failed to create framebuffer object %x\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    [self drawView:nil];
}

- (void)startAnimation
{
    if(!isAnimating)
    {
        displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink setPreferredFramesPerSecond:animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

        isAnimating = YES;
    }
}

- (void)stopAnimation
{
    if(isAnimating)
    {
        [displayLink invalidate];
        displayLink = nil;

        isAnimating = NO;
    }
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    //code
}

- (void)onSingleTap:(UITapGestureRecognizer *)gr
{
    gbLight = true;
}

- (void)onDoubleTap:(UITapGestureRecognizer *)gr
{
    keyPress = keyPress + 1;
    if(keyPress > 3)
    {
        keyPress = 1;
    }
}


- (void)onSwipe:(UISwipeGestureRecognizer *)gr
{
    [self release];
    exit(0);
}

- (void)onLongPress:(UILongPressGestureRecognizer *)gr
{

}

- (void)dealloc
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

    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }

    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1, &colorRenderbuffer);
        colorRenderbuffer = 0;
    }

    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1, &defaultFramebuffer);
        defaultFramebuffer = 0;
    }

    if([EAGLContext currentContext] == eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
    }
    [eaglContext release];
    eaglContext = nil;

    [super dealloc];
}

- (void)setMaterialSphere
{
	//material 1 : Emerald
	MaterialAmbient[0][0] = 0.0215f;
    MaterialAmbient[0][1] = 0.1745f;
    MaterialAmbient[0][2] = 0.0215f;
    MaterialAmbient[0][3] = 1.0f;

    MaterialDiffuse[0][0] = 0.07568f;
    MaterialDiffuse[0][1] = 0.61424f;
    MaterialDiffuse[0][2] = 0.07568f;
    MaterialDiffuse[0][3] = 1.0f;

	MaterialSpecular[0][0] = 0.633f;
	MaterialSpecular[0][1] = 0.727811f;
    MaterialSpecular[0][2] = 0.633f;
    MaterialSpecular[0][3] = 1.0f;
    
    MaterialShininess[0] = 0.6f * 128.0f;

	//material 2: Jade
    MaterialAmbient[1][0] = 0.135f;
    MaterialAmbient[1][1] = 0.225f;
	MaterialAmbient[1][2] = 0.1575f;
	MaterialAmbient[1][3] = 1.0f;

	MaterialDiffuse[1][0] = 0.54f;
	MaterialDiffuse[1][1] = 0.89f;
	MaterialDiffuse[1][2] = 0.63f;
	MaterialDiffuse[1][3] = 1.0f;

	MaterialSpecular[1][0] = 0.316228f;
	MaterialSpecular[1][1] = 0.316228f;
	MaterialSpecular[1][2] = 0.316228f;
	MaterialSpecular[1][3] = 1.0f;

	MaterialShininess[1] = 0.1f * 128.0f;

	//material 3 : Obsidian
	MaterialAmbient[2][0] = 0.05375f;
	MaterialAmbient[2][1] = 0.05f;
	MaterialAmbient[2][2] = 0.06625f;
	MaterialAmbient[2][3] = 1.0f;

	MaterialDiffuse[2][0] = 0.18275f;
	MaterialDiffuse[2][1] = 0.17f;
	MaterialDiffuse[2][2] = 0.22525f;
	MaterialDiffuse[2][3] = 1.0f;

	MaterialSpecular[2][0] = 0.332741f;
	MaterialSpecular[2][1] = 0.328634f;
	MaterialSpecular[2][2] = 0.346435f;
	MaterialSpecular[2][3] = 1.0f;

	MaterialShininess[2] = 0.3f * 128.0f;


	//material 4 : Pearl
	MaterialAmbient[3][0] = 0.25f;
	MaterialAmbient[3][1] = 0.20725f;
	MaterialAmbient[3][2] = 0.20725f;
	MaterialAmbient[3][3] = 1.0f;

	MaterialDiffuse[3][0] = 1.0f;
	MaterialDiffuse[3][1] = 0.829f;
	MaterialDiffuse[3][2] = 0.829f;
	MaterialDiffuse[3][3] = 1.0f;

	MaterialSpecular[3][0] = 0.296648f;
	MaterialSpecular[3][1] = 0.296648f;
	MaterialSpecular[3][2] = 0.296648f;
	MaterialSpecular[3][3] = 1.0f;

	MaterialShininess[3] = 0.088f * 128.0f;

	//material 5 : Ruby

	MaterialAmbient[4][0] = 0.1745f;
	MaterialAmbient[4][1] = 0.01175f;
	MaterialAmbient[4][2] = 0.01175f;
	MaterialAmbient[4][3] = 1.0f;

	MaterialDiffuse[4][0] = 0.61424f;
	MaterialDiffuse[4][1] = 0.04136f;
	MaterialDiffuse[4][2] = 0.04136f;
	MaterialDiffuse[4][3] = 1.0f;

	MaterialSpecular[4][0] = 0.727811f;
	MaterialSpecular[4][1] = 0.626959f;
	MaterialSpecular[4][2] = 0.626959f;
	MaterialSpecular[4][3] = 1.0f;

	MaterialShininess[4] = 0.6f * 128.0f;

	//material 6 : Turquoise

	MaterialAmbient[5][0] = 0.1f;
	MaterialAmbient[5][1] = 0.18725f;
	MaterialAmbient[5][2] = 0.1745f;
	MaterialAmbient[5][3] = 1.0f;

	MaterialDiffuse[5][0] = 0.396f;
	MaterialDiffuse[5][1] = 0.74151f;
	MaterialDiffuse[5][2] = 0.69102f;
	MaterialDiffuse[5][3] = 1.0f;

	MaterialSpecular[5][0] = 0.297254f;
	MaterialSpecular[5][1] = 0.30829f;
	MaterialSpecular[5][2] = 0.306678f;
	MaterialSpecular[5][3] = 1.0f;

	MaterialShininess[5] = 0.1f * 128.0f;

	//---------------COLUMN 2 : METAL -----------------------------
	//material 1 : Brass
	MaterialAmbient[6][0] = 0.329412f;
	MaterialAmbient[6][1] = 0.223529f;
	MaterialAmbient[6][2] = 0.027451f;
	MaterialAmbient[6][3] = 1.0f;

	MaterialDiffuse[6][0] = 0.780392f;
	MaterialDiffuse[6][1] = 0.568627f;
	MaterialDiffuse[6][2] = 0.113725f;
	MaterialDiffuse[6][3] = 1.0f;

	MaterialSpecular[6][0] = 0.992157f;
	MaterialSpecular[6][1] = 0.941176f;
	MaterialSpecular[6][2] = 0.807843f;
	MaterialSpecular[6][3] = 1.0f;

	MaterialShininess[6] = 0.21794872f * 128.0f;

	//material  2 : Bronze
	MaterialAmbient[7][0] = 0.2125f;
	MaterialAmbient[7][1] = 0.1275f;
	MaterialAmbient[7][2] = 0.054f;
	MaterialAmbient[7][3] = 1.0f;

	MaterialDiffuse[7][0] = 0.714f;
	MaterialDiffuse[7][1] = 0.4284f;
	MaterialDiffuse[7][2] = 0.18144f;
	MaterialDiffuse[7][3] = 1.0f;

	MaterialSpecular[7][0] = 0.393548f;
	MaterialSpecular[7][1] = 0.271906f;
	MaterialSpecular[7][2] = 0.166721f;
	MaterialSpecular[7][3] = 1.0f;

	MaterialShininess[7] = 0.2f * 128.0f;

	//material 3 : Chrome
	MaterialAmbient[8][0] = 0.25f;
	MaterialAmbient[8][1] = 0.25f;
	MaterialAmbient[8][2] = 0.25f;
	MaterialAmbient[8][3] = 1.0f;

	MaterialDiffuse[8][0] = 0.4f;
	MaterialDiffuse[8][1] = 0.4f;
	MaterialDiffuse[8][2] = 0.4f;
	MaterialDiffuse[8][3] = 1.0f;

	MaterialSpecular[8][0] = 0.774597f;
	MaterialSpecular[8][1] = 0.774597f;
	MaterialSpecular[8][2] = 0.774597f;
	MaterialSpecular[8][3] = 1.0f;

	MaterialShininess[8] = 0.6f * 128.0f;

	//material 4 : Copper
	MaterialAmbient[9][0] = 0.19125f;
	MaterialAmbient[9][1] = 0.0735f;
	MaterialAmbient[9][2] = 0.0225f;
	MaterialAmbient[9][3] = 1.0f;

	MaterialDiffuse[9][0] = 0.7038f;
	MaterialDiffuse[9][1] = 0.27048f;
	MaterialDiffuse[9][2] = 0.0828f;
	MaterialDiffuse[9][3] = 1.0f;

	MaterialSpecular[9][0] = 0.256777f;
	MaterialSpecular[9][1] = 0.1376222f;
	MaterialSpecular[9][2] = 0.086014f;
	MaterialSpecular[9][3] = 1.0f;

	MaterialShininess[9] = 0.1f * 128.0f;

	//material 5 : Gold
	MaterialAmbient[10][0] = 0.24725f;
	MaterialAmbient[10][1] = 0.1995f;
	MaterialAmbient[10][2] = 0.0745f;
	MaterialAmbient[10][3] = 1.0f;

	MaterialDiffuse[10][0] = 0.75164f;
	MaterialDiffuse[10][1] = 0.60648f;
	MaterialDiffuse[10][2] = 0.22648f;
	MaterialDiffuse[10][3] = 1.0f;

	MaterialSpecular[10][0] = 0.628281f;
	MaterialSpecular[10][1] = 0.555802f;
	MaterialSpecular[10][2] = 0.366065f;
	MaterialSpecular[10][3] = 1.0f;

	MaterialShininess[10] = 0.4f * 128.0f;

	//material 6 : Silver
	MaterialAmbient[11][0] = 0.19225f;
	MaterialAmbient[11][1] = 0.19225f;
	MaterialAmbient[11][2] = 0.19225f;
	MaterialAmbient[11][3] = 1.0f;

	MaterialDiffuse[11][0] = 0.50754f;
	MaterialDiffuse[11][1] = 0.50754f;
	MaterialDiffuse[11][2] = 0.50754f;
	MaterialDiffuse[11][3] = 1.0f;

	MaterialSpecular[11][0] = 0.508273f;
	MaterialSpecular[11][1] = 0.508273f;
	MaterialSpecular[11][2] = 0.508273f;
	MaterialSpecular[11][3] = 1.0f;

	MaterialShininess[11] = 0.4f * 128.0f;

	//---------------COLUMN 3 : PLASTIC -----------------------------
	//material 1 : Black

	MaterialAmbient[12][0] = 0.0f;
	MaterialAmbient[12][1] = 0.0f;
	MaterialAmbient[12][2] = 0.0f;
	MaterialAmbient[12][3] = 1.0f;

	MaterialDiffuse[12][0] = 0.0f;
	MaterialDiffuse[12][1] = 0.0f;
	MaterialDiffuse[12][2] = 0.0f;
	MaterialDiffuse[12][3] = 1.0f;

	MaterialSpecular[12][0] = 0.50f;
	MaterialSpecular[12][1] = 0.50f;
	MaterialSpecular[12][2] = 0.50f;
	MaterialSpecular[12][3] = 1.0f;

	MaterialShininess[12] = 0.25f * 128.0f;

	//material  2 : Cyan
	MaterialAmbient[13][0] = 0.0f;
	MaterialAmbient[13][1] = 0.1f;
	MaterialAmbient[13][2] = 0.06f;
	MaterialAmbient[13][3] = 1.0f;

	MaterialDiffuse[13][0] = 0.0f;
	MaterialDiffuse[13][1] = 0.50980329f;
	MaterialDiffuse[13][2] = 0.50980329f;
	MaterialDiffuse[13][3] = 1.0f;

	MaterialSpecular[13][0] = 0.50196078f;
	MaterialSpecular[13][1] = 0.50196078f;
	MaterialSpecular[13][2] = 0.50196078f;
	MaterialSpecular[13][3] = 1.0f;

	MaterialShininess[13] = 0.25f * 128.0f;

	//material 3 : Green
	MaterialAmbient[14][0] = 0.0f;
	MaterialAmbient[14][1] = 0.0f;
	MaterialAmbient[14][2] = 0.0f;
	MaterialAmbient[14][3] = 1.0f;

	MaterialDiffuse[14][0] = 0.1f;
	MaterialDiffuse[14][1] = 0.35f;
	MaterialDiffuse[14][2] = 0.1f;
	MaterialDiffuse[14][3] = 1.0f;


	MaterialSpecular[14][0] = 0.45f;
	MaterialSpecular[14][1] = 0.55f;
	MaterialSpecular[14][2] = 0.45f;
	MaterialSpecular[14][3] = 1.0f;

	MaterialShininess[14] = 0.25f * 128.0f;

	//material 4 : red
	MaterialAmbient[15][0] = 0.0f;
	MaterialAmbient[15][1] = 0.0f;
	MaterialAmbient[15][2] = 0.0f;
	MaterialAmbient[15][3] = 1.0f;

	MaterialDiffuse[15][0] = 0.5f;
	MaterialDiffuse[15][1] = 0.0f;
	MaterialDiffuse[15][2] = 0.0f;
	MaterialDiffuse[15][3] = 1.0f;

	MaterialSpecular[15][0] = 0.7f;
	MaterialSpecular[15][1] = 0.6f;
	MaterialSpecular[15][2] = 0.6f;
	MaterialSpecular[15][3] = 1.0f;

	MaterialShininess[15] = 0.25f * 128.0f;

	//material 5 : white
	MaterialAmbient[16][0] = 0.0f;
	MaterialAmbient[16][1] = 0.0f;
	MaterialAmbient[16][2] = 0.0f;
	MaterialAmbient[16][3] = 1.0f;

	MaterialDiffuse[16][0] = 0.55f;
	MaterialDiffuse[16][1] = 0.55f;
	MaterialDiffuse[16][2] = 0.55f;
	MaterialDiffuse[16][3] = 1.0f;

	MaterialSpecular[16][0] = 0.70f;
	MaterialSpecular[16][1] = 0.70f;
	MaterialSpecular[16][2] = 0.70f;
	MaterialSpecular[16][3] = 1.0f;

	MaterialShininess[16] = 0.4f * 128.0f;

	//material 6 : yellow
	MaterialAmbient[17][0] = 0.0f;
	MaterialAmbient[17][1] = 0.0f;
	MaterialAmbient[17][2] = 0.0f;
	MaterialAmbient[17][3] = 1.0f;

	MaterialDiffuse[17][0] = 0.5f;
	MaterialDiffuse[17][1] = 0.5f;
	MaterialDiffuse[17][2] = 0.0f;
	MaterialDiffuse[17][3] = 1.0f;

	MaterialSpecular[17][0] = 0.60f;
	MaterialSpecular[17][1] = 0.60f;
	MaterialSpecular[17][2] = 0.50f;
	MaterialSpecular[17][3] = 1.0f;

	MaterialShininess[17] = 0.35f * 128.0f;


	//---------------COLUMN 4 : RUBBER -----------------------------
	//material 1 : Black
	MaterialAmbient[18][0] = 0.02f;
	MaterialAmbient[18][1] = 0.02f;
	MaterialAmbient[18][2] = 0.02f;
	MaterialAmbient[18][3] = 1.0f;

	MaterialDiffuse[18][0] = 0.01f;
	MaterialDiffuse[18][1] = 0.01f;
	MaterialDiffuse[18][2] = 0.01f;
	MaterialDiffuse[18][3] = 1.0f;


	MaterialSpecular[18][0] = 0.4f;
	MaterialSpecular[18][1] = 0.4f;
	MaterialSpecular[18][2] = 0.4f;
	MaterialSpecular[18][3] = 1.0f;

	MaterialShininess[18] = 0.078125f * 128.0f;

	//material  2 : Cyan
	MaterialAmbient[19][0] = 0.0f;
	MaterialAmbient[19][1] = 0.05f;
	MaterialAmbient[19][2] = 0.05f;
	MaterialAmbient[19][3] = 1.0f;

	MaterialDiffuse[19][0] = 0.4f;
	MaterialDiffuse[19][1] = 0.5f;
	MaterialDiffuse[19][2] = 0.5f;
	MaterialDiffuse[19][3] = 1.0f;

	MaterialSpecular[19][0] = 0.04f;
	MaterialSpecular[19][1] = 0.7f;
	MaterialSpecular[19][2] = 0.7f;
	MaterialSpecular[19][3] = 1.0f;

	MaterialShininess[19] = 0.078125f * 128.0f;

	//material 3 : Green
	MaterialAmbient[20][0] = 0.0f;
	MaterialAmbient[20][1] = 0.05f;
	MaterialAmbient[20][2] = 0.0f;
	MaterialAmbient[20][3] = 1.0f;

	MaterialDiffuse[20][0] = 0.4f;
	MaterialDiffuse[20][1] = 0.5f;
	MaterialDiffuse[20][2] = 0.4f;
	MaterialDiffuse[20][3] = 1.0f;

	MaterialSpecular[20][0] = 0.04f;
	MaterialSpecular[20][1] = 0.7f;
	MaterialSpecular[20][2] = 0.04f;
	MaterialSpecular[20][3] = 1.0f;

	MaterialShininess[20] = 0.078125f * 128.0f;

	//material 4 : red
	MaterialAmbient[21][0] = 0.05f;
	MaterialAmbient[21][1] = 0.0f;
	MaterialAmbient[21][2] = 0.0f;
	MaterialAmbient[21][3] = 1.0f;

	MaterialDiffuse[21][0] = 0.5f;
	MaterialDiffuse[21][1] = 0.4f;
	MaterialDiffuse[21][2] = 0.4f;
	MaterialDiffuse[21][3] = 1.0f;

	MaterialSpecular[21][0] = 0.7f;
	MaterialSpecular[21][1] = 0.04f;
	MaterialSpecular[21][2] = 0.04f;
	MaterialSpecular[21][3] = 1.0f;

	MaterialShininess[21] = 0.078125f * 128.0f;

	//material 5 : white
	MaterialAmbient[22][0] = 0.05f;
	MaterialAmbient[22][1] = 0.05f;
	MaterialAmbient[22][2] = 0.05f;
	MaterialAmbient[22][3] = 1.0f;

	MaterialDiffuse[22][0] = 0.5f;
	MaterialDiffuse[22][1] = 0.5f;
	MaterialDiffuse[22][2] = 0.5f;
	MaterialDiffuse[22][3] = 1.0f;

	MaterialSpecular[22][0] = 0.7f;
	MaterialSpecular[22][1] = 0.7f;
	MaterialSpecular[22][2] = 0.7f;
	MaterialSpecular[22][3] = 1.0f;

	MaterialShininess[22] = 0.078125f * 128.0f;

	//material 6 : yellow
	MaterialAmbient[23][0] = 0.05f;
	MaterialAmbient[23][1] = 0.05f;
	MaterialAmbient[23][2] = 0.0f;
	MaterialAmbient[23][3] = 1.0f;

	MaterialDiffuse[23][0] = 0.5f;
	MaterialDiffuse[23][1] = 0.5f;
	MaterialDiffuse[23][2] = 0.4f;
	MaterialDiffuse[23][3] = 1.0f;

	MaterialSpecular[23][0] = 0.7f;
	MaterialSpecular[23][1] = 0.7f;
	MaterialSpecular[23][2] = 0.04f;
	MaterialSpecular[23][3] = 1.0f;

	MaterialShininess[23] = 0.078125f * 128.0f;

}

-(void)setViewports:(int)x :(int)y :(int)w :(int)h
{

    glViewport(x, y, w, h);

    GLfloat fw = (GLfloat)w;
    GLfloat fh = (GLfloat)h;
    perspectiveProjectionMatrix = vmath::perspective(45.0f, fw / fh, 0.1f, 100.0f);
}

@end
