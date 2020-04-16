package com.rtr.lightsonrotatingpyramid;

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

    private int[] vao_pyramid = new int[1];

    private int[] vbo_position_pyramid = new int[1];
    private int[] vbo_normal_pyramid = new int[1];

    private int viewUniform;
    private int modelUniform;
    private int projectionUniform;

    private int laUniformZero;
    private int ldUniformZero;
    private int lsUniformZero;
    private int lightPositionUniformZero;

    private int laUniformOne;
    private int ldUniformOne;
    private int lsUniformOne;
    private int lightPositionUniformOne;

    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int materialShininessUniform;
    private int lKeyPressedUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private float anglePyramid = 0.0f;
    private boolean gbAnimation = false;
    private boolean gbLight = false;

    private int numVertices;
    private int numElements;

    private float[] LightAmbientZero = new float[]{ 0.0f,0.0f,0.0f,1.0f };
    private float[] LightDiffuseZero = new float[]{ 1.0f,0.0f,0.0f,1.0f };
    private float[] LightSpecularZero = new float[]{ 1.0f,0.0f,0.0f,1.0f };
    private float[] LightPositionZero = new float[]{ -2.0f,0.0f,0.0f,1.0f };

    private float[] LightAmbientOne = new float[]{ 0.0f,0.0f,0.0f,1.0f };
    private float[] LightDiffuseOne = new float[]{ 0.0f,0.0f,1.0f,1.0f };
    private float[] LightSpecularOne = new float[]{ 0.0f,0.0f,1.0f,1.0f };
    private float[] LightPositionOne = new float[]{ 2.0f,0.0f,0.0f,1.0f };

    private float[] MaterialAmbient = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float[] MaterialDiffuse = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float[] MaterialSpecular = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float MaterialShininess = 128.0f;

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
        if(gbAnimation == true)
        {
            update();
        }

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
            "in vec3 vNormal;" +
            "uniform mat4 u_view_matrix;" +
            "uniform mat4 u_model_matrix;" +
            "uniform mat4 u_projection_matrix;" +
            "uniform int u_lKeyPressed;" +
            "uniform vec3 u_la_zero;" +
            "uniform vec3 u_ld_zero;" +
            "uniform vec3 u_ls_zero;" +
            "uniform vec4 u_light_position_zero;" +
            "uniform vec3 u_la_one;" +
            "uniform vec3 u_ld_one;" +
            "uniform vec3 u_ls_one;" +
            "uniform vec4 u_light_position_one;" +
            "uniform vec3 u_ka;" +
            "uniform vec3 u_kd;" +
            "uniform vec3 u_ks;" +
            "uniform float u_material_shininess;" +
            "out vec3 phong_ads_light;" +
            "void main(void)" +
            "{" +
            "   if(u_lKeyPressed == 1)" +
            "   {" +
            "       vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" +
            "       vec3 tNormal = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +
            "       vec3 light_direction_zero = normalize(vec3(u_light_position_zero - eyeCoords));" +
            "       vec3 light_direction_one = normalize(vec3(u_light_position_one - eyeCoords));" +
            "       float tNorm_Dot_LightDirection_zero = max(dot(light_direction_zero, tNormal), 0.0);" +
            "       float tNorm_Dot_LightDirection_one = max(dot(light_direction_one, tNormal), 0.0);" +
            "       vec3 reflection_vector_zero = reflect(-light_direction_zero, tNormal);" +
            "       vec3 reflection_vector_one = reflect(-light_direction_one, tNormal);" +
            "       vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));" +
            "       vec3 ambient_zero = u_la_zero * u_ka;" +
            "       vec3 diffuse_zero = u_ld_zero * u_kd * tNorm_Dot_LightDirection_zero;" +
            "       vec3 specular_zero = u_ls_zero * u_ks * pow(max(dot(reflection_vector_zero,viewer_vector), 0.0), u_material_shininess);" +
            "       vec3 ambient_one = u_la_one * u_ka;" +
            "       vec3 diffuse_one = u_ld_one * u_kd * tNorm_Dot_LightDirection_one;" +
            "       vec3 specular_one = u_ls_one * u_ks * pow(max(dot(reflection_vector_one,viewer_vector), 0.0), u_material_shininess);" +
            "       phong_ads_light = ambient_zero + ambient_one + diffuse_zero + diffuse_one + specular_zero + specular_one;" +
            "   }" +
            "   else" +
            "   {" +
            "       phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "   }" +
            "   gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
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
            "in vec3 phong_ads_light;" +
            "out vec4 fragColor;" +
            "void main(void)" +
            "{" +
            "   fragColor = vec4(phong_ads_light,1.0);" +
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
                        GLESMacros.AMC_ATTRIBUTE_NORMAL,
                        "vNormal");

        GLES32.glLinkProgram(shaderProgramObject);
        //compilation error checking

        int[] iProgramLinkStatus = new int[1];
        iInfoLogLength[0] = 0;
        szInfoLog = null;

        GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_LINK_STATUS,iProgramLinkStatus,0);
        if(iProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObject);
                System.out.println("RTR: Shader Program linking log: " + szInfoLog);

                uninitialize();
                System.exit(0);
            }
        }

        //get uniform location
        viewUniform = GLES32.glGetUniformLocation(shaderProgramObject,
									   "u_view_matrix");

        modelUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                            "u_model_matrix");

        projectionUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                                "u_projection_matrix");

        lKeyPressedUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                                "u_lKeyPressed");

        laUniformZero = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_la_zero");

        laUniformOne = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_la_one");

        ldUniformZero = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ld_zero");

        ldUniformOne = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ld_one");

        lsUniformZero = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ls_zero");

        lsUniformOne = GLES32.glGetUniformLocation(shaderProgramObject,
                                "u_ls_one");

        lightPositionUniformZero = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_light_position_zero");

        lightPositionUniformOne = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_light_position_one");

        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ka");

        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_kd");

        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ks");

        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_material_shininess");

        //create vao and bind vao
        final float pyramidVertices[] = new float[]{
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
            -1.0f, -1.0f, -1.0f };

        final float pyramidNormals[] = new float[]{
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
            -0.894427f, 0.447214f, 0.0f };

        // vao
        //cube
        GLES32.glGenVertexArrays(1, vao_pyramid, 0);

        GLES32.glBindVertexArray(vao_pyramid[0]);

        //position
        GLES32.glGenBuffers(1, vbo_position_pyramid, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_pyramid[0]);

        ByteBuffer byteBufferPositionPyramid = ByteBuffer.allocateDirect(pyramidVertices.length * 4);
        byteBufferPositionPyramid.order(ByteOrder.nativeOrder());

        FloatBuffer positionBufferPyramid = byteBufferPositionPyramid.asFloatBuffer();
        positionBufferPyramid.put(pyramidVertices);
        positionBufferPyramid.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        pyramidVertices.length * 4,
                        positionBufferPyramid,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                        3,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);


        //normals
        GLES32.glGenBuffers(1, vbo_normal_pyramid, 0);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_normal_pyramid[0]);

        ByteBuffer byteBufferNormalPyramid = ByteBuffer.allocateDirect(pyramidNormals.length * 4);
        byteBufferNormalPyramid.order(ByteOrder.nativeOrder());

        FloatBuffer normalBufferPyramid = byteBufferNormalPyramid.asFloatBuffer();
        normalBufferPyramid.put(pyramidNormals);
        normalBufferPyramid.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                        pyramidNormals.length * 4,
                        normalBufferPyramid,
                        GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL,
                        3,
                        GLES32.GL_FLOAT,
                        false,
                        0,
                        0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);

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

        float[] viewMatrix = new float[16];
        float[] modelMatrix = new float[16];
        float[] projectionMatrix = new float[16];
        float[] translationMatrix = new float[16];
        float[] rotationMatrix = new float[16];

        //cube
        //identity
        Matrix.setIdentityM(viewMatrix,0);
        Matrix.setIdentityM(modelMatrix,0);
        Matrix.setIdentityM(projectionMatrix,0);
        Matrix.setIdentityM(translationMatrix,0);
        Matrix.setIdentityM(rotationMatrix,0);

        //tranformation
        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -5.0f);
        Matrix.setRotateM(rotationMatrix, 0, anglePyramid, 0.0f, 1.0f, 0.0f);
        Matrix.multiplyMM(modelMatrix, 0, modelMatrix, 0, rotationMatrix, 0);

        Matrix.multiplyMM(projectionMatrix, 0,
                            perspectiveProjectionMatrix, 0,
                            projectionMatrix, 0);

        GLES32.glUniformMatrix4fv(viewUniform, 1, false, viewMatrix, 0);
        GLES32.glUniformMatrix4fv(modelUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(projectionUniform, 1, false, projectionMatrix, 0);

        if(gbLight == true)
        {
            GLES32.glUniform1i(lKeyPressedUniform, 1);
            GLES32.glUniform3fv(laUniformZero, 1, LightAmbientZero, 0);
            GLES32.glUniform3fv(ldUniformZero, 1, LightDiffuseZero, 0);
            GLES32.glUniform3fv(lsUniformZero, 1, LightSpecularZero, 0);
            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient, 0);
            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular, 0);
            GLES32.glUniform1f(materialShininessUniform, MaterialShininess);
            GLES32.glUniform4fv(lightPositionUniformZero, 1, LightPositionZero, 0);

            GLES32.glUniform3fv(laUniformOne, 1, LightAmbientOne, 0);
            GLES32.glUniform3fv(ldUniformOne, 1, LightDiffuseOne, 0);
            GLES32.glUniform3fv(lsUniformOne, 1, LightSpecularOne, 0);
            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient, 0);
            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular, 0);
            GLES32.glUniform1f(materialShininessUniform, MaterialShininess);
            GLES32.glUniform4fv(lightPositionUniformOne, 1, LightPositionOne, 0);
        }
        else
        {
            GLES32.glUniform1i(lKeyPressedUniform, 0);
        }

        GLES32.glBindVertexArray(vao_pyramid[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLES, 0, 12);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);

        requestRender();
    }

    private void update()
    {
        anglePyramid = anglePyramid + 1.0f;
        if (anglePyramid >= 360.0f)
        {
            anglePyramid = 0.0f;
        }
    }

    private void uninitialize()
    {
        // destroy vao
        if (vbo_normal_pyramid[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_normal_pyramid, 0);
            vbo_normal_pyramid[0] = 0;
        }

        if (vbo_position_pyramid[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_position_pyramid, 0);
            vbo_position_pyramid[0] = 0;
        }

        if (vao_pyramid[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_pyramid, 0);
            vao_pyramid[0] = 0;
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
        if (gbAnimation == false)
        {
            gbAnimation = true;
        }
        else
        {
            gbAnimation = false;
        }
        return(true);
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e){
        return(true);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e){
        if (gbLight == false)
        {
            gbLight = true;
        }
        else
        {
            gbLight = false;
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
