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

var vao;

var vbo_position;
var vbo_texture;

var mvpUniform;
var samplerUniform;

var check_image_width = 64;
var check_image_height = 64;

var texImage = 0;
var checkImage = null;

var rectVertices = [];

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
        "in vec4 vPosition;" +
        "in vec2 vTexCoord;" +
        "uniform mat4 u_mvp_matrix;" +
        "out vec2 out_texcoord;" +
        "void main(void)" +
        "{" +
        "gl_Position = u_mvp_matrix * vPosition;" +
        "out_texcoord = vTexCoord;" +
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
    //load texure
    texImage = gl.createTexture();
    texImage.image = new Image();
    texImage.image.src = "Smile.png";
    texImage.image.onload = function() {
        gl.bindTexture(gl.TEXTURE_2D, texImage);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texImage.image);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.bindTexture(gl.TEXTURE_2D, null);
    };
    //get uniform location
    mvpUniform = gl.getUniformLocation(gShaderProgramObject, "u_mvp_matrix");
    samplerUniform = gl.getUniformLocation(gShaderProgramObject, "u_sampler");

    //vertices, color, texture, vao, vbo , shader attribs
    //var rectVertices = new Float32Array([1.0, 1.0, 0.0, -1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0]);

    var rectTexCoord = new Float32Array([0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0]);

    texImage = loadTexture();

    //create vao

    //rectangle
    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);

    //create vbo
    vbo_position = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
    gl.bufferData(gl.ARRAY_BUFFER, rectVertices, gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_texture = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture);
    gl.bufferData(gl.ARRAY_BUFFER, rectTexCoord, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXTURE0);

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
    var modelViewProjectionMatrix = mat4.create();

    //triangle
    //identity
    mat4.identity(modelViewMatrix);
    mat4.identity(modelViewProjectionMatrix);

    //transformations
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.6]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    //gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texImage);
    gl.uniform1i(samplerUniform, 0);

    gl.bindVertexArray(vao);

    rectVertices = new Float32Array([-2.0, -1.0, 0.0, -2.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0]);

    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
    gl.bufferData(gl.ARRAY_BUFFER, rectVertices, gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

    rectVertices = new Float32Array([1.0, -1.0, 0.0, 1.0, 1.0, 0.0, 2.41421, 1.0, -1.41421, 2.41421, -1.0, -1.41421]);

    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position);
    gl.bufferData(gl.ARRAY_BUFFER, rectVertices, gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    //animation loop
    update();
    requestAnimationFrame(draw, canvas);
}

function update() {}

function uninitialize() {
    if (texImage) {
        gl.deleteTexture(texImage);
        texImage = 0;
    }

    if (vao) {
        gl.deleteVertexArray(vao);
        vao = null;
    }

    if (vbo_position) {
        gl.deleteBuffer(vbo_position);
        vbo_position = null;
    }

    if (vbo_texture) {
        gl.deleteBuffer(vbo_texture);
        vbo_texture = null;
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

function mouseDown() {}

function degToRad(degrees) {
    var d = Math.PI / 180;
    return degrees * d;
}

function loadTexture() {
    makeCheckImage();

    var imageData = new ImageData(checkImage, check_image_width, check_image_height);

    var texObj;

    texObj = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texObj);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    gl.texImage2D(
        gl.TEXTURE_2D,
        0,
        gl.RGBA,
        check_image_width,
        check_image_height,
        0,
        gl.RGBA,
        gl.UNSIGNED_BYTE,
        imageData
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.bindTexture(gl.TEXTURE_2D, null);

    return texObj;
}

function makeCheckImage() {
    var c = 0;
    texturedata = new Array(check_image_width * check_image_height * 4);

    for (var i = 0; i < check_image_height; i++) {
        for (var j = 0; j < check_image_width; j++) {
            for (var k = 0; k < 4; k++) {
                c = ((i & 0x8) == 0) ^ ((j & 0x8) == 0) || k === 3;
                texturedata[i * check_image_height * 4 + j * 4 + k] = 255 * c;
            }
        }
    }

    checkImage = new Uint8ClampedArray(texturedata);
}
