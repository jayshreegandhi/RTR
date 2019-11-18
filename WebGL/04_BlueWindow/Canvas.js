// onload function
var canvas = null;
var gl = null;
var bFullscreen = false;
var canvas_original_width;
var canvas_original_height;

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

  //set clear color
  gl.clearColor(0.0, 0.0, 1.0, 1.0);
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
}

function draw() {
  gl.clear(gl.COLOR_BUFFER_BIT);

  //animation loop
  requestAnimationFrame(draw, canvas);
}

function keyDown(event) {
  switch (event.keyCode) {
    case 70:
      toggleFullScreen();
      break;
  }
}

function mouseDown() {}
