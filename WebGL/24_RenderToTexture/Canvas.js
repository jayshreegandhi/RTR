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

var vao_cube;
var vbo_position_cube;
var vbo_texture_cube;

var mvpUniform;
var samplerUniform;

var perspectiveProjectionMatrix;

var texture_stone = 0;

var angleCube = 0.0;

//frame buffer
var fbo;
var rbo;
var cubeTexture;
var cubeDepth;
var gWidth;
var gHeight;

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

    gWidth = canvas.width;
    gHeight = canvas.height;

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
        "in vec4 vPosition;" +
        "in vec2 vTexCoord;" +
        "uniform mat4 u_mvp_matrix;" +
        "out vec2 out_texcoord;" +
        "void main(void)" +
        "{" +
        "   gl_Position = u_mvp_matrix * vPosition;" +
        "   out_texcoord = vTexCoord;" +
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
        "in vec2 out_texcoord;" +
        "uniform highp sampler2D u_sampler;" +
        "out vec4 FragColor;" +
        "void main(void)" +
        "{" +
        "   FragColor = texture(u_sampler, out_texcoord);" +
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
    gl.bindAttribLocation(gShaderProgramObject, WebGLMacros.AMC_ATTRIBUTE_TEXTURE0, "vTexCoord");

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
    mvpUniform = gl.getUniformLocation(gShaderProgramObject, "u_mvp_matrix");
    samplerUniform = gl.getUniformLocation(gShaderProgramObject, "u_sampler");

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

    var cubeTexCoord = new Float32Array([
        0.0,
        0.0,
        1.0,
        0.0,
        1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        1.0,
        1.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0,
        0.0,
        1.0,
        1.0,
        0.0,
        1.0
    ]);

    //create vao
    //cube
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

    vbo_texture_cube = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture_cube);
    gl.bufferData(gl.ARRAY_BUFFER, cubeTexCoord, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXTURE0);

    //unbind vbo and vao
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //frame buffer
    fbo = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    cubeTexture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
    gl.texStorage2D(gl.TEXTURE_2D, 9, gl.RGBA8, 1536, 1536);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.bindTexture(gl.TEXTURE_2D, null);

    cubeDepth = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, cubeDepth);
    gl.texStorage2D(gl.TEXTURE_2D, 9, gl.DEPTH_COMPONENT32F, 1536, 1536);

    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, cubeTexture, 0);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, cubeDepth, 0);

    gl.drawBuffers([gl.COLOR_ATTACHMENT0]);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    //load texture
    texture_stone = loadTexture("stone.png");
    if (!texture_stone) {
        console.log("Failed to load texture stone");
    }

    //depth
    gl.enable(gl.DEPTH_TEST);

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
    var gray = new Float32Array([0.25, 0.25, 0.25, 1.0]);
    var one_depth = 1.0;

    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);

    gl.clearBufferfv(gl.COLOR, 0, gray);
    gl.clearBufferfi(gl.DEPTH_STENCIL, 0, one_depth, 0);

    gl.viewport(0, 0, 1536, 1536);

    gl.useProgram(gShaderProgramObject);

    //matrices
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //cube
    //identity
    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    //transformations
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
    mat4.scale(modelViewMatrix, modelViewMatrix, [0.75, 0.75, 0.75]);
    mat4.rotateX(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.rotateZ(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.scale(modelViewMatrix, modelViewMatrix, [0.75, 0.75, 0.75]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindTexture(gl.TEXTURE_2D, texture_stone);
    gl.uniform1i(samplerUniform, 0);

    gl.bindVertexArray(vao_cube);

    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    gl.viewport(0, 0, canvas.width, canvas.height);

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(gShaderProgramObject);
    //matrices
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();

    //cube
    //identity
    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    //transformations
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -4.0]);
    mat4.scale(modelViewMatrix, modelViewMatrix, [0.75, 0.75, 0.75]);
    mat4.rotateX(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.rotateY(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.rotateZ(modelViewMatrix, modelViewMatrix, degToRad(angleCube));
    mat4.scale(modelViewMatrix, modelViewMatrix, [0.75, 0.75, 0.75]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
    gl.uniform1i(samplerUniform, 0);

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
    update();
    requestAnimationFrame(draw, canvas);
}

function update() {

    angleCube = angleCube - 1.0;
    if (angleCube <= -360.0) {
        angleCube = 0.0;
    }
}

function uninitialize() {

    if (fbo) {
        gl.deleteFramebuffer(fbo);
        fbo = 0;
    }

    if (texture_stone) {
        gl.deleteTexture(texture_stone);
        texture_stone = 0;
    }

    if (cubeTexture) {
        gl.deleteTexture(cubeTexture);
        cubeTexture = 0;
    }

    if (cubeDepth) {
        gl.deleteTexture(cubeDepth);
        cubeDepth = 0;
    }

    if (vao_cube) {
        gl.deleteVertexArray(vao_cube);
        vao_cube = null;
    }

    if (vbo_position_cube) {
        gl.deleteBuffer(vbo_position_cube);
        vbo_position_cube = null;
    }

    if (vbo_texture_cube) {
        gl.deleteBuffer(vbo_texture_cube);
        vbo_texture_cube = null;
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

        case 70:
            toggleFullScreen();
            break;
    }
}

function mouseDown() { }

function degToRad(degrees) {
    var d = Math.PI / 180;
    return degrees * d;
}

function loadTexture(imageResource) {
    let texObj = 0;

    texObj = gl.createTexture();
    texObj.image = new Image();
    texObj.image.src = imageResource;
    texObj.image.onload = function () {
        gl.bindTexture(gl.TEXTURE_2D, texObj);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texObj.image);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.bindTexture(gl.TEXTURE_2D, null);
    };

    return texObj;
}
