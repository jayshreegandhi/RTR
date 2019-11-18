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
var vbo;

var mvpUniform;

var orthographicProjectionMatrix;

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

  console.log(
    "Canvas width : " + canvas.width + "Canvas height :" + canvas.height
  );

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
    "uniform mat4 u_mvp_matrix;" +
    "void main(void)" +
    "{" +
    "gl_Position = u_mvp_matrix * vPosition;" +
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
    "out vec4 FragColor;" +
    "void main(void)" +
    "{" +
    "FragColor = vec4(1.0, 1.0, 1.0, 1.0);" +
    "}";

  gFragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
  gl.shaderSource(gFragmentShaderObject, fragmentShaderSourceCode);
  gl.compileShader(gFragmentShaderObject);
  if (
    gl.getShaderParameter(gFragmentShaderObject, gl.COMPILE_STATUS) == false
  ) {
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
  gl.bindAttribLocation(
    gShaderProgramObject,
    WebGLMacros.AMC_ATTRIBUTE_POSITION,
    "vPosition"
  );

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

  //vertices, color, texture, vao, vbo , shader attribs
  var triangleVertices = new Float32Array([
    0.0,
    50.0,
    0.0,
    -50.0,
    -50.0,
    0.0,
    50.0,
    -50.0,
    0.0
  ]);

  //create vao
  vao = gl.createVertexArray();
  gl.bindVertexArray(vao);

  //create vbo
  vbo = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
  gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
  gl.vertexAttribPointer(
    WebGLMacros.AMC_ATTRIBUTE_POSITION,
    3,
    gl.FLOAT,
    false,
    0,
    0
  );
  gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);

  //unbind vbo and vao
  gl.bindBuffer(gl.ARRAY_BUFFER, null);
  gl.bindVertexArray(null);

  //set clear color
  gl.clearColor(0.0, 0.0, 1.0, 1.0);

  orthographicProjectionMatrix = mat4.create();
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

  //orthographic
  if (canvas.width <= canvas.height) {
    mat4.ortho(
      orthographicProjectionMatrix,
      -100.0,
      100.0,
      -100.0 * (canvas.height / canvas.width),
      100.0 * (canvas.height / canvas.width),
      -100.0,
      100.0
    );
  } else {
    mat4.ortho(
      orthographicProjectionMatrix,
      -100.0 * (canvas.width / canvas.height),
      100.0 * (canvas.width / canvas.height),
      -100.0,
      100.0,
      -100.0,
      100.0
    );
  }
}

function draw() {
  gl.clear(gl.COLOR_BUFFER_BIT);

  gl.useProgram(gShaderProgramObject);

  var modelViewMatrix = mat4.create();
  var modelViewProjectionMatrix = mat4.create();

  mat4.multiply(
    modelViewProjectionMatrix,
    orthographicProjectionMatrix,
    modelViewMatrix
  );

  gl.uniformMatrix4fv(mvpUniform, false, modelViewProjectionMatrix);

  gl.bindVertexArray(vao);

  gl.drawArrays(gl.TRIANGLES, 0, 3);

  gl.bindVertexArray(null);

  gl.useProgram(null);

  //animation loop
  requestAnimationFrame(draw, canvas);
}

function uninitialize() {
  if (vao) {
    gl.deleteVertexArray(vao);
    vao = null;
  }

  if (vbo) {
    gl.deleteBuffer(vbo);
    vbo = null;
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
