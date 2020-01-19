// onload function
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_COLOR: 1,
    AMC_ATTRIBUTE_NORMAL: 2,
    AMC_ATTRIBUTE_TEXTURE0: 3
};

var gShaderProgramObject;
var gVertexShaderObject;
var gFragmentShaderObject;

var vao_pyramid;
var vbo_position_pyramid;
var vbo_normal_pyramid;

var modelUniform;
var viewUniform;
var projectionUniform;

var laUniformZero;
var ldUniformZero;
var lsUniformZero;
var lightPositionUniformZero;

var laUniformOne;
var ldUniformOne;
var lsUniformOne;
var lightPositionUniformOne;

var kaUniform;
var kdUniform;
var ksUniform;
var materialShininessUniform;

var lKeyPressedUniform;

var anglePyramid = 0.0;
var gbLight = false;

var LightAmbientZero = [0.0, 0.0, 0.0];
var LightDiffuseZero = [1.0, 0.0, 0.0];
var LightSpecularZero = [1.0, 0.0, 0.0];
var LightPositionZero = [-2.0, 0.0, 0.0, 1.0];

var LightAmbientOne = [0.0, 0.0, 0.0];
var LightDiffuseOne = [0.0, 0.0, 1.0];
var LightSpecularOne = [0.0, 0.0, 1.0];
var LightPositionOne = [2.0, 0.0, 0.0, 1.0];

var MaterialAmbient = [0.0, 0.0, 0.0];
var MaterialDiffuse = [1.0, 1.0, 1.0];
var MaterialSpecular = [1.0, 1.0, 1.0];
var MaterialShininess = 128.0;

var perspectiveProjectionMatrix;

var requestAnimationFrame =
    window.requestAnimationFrame ||
    window.webkitRequestAnimationFrame ||
    window.mozRequestAnimationFrame ||
    window.oRequestAnimationFrame ||
    window.msRequestAnimationFrame;

var cancelAnimationFrame =
    window.cancelAnimationFrame ||
    window.webkitCancelRequestAnimationFrame ||
    window.webkitCancelAnimationFrame ||
    window.mozCancelRequestAnimationFrame ||
    window.mozCancelAnimationFrame ||
    window.oCancelRequestAnimationFrame ||
    window.oCancelAnimationFrame ||
    window.msCancelRequestAnimationFrame ||
    window.msCancelAnimationFrame;

function main() {
    //S1: Get canvas
    canvas = document.getElementById("AMC");
    if (!canvas) console.log("Obtaining Canvas failed!\n");
    else console.log("Obtaining Canvas successful!\n");

    canvas_original_width = canvas.width;
    canvas_original_height = canvas.height;

    console.log("Canvas width : " + canvas.width + "Canvas height :" + canvas.height);

    //register keybord and mouse event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);

    //initialize WebGL
    init();

    //start drawing and warming up
    resize();
    draw();
}

function toggleFullScreen() {
    var fullscreen_element =
        document.fullscreenElement ||
        document.webkitFullscreenElement ||
        document.mozFullScreenElement ||
        document.msFullscreenElement ||
        null;

    if (fullscreen_element == null) {
        if (canvas.requestFullscreen) canvas.requestFullscreen();
        else if (canvas.mozRequestFullScreen) canvas.mozRequestFullScreen();
        else if (canvas.webkitFullscreen) canvas.webkitFullscreen();
        else if (canvas.msRequestFullscreen) canvas.msRequestFullscreen();
        bFullscreen = true;
    } else {
        if (document.exitFullscreen) document.exitFullscreen();
        else if (document.mozCancelFullScreen) document.mozCancelFullScreen();
        else if (document.webkitExitFullscreen) document.webkitExitFullscreen();
        else if (document.msExitFullscreen) document.msExitFullscreen();
        bFullscreen = false;
    }
}

function init() {
    //S2: Get webgl 2.0 context
    gl = canvas.getContext("webgl2");
    if (!gl) {
        console.log("Failed to get rendering context for WebGL\n");
        return;
    } else console.log("Got rendering context for WebGL!\n");

    //set viewport width and height
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    //vertex shader
    var vertexShaderSourceCode =
        "#version 300 es" +
        "\n" +
        "precision mediump int;" +
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
        "if(u_lKeyPressed == 1)" +
        "{" +
        "vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" +
        "vec3 tNormal = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +
        "vec3 light_direction_zero = normalize(vec3(u_light_position_zero - eyeCoords));" +
        "vec3 light_direction_one = normalize(vec3(u_light_position_one - eyeCoords));" +
        "float tNorm_Dot_LightDirection_zero = max(dot(light_direction_zero, tNormal), 0.0);" +
        "float tNorm_Dot_LightDirection_one = max(dot(light_direction_one, tNormal), 0.0);" +
        "vec3 reflection_vector_zero = reflect(-light_direction_zero, tNormal);" +
        "vec3 reflection_vector_one = reflect(-light_direction_one, tNormal);" +
        "vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));" +
        "vec3 ambient_zero = u_la_zero * u_ka;" +
        "vec3 diffuse_zero = u_ld_zero * u_kd * tNorm_Dot_LightDirection_zero;" +
        "vec3 specular_zero = u_ls_zero * u_ks * pow(max(dot(reflection_vector_zero,viewer_vector), 0.0), u_material_shininess);" +
        "vec3 ambient_one = u_la_one * u_ka;" +
        "vec3 diffuse_one = u_ld_one * u_kd * tNorm_Dot_LightDirection_one;" +
        "vec3 specular_one = u_ls_one * u_ks * pow(max(dot(reflection_vector_one,viewer_vector), 0.0), u_material_shininess);" +
        "phong_ads_light = ambient_zero + ambient_one + diffuse_zero + diffuse_one + specular_zero + specular_one;" +
        "}" +
        "else" +
        "{" +
        "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
        "}" +
        "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
        "}";

    gVertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(gVertexShaderObject, vertexShaderSourceCode);
    gl.compileShader(gVertexShaderObject);
    if (gl.getShaderParameter(gVertexShaderObject, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(gVertexShaderObject);
        if (error.length > 0) {
            alert("\nVertex Shader compilation log : " + error);
            uninitialize();
        }
    }

    //fragment shader
    var fragmentShaderSourceCode =
        "#version 300 es" +
        "\n" +
        "precision highp float;" +
        "in vec3 phong_ads_light;" +
        "out vec4 FragColor;" +
        "void main(void)" +
        "{" +
        "   FragColor = vec4(phong_ads_light,1.0);" +
        "}";

    gFragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(gFragmentShaderObject, fragmentShaderSourceCode);
    gl.compileShader(gFragmentShaderObject);
    if (gl.getShaderParameter(gFragmentShaderObject, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(gFragmentShaderObject);
        if (error.length > 0) {
            alert("\nFragment Shader compilation log : " + error);
            uninitialize();
        }
    }

    //shader program
    gShaderProgramObject = gl.createProgram();
    gl.attachShader(gShaderProgramObject, gVertexShaderObject);
    gl.attachShader(gShaderProgramObject, gFragmentShaderObject);

    //pre-linking
    gl.bindAttribLocation(gShaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
    gl.bindAttribLocation(gShaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    //linking
    gl.linkProgram(gShaderProgramObject);
    if (gl.getProgramParameter(gShaderProgramObject, gl.LINK_STATUS) == false) {
        var error = gl.getProgramInfoLog(gShaderProgramObject);
        if (error.length > 0) {
            alert("\nProgram linking log : " + error);
            uninitialize();
        }
    }

    //get uniform location
    viewUniform = gl.getUniformLocation(gShaderProgramObject, "u_view_matrix");

    modelUniform = gl.getUniformLocation(gShaderProgramObject, "u_model_matrix");

    projectionUniform = gl.getUniformLocation(gShaderProgramObject, "u_projection_matrix");

    lKeyPressedUniform = gl.getUniformLocation(gShaderProgramObject, "u_lKeyPressed");

    laUniformZero = gl.getUniformLocation(gShaderProgramObject, "u_la_zero");

    ldUniformZero = gl.getUniformLocation(gShaderProgramObject, "u_ld_zero");

    lsUniformZero = gl.getUniformLocation(gShaderProgramObject, "u_ls_zero");

    lightPositionUniformZero = gl.getUniformLocation(gShaderProgramObject, "u_light_position_zero");

    kaUniform = gl.getUniformLocation(gShaderProgramObject, "u_ka");

    kdUniform = gl.getUniformLocation(gShaderProgramObject, "u_kd");

    ksUniform = gl.getUniformLocation(gShaderProgramObject, "u_ks");

    laUniformOne = gl.getUniformLocation(gShaderProgramObject, "u_la_one");

    ldUniformOne = gl.getUniformLocation(gShaderProgramObject, "u_ld_one");

    lsUniformOne = gl.getUniformLocation(gShaderProgramObject, "u_ls_one");

    lightPositionUniformOne = gl.getUniformLocation(gShaderProgramObject, "u_light_position_one");

    materialShininessUniform = gl.getUniformLocation(gShaderProgramObject, "u_material_shininess");

    //vertices, color, texture, vao, vbo , shader attribs
    var pyramidVertices = new Float32Array([
        -1.0,
        -1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        0.0,
        1.0,
        0.0,
        1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        -1.0,
        -1.0,
        -1.0
    ]);

    var pyramidNormals = new Float32Array([
        0.0,
        0.447214,
        0.894427,
        0.0,
        0.447214,
        0.894427,
        0.0,
        0.447214,
        0.894427,
        0.894427,
        0.447214,
        0.0,
        0.894427,
        0.447214,
        0.0,
        0.894427,
        0.447214,
        0.0,
        0.0,
        0.447214,
        -0.894427,
        0.0,
        0.447214,
        -0.894427,
        0.0,
        0.447214,
        -0.894427,
        -0.894427,
        0.447214,
        0.0,
        -0.894427,
        0.447214,
        0.0,
        -0.894427,
        0.447214,
        0.0
    ]);

    //create vao
    //triangle
    vao_pyramid = gl.createVertexArray();
    gl.bindVertexArray(vao_pyramid);

    //create vbo
    vbo_position_pyramid = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_pyramid);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_normal_pyramid = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_normal_pyramid);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidNormals, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_NORMAL);

    //unbind vbo and vao
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //depth
    gl.enable(gl.DEPTH_TEST);

    //gl.enable(gl.CULL_FACE);

    //set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();
}

function resize() {
    if (bFullscreen == true) {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    } else {
        canvas.width = canvas_original_width;
        canvas.height = canvas_original_height;
    }

    gl.viewport(0, 0, canvas.width, canvas.height);

    //perspective
    mat4.perspective(
        perspectiveProjectionMatrix,
        45.0,
        parseFloat(canvas.width) / parseFloat(canvas.height),
        0.1,
        100.0
    );
}

function draw() {
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(gShaderProgramObject);

    //matrices
    var modelMatrix = mat4.create();
    var viewMatrix = mat4.create();
    var projectionMatrix = mat4.create();

    //triangle
    //identity
    mat4.identity(modelMatrix);
    mat4.identity(viewMatrix);
    mat4.identity(projectionMatrix);

    //transformations
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.0]);
    mat4.rotateY(modelMatrix, modelMatrix, degToRad(anglePyramid));

    mat4.multiply(projectionMatrix, perspectiveProjectionMatrix, projectionMatrix);

    gl.uniformMatrix4fv(modelUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionUniform, false, projectionMatrix);

    if (gbLight == true) {
        gl.uniform1i(lKeyPressedUniform, 1);
        gl.uniform3fv(laUniformZero, LightAmbientZero);
        gl.uniform3fv(ldUniformZero, LightDiffuseZero);
        gl.uniform3fv(lsUniformZero, LightSpecularZero);
        gl.uniform3fv(kaUniform, MaterialAmbient);
        gl.uniform3fv(kdUniform, MaterialDiffuse);
        gl.uniform3fv(ksUniform, MaterialSpecular);
        gl.uniform1f(materialShininessUniform, MaterialShininess);
        gl.uniform4fv(lightPositionUniformZero, LightPositionZero);

        gl.uniform3fv(laUniformOne, LightAmbientOne);
        gl.uniform3fv(ldUniformOne, LightDiffuseOne);
        gl.uniform3fv(lsUniformOne, LightSpecularOne);
        gl.uniform3fv(kaUniform, MaterialAmbient);
        gl.uniform3fv(kdUniform, MaterialDiffuse);
        gl.uniform3fv(ksUniform, MaterialSpecular);
        gl.uniform1f(materialShininessUniform, MaterialShininess);
        gl.uniform4fv(lightPositionUniformOne, LightPositionOne);
    } else {
        gl.uniform1i(lKeyPressedUniform, 0);
    }

    gl.bindVertexArray(vao_pyramid);

    gl.drawArrays(gl.TRIANGLES, 0, 12);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    //animation loop
    update();
    requestAnimationFrame(draw, canvas);
}

function update() {
    anglePyramid = anglePyramid + 1.0;
    if (anglePyramid >= 360.0) {
        anglePyramid = 0.0;
    }
}

function uninitialize() {
    if (vao_pyramid) {
        gl.deleteVertexArray(vao_pyramid);
        vao_pyramid = null;
    }

    if (vbo_position_pyramid) {
        gl.deleteBuffer(vbo_position_pyramid);
        vbo_position_pyramid = null;
    }

    if (vbo_normal_pyramid) {
        gl.deleteBuffer(vbo_normal_pyramid);
        vbo_normal_pyramid = null;
    }

    if (gShaderProgramObject) {
        if (gFragmentShaderObject) {
            gl.detachShader(gShaderProgramObject, gFragmentShaderObject);
            gl.deleteShader(gFragmentShaderObject);
            gFragmentShaderObject = null;
        }
        if (gVertexShaderObject) {
            gl.detachShader(gShaderProgramObject, gVertexShaderObject);
            gl.deleteShader(gVertexShaderObject);
            gVertexShaderObject = null;
        }

        gl.deleteProgram(gShaderProgramObject);
        gShaderProgramObject = null;
    }
}

function keyDown(event) {
    switch (event.keyCode) {
        case 27:
            uninitialize();
            window.close();
            break;

        case 76:
            if (gbLight == false) {
                gbLight = true;
            } else {
                gbLight = false;
            }
            break;

        case 70:
            toggleFullScreen();
            break;
    }
}

function mouseDown() {}

function degToRad(degrees) {
    var d = Math.PI / 180;
    return degrees * d;
}
