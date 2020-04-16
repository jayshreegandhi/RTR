package com.rtr.twentyfoursphere;

//by user
import android.content.Context;
import android.view.Gravity;
import android.graphics.Color;

import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import java.lang.Math;

//for opengl
import android.opengl.GLSurfaceView;
import android.opengl.GLES32;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

//opengl buffers
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

//matrix math
import android.opengl.Matrix;


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer,OnGestureListener,OnDoubleTapListener{

    private final Context context;
    private GestureDetector gestureDetector;

    private int shaderProgramObject;
    private int vertexShaderObject;
    private int fragmentShaderObject;

    private int[] vao_sphere = new int[1];

    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];


    private int viewUniform;
    private int modelUniform;
    private int projectionUniform;

    private int laUniform;
    private int ldUniform;
    private int lsUniform;
    private int lightPositionUniform;

    private int kaUniform;
    private int kdUniform;
    private int ksUniform;
    private int materialShininessUniform;

    private int lKeyPressedUniform;

    private float[] perspectiveProjectionMatrix = new float[16];

    private boolean gbLight = false;

    private int numVertices;
    private int numElements;

    private float[] LightAmbient = new float[] { 0.0f,0.0f,0.0f,1.0f };
    private float[] LightDiffuse = new float[]{ 1.0f,1.0f,1.0f,1.0f };
    private float[] LightSpecular = new float[]{ 1.0f,1.0f,1.0f,1.0f };
    private float[] LightPosition = new float[]{ 0.0f,0.0f,0.0f,1.0f };

    private float[][] MaterialAmbient = new float[24][4];
    private float[][] MaterialDiffuse = new float[24][4];
    private float[][] MaterialSpecular = new float[24][4];
    private float[] MaterialShininess = new float[24];

    private float rotationAngleX = 0.0f;
    private float rotationAngleY = 0.0f;
    private float rotationAngleZ = 0.0f;

    private int keyPress = 0;
    private int doubleTapCounter = 0;
    private int gWidth;
    private int gHeight;

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
        gWidth = width;
        gHeight = height;
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
            "in vec3 vNormal;" +
            "uniform int u_lKeyPressed;" +
            "uniform mat4 u_view_matrix;" +
            "uniform mat4 u_model_matrix;" +
            "uniform mat4 u_projection_matrix;" +
            "uniform vec4 u_light_position;" +
            "out vec3 tNormal;" +
            "out vec3 light_direction;" +
            "out vec3 viewer_vector;" +
            "void main(void)" +
            "{" +
            "vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" +
            "tNormal = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
            "light_direction = vec3(u_light_position - eyeCoords);" +
            "viewer_vector = vec3(-eyeCoords.xyz);" +
            "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
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
            "precision mediump int;" +
            "in vec3 tNormal;" +
            "in vec3 light_direction;" +
            "in vec3 viewer_vector;" +
            "uniform int u_lKeyPressed;" +
            "uniform vec3 u_la;" +
            "uniform vec3 u_ld;" +
            "uniform vec3 u_ls;" +
            "uniform vec3 u_ka;" +
            "uniform vec3 u_kd;" +
            "uniform vec3 u_ks;" +
            "uniform float u_material_shininess;" +
            "out vec3 phong_ads_light;" +
            "out vec4 fragColor;" +
            "void main(void)" +
            "{" +
            "if(u_lKeyPressed == 1)" +
            "{" +
            "vec3 normalized_tNormal = normalize(tNormal);" +
            "vec3 normalized_light_direction = normalize(light_direction);" +
            "float tNorm_Dot_LightDirection = max(dot(normalized_light_direction, normalized_tNormal), 0.0);" +
            "vec3 reflection_vector = reflect(-normalized_light_direction, normalized_tNormal);" +
            "vec3 normalized_viewer_vector = normalize(viewer_vector);" +
            "vec3 ambient = u_la * u_ka;" +
            "vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;" +
            "vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,normalized_viewer_vector), 0.0), u_material_shininess);" +
            "phong_ads_light = ambient + diffuse + specular;" +
            "}" +
            "else" +
            "{" +
            "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "}" +
            "fragColor = vec4(phong_ads_light,1.0);" +
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

        laUniform = GLES32.glGetUniformLocation(shaderProgramObject,
		"u_la_");

        ldUniform = GLES32.glGetUniformLocation(shaderProgramObject,
            "u_ld");

        lsUniform = GLES32.glGetUniformLocation(shaderProgramObject,
            "u_ls");

        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject,
            "u_light_position");

        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ka");

        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_kd");

        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_ks");

        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_material_shininess");

        //create vao and bind vao
        Sphere sphere=new Sphere();
        float sphere_vertices[]=new float[1146];
        float sphere_normals[]=new float[1146];
        float sphere_textures[]=new float[764];
        short sphere_elements[]=new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();

        // vao
        GLES32.glGenVertexArrays(1,vao_sphere,0);
        GLES32.glBindVertexArray(vao_sphere[0]);

        // position vbo
        GLES32.glGenBuffers(1,vbo_sphere_position,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_position[0]);

        ByteBuffer byteBuffer=ByteBuffer.allocateDirect(sphere_vertices.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_vertices);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_vertices.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);

        // normal vbo
        GLES32.glGenBuffers(1,vbo_sphere_normal,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,vbo_sphere_normal[0]);

        byteBuffer=ByteBuffer.allocateDirect(sphere_normals.length * 4);
        byteBuffer.order(ByteOrder.nativeOrder());
        verticesBuffer=byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_normals);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                            sphere_normals.length * 4,
                            verticesBuffer,
                            GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL,
                                     3,
                                     GLES32.GL_FLOAT,
                                     false,0,0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);

        // element vbo
        GLES32.glGenBuffers(1,vbo_sphere_element,0);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);

        byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer elementsBuffer=byteBuffer.asShortBuffer();
        elementsBuffer.put(sphere_elements);
        elementsBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER,
                            sphere_elements.length * 2,
                            elementsBuffer,
                            GLES32.GL_STATIC_DRAW);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);

        GLES32.glBindVertexArray(0);

        setMaterialSphere();

        GLES32.glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

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

    private void setViewports(int x, int y, int width, int height)
    {
        if(height < 0)
        {
            height = 1;
        }

        GLES32.glViewport(x,y,width,height);
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

        int wid = gWidth / 4;
	    int ht = gHeight / 6;

        for(int x = 0; x < 4; x++)
        {
            for(int y = 0; y < 6; y++)
            {
                setViewports(x * wid, y*ht, wid, ht);

                GLES32.glUseProgram(shaderProgramObject);

                float[] viewMatrix = new float[16];
                float[] modelMatrix = new float[16];
                float[] projectionMatrix = new float[16];

                //cube
                //identity
                Matrix.setIdentityM(viewMatrix,0);
                Matrix.setIdentityM(modelMatrix,0);
                Matrix.setIdentityM(projectionMatrix,0);

                //tranformation
                Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -2.0f);

                Matrix.multiplyMM(projectionMatrix, 0,
                                    perspectiveProjectionMatrix, 0,
                                    projectionMatrix, 0);

                GLES32.glUniformMatrix4fv(viewUniform, 1, false, viewMatrix, 0);
                GLES32.glUniformMatrix4fv(modelUniform, 1, false, modelMatrix, 0);
                GLES32.glUniformMatrix4fv(projectionUniform, 1, false, projectionMatrix, 0);

                if(gbLight == true)
                {
                    GLES32.glUniform1i(lKeyPressedUniform, 1);

                    GLES32.glUniform3fv(laUniform, 1, LightAmbient, 0);
                    GLES32.glUniform3fv(ldUniform, 1, LightDiffuse, 0);
                    GLES32.glUniform3fv(lsUniform, 1, LightSpecular, 0);


                    if(keyPress == 1)
                    {
                        LightPosition[0] = 0.0f;
                        LightPosition[1] = (float)Math.cos(rotationAngleX);
                        LightPosition[2] = (float)Math.sin(rotationAngleX);
                        GLES32.glUniform4fv(lightPositionUniform, 1, LightPosition, 0);
                    }
                    else if(keyPress == 2)
                    {
                        LightPosition[0] = (float)Math.cos(rotationAngleY);
                        LightPosition[1] = 0.0f;
                        LightPosition[2] = (float)Math.sin(rotationAngleY);
                        GLES32.glUniform4fv(lightPositionUniform, 1, LightPosition, 0);
                    }
                    else if(keyPress == 3)
                    {
                        LightPosition[0] = (float)Math.cos(rotationAngleZ);
                        LightPosition[1] = (float)Math.sin(rotationAngleZ);
                        LightPosition[2] = 0.0f;
                        GLES32.glUniform4fv(lightPositionUniform, 1, LightPosition, 0);
                    }

           			if (x == 0)
                    {
                        if (y == 5)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[0], 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[0], 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[0], 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[0]);
                        }
                        else if (y == 4)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[1], 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[1], 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[1] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[1]);
                        }
                        else if (y == 3)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[2] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[2] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[2] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[2]);
                        }
                        else if (y == 2)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[3] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[3] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[3] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[3]);
                        }
                        else if (y == 1)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[4] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[4] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[4] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[4]);
                        }
                        else if (y == 0)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[5] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[5] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[5] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[5]);
                        }
                    }
                    else if (x == 1)
                    {
                        if (y == 5)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[6] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[6] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[6] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[6]);
                        }
                        else if (y == 4)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[7] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[7] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[7] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[7]);
                        }
                        else if (y == 3)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[8] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[8] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[8] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[8]);
                        }
                        else if (y == 2)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[9] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[9] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[9] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[9]);
                        }
                        else if (y == 1)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[10] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[10] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[10] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[10]);
                        }
                        else if (y == 0)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[11] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[11] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[11] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[11]);
                        }
                    }
                    else if (x == 2)
                    {
                        if (y == 5)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[12] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[12] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[12] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[12]);
                        }
                        else if (y == 4)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[13] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[13] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[13] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[13]);
                        }
                        else if (y == 3)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[14] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[14] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[14] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[14]);
                        }
                        else if (y == 2)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[15] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[15] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[15] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[15]);
                        }
                        else if (y == 1)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[16] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[16] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[16] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[16]);
                        }
                        else if (y == 0)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[17] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[17] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[17] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[17]);
                        }
                    }
                    else if (x == 3)
                    {
                        if (y == 5)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[18] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[18] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[18] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[18]);
                        }
                        else if (y == 4)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[19] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[19] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[19] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[19]);
                        }
                        else if (y == 3)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[20] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[20] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[20] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[20]);
                        }
                        else if (y == 2)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[21] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[21] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[21] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[21]);
                        }
                        else if (y == 1)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[22] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[22] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[22] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[22]);
                        }
                        else if (y == 0)
                        {
                            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient[23] , 0);
                            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse[23] , 0);
                            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular[23] , 0);
                            GLES32.glUniform1f(materialShininessUniform, MaterialShininess[23]);
                        }
				    }
                }
                else
                {
                    GLES32.glUniform1i(lKeyPressedUniform, 0);
                }

                // bind vao
                GLES32.glBindVertexArray(vao_sphere[0]);

                // *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
                GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
                GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

                // unbind vao
                GLES32.glBindVertexArray(0);


                GLES32.glUseProgram(0);
            }
        }
        requestRender();
    }

    private void update()
    {
        rotationAngleX = rotationAngleX + 0.05f;
        if (rotationAngleX >= 360.0f)
        {
            rotationAngleX = 0.0f;
        }

        rotationAngleY = rotationAngleY + 0.05f;
        if (rotationAngleY >= 360.0f)
        {
            rotationAngleY = 0.0f;
        }

        rotationAngleZ = rotationAngleZ + 0.05f;
        if (rotationAngleZ >= 360.0f)
        {
            rotationAngleZ = 0.0f;
        }
    }

    private void uninitialize()
    {
        // destroy vao
        if(vao_sphere[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0]=0;
        }

        // destroy position vbo
        if(vbo_sphere_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_position, 0);
            vbo_sphere_position[0]=0;
        }

        // destroy normal vbo
        if(vbo_sphere_normal[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_normal, 0);
            vbo_sphere_normal[0]=0;
        }

        // destroy element vbo
        if(vbo_sphere_element[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_element, 0);
            vbo_sphere_element[0]=0;
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
        doubleTapCounter = doubleTapCounter + 1;
        if(doubleTapCounter == 1)
        {
            keyPress = 1;
            rotationAngleX = 0.0f;
        }
        else if(doubleTapCounter == 2)
        {
            keyPress = 2;
            rotationAngleY = 0.0f;

        }
        else if(doubleTapCounter == 3)
        {
            keyPress = 3;
            rotationAngleZ = 0.0f;
            doubleTapCounter = 0;
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

    private void setMaterialSphere()
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
}
