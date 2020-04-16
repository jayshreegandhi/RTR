package com.rtr.tessellation;

//by user
import android.content.Context;
import android.view.Gravity;
import android.graphics.Color;

import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

//for opengl
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

//opengl buffers
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

//matrix math
import android.opengl.Matrix;


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer,OnGestureListener,OnDoubleTapListener{

    private final Context context;
    private GestureDetector gestureDetector;

    private int shaderProgramObject;
    private int vertexShaderObject;
    private int tessellationControlShaderObject;
    private int tessellationEvaluationShaderObject;
    private int fragmentShaderObject;

    private int[] vao = new int[1];
    private int[] vbo = new int[1];

    private int mvpUniform;
    private int gNumberOfSegmentsUniform;
    private int gNumberOfStripsUniform;
    private int gLineColorUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private int gNumberOfLineSegments;

    public GLESView(Context drawingContext){
        super(drawingContext);
        context = drawingContext;

        setEGLContextClientVersion(3);
        setRenderer(this);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(drawingContext,this,null,false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    //Renderer's method
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        String version = gl.glGetString(GL10.GL_VERSION);
        String glslVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);
        //String vendor = gl.glGetString(GLES32.vendor);
        //String renderer = gl.glGetString(GLES32.renderer);

        System.out.println("RTR: " + version);
        System.out.println("RTR: " + glslVersion);
        //System.out.println("RTR: " + vendor);
        //System.out.println("RTR: " + renderer);

        initialize();
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height)
    {
        resize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 unused)
    {
        display();
    }

    //our callbacks/ custom methods

    private void initialize()
    {

        //vertex shader
        vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        final String vertexShaderSourceCode = String.format(
            "#version 320 es" +
            "\n" +
            "in vec2 vPosition;" +
            "void main(void)" +
            "{" +
            "   gl_Position = vec4(vPosition, 0.0, 1.0);" +
            "}");

        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);

        GLES32.glCompileShader(vertexShaderObject);

        //compilation error checking

        int[] iShaderCompileStatus = new int[1];
        int[] iInfoLogLength = new int[1];
        String szInfoLog = null;

        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus,0);
        if(iShaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("RTR: Vertex Shader Compilation log: " + szInfoLog);

                uninitialize();
                System.exit(0);
            }
        }

        //TC shader
        tessellationControlShaderObject = GLES32.glCreateShader(GLES32.GL_TESS_CONTROL_SHADER);

        final String tessellationControlShaderSourceCode = String.format(
            "#version 320 es" +
            "\n" +
            "precision mediump int;" +
            "precision highp float;" +
            "layout(vertices=4)out;" +
            "uniform int u_numberOfSegments;" +
            "uniform int u_numberOfStrips;" +
            "void main(void)" +
            "{" +
            "	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;" +
            "	gl_TessLevelOuter[0] = float(u_numberOfStrips);" +
            "	gl_TessLevelOuter[1] = float(u_numberOfSegments);" +
            "}");

        GLES32.glShaderSource(tessellationControlShaderObject, tessellationControlShaderSourceCode);

        GLES32.glCompileShader(tessellationControlShaderObject);

        //compilation error checking

        iShaderCompileStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(tessellationControlShaderObject, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus,0);
        if(iShaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(tessellationControlShaderObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(tessellationControlShaderObject);
                System.out.println("RTR: TC Shader Compilation log: " + szInfoLog);

                uninitialize();
                System.exit(0);
            }
        }

        //TC shader
        tessellationEvaluationShaderObject = GLES32.glCreateShader(GLES32.GL_TESS_EVALUATION_SHADER);

        final String tessellationEvaluationShaderSourceCode = String.format(
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "layout(isolines)in;" +
            "uniform mat4 u_mvp_matrix;" +
            "void main(void)" +
            "{" +
            "	float u = gl_TessCoord.x;" +
            "	vec3 p0 = gl_in[0].gl_Position.xyz;" +
            "	vec3 p1 = gl_in[1].gl_Position.xyz;" +
            "	vec3 p2 = gl_in[2].gl_Position.xyz;" +
            "	vec3 p3 = gl_in[3].gl_Position.xyz;" +
            "	float u1 = (1.0 - u);" +
            "	float u2 = u * u;" +
            "	float b3 = u2 * u;" +
            "	float b2 = 3.0 * u2 * u1;" +
            "	float b1 = 3.0 * u * u1 * u1;" +
            "	float b0 = u1 * u1 * u1;" +
            "	vec3 p = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;" +
            "	gl_Position = u_mvp_matrix * vec4(p, 1.0);" +
            "}");

        GLES32.glShaderSource(tessellationEvaluationShaderObject, tessellationEvaluationShaderSourceCode);

        GLES32.glCompileShader(tessellationEvaluationShaderObject);

        //compilation error checking

        iShaderCompileStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(tessellationEvaluationShaderObject, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus,0);
        if(iShaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(tessellationEvaluationShaderObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(tessellationEvaluationShaderObject);
                System.out.println("RTR: TE Shader Compilation log: " + szInfoLog);

                uninitialize();
                System.exit(0);
            }
        }

        //fragment shader
        fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        final String fragmentShaderSourceCode = String.format(
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "uniform vec4 u_lineColor;" +
            "out vec4 fragColor;" +
            "void main(void)" +
            "{" +
            "fragColor = vec4(u_lineColor);" +
            "}");

        GLES32.glShaderSource(fragmentShaderObject, fragmentShaderSourceCode);

        GLES32.glCompileShader(fragmentShaderObject);

        //compilation error checking

        iShaderCompileStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus,0);
        if(iShaderCompileStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject);
                System.out.println("RTR: Fragment Shader Compilation log: " + szInfoLog);

                uninitialize();
                System.exit(0);
            }
        }

        //Shader program
        shaderProgramObject = GLES32.glCreateProgram();

        GLES32.glAttachShader(shaderProgramObject, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject, tessellationControlShaderObject);
        GLES32.glAttachShader(shaderProgramObject, tessellationEvaluationShaderObject);
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

        //prelinking binding to attributes
        GLES32.glBindAttribLocation(shaderProgramObject,
                        GLESMacros.AMC_ATTRIBUTE_POSITION,
                        "vPosition");


        GLES32.glLinkProgram(shaderProgramObject);
        //compilation error checking

        int[] iShaderLinkStatus = new int[1];
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS,iShaderLinkStatus,0);
        if(iShaderLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(shaderProgramObject);
                System.out.println("RTR: Shader linking log: " + szInfoLog);

                uninitialize();
                System.exit(0);
            }
        }

        //get uniform location
        mvpUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_mvp_matrix");
        gNumberOfSegmentsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_numberOfSegments");
        gNumberOfStripsUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_numberOfStrips");
        gLineColorUniform = GLES32.glGetUniformLocation(shaderProgramObject, "u_lineColor");

        final float vertices[] = new float[]{
			-1.0f,-1.0f,
            -0.5f,1.0f,
            0.5f,-1.0f,
            1.0f,-1.0f };

        //create vao and bind vao
        GLES32.glGenVertexArrays(1, vao, 0);

        GLES32.glBindVertexArray(vao[0]);

        //create and bind vbo
        GLES32.glGenBuffers(1, vbo, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo[0]);

        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(8 * 4);
        byteBuffer.order(ByteOrder.nativeOrder());

        FloatBuffer positionBuffer = byteBuffer.asFloatBuffer();
        positionBuffer.put(vertices);
        positionBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        8 * 4,
                        positionBuffer,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                        2,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glBindVertexArray(0);

        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        GLES32.glLineWidth(5.0f);

	    GLES32.glEnable(GLES32.GL_DEPTH_TEST);
	    GLES32.glDepthFunc(GLES32.GL_LEQUAL);

        gNumberOfLineSegments = 1;

        Matrix.setIdentityM(perspectiveProjectionMatrix,0);

    }


    private void resize(int width, int height)
    {
        if(height < 0)
        {
            height = 1;
        }

        GLES32.glViewport(0,0,width,height);
        Matrix.perspectiveM(perspectiveProjectionMatrix,
                0,
                45.0f,
                width / height,
                0.1f,
                100.0f);
    }

    private void display()
    {
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        GLES32.glUseProgram(shaderProgramObject);

        float[] modelViewMatrix = new float[16];
        float[] modelViewProjectionMatrix = new float[16];

        Matrix.setIdentityM(modelViewMatrix,0);
        Matrix.setIdentityM(modelViewProjectionMatrix,0);
        Matrix.translateM(modelViewMatrix, 0,0.5f, 0.5f, -3.0f);
        Matrix.multiplyMM(modelViewProjectionMatrix, 0,
                            perspectiveProjectionMatrix, 0,
                            modelViewMatrix, 0);

        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glUniform1i(gNumberOfSegmentsUniform, gNumberOfLineSegments);
        GLES32.glUniform1i(gNumberOfStripsUniform, 1);
        if (gNumberOfLineSegments == 1)
        {
            GLES32.glUniform4f(gLineColorUniform, 1.0f, 1.0f, 0.0f, 1.0f);
        }
        else if (gNumberOfLineSegments < 50)
        {
            GLES32.glUniform4f(gLineColorUniform,  1.0f, 0.0f, 0.0f, 1.0f);
        }
        else if (gNumberOfLineSegments == 50)
        {
            GLES32.glUniform4f(gLineColorUniform,  0.0f, 1.0f, 0.0f, 1.0f);
        }

        GLES32.glBindVertexArray(vao[0]);
        GLES32.glDrawArrays(GLES32.GL_PATCHES, 0, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);
        requestRender();
    }

    private void uninitialize()
    {
        if (vbo[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo, 0);
            vbo[0] = 0;
        }

        if (vao[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao, 0);
            vao[0] = 0;
        }

        if (shaderProgramObject != 0)
        {
            int[] shaderCount = new int[1];
            int shaderNumber;

            GLES32.glUseProgram(shaderProgramObject);

            //ask the program how many shaders are attached to you?
            GLES32.glGetProgramiv(shaderProgramObject,
                GLES32.GL_ATTACHED_SHADERS,
                shaderCount,
                0);

            int[] shaders = new int[shaderCount[0]];

            if (shaders[0] != 0)
            {
                //get shaders
                GLES32.glGetAttachedShaders(shaderProgramObject,
                    shaderCount[0],
                    shaderCount,
                    0,
                    shaders,
                    0);

                for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
                {
                    //detach
                    GLES32.glDetachShader(shaderProgramObject,
                        shaders[shaderNumber]);

                    //delete
                    GLES32.glDeleteShader(shaders[shaderNumber]);

                    //explicit 0
                    shaders[shaderNumber] = 0;
                }
            }

            GLES32.glDeleteProgram(shaderProgramObject);
            shaderProgramObject = 0;

            GLES32.glUseProgram(0);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event){

        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event)){
            super.onTouchEvent(event);
        }

        return(true);
    }

    @Override
    public boolean onDoubleTap(MotionEvent e){
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e){
        gNumberOfLineSegments--;
        if (gNumberOfLineSegments <= 0)
        {
            gNumberOfLineSegments = 1;
        }

        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e){
        gNumberOfLineSegments++;
        if (gNumberOfLineSegments >= 50)
        {
            gNumberOfLineSegments = 50;
        }
        return(true);
    }

    @Override
    public boolean onDown(MotionEvent e){
        return(true);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY){
        return(true);
    }

    @Override
    public void onLongPress(MotionEvent e){
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2,float distanceX, float distanceY){
        uninitialize();
        System.exit(0);
        return(true);
    }

    @Override
    public void onShowPress(MotionEvent e){

    }

    @Override
    public boolean onSingleTapUp(MotionEvent e){
        return(true);
    }
}
