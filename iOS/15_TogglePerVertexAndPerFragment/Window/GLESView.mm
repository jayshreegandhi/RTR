//
//  GLESView.mm
//  Window - ortho
//
//  Created by sugat mankar on 21/02/20.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
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

    GLuint shaderProgramObject_perFrag;
	GLuint shaderProgramObject_perVert;

	GLuint vertexShaderObject_perFrag;
	GLuint vertexShaderObject_perVert;

	GLuint fragmentShaderObject_perFrag;
	GLuint fragmentShaderObject_perVert;

	GLuint vao_sphere;
	GLuint vbo_position_sphere;
	GLuint vbo_normal_sphere;
	GLuint vbo_element_sphere;

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

	bool gbLight;
	bool isFKeyPressed;
	bool isVKeyPressed;

	float sphere_vertices[1146];
	float sphere_normals[1146];
	float sphere_texture[746];
	short sphere_elements[2280];
	int gNumVertices;
	int gNumElements;

	float LightAmbient[4];
	float LightDiffuse[4];
	float LightSpecular[4];
	float LightPosition[4];

	float MaterialAmbient[4];
	float MaterialDiffuse[4];
	float MaterialSpecular[4];
	float MaterialShininess;

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

        LightPosition[0] = 100.0f;
        LightPosition[1] = 100.0f;
        LightPosition[2] = 100.0f;
        LightPosition[3] = 1.0f;

        MaterialAmbient[0] = 0.0f;
        MaterialAmbient[1] = 0.0f;
        MaterialAmbient[2] = 0.0f;
        MaterialAmbient[3] = 0.0f;

        MaterialDiffuse[0] = 1.0f;
        MaterialDiffuse[1] = 1.0f;
        MaterialDiffuse[2] = 1.0f;
        MaterialDiffuse[3] = 1.0f;

        MaterialSpecular[0] = 1.0f;
        MaterialSpecular[1] = 1.0f;
        MaterialSpecular[2] = 1.0f;
        MaterialSpecular[3] = 1.0f;

        MaterialShininess = 128.0f;

        //***************** 1. VERTEX SHADER ************************************
        //define vertex shader object
        //create vertex shader object
        vertexShaderObject_perFrag = glCreateShader(GL_VERTEX_SHADER);
        vertexShaderObject_perVert = glCreateShader(GL_VERTEX_SHADER);

        //Write vertex shader code
        const GLchar *vertexShaderSourceCode_perFrag =
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


        //Write vertex shader code
        const GLchar *vertexShaderSourceCode_perVert =
            "#version 300 es" \
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

                    printf("\nVertex Shader Compilation Log (Per Frag): %s\n", szInfoLog_perFrag);

                    //free the memory
                    free(szInfoLog_perFrag);


                    [self release];
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

                    printf("\nVertex Shader Compilation Log (Per Vertex): %s\n", szInfoLog_perVert);

                    //free the memory
                    free(szInfoLog_perVert);


                    [self release];


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

        const GLchar *fragmentShaderSourceCode_perVert =
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

                    printf("\nFragment Shader Compilation Log (Per Frag): %s\n", szInfoLog_perFrag);

                    //free the memory
                    free(szInfoLog_perFrag);


                    [self release];


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

                    printf("\nFragment Shader Compilation Log (Per Vert): %s\n", szInfoLog_perFrag);

                    //free the memory
                    free(szInfoLog_perVert);

                    [self release];


                }
            }
        }

        //create shader program object
        shaderProgramObject_perFrag = glCreateProgram();

        shaderProgramObject_perVert = glCreateProgram();

        //Attach vertex shader to shader program
        glAttachShader(shaderProgramObject_perFrag,//to whom?
            vertexShaderObject_perFrag);//what to attach?

        //Attach fragment shader to shader program
        glAttachShader(shaderProgramObject_perFrag,
            fragmentShaderObject_perFrag);

        glAttachShader(shaderProgramObject_perVert,//to whom?
            vertexShaderObject_perVert);//what to attach?

        //Attach fragment shader to shader program
        glAttachShader(shaderProgramObject_perVert,
            fragmentShaderObject_perVert);

        //Pre-Linking binding to vertex attribute
        glBindAttribLocation(shaderProgramObject_perFrag,
            AMC_ATTRIBUTE_POSITION,
            "vPosition");

        glBindAttribLocation(shaderProgramObject_perFrag,
            AMC_ATTRIBUTE_NORMAL,
            "vNormal");

        //Pre-Linking binding to vertex attribute
        glBindAttribLocation(shaderProgramObject_perVert,
            AMC_ATTRIBUTE_POSITION,
            "vPosition");

        glBindAttribLocation(shaderProgramObject_perVert,
            AMC_ATTRIBUTE_NORMAL,
            "vNormal");

        //Link the shader program
        glLinkProgram(shaderProgramObject_perFrag);//link to whom?

        glLinkProgram(shaderProgramObject_perVert);//link to whom?

        //Error checking for linking
        GLint iProgramLinkStatus_perFrag = 0;
        iInfoLogLength_perFrag = 0;
        szInfoLog_perFrag = NULL;

        //Step 1 : Call glGetShaderiv() to get comiple status of particular shader
        glGetProgramiv(shaderProgramObject_perFrag, // whose?
            GL_LINK_STATUS,//what to get?
            &iProgramLinkStatus_perFrag);//in what?

        //Step 2 : Check shader compile status for GL_FALSE
        if (iProgramLinkStatus_perFrag == GL_FALSE)
        {
            //Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
            glGetProgramiv(shaderProgramObject_perFrag,
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

                    glGetProgramInfoLog(shaderProgramObject_perFrag,//whose?
                        iInfoLogLength_perFrag,//length?
                        &written,//might have not used all, give that much only which have been used in what?
                        szInfoLog_perFrag);//store in what?

                    printf("\nShader Program Linking Log (Per Frag): %s\n", szInfoLog_perFrag);

                    //free the memory
                    free(szInfoLog_perFrag);


                    [self release];


                }
            }
        }

        //Error checking for linking
        GLint iProgramLinkStatus_perVert = 0;
        iInfoLogLength_perVert = 0;
        szInfoLog_perVert = NULL;

        //Step 1 : Call glGetShaderiv() to get comiple status of particular shader
        glGetProgramiv(shaderProgramObject_perVert, // whose?
            GL_LINK_STATUS,//what to get?
            &iProgramLinkStatus_perVert);//in what?

        //Step 2 : Check shader compile status for GL_FALSE
        if (iProgramLinkStatus_perVert == GL_FALSE)
        {
            //Step 3 : If GL_FALSE , call glGetShaderiv() again , but this time to get info log length
            glGetProgramiv(shaderProgramObject_perVert,
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

                    glGetProgramInfoLog(shaderProgramObject_perVert,//whose?
                        iInfoLogLength_perVert,//length?
                        &written,//might have not used all, give that much only which have been used in what?
                        szInfoLog_perVert);//store in what?

                    printf("\nShader Program Linking Log (Per Vert) : %s\n", szInfoLog_perFrag);

                    //free the memory
                    free(szInfoLog_perVert);

                    [self release];
                }
            }
        }

        //Post-Linking reteriving uniform location
        viewUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_view_matrix");

        modelUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_model_matrix");

        projectionUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_projection_matrix");

        lKeyPressedUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_lKeyPressed");

        laUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_la");

        ldUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ld");

        lsUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ls");

        lightPositionUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_light_position");

        kaUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ka");

        kdUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_kd");

        ksUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ks");

        materialShininessUniform_pf = glGetUniformLocation(shaderProgramObject_perFrag,
            "u_material_shininess");

        //Post-Linking reteriving uniform location
        viewUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_view_matrix");

        modelUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_model_matrix");

        projectionUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_projection_matrix");

        lKeyPressedUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_lKeyPressed");

        laUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_la");

        ldUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_ld");

        lsUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_ls");

        lightPositionUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_light_position");

        kaUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_ka");

        kdUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_kd");

        ksUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
            "u_ks");

        materialShininessUniform_pv = glGetUniformLocation(shaderProgramObject_perVert,
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
        isFKeyPressed = false;
        isVKeyPressed = false;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

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

   if (isFKeyPressed == true)
	{
		glUseProgram(shaderProgramObject_perFrag);

		//matrices
		vmath::mat4 viewMatrix;
		vmath::mat4 modelMatrix;
		vmath::mat4 projectionMatrix;
		vmath::mat4 translationMatrix;

		//make identity
		viewMatrix = vmath::mat4::identity();;
		modelMatrix = vmath::mat4::identity();;
		projectionMatrix = vmath::mat4::identity();
		translationMatrix = vmath::mat4::identity();

		//do necessary transformation
		translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
		//do necessary matrix multiplication
		//this was internally done by gluOrtho() in ffp
		modelMatrix = translationMatrix;
		projectionMatrix = perspectiveProjectionMatrix;

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
			projectionMatrix);//actual matrix

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
		glUseProgram(shaderProgramObject_perVert);

		//matrices
		vmath::mat4 viewMatrix;
		vmath::mat4 modelMatrix;
		vmath::mat4 projectionMatrix;
		vmath::mat4 translationMatrix;

		//make identity
		viewMatrix = vmath::mat4::identity();;
		modelMatrix = vmath::mat4::identity();;
		projectionMatrix = vmath::mat4::identity();
		translationMatrix = vmath::mat4::identity();

		//do necessary transformation
		translationMatrix = vmath::translate(0.0f, 0.0f, -2.0f);
		//do necessary matrix multiplication
		//this was internally done by gluOrtho() in ffp
		modelMatrix = translationMatrix;
		projectionMatrix = perspectiveProjectionMatrix;

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
			projectionMatrix);//actual matrix

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

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];

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
   isFKeyPressed = true;
   isVKeyPressed = false;
}


- (void)onSwipe:(UISwipeGestureRecognizer *)gr
{
    [self release];
    exit(0);
}

- (void)onLongPress:(UILongPressGestureRecognizer *)gr
{
  isFKeyPressed = false;
  isVKeyPressed = true;
}

- (void)dealloc
{
    [super dealloc];

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

    if (shaderProgramObject_perVert)
    {
        GLsizei shaderCount;
        GLsizei shaderNumber;

        glUseProgram(shaderProgramObject_perVert);

        //ask the program how many shaders are attached to you?
        glGetProgramiv(shaderProgramObject_perVert,
                       GL_ATTACHED_SHADERS,
                       &shaderCount);

        GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

        if (pShaders)
        {

            //get shaders
            glGetAttachedShaders(shaderProgramObject_perVert,
                                 shaderCount,
                                 &shaderCount,
                                 pShaders);

            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                //detach
                glDetachShader(shaderProgramObject_perVert,
                               pShaders[shaderNumber]);

                //delete
                glDeleteShader(pShaders[shaderNumber]);

                //explicit 0
                pShaders[shaderNumber] = 0;
            }

            free(pShaders);
        }

        glDeleteProgram(shaderProgramObject_perVert);
        shaderProgramObject_perVert = 0;

        glUseProgram(0);
    }
    
    
    if (shaderProgramObject_perFrag)
    {
        GLsizei shaderCount;
        GLsizei shaderNumber;

        glUseProgram(shaderProgramObject_perFrag);

        //ask the program how many shaders are attached to you?
        glGetProgramiv(shaderProgramObject_perFrag,
                       GL_ATTACHED_SHADERS,
                       &shaderCount);

        GLuint *pShaders = (GLuint *)malloc(sizeof(GLuint) * shaderCount);

        if (pShaders)
        {

            //get shaders
            glGetAttachedShaders(shaderProgramObject_perFrag,
                                 shaderCount,
                                 &shaderCount,
                                 pShaders);

            for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
            {
                //detach
                glDetachShader(shaderProgramObject_perFrag,
                               pShaders[shaderNumber]);

                //delete
                glDeleteShader(pShaders[shaderNumber]);

                //explicit 0
                pShaders[shaderNumber] = 0;
            }

            free(pShaders);
        }

        glDeleteProgram(shaderProgramObject_perFrag);
        shaderProgramObject_perFrag = 0;

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
