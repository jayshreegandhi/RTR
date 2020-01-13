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

var vao_rect;

var vbo_position_rect;
var vbo_texture_rect;

var mvpUniform;
var samplerUniform;

var texture_smile = 0;

var perspectiveProjectionMatrix;

var rectTexCoord = [];
var keypressed = 0;

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
    texture_smile = gl.createTexture();
    texture_smile.image = new Image();
    texture_smile.image.src = "Smile.png";
    texture_smile.image.onload = function() {
        gl.bindTexture(gl.TEXTURE_2D, texture_smile);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture_smile.image);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.bindTexture(gl.TEXTURE_2D, null);
    };
    //get uniform location
    mvpUniform = gl.getUniformLocation(gShaderProgramObject, "u_mvp_matrix");
    samplerUniform = gl.getUniformLocation(gShaderProgramObject, "u_sampler");

    //vertices, color, texture, vao, vbo , shader attribs
    var rectVertices = new Float32Array([1.0, 1.0, 0.0, -1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0]);

    //create vao

    //rectangle
    vao_rect = gl.createVertexArray();
    gl.bindVertexArray(vao_rect);

    //create vbo
    vbo_position_rect = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_position_rect);
    gl.bufferData(gl.ARRAY_BUFFER, rectVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vbo_texture_rect = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture_rect);
    gl.bufferData(gl.ARRAY_BUFFER, rectTexCoord, gl.DYNAMIC_DRAW);
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
    mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

    //gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture_smile);
    gl.uniform1i(samplerUniform, 0);

    gl.bindVertexArray(vao_rect);

    if (keypressed == 1) {
        rectTexCoord = new Float32Array([0.5, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0]);
    } else if (keypressed == 2) {
        rectTexCoord = new Float32Array([1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0]);
    } else if (keypressed == 3) {
        rectTexCoord = new Float32Array([2.0, 2.0, 0.0, 2.0, 0.0, 0.0, 2.0, 0.0]);
    } else if (keypressed == 4) {
        rectTexCoord = new Float32Array([0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5]);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, vbo_texture_rect);
    gl.bufferData(gl.ARRAY_BUFFER, rectTexCoord, gl.DYNAMIC_DRAW);
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
    if (texture_smile) {
        gl.deleteTexture(texture_smile);
        texture_smile = 0;
    }

    if (vao_rect) {
        gl.deleteVertexArray(vao_rect);
        vao_rect = null;
    }

    if (vbo_position_rect) {
        gl.deleteBuffer(vbo_position_rect);
        vbo_position_rect = null;
    }

    if (vbo_texture_rect) {
        gl.deleteBuffer(vbo_texture_rect);
        vbo_texture_rect = null;
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

        case 49:
            keypressed = 1;
            break;

        case 50:
            keypressed = 2;
            break;

        case 51:
            keypressed = 3;
            break;

        case 52:
            keypressed = 4;
            break;

        case 70:
            toggleFullScreen();
            break;

        default:
            keypressed = 0;
            break;
    }
}

function mouseDown() {}

function degToRad(degrees) {
    var d = Math.PI / 180;
    return degrees * d;
}
