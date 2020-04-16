package com.rtr.togglepvandpf;

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
import java.nio.ShortBuffer;

//matrix math
import android.opengl.Matrix;


public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer,OnGestureListener,OnDoubleTapListener{

    private final Context context;
    private GestureDetector gestureDetector;

    private int shaderProgramObject_perFrag;
    private int shaderProgramObject_perVert;

    private int vertexShaderObject_perFrag;
    private int vertexShaderObject_perVert;
    private int fragmentShaderObject_perFrag;
    private int fragmentShaderObject_perVert;

    private int[] vao_sphere = new int[1];

    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_element = new int[1];

    private int viewUniform_pf;
    private int modelUniform_pf;
    private int projectionUniform_pf;

    private int laUniform_pf;
    private int ldUniform_pf;
    private int lsUniform_pf;
    private int lightPositionUniform_pf;

    private int kaUniform_pf;
    private int kdUniform_pf;
    private int ksUniform_pf;
    private int materialShininessUniform_pf;

    private int lKeyPressedUniform_pf;

    //uniforms
    private int viewUniform_pv;
    private int modelUniform_pv;
    private int projectionUniform_pv;

    private int laUniform_pv;
    private int ldUniform_pv;
    private int lsUniform_pv;
    private int lightPositionUniform_pv;

    private int kaUniform_pv;
    private int kdUniform_pv;
    private int ksUniform_pv;
    private int materialShininessUniform_pv;

    private int lKeyPressedUniform_pv;

    private float[] perspectiveProjectionMatrix = new float[16];

    private boolean gbAnimation = false;
    private boolean gbLight = false;
    private boolean isFKeyPressed = false;
    private boolean isVKeyPressed = false;

    private int numVertices;
    private int numElements;

    private float[] LightAmbient = new float[]{0.0f, 0.0f, 0.0f, 0.0f};
    private float[] LightDiffuse = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float[] LightSpecular = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
    private float[] LightPosition = new float[]{100.0f, 100.0f, 100.0f, 1.0f};

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
        vertexShaderObject_perVert = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);
        vertexShaderObject_perFrag = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        final String vertexShaderSourceCode_perFrag = String.format(
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
            "   vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" +
            "   tNormal = mat3(u_view_matrix * u_model_matrix) * vNormal;" +
            "   light_direction = vec3(u_light_position - eyeCoords);" +
            "   viewer_vector = vec3(-eyeCoords.xyz);" +
            "   gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
            "}");

        final String vertexShaderSourceCode_perVert = String.format(
            "#version 320 es" +
            "\n" +
            "in vec4 vPosition;" +
            "in vec3 vNormal;" +
            "uniform mat4 u_view_matrix;" +
            "uniform mat4 u_model_matrix;" +
            "uniform mat4 u_projection_matrix;" +
            "uniform int u_lKeyPressed;" +
            "uniform vec3 u_la;" +
            "uniform vec3 u_ld;" +
            "uniform vec3 u_ls;" +
            "uniform vec4 u_light_position;" +
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
            "       vec3 light_direction = normalize(vec3(u_light_position - eyeCoords));" +
            "       float tNorm_Dot_LightDirection = max(dot(light_direction, tNormal), 0.0);" +
            "       vec3 reflection_vector = reflect(-light_direction, tNormal);" +
            "       vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));" +
            "       vec3 ambient = u_la * u_ka;" +
            "       vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;" +
            "       vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,viewer_vector), 0.0), u_material_shininess);" +
            "       phong_ads_light = ambient + diffuse + specular;" +
            "   }" +
            "   else" +
            "   {" +
            "       phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "   }" +
            "   gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
            "}");



        GLES32.glShaderSource(vertexShaderObject_perFrag, vertexShaderSourceCode_perFrag);

        GLES32.glCompileShader(vertexShaderObject_perFrag);

        //compilation error checking

        int[] iShaderCompileStatus_perFrag = new int[1];
        int[] iInfoLogLength_perFrag = new int[1];
        String szInfoLog_perFrag = null;

        GLES32.glGetShaderiv(vertexShaderObject_perFrag, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus_perFrag,0);
        if(iShaderCompileStatus_perFrag[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject_perFrag, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength_perFrag,0);
            if(iInfoLogLength_perFrag[0] > 0)
            {
                szInfoLog_perFrag = GLES32.glGetShaderInfoLog(vertexShaderObject_perFrag);
                System.out.println("RTR: Vertex Shader Compilation log: " + szInfoLog_perFrag);

                uninitialize();
                System.exit(0);
            }
        }

        GLES32.glShaderSource(vertexShaderObject_perVert, vertexShaderSourceCode_perVert);

        GLES32.glCompileShader(vertexShaderObject_perVert);

        //compilation error checking

        int[] iShaderCompileStatus_perVert = new int[1];
        int[] iInfoLogLength_perVert = new int[1];
        String szInfoLog_perVert = null;

        GLES32.glGetShaderiv(vertexShaderObject_perVert, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus_perVert,0);
        if(iShaderCompileStatus_perVert[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject_perVert, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength_perVert,0);
            if(iInfoLogLength_perVert[0] > 0)
            {
                szInfoLog_perVert = GLES32.glGetShaderInfoLog(vertexShaderObject_perVert);
                System.out.println("RTR: Vertex Shader Compilation log: " + szInfoLog_perVert);

                uninitialize();
                System.exit(0);
            }
        }

        //fragment shader
        fragmentShaderObject_perFrag = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);
        fragmentShaderObject_perVert = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        final String fragmentShaderSourceCode_perFrag = String.format(
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "precision highp int;" +
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
            "   if(u_lKeyPressed == 1)" +
            "   {" +
            "       vec3 normalized_tNormal = normalize(tNormal);" +
            "       vec3 normalized_light_direction = normalize(light_direction);" +
            "       float tNorm_Dot_LightDirection = max(dot(normalized_light_direction, normalized_tNormal), 0.0);" +
            "       vec3 reflection_vector = reflect(-normalized_light_direction, normalized_tNormal);" +
            "       vec3 normalized_viewer_vector = normalize(viewer_vector);" +
            "       vec3 ambient = u_la * u_ka;" +
            "       vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;" +
            "       vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,normalized_viewer_vector), 0.0), u_material_shininess);" +
            "       phong_ads_light = ambient + diffuse + specular;" +
            "   }" +
            "   else" +
            "   {" +
            "       phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "   }" +
            "   fragColor = vec4(phong_ads_light,1.0);" +
            "}");

        final String fragmentShaderSourceCode_perVert = String.format(
            "#version 320 es" +
            "\n" +
            "precision highp float;" +
            "precision highp int;" +
            "uniform int u_lKeyPressed;" +
            "in vec3 phong_ads_light;" +
            "out vec4 fragColor;" +
            "void main(void)" +
            "{" +
            "   fragColor = vec4(phong_ads_light,1.0);" +
            "}");

        GLES32.glShaderSource(fragmentShaderObject_perFrag, fragmentShaderSourceCode_perFrag);

        GLES32.glCompileShader(fragmentShaderObject_perFrag);

        //compilation error checking

        iShaderCompileStatus_perFrag[0] = 0;
        iInfoLogLength_perFrag[0] = 0;
        szInfoLog_perFrag = null;

        GLES32.glGetShaderiv(fragmentShaderObject_perFrag, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus_perFrag,0);
        if(iShaderCompileStatus_perFrag[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject_perFrag, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength_perFrag,0);
            if(iInfoLogLength_perFrag[0] > 0)
            {
                szInfoLog_perFrag = GLES32.glGetShaderInfoLog(fragmentShaderObject_perFrag);
                System.out.println("RTR: Fragment Shader Compilation log: " + szInfoLog_perFrag);

                uninitialize();
                System.exit(0);
            }
        }

        GLES32.glShaderSource(fragmentShaderObject_perVert, fragmentShaderSourceCode_perVert);

        GLES32.glCompileShader(fragmentShaderObject_perVert);

        //compilation error checking

        iShaderCompileStatus_perVert[0] = 0;
        iInfoLogLength_perVert[0] = 0;
        szInfoLog_perVert = null;

        GLES32.glGetShaderiv(fragmentShaderObject_perVert, GLES32.GL_COMPILE_STATUS,iShaderCompileStatus_perVert,0);
        if(iShaderCompileStatus_perVert[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject_perVert, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength_perVert,0);
            if(iInfoLogLength_perVert[0] > 0)
            {
                szInfoLog_perVert = GLES32.glGetShaderInfoLog(fragmentShaderObject_perVert);
                System.out.println("RTR: Fragment Shader Compilation log: " + szInfoLog_perVert);

                uninitialize();
                System.exit(0);
            }
        }

        //Shader program
        shaderProgramObject_perFrag = GLES32.glCreateProgram();
        shaderProgramObject_perVert = GLES32.glCreateProgram();

        GLES32.glAttachShader(shaderProgramObject_perFrag, vertexShaderObject_perFrag);
        GLES32.glAttachShader(shaderProgramObject_perFrag, fragmentShaderObject_perFrag);

        //prelinking binding to attributes
        GLES32.glBindAttribLocation(shaderProgramObject_perFrag,
                        GLESMacros.AMC_ATTRIBUTE_POSITION,
                        "vPosition");

        GLES32.glBindAttribLocation(shaderProgramObject_perFrag,
                        GLESMacros.AMC_ATTRIBUTE_NORMAL,
                        "vNormal");

        GLES32.glLinkProgram(shaderProgramObject_perFrag);
        //compilation error checking

        int[] iProgramLinkStatus_perFrag = new int[1];
        iInfoLogLength_perFrag[0] = 0;
        szInfoLog_perFrag = null;

        GLES32.glGetProgramiv(shaderProgramObject_perFrag, GLES32.GL_LINK_STATUS,iProgramLinkStatus_perFrag,0);
        if(iProgramLinkStatus_perFrag[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject_perFrag, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength_perFrag,0);
            if(iInfoLogLength_perFrag[0] > 0)
            {
                szInfoLog_perFrag = GLES32.glGetProgramInfoLog(shaderProgramObject_perFrag);
                System.out.println("RTR: Shader Program linking log: " + szInfoLog_perFrag);

                uninitialize();
                System.exit(0);
            }
        }

        GLES32.glAttachShader(shaderProgramObject_perVert, vertexShaderObject_perVert);
        GLES32.glAttachShader(shaderProgramObject_perVert, fragmentShaderObject_perVert);

        //prelinking binding to attributes
        GLES32.glBindAttribLocation(shaderProgramObject_perVert,
                        GLESMacros.AMC_ATTRIBUTE_POSITION,
                        "vPosition");

        GLES32.glBindAttribLocation(shaderProgramObject_perVert,
                        GLESMacros.AMC_ATTRIBUTE_NORMAL,
                        "vNormal");

        GLES32.glLinkProgram(shaderProgramObject_perVert);
        //compilation error checking

        int[] iProgramLinkStatus_perVert = new int[1];
        iInfoLogLength_perVert[0] = 0;
        szInfoLog_perVert = null;

        GLES32.glGetProgramiv(shaderProgramObject_perVert, GLES32.GL_LINK_STATUS,iProgramLinkStatus_perVert,0);
        if(iProgramLinkStatus_perVert[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject_perVert, GLES32.GL_INFO_LOG_LENGTH,iInfoLogLength_perVert,0);
            if(iInfoLogLength_perVert[0] > 0)
            {
                szInfoLog_perVert = GLES32.glGetProgramInfoLog(shaderProgramObject_perVert);
                System.out.println("RTR: Shader Program linking log: " + szInfoLog_perVert);

                uninitialize();
                System.exit(0);
            }
        }

        //get uniform location
        viewUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
		    "u_view_matrix");

        modelUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_model_matrix");

        projectionUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_projection_matrix");

        lKeyPressedUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_lKeyPressed");

        laUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_la");

        ldUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ld");

        lsUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ls");

        lightPositionUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_light_position");

        kaUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ka");

        kdUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_kd");

        ksUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_ks");

        materialShininessUniform_pf = GLES32.glGetUniformLocation(shaderProgramObject_perFrag,
            "u_material_shininess");

        //Post-Linking reteriving uniform location
        viewUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_view_matrix");

        modelUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_model_matrix");

        projectionUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_projection_matrix");

        lKeyPressedUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_lKeyPressed");

        laUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_la");

        ldUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_ld");

        lsUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_ls");

        lightPositionUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_light_position");

        kaUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_ka");

        kdUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_kd");

        ksUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
            "u_ks");

        materialShininessUniform_pv = GLES32.glGetUniformLocation(shaderProgramObject_perVert,
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

        if(isFKeyPressed == true)
        {
            GLES32.glUseProgram(shaderProgramObject_perFrag);

            float[] viewMatrix_pf = new float[16];
            float[] modelMatrix_pf = new float[16];
            float[] projectionMatrix_pf = new float[16];

            //cube
            //identity
            Matrix.setIdentityM(viewMatrix_pf,0);
            Matrix.setIdentityM(modelMatrix_pf,0);
            Matrix.setIdentityM(projectionMatrix_pf,0);

            //tranformation
            Matrix.translateM(modelMatrix_pf, 0, 0.0f, 0.0f, -3.0f);

            Matrix.multiplyMM(projectionMatrix_pf, 0,
                                perspectiveProjectionMatrix, 0,
                                projectionMatrix_pf, 0);

            GLES32.glUniformMatrix4fv(viewUniform_pf, 1, false, viewMatrix_pf, 0);
            GLES32.glUniformMatrix4fv(modelUniform_pf, 1, false, modelMatrix_pf, 0);
            GLES32.glUniformMatrix4fv(projectionUniform_pf, 1, false, projectionMatrix_pf, 0);

            if(gbLight == true)
            {
                GLES32.glUniform1i(lKeyPressedUniform_pf, 1);
                GLES32.glUniform3fv(laUniform_pf, 1, LightAmbient, 0);
                GLES32.glUniform3fv(ldUniform_pf, 1, LightDiffuse, 0);
                GLES32.glUniform3fv(lsUniform_pf, 1, LightSpecular, 0);
                GLES32.glUniform3fv(kaUniform_pf, 1, MaterialAmbient, 0);
                GLES32.glUniform3fv(kdUniform_pf, 1, MaterialDiffuse, 0);
                GLES32.glUniform3fv(ksUniform_pf, 1, MaterialSpecular, 0);
                GLES32.glUniform1f(materialShininessUniform_pf, MaterialShininess);
                GLES32.glUniform4fv(lightPositionUniform_pf, 1, LightPosition, 0);
            }
            else
            {
                GLES32.glUniform1i(lKeyPressedUniform_pf, 0);
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

        if(isVKeyPressed == true)
        {
            GLES32.glUseProgram(shaderProgramObject_perVert);

            float[] viewMatrix_pv = new float[16];
            float[] modelMatrix_pv = new float[16];
            float[] projectionMatrix_pv = new float[16];

            //cube
            //identity
            Matrix.setIdentityM(viewMatrix_pv,0);
            Matrix.setIdentityM(modelMatrix_pv,0);
            Matrix.setIdentityM(projectionMatrix_pv,0);

            //tranformation
            Matrix.translateM(modelMatrix_pv, 0, 0.0f, 0.0f, -3.0f);

            Matrix.multiplyMM(projectionMatrix_pv, 0,
                                perspectiveProjectionMatrix, 0,
                                projectionMatrix_pv, 0);

            GLES32.glUniformMatrix4fv(viewUniform_pv, 1, false, viewMatrix_pv, 0);
            GLES32.glUniformMatrix4fv(modelUniform_pv, 1, false, modelMatrix_pv, 0);
            GLES32.glUniformMatrix4fv(projectionUniform_pv, 1, false, projectionMatrix_pv, 0);

            if(gbLight == true)
            {
                GLES32.glUniform1i(lKeyPressedUniform_pv, 1);
                GLES32.glUniform3fv(laUniform_pv, 1, LightAmbient, 0);
                GLES32.glUniform3fv(ldUniform_pv, 1, LightDiffuse, 0);
                GLES32.glUniform3fv(lsUniform_pv, 1, LightSpecular, 0);
                GLES32.glUniform3fv(kaUniform_pv, 1, MaterialAmbient, 0);
                GLES32.glUniform3fv(kdUniform_pv, 1, MaterialDiffuse, 0);
                GLES32.glUniform3fv(ksUniform_pv, 1, MaterialSpecular, 0);
                GLES32.glUniform1f(materialShininessUniform_pv, MaterialShininess);
                GLES32.glUniform4fv(lightPositionUniform_pv, 1, LightPosition, 0);
            }
            else
            {
                GLES32.glUniform1i(lKeyPressedUniform_pv, 0);
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
        requestRender();
    }

    private void update()
    {

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

        if (shaderProgramObject_perFrag != 0)
        {
            int[] shaderCount = new int[1];
            int shaderNumber;

            GLES32.glUseProgram(shaderProgramObject_perFrag);

            //ask the program how many shaders are attached to you?
            GLES32.glGetProgramiv(shaderProgramObject_perFrag,
                GLES32.GL_ATTACHED_SHADERS,
                shaderCount,
                0);

            int[] shaders = new int[shaderCount[0]];

            if (shaders[0] != 0)
            {
                //get shaders
                GLES32.glGetAttachedShaders(shaderProgramObject_perFrag,
                    shaderCount[0],
                    shaderCount,
                    0,
                    shaders,
                    0);

                for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
                {
                    //detach
                    GLES32.glDetachShader(shaderProgramObject_perFrag,
                        shaders[shaderNumber]);

                    //delete
                    GLES32.glDeleteShader(shaders[shaderNumber]);

                    //explicit 0
                    shaders[shaderNumber] = 0;
                }
            }

            GLES32.glDeleteProgram(shaderProgramObject_perFrag);
            shaderProgramObject_perFrag = 0;

            GLES32.glUseProgram(0);
        }

        if (shaderProgramObject_perVert != 0)
        {
            int[] shaderCount = new int[1];
            int shaderNumber;

            GLES32.glUseProgram(shaderProgramObject_perVert);

            //ask the program how many shaders are attached to you?
            GLES32.glGetProgramiv(shaderProgramObject_perVert,
                GLES32.GL_ATTACHED_SHADERS,
                shaderCount,
                0);

            int[] shaders = new int[shaderCount[0]];

            if (shaders[0] != 0)
            {
                //get shaders
                GLES32.glGetAttachedShaders(shaderProgramObject_perVert,
                    shaderCount[0],
                    shaderCount,
                    0,
                    shaders,
                    0);

                for (shaderNumber = 0; shaderNumber < shaderCount[0]; shaderNumber++)
                {
                    //detach
                    GLES32.glDetachShader(shaderProgramObject_perVert,
                        shaders[shaderNumber]);

                    //delete
                    GLES32.glDeleteShader(shaders[shaderNumber]);

                    //explicit 0
                    shaders[shaderNumber] = 0;
                }
            }

            GLES32.glDeleteProgram(shaderProgramObject_perVert);
            shaderProgramObject_perVert = 0;

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
        if (isVKeyPressed == false)
        {
            isVKeyPressed = true;
            isFKeyPressed = false;
        }
        else
        {
            isVKeyPressed = false;
            //isFKeyPressed = true;
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
        if (isFKeyPressed == false)
        {
            isFKeyPressed = true;
            isVKeyPressed = false;
        }
        else
        {
            isFKeyPressed = false;
            //isVKeyPressed = true;
        }
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
