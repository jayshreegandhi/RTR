// onload function
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

var angleCube = 0.0;

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION: 0,
    AMC_ATTRIBUTE_COLOR: 1,
    AMC_ATTRIBUTE_NORMAL: 2,
    AMC_ATTRIBUTE_TEXTURE0: 3
};

var gShaderProgramObject;
var gVertexShaderObject;
var gFragmentShaderObject;

var vao_cube;
var vbo_position_cube;
var vbo_normal_cube;

var mvUniform;
var pUniform;
var ldUniform;
var kdUniform;
var lightPositionUniform;
var lKeyPressedUniform;

var gbAnimation = false;
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
        "uniform mat4 u_mv_matrix;" +
        "uniform mat4 u_p_matrix;" +
        "uniform int u_lKeyPressed;" +
        "uniform vec3 u_ld;" +
        "uniform vec3 u_kd;" +
        "uniform vec4 u_light_position;" +
        "out vec3 diffuse_color;" +
        "void main(void)" +
        "{" +
        "   if(u_lKeyPressed == 1)" +
        "   {" +
        "       vec4 eyeCoords = u_mv_matrix * vPosition;" +
        "       mat3 normal_matrix = mat3(transpose(inverse(u_mv_matrix)));" +
        "       vec3 tNormal = normalize(normal_matrix * vNormal);" +
        "       vec3 s = normalize(vec3(u_light_position - eyeCoords));" +
        "       diffuse_color = u_ld * u_kd * max(dot(s, tNormal), 0.0);" +
        "   }" +
        "   gl_Position = u_p_matrix * u_mv_matrix * vPosition;" +
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
        "uniform int u_lKeyPressed;" +
        "in vec3 diffuse_color;" +
        "out vec4 FragColor;" +
        "void main(void)" +
        "{" +
        "   if(u_lKeyPressed == 1)" +
        "   {" +
        "       FragColor = vec4(diffuse_color,1.0);" +
        "   }" +
        "   else" +
        "   {" +
        "       FragColor = vec4(1.0, 1.0, 1.0, 1.0);" +
        "   }" +
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
    mvUniform = gl.getUniformLocation(gShaderProgramObject, "u_mv_matrix");
    pUniform = gl.getUniformLocation(gShaderProgramObject, "u_p_matrix");
    lKeyPressedUniform = gl.getUniformLocation(gShaderProgramObject, "u_lKeyPressed");

    ldUniform = gl.getUniformLocation(gShaderProgramObject, "u_ld");

    kdUniform = gl.getUniformLocation(gShaderProgramObject, "u_kd");

    lightPositionUniform = gl.getUniformLocation(gShaderProgramObject, "u_light_position");

    //vertices, color, texture, vao, vbo , shader attribs
    var cubeVertices = new Float32Array([
        1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        1.0,
        1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        -1.0,
        -1.0,
        -1.0,
        -1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        1.0,
        1.0,
        1.0,
        1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        -1.0,
        -1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        1.0,
        -1.0,
        -1.0,
        1.0,
        -1.0,
        -1.0,
        -1.0
    ]);

    var cubeNormals = new Float32Array([
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,

        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,

        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,

        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,

        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,

        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0,
        -1.0,
        0.0,
        0.0
    ]);

    //create vao
    //rectangle
    vao_cube = gl.createVertexArray();
    gl.bindVertexArray(vao_cube);

    //create vbo
    vbo_position_cube = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_cube);
    gl.bufferData(gl.ARRAY_BUFFER, cubeVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_normal_cube = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_normal_cube);
    gl.bufferData(gl.ARRAY_BUFFER, cubeNormals, gl.STATIC_DRAW);
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
    var modelViewMatrix = mat4.create();
    var projectionMatrix = mat4.create();

    //var modelViewProjectionMatrix = mat4.create();

    //triangle
    //identity
    mat4.identity(modelViewMatrix);
    mat4.identity(projectionMatrix);

    //transformations
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
    mat4.rotateX(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.rotateZ(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.scale(modelViewMatrix, modelViewMatrix, [0.75, 0.75, 0.75]);
    mat4.multiply(projectionMatrix, perspectiveProjectionMatrix, projectionMatrix);

    gl.uniformMatrix4fv(mvUniform, false, modelViewMatrix);
    gl.uniformMatrix4fv(pUniform, false, projectionMatrix);

    if (gbLight == true) {
        gl.uniform1i(lKeyPressedUniform, 1);
        gl.uniform3f(ldUniform, 1.0, 1.0, 1.0);
        gl.uniform3f(kdUniform, 0.5, 0.5, 0.5);
        gl.uniform4f(lightPositionUniform, 0.0, 0.0, 2.0, 1.0);
    } else {
        gl.uniform1i(lKeyPressedUniform, 0);
    }

    gl.bindVertexArray(vao_cube);

    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    //animation loop
    if (gbAnimation == true) {
        update();
    }
    requestAnimationFrame(draw, canvas);
}

function update() {
    angleCube = angleCube - 1.0;
    if (angleCube <= -360.0) {
        angleCube = 0.0;
    }
}

function uninitialize() {
    if (vao_cube) {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    if (vbo_position_cube) {
        gl.deleteBuffer(vbo_position_cube);
        vbo = null;
    }

    if (vbo_normal_cube) {
        gl.deleteBuffer(vbo_normal_cube);
        vbo_normal_cube = null;
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

        case 65:
            if (gbAnimation == false) {
                gbAnimation = true;
            } else {
                gbAnimation = false;
            }
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
