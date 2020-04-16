//
//  GLESView.mm
//  Window - ortho
//
//  Created by sugat mankar on 21/02/20.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "vmath.h"

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

	GLuint vao_pyramid;

	GLuint vbo_position_pyramid;
	GLuint vbo_normal_pyramid;

	GLfloat anglePyramid;

	GLuint viewUniform;
	GLuint modelUniform;
	GLuint projectionUniform;

	GLuint laUniformZero;
	GLuint ldUniformZero;
	GLuint lsUniformZero;
	GLuint lightPositionUniformZero;

	GLuint laUniformOne;
	GLuint ldUniformOne;
	GLuint lsUniformOne;
	GLuint lightPositionUniformOne;

	GLuint kaUniform;
	GLuint kdUniform;
	GLuint ksUniform;

	GLuint materialShininessUniform;

	GLuint lKeyPressedUniform;

	GLfloat LightAmbientZero[4];
	GLfloat LightDiffuseZero[4];
	GLfloat LightSpecularZero[4];
	GLfloat LightPositionZero[4];

	GLfloat LightAmbientOne[4];
	GLfloat LightDiffuseOne[4];
	GLfloat LightSpecularOne[4];
	GLfloat LightPositionOne[4];

	GLfloat MaterialAmbient[4];
	GLfloat MaterialDiffuse[4];
	GLfloat MaterialSpecular[4];
	GLfloat MaterialShininess;

	bool gbLight;

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

       	LightAmbientZero[0] = 0.0f;
		LightAmbientZero[1] = 0.0f;
		LightAmbientZero[2] = 0.0f;
		LightAmbientZero[3] = 1.0f;

		LightDiffuseZero[0] = 1.0f;
		LightDiffuseZero[1] = 0.0f;
		LightDiffuseZero[2] = 0.0f;
		LightDiffuseZero[3] = 1.0f;

		LightSpecularZero[0] = 1.0f;
		LightSpecularZero[1] = 0.0f;
		LightSpecularZero[2] = 0.0f;
		LightSpecularZero[3] = 1.0f;

		LightPositionZero[0] = -2.0f;
		LightPositionZero[1] = 0.0f;
		LightPositionZero[2] = 0.0f;
		LightPositionZero[3] = 1.0f;

    	LightAmbientOne[0] = 0.0f;
		LightAmbientOne[1] = 0.0f;
		LightAmbientOne[2] = 0.0f;
		LightAmbientOne[3] = 1.0f;

		LightDiffuseOne[0] = 0.0f;
		LightDiffuseOne[1] = 0.0f;
		LightDiffuseOne[2] = 1.0f;
		LightDiffuseOne[3] = 1.0f;

		LightSpecularOne[0] = 0.0f;
		LightSpecularOne[1] = 0.0f;
		LightSpecularOne[2] = 1.0f;
		LightSpecularOne[3] = 1.0f;

		LightPositionOne[0] = 2.0f;
		LightPositionOne[1] = 0.0f;
		LightPositionOne[2] = 0.0f;
		LightPositionOne[3] = 1.0f;

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
        "#version 300 es" \
        "\n" \
       	"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_lKeyPressed;" \
		"uniform vec3 u_la_zero;" \
		"uniform vec3 u_ld_zero;" \
		"uniform vec3 u_ls_zero;" \
		"uniform vec4 u_light_position_zero;" \
		"uniform vec3 u_la_one;" \
		"uniform vec3 u_ld_one;" \
		"uniform vec3 u_ls_one;" \
		"uniform vec4 u_light_position_one;" \
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
		"vec3 light_direction_zero = normalize(vec3(u_light_position_zero - eyeCoords));" \
		"vec3 light_direction_one = normalize(vec3(u_light_position_one - eyeCoords));" \
		"float tNorm_Dot_LightDirection_zero = max(dot(light_direction_zero, tNormal), 0.0);" \
		"float tNorm_Dot_LightDirection_one = max(dot(light_direction_one, tNormal), 0.0);" \
		"vec3 reflection_vector_zero = reflect(-light_direction_zero, tNormal);" \
		"vec3 reflection_vector_one = reflect(-light_direction_one, tNormal);" \
		"vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));" \
		"vec3 ambient_zero = u_la_zero * u_ka;" \
		"vec3 diffuse_zero = u_ld_zero * u_kd * tNorm_Dot_LightDirection_zero;" \
		"vec3 specular_zero = u_ls_zero * u_ks * pow(max(dot(reflection_vector_zero,viewer_vector), 0.0), u_material_shininess);" \
		"vec3 ambient_one = u_la_one * u_ka;" \
		"vec3 diffuse_one = u_ld_one * u_kd * tNorm_Dot_LightDirection_one;" \
		"vec3 specular_one = u_ls_one * u_ks * pow(max(dot(reflection_vector_one,viewer_vector), 0.0), u_material_shininess);" \
		"phong_ads_light = ambient_zero + ambient_one + diffuse_zero + diffuse_one + specular_zero + specular_one;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light = vec3(1.0, 1.0, 1.0);" \
		"}" \
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
        "uniform int u_lKeyPressed;" \
		"in vec3 phong_ads_light;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{" \
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

        laUniformZero = glGetUniformLocation(shaderProgramObject,
            "u_la_zero");

        ldUniformZero = glGetUniformLocation(shaderProgramObject,
            "u_ld_zero");

        lsUniformZero = glGetUniformLocation(shaderProgramObject,
            "u_ls_zero");

        lightPositionUniformZero = glGetUniformLocation(shaderProgramObject,
            "u_light_position_zero");

        kaUniform = glGetUniformLocation(shaderProgramObject,
            "u_ka");

        kdUniform = glGetUniformLocation(shaderProgramObject,
            "u_kd");

        ksUniform = glGetUniformLocation(shaderProgramObject,
            "u_ks");

        laUniformOne = glGetUniformLocation(shaderProgramObject,
            "u_la_one");

        ldUniformOne = glGetUniformLocation(shaderProgramObject,
            "u_ld_one");

        lsUniformOne = glGetUniformLocation(shaderProgramObject,
            "u_ls_one");

        lightPositionUniformOne = glGetUniformLocation(shaderProgramObject,
            "u_light_position_one");

        materialShininessUniform = glGetUniformLocation(shaderProgramObject,
            "u_material_shininess");

        //above is the preparation of data transfer from CPU to GPU
        //i.e glBindAttribLocation() & glGetUniformLocation()

        //array initialization (glBegin() and glEnd())

        const GLfloat pyramidVertices[] = {
            -1.0f, -1.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, -1.0f};

        const GLfloat pyramidNormals[] = {
            0.0f, 0.447214f, 0.894427f ,
            0.0f, 0.447214f, 0.894427f ,
            0.0f, 0.447214f, 0.894427f ,
            0.894427f, 0.447214f, 0.0f ,
            0.894427f, 0.447214f, 0.0f ,
            0.894427f, 0.447214f, 0.0f ,
            0.0f, 0.447214f, -0.894427f ,
            0.0f, 0.447214f, -0.894427f ,
            0.0f, 0.447214f, -0.894427f ,
            -0.894427f, 0.447214f, 0.0f ,
            -0.894427f, 0.447214f, 0.0f ,
            -0.894427f, 0.447214f, 0.0f};


        //------------------------------cube-------------------------------
        //create vao rectangle(vertex array object)
        glGenVertexArrays(1, &vao_pyramid);

        //Bind vao
        glBindVertexArray(vao_pyramid);

        //generate vertex buffers
        glGenBuffers(1, &vbo_position_pyramid);

        //bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_pyramid);

        //transfer vertex data(CPU) to GPU buffer
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(pyramidVertices),
            pyramidVertices,
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
        glGenBuffers(1, &vbo_normal_pyramid);

        //bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_pyramid);

        //transfer vertex data(CPU) to GPU buffer
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(pyramidNormals),
            pyramidNormals,
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

        //unbind vao
        glBindVertexArray(0);

        gbLight = false;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        //glEnable(GL_CULL_FACE);

        //set bk color
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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

    //Binding Opengl code to shader program object
 	glUseProgram(shaderProgramObject);

	//matrices
	vmath::mat4 modelMatrix;
	vmath::mat4 viewMatrix;
	vmath::mat4 projectionMatrix;
	vmath::mat4 translationMatrix;
	vmath::mat4 rotationMatrix;

	//-----------------------------cube----------------------
	//make identity
	modelMatrix = vmath::mat4::identity();
	viewMatrix = vmath::mat4::identity();
	projectionMatrix = vmath::mat4::identity();
	rotationMatrix = vmath::mat4::identity();
	translationMatrix = vmath::mat4::identity();

	//do necessary transformation
	translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
	rotationMatrix = vmath::rotate(0.0f, anglePyramid, 0.0f);

	//do necessary matrix multiplication
	//this was internally done by gluOrtho() in ffp
	modelMatrix = translationMatrix * rotationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;


	//send necessary matrices to shader in respective uniforms
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
		glUniform3fv(laUniformZero, 1, LightAmbientZero);
		glUniform3fv(ldUniformZero, 1, LightDiffuseZero);
		glUniform3fv(lsUniformZero, 1, LightSpecularZero);
		glUniform3fv(kaUniform, 1, MaterialAmbient);
		glUniform3fv(kdUniform, 1, MaterialDiffuse);
		glUniform3fv(ksUniform, 1, MaterialSpecular);
		glUniform1f(materialShininessUniform, MaterialShininess);
		glUniform4fv(lightPositionUniformZero, 1, LightPositionZero);

		glUniform3fv(laUniformOne, 1, LightAmbientOne);
		glUniform3fv(ldUniformOne, 1, LightDiffuseOne);
		glUniform3fv(lsUniformOne, 1, LightSpecularOne);
		glUniform3fv(kaUniform, 1, MaterialAmbient);
		glUniform3fv(kdUniform, 1, MaterialDiffuse);
		glUniform3fv(ksUniform, 1, MaterialSpecular);
		glUniform1f(materialShininessUniform, MaterialShininess);
		glUniform4fv(lightPositionUniformOne, 1, LightPositionOne);
	}
	else
	{
		glUniform1i(lKeyPressedUniform, 0);
	}

	//bind with vao
	glBindVertexArray(vao_pyramid);

	//now draw the necessary scene
	glDrawArrays(GL_TRIANGLES,
		0,
		12);

	//unbind vao
	glBindVertexArray(0);
	//unbinding program
	glUseProgram(0);

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];

    anglePyramid = anglePyramid + 1.0f;
    if (anglePyramid >= 360.0f)
    {
        anglePyramid = 0.0f;
    }

}

- (void)layoutSubviews
{
    GLint width;
    GLint height;

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

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
    [super dealloc];

     if (vbo_position_pyramid)
	{
		glDeleteBuffers(1, &vbo_position_pyramid);
		vbo_position_pyramid = 0;
	}

	if (vbo_normal_pyramid)
	{
		glDeleteBuffers(1, &vbo_normal_pyramid);
		vbo_normal_pyramid = 0;
	}

	if (vao_pyramid)
	{
		glDeleteVertexArrays(1, &vao_pyramid);
		vao_pyramid = 0;
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

}

@end
