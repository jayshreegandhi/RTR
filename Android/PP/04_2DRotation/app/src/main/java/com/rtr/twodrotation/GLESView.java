package com.rtr.twodrotation;

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
    private int fragmentShaderObject;

    private int[] vao_triangle = new int[1];
    private int[] vao_rectangle = new int[1];

    private int[] vbo_position_triangle = new int[1];
    private int[] vbo_position_rectangle = new int[1];

    private int[] vbo_color_triangle = new int[1];
    private int[] vbo_color_rectangle = new int[1];

    private int mvpUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private float angleTriangle;
    private float angleRectangle;

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
        update();
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
            "in vec4 vPosition;" +
            "in vec4 vColor;" +
            "uniform mat4 u_mvp_matrix;" +
            "out vec4 out_color;" +
            "void main(void)" +
            "{" +
            "   gl_Position = u_mvp_matrix * vPosition;" +
            "   out_color = vColor;" +
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

        //fragment shader
        fragmentShaderObject = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        final String fragmentShaderSourceCode = String.format(
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "in vec4 out_color;" +
            "out vec4 fragColor;" +
            "void main(void)" +
            "{" +
            "   fragColor = out_color;" +
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
        GLES32.glAttachShader(shaderProgramObject, fragmentShaderObject);

        //prelinking binding to attributes
        GLES32.glBindAttribLocation(shaderProgramObject,
                        GLESMacros.AMC_ATTRIBUTE_POSITION,
                        "vPosition");

        GLES32.glBindAttribLocation(shaderProgramObject,
                        GLESMacros.AMC_ATTRIBUTE_COLOR,
                        "vColor");

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

        final float traingleVertices[] = new float[]{
            0.0f,1.0f,0.0f,
            -1.0f,-1.0f,0.0f,
            1.0f,-1.0f,0.0f };

        final float rectangleVertices[] = new float[]{
            1.0f, 1.0f, 0.0f,
            -1.0f,1.0f,0.0f,
            -1.0f,-1.0f,0.0f,
            1.0f,-1.0f,0.0f };

	    final float traingleColor[] = new float[]{
		    1.0f, 0.0f, 0.0f,
		    0.0f, 1.0f, 0.0f,
	    	0.0f, 0.0f, 1.0f };

        final float rectangleColor[] = new float[]{
            0.0f, 0.0f, 1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f,
            0.0f,0.0f,1.0f };


        //create vao and bind vao
        //triangle
        GLES32.glGenVertexArrays(1, vao_triangle, 0);

        GLES32.glBindVertexArray(vao_triangle[0]);

        //create and bind vbo
        //position
        GLES32.glGenBuffers(1, vbo_position_triangle, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_triangle[0]);

        ByteBuffer byteBufferPositionTriangle = ByteBuffer.allocateDirect(traingleVertices.length * 4);
        byteBufferPositionTriangle.order(ByteOrder.nativeOrder());

        FloatBuffer positionBufferTriangle = byteBufferPositionTriangle.asFloatBuffer();
        positionBufferTriangle.put(traingleVertices);
        positionBufferTriangle.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        traingleVertices.length * 4,
                        positionBufferTriangle,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                        3,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        //color
        GLES32.glGenBuffers(1, vbo_color_triangle, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_color_triangle[0]);

        ByteBuffer byteBufferColorTriangle = ByteBuffer.allocateDirect(traingleColor.length * 4);
        byteBufferColorTriangle.order(ByteOrder.nativeOrder());

        FloatBuffer colorBufferTriangle = byteBufferColorTriangle.asFloatBuffer();
        colorBufferTriangle.put(traingleColor);
        colorBufferTriangle.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        traingleColor.length * 4,
                        colorBufferTriangle,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR,
                        3,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glBindVertexArray(0);

        //rectangle
        GLES32.glGenVertexArrays(1, vao_rectangle, 0);

        GLES32.glBindVertexArray(vao_rectangle[0]);

        //position
        GLES32.glGenBuffers(1, vbo_position_rectangle, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_rectangle[0]);

        ByteBuffer byteBufferPositionRectangle = ByteBuffer.allocateDirect(rectangleVertices.length * 4);
        byteBufferPositionRectangle.order(ByteOrder.nativeOrder());

        FloatBuffer positionBufferRectangle = byteBufferPositionRectangle.asFloatBuffer();
        positionBufferRectangle.put(rectangleVertices);
        positionBufferRectangle.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        rectangleVertices.length * 4,
                        positionBufferRectangle,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                        3,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        //color
        GLES32.glGenBuffers(1, vbo_color_rectangle, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_color_rectangle[0]);

        ByteBuffer byteBufferColorRectangle = ByteBuffer.allocateDirect(rectangleColor.length * 4);
        byteBufferColorRectangle.order(ByteOrder.nativeOrder());

        FloatBuffer colorBufferRectangle = byteBufferColorRectangle.asFloatBuffer();
        colorBufferRectangle.put(rectangleColor);
        colorBufferRectangle.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        rectangleColor.length * 4,
                        colorBufferRectangle,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_COLOR,
                        3,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_COLOR);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        GLES32.glBindVertexArray(0);

        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        GLES32.glDisable(GLES32.GL_CULL_FACE);

	    GLES32.glEnable(GLES32.GL_DEPTH_TEST);
	    GLES32.glDepthFunc(GLES32.GL_LEQUAL);

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
        float[] rotationMatrix = new float[16];

        //triangle
        //identity
        Matrix.setIdentityM(modelViewMatrix,0);
        Matrix.setIdentityM(modelViewProjectionMatrix,0);
        Matrix.setIdentityM(rotationMatrix, 0);

        //tranformation
        Matrix.translateM(modelViewMatrix, 0,-1.5f, 0.0f, -6.0f);
        Matrix.setRotateM(rotationMatrix, 0, angleTriangle, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);

        Matrix.multiplyMM(modelViewProjectionMatrix, 0,
                            perspectiveProjectionMatrix, 0,
                            modelViewMatrix, 0);

        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glBindVertexArray(vao_triangle[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 3);
        GLES32.glBindVertexArray(0);

        //rectangle
        //identity
        Matrix.setIdentityM(modelViewMatrix,0);
        Matrix.setIdentityM(modelViewProjectionMatrix,0);
        Matrix.setIdentityM(rotationMatrix, 0);

        //tranformation
        Matrix.translateM(modelViewMatrix, 0, 1.5f, 0.0f, -6.0f);
        Matrix.setRotateM(rotationMatrix, 0, angleRectangle, 1.0f, 0.0f, 0.0f);
        Matrix.multiplyMM(modelViewMatrix, 0, modelViewMatrix, 0, rotationMatrix, 0);

        Matrix.multiplyMM(modelViewProjectionMatrix, 0,
                            perspectiveProjectionMatrix, 0,
                            modelViewMatrix, 0);

        GLES32.glUniformMatrix4fv(mvpUniform, 1, false, modelViewProjectionMatrix, 0);

        GLES32.glBindVertexArray(vao_rectangle[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glBindVertexArray(0);


        GLES32.glUseProgram(0);
        requestRender();
    }

    private void update()
    {
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

    private void uninitialize()
    {
        if (vbo_color_rectangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_color_rectangle, 0);
            vbo_color_rectangle[0] = 0;
        }

        if (vbo_position_rectangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_position_rectangle, 0);
            vbo_position_rectangle[0] = 0;
        }

        if (vao_rectangle[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_rectangle, 0);
            vao_rectangle[0] = 0;
        }

        if (vbo_color_triangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_color_triangle, 0);
            vbo_color_triangle[0] = 0;
        }

        if (vbo_position_triangle[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_position_triangle, 0);
            vbo_position_triangle[0] = 0;
        }

        if (vao_triangle[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_triangle, 0);
            vao_triangle[0] = 0;
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
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e){
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
