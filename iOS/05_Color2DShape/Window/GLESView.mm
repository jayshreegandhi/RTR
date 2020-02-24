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
    
    GLuint vao_triangle;
    GLuint vao_rectangle;
    
    GLuint vbo_position_triangle;
    GLuint vbo_position_rectangle;
    GLuint vbo_color_triangle;
    //GLuint vbo_color_rectangle;
    
    GLfloat angleTriangle;
    GLfloat angleRectangle;
    
    GLuint mvpUniform;
    

    
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
        "in vec4 vColor;" \
        "uniform mat4 u_mvp_matrix;" \
        "out vec4 out_color;" \
        "void main(void)" \
        "{" \
        "    gl_Position = u_mvp_matrix * vPosition;" \
        "    out_color = vColor;" \
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
        "in vec4 out_color;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "    FragColor = out_color;" \
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
                             AMC_ATTRIBUTE_COLOR,
                             "vColor");

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
        mvpUniform = glGetUniformLocation(shaderProgramObject,
                                          "u_mvp_matrix");
        
        //above is the preparation of data transfer from CPU to GPU
        //i.e glBindAttribLocation() & glGetUniformLocation()
        
        //array initialization (glBegin() and glEnd())
        const GLfloat traingleVertices[] = {
            0.0f,1.0f,0.0f,
            -1.0f,-1.0f,0.0f,
            1.0f,-1.0f,0.0f };
        
        const GLfloat rectangleVertices[] = {
            1.0f, 1.0f, 0.0f,
            -1.0f,1.0f,0.0f,
            -1.0f,-1.0f,0.0f,
            1.0f,-1.0f,0.0f };
        
        const GLfloat traingleColor[] = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f };
        
        /*const GLfloat rectangleColor[] = {
         0.0f, 0.0f, 1.0f,
         0.0f,0.0f,1.0f,
         0.0f,0.0f,1.0f,
         0.0f,0.0f,1.0f };
         */
        
        //create vao (vertex array object)
        glGenVertexArrays(1, &vao_triangle);
        
        //Bind vao
        glBindVertexArray(vao_triangle);
        //---------------------position---------------------------
        //generate vertex buffers
        glGenBuffers(1, &vbo_position_triangle);
        
        //bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_triangle);
        
        //transfer vertex data(CPU) to GPU buffer
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(traingleVertices),
                     traingleVertices,
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
        
        
        //--------------------------------color------------------------
        //generate vertex buffers
        glGenBuffers(1, &vbo_color_triangle);
        
        //bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color_triangle);
        
        //transfer vertex data(CPU) to GPU buffer
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(traingleColor),
                     traingleColor,
                     GL_STATIC_DRAW);
        
        //attach or map attribute pointer to vbo's buffer
        glVertexAttribPointer(AMC_ATTRIBUTE_COLOR,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              NULL);
        
        //enable vertex attribute array
        glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
        
        //unbind vbo
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        //unbind vao
        glBindVertexArray(0);
        
        //------------------------------rectangle-------------------------------
        //create vao rectangle(vertex array object)
        glGenVertexArrays(1, &vao_rectangle);
        
        //Bind vao
        glBindVertexArray(vao_rectangle);
        
        //generate vertex buffers
        glGenBuffers(1, &vbo_position_rectangle);
        
        //bind buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo_position_rectangle);
        
        //transfer vertex data(CPU) to GPU buffer
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(rectangleVertices),
                     rectangleVertices,
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
        
        /*
         //--------------------color------------------------
         //generate vertex buffers
         glGenBuffers(1, &vbo_color_rectangle);
         
         //bind buffer
         glBindBuffer(GL_ARRAY_BUFFER, vbo_color_rectangle);
         
         //transfer vertex data(CPU) to GPU buffer
         glBufferData(GL_ARRAY_BUFFER,
         sizeof(rectangleColor),
         rectangleColor,
         GL_STATIC_DRAW);
         
         //attach or map attribute pointer to vbo's buffer
         glVertexAttribPointer(AMC_ATTRIBUTE_COLOR,
         3,
         GL_FLOAT,
         GL_FALSE,
         0,
         NULL);
         
         //enable vertex attribute array
         glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
         */
        
        glVertexAttrib3f(AMC_ATTRIBUTE_COLOR,
                         0.0f,
                         0.0f,
                         1.0f);
        
        //unbind vbo
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        //unbind vao
        glBindVertexArray(0);

        
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
    vmath::mat4 modelViewProjectionMatrix;
    vmath::mat4 translationMatrix;
    vmath::mat4 rotationMatrix;
    
    //---------------------------------Triangle--------------------
    //make identity
    modelViewMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    rotationMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    
    translationMatrix = vmath::translate(-1.5f, 0.0f, -6.0f);
    rotationMatrix = vmath::rotate(angleTriangle, 0.0f, 1.0f, 0.0f);
    
    //do necessary matrix multiplication
    //this was internally done by gluOrtho() in ffp
    modelViewMatrix = translationMatrix * rotationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    
    
    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(mvpUniform,//which uniform?
                       1,//how many matrices
                       GL_FALSE,//have to transpose?
                       modelViewProjectionMatrix);//actual matrix
    
    //bind with vao
    glBindVertexArray(vao_triangle);
    
    //similarly bind with textures if any
    
    //now draw the necessary scene
    glDrawArrays(GL_TRIANGLES,
                 0,
                 3);
    
    //unbind vao
    glBindVertexArray(0);
    
    //-----------------------------Rectangle----------------------
    //make identity
    modelViewMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    rotationMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    
    //do necessary transformation
    translationMatrix = vmath::translate(1.5f, 0.0f, -6.0f);
    rotationMatrix = vmath::rotate(angleRectangle, 1.0f, 0.0f, 0.0f);
    
    //do necessary matrix multiplication
    //this was internally done by gluOrtho() in ffp
    modelViewMatrix = translationMatrix * rotationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    
    
    //send necessary matrices to shader in respective uniforms
    glUniformMatrix4fv(mvpUniform,//which uniform?
                       1,//how many matrices
                       GL_FALSE,//have to transpose?
                       modelViewProjectionMatrix);//actual matrix
    
    //bind with vao
    glBindVertexArray(vao_rectangle);
    
    //similarly bind with textures if any
    
    //now draw the necessary scene
    glDrawArrays(GL_TRIANGLE_FAN,
                 0,
                 4);
    
    //unbind vao
    glBindVertexArray(0);
    //unbinding program
    glUseProgram(0);

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
    
    angleTriangle = angleTriangle + 1.0f;
    if (angleTriangle >= 360.0f)
    {
        angleTriangle = 0.0f;
    }
    
    angleRectangle = angleRectangle - 1.0f;
    
    if (angleRectangle <= -360.0f)
    {
        angleRectangle = 0.0f;
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
    
    /*if (vbo_color_rectangle)
     {
     glDeleteBuffers(1, &vbo_color_rectangle);
     vbo_color_rectangle = 0;
     }*/
    
    if (vbo_position_rectangle)
    {
        glDeleteBuffers(1, &vbo_position_rectangle);
        vbo_position_rectangle = 0;
    }
    
    if (vao_rectangle)
    {
        glDeleteVertexArrays(1, &vao_rectangle);
        vao_rectangle = 0;
    }
    
    if (vbo_color_triangle)
    {
        glDeleteBuffers(1, &vbo_color_triangle);
        vbo_color_triangle = 0;
    }
    
    if (vbo_position_triangle)
    {
        glDeleteBuffers(1, &vbo_position_triangle);
        vbo_position_triangle = 0;
    }
    
    if (vao_triangle)
    {
        glDeleteVertexArrays(1, &vao_triangle);
        vao_triangle = 0;
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
