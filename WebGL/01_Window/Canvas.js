// onload function

function main() {
    //S1: Get canvas
    var canvas = document.getElementById("AMC");
    if (!canvas) console.log("Obtaining Canvas failed!\n");
    else console.log("Obtaining Canvas successful!\n");

    console.log("Canvas width : " + canvas.width + "Canvas height :" + canvas.height);

    //S2: Get 2D context
    var context = canvas.getContext("2d");
    if (!context) console.log("Obtaining 2D context failed!\n");
    else console.log("Obtaining 2D context successful!\n");

    //S3:Fill contex/canvas with black color
    context.fillStyle = "black";
    context.fillRect(0, 0, canvas.width, canvas.height);

    //S4:Align text to middle
    context.textAlign = "center";
    context.textBaseline = "middle";

    //S6:set string and font for the string
    var str = "Hello World !!!";

    context.font = "48px sans-serif";

    context.fillStyle = "green";

    //S7: Display the text
    context.fillText(str, canvas.width / 2, canvas.height / 2);
}
