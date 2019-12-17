package com.rtr.adslightspherepv;

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

    private boolean gbAnimation = false;
    private boolean gbLight = false;

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
            "precision highp int;" +
            "uniform int u_lKeyPressed;" +
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

        laUniform = GLES32.glGetUniformLocation(shaderProgramObject,
                                        "u_la");

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

        //cube
        //identity
        Matrix.setIdentityM(viewMatrix,0);
        Matrix.setIdentityM(modelMatrix,0);
        Matrix.setIdentityM(projectionMatrix,0);

        //tranformation
        Matrix.translateM(modelMatrix, 0, 0.0f, 0.0f, -3.0f);

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
            GLES32.glUniform3fv(kaUniform, 1, MaterialAmbient, 0);
            GLES32.glUniform3fv(kdUniform, 1, MaterialDiffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, MaterialSpecular, 0);
            GLES32.glUniform1f(materialShininessUniform, MaterialShininess);
            GLES32.glUniform4fv(lightPositionUniform, 1, LightPosition, 0);
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
