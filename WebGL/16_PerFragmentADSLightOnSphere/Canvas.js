// onload function
var jgg_canvas = null;
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

var vao_sphere;

var modelUniform;
var viewUniform;
var projectionUniform;

var laUniform;
var ldUniform;
var lsUniform;
var lightPositionUniform;

var kaUniform;
var kdUniform;
var ksUniform;
var materialShininessUniform;

var lKeyPressedUniform;

var LightAmbient = [0.0, 0.0, 0.0];
var LightDiffuse = [1.0, 1.0, 1.0];
var LightSpecular = [1.0, 1.0, 1.0];
var LightPosition = [100.0, 100.0, 100.0, 1.0];

var MaterialAmbient = [0.0, 0.0, 0.0];
var MaterialDiffuse = [1.0, 1.0, 1.0];
var MaterialSpecular = [1.0, 1.0, 1.0];
var MaterialShininess = 128.0;

var gbLight = false;

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
    jgg_canvas = document.getElementById("AMC");
    if (!jgg_canvas) console.log("Obtaining Canvas failed!\n");
    else console.log("Obtaining Canvas successful!\n");

    canvas_original_width = jgg_canvas.width;
    canvas_original_height = jgg_canvas.height;

    console.log("Canvas width : " + jgg_canvas.width + "Canvas height :" + jgg_canvas.height);

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
        if (jgg_canvas.requestFullscreen) jgg_canvas.requestFullscreen();
        else if (jgg_canvas.mozRequestFullScreen) jgg_canvas.mozRequestFullScreen();
        else if (jgg_canvas.webkitFullscreen) jgg_canvas.webkitFullscreen();
        else if (jgg_canvas.msRequestFullscreen) jgg_canvas.msRequestFullscreen();
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
    gl = jgg_canvas.getContext("webgl2");
    if (!gl) {
        console.log("Failed to get rendering context for WebGL\n");
        return;
    } else console.log("Got rendering context for WebGL!\n");

    //set viewport width and height
    gl.viewportWidth = jgg_canvas.width;
    gl.viewportHeight = jgg_canvas.height;

    //vertex shader
    var vertexShaderSourceCode =
        "#version 300 es" +
        "\n" +
        "precision mediump int;" +
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
        "vec3 phong_ads_light;" +
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

    laUniform = gl.getUniformLocation(gShaderProgramObject, "u_la");
    ldUniform = gl.getUniformLocation(gShaderProgramObject, "u_ld");
    lsUniform = gl.getUniformLocation(gShaderProgramObject, "u_ls");
    lightPositionUniform = gl.getUniformLocation(gShaderProgramObject, "u_light_position");

    kaUniform = gl.getUniformLocation(gShaderProgramObject, "u_ka");
    kdUniform = gl.getUniformLocation(gShaderProgramObject, "u_kd");
    ksUniform = gl.getUniformLocation(gShaderProgramObject, "u_ks");
    materialShininessUniform = gl.getUniformLocation(gShaderProgramObject, "u_material_shininess");

    //vertices, color, texture, vao, vbo , shader attribs
    vao_sphere = new Mesh();
    makeSphere(vao_sphere, 2.0, 30, 30);

    //depth
    gl.enable(gl.DEPTH_TEST);

    //gl.enable(gl.CULL_FACE);

    //set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjectionMatrix = mat4.create();
}

function resize() {
    if (bFullscreen == true) {
        jgg_canvas.width = window.innerWidth;
        jgg_canvas.height = window.innerHeight;
    } else {
        jgg_canvas.width = canvas_original_width;
        jgg_canvas.height = canvas_original_height;
    }

    gl.viewport(0, 0, jgg_canvas.width, jgg_canvas.height);

    //perspective
    mat4.perspective(
        perspectiveProjectionMatrix,
        45.0,
        parseFloat(jgg_canvas.width) / parseFloat(jgg_canvas.height),
        0.1,
        100.0
    );
}

function draw() {
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(gShaderProgramObject);

    //matrices
    var viewMatrix = mat4.create();
    var modelMatrix = mat4.create();
    var projectionMatrix = mat4.create();

    //identity
    mat4.identity(viewMatrix);
    mat4.identity(modelMatrix);
    mat4.identity(projectionMatrix);

    //transformations
    mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -6.0]);
    mat4.multiply(projectionMatrix, perspectiveProjectionMatrix, projectionMatrix);

    gl.uniformMatrix4fv(modelUniform, false, modelMatrix);
    gl.uniformMatrix4fv(viewUniform, false, viewMatrix);
    gl.uniformMatrix4fv(projectionUniform, false, projectionMatrix);

    if (gbLight == true) {
        gl.uniform1i(lKeyPressedUniform, 1);
        gl.uniform3fv(laUniform, LightAmbient);
        gl.uniform3fv(ldUniform, LightDiffuse);
        gl.uniform3fv(lsUniform, LightSpecular);
        gl.uniform3fv(kaUniform, MaterialAmbient);
        gl.uniform3fv(kdUniform, MaterialDiffuse);
        gl.uniform3fv(ksUniform, MaterialSpecular);
        gl.uniform1f(materialShininessUniform, MaterialShininess);
        gl.uniform4fv(lightPositionUniform, LightPosition);
    } else {
        gl.uniform1i(lKeyPressedUniform, 0);
    }

    vao_sphere.draw();

    gl.useProgram(null);

    //animation loop

    requestAnimationFrame(draw, jgg_canvas);
}

function update() {}

function uninitialize() {
    if (vao_sphere) {
        vao_sphere.deallocate();
        vao_sphere = null;
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
