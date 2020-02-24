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
    
    GLuint shaderProgramObject;
    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;
    
    GLuint vao_sphere;
    GLuint vbo_position_sphere;
    GLuint vbo_normal_sphere;
    GLuint vbo_element_sphere;
    
    GLuint mvUniform;
    GLuint pUniform;
    GLuint ldUniform;
    GLuint kdUniform;
    GLuint lightPositionUniform;
    GLuint lKeyPressedUniform;
    
    bool gbLight;
    
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
        "uniform mat4 u_mv_matrix;" \
        "uniform mat4 u_p_matrix;" \
        "uniform mediump int u_lKeyPressed;" \
        "uniform vec3 u_ld;" \
        "uniform vec3 u_kd;" \
        "uniform vec4 u_light_position;" \
        "out vec3 diffuse_color;" \
        "void main(void)" \
        "{" \
        "    if(u_lKeyPressed == 1)" \
        "    {" \
        "        vec4 eyeCoords = u_mv_matrix * vPosition;" \
        "        mat3 normal_matrix = mat3(transpose(inverse(u_mv_matrix)));" \
        "        vec3 tNormal = normalize(normal_matrix * vNormal);" \
        "        vec3 s = normalize(vec3(u_light_position - eyeCoords));" \
        "        diffuse_color = u_ld * u_kd * max(dot(s, tNormal), 0.0);" \
        "    }" \
        "    gl_Position = u_p_matrix * u_mv_matrix * vPosition;" \
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
        "in vec3 diffuse_color;" \
        "out vec4 fragColor;" \
        "void main(void)" \
        "{" \
        "    if(u_lKeyPressed == 1)" \
        "    {" \
        "        fragColor = vec4(diffuse_color,1.0);" \
        "    }" \
        "    else" \
        "    {" \
        "        fragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
        "    }" \
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
        mvUniform = glGetUniformLocation(shaderProgramObject,
                                         "u_mv_matrix");
        
        pUniform = glGetUniformLocation(shaderProgramObject,
                                        "u_p_matrix");
        
        lKeyPressedUniform = glGetUniformLocation(shaderProgramObject,
                                        "u_lKeyPressed");
        
        ldUniform = glGetUniformLocation(shaderProgramObject,
                                         "u_ld");
        
        kdUniform = glGetUniformLocation(shaderProgramObject,
                                         "u_kd");
        
        lightPositionUniform = glGetUniformLocation(shaderProgramObject,
                                        "u_light_position");
        
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
    vmath::mat4 modelViewMatrix;
    vmath::mat4 projectionMatrix;
    vmath::mat4 translationMatrix;
    
    //make identity
    modelViewMatrix = vmath::mat4::identity();
    projectionMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    
    //do necessary transformation
    translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
    
    //do necessary matrix multiplication
    //this was internally done by gluOrtho() in ffp
    modelViewMatrix = translationMatrix;
    projectionMatrix = perspectiveProjectionMatrix;
    
    
    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(mvUniform,//which uniform?
                       1,//how many matrices
                       GL_FALSE,//have to transpose?
                       modelViewMatrix);//actual matrix
    
    glUniformMatrix4fv(pUniform,//which uniform?
                       1,//how many matrices
                       GL_FALSE,//have to transpose?
                       projectionMatrix);//actual matrix
    
    if (gbLight == true)
    {
        glUniform1i(lKeyPressedUniform, 1);
        glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
        glUniform4f(lightPositionUniform, 10.0f, 10.0f, 10.0f, 1.0f);
    }
    else
    {
        glUniform1i(lKeyPressedUniform, 0);
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
    
}

@end
