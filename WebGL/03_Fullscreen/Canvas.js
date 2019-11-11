// onload function
var canvas = null;
var context = null;

function main() {
    //S1: Get canvas
    canvas = document.getElementById("AMC");
    if (!canvas) console.log("Obtaining Canvas failed!\n");
    else console.log("Obtaining Canvas successful!\n");

    console.log("Canvas width : " + canvas.width + "Canvas height :" + canvas.height);

    //S2: Get 2D context
    context = canvas.getContext("2d");
    if (!context) console.log("Obtaining 2D context failed!\n");
    else console.log("Obtaining 2D context successful!\n");

    //S3:Fill contex/canvas with black color
    context.fillStyle = "black";
    context.fillRect(0, 0, canvas.width, canvas.height);

    drawText("Hello World !!!");

    //register keybord and mouse event handler
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
}

function drawText(text) {
    //S4:Align text to middle
    context.textAlign = "center";
    context.textBaseline = "middle";

    context.font = "48px sans-serif";

    context.fillStyle = "green";

    //S7: Display the text
    context.fillText(text, canvas.width / 2, canvas.height / 2);
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
    } else {
        if (document.exitFullscreen) document.exitFullscreen();
        else if (document.mozCancelFullScreen) document.mozCancelFullScreen();
        else if (document.webkitExitFullscreen) document.webkitExitFullscreen();
        else if (document.msExitFullscreen) document.msExitFullscreen();
    }
}

function keyDown(event) {
    switch (event.keyCode) {
        case 70:
            toggleFullScreen();
            drawText("Hello World !!!");
            break;
    }
}

function mouseDown() {}
