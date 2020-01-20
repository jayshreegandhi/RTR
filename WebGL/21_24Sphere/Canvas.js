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
var LightPosition = [0.0, 0.0, 0.0, 1.0];

var MaterialAmbient;
var MaterialDiffuse;
var MaterialSpecular;
var MaterialShininess = [];

var gbLight = false;
var rotationAngleX = 0.0;
var rotationAngleY = 0.0;
var rotationAngleZ = 0.0;
var keyPress = 0;

var perspectiveProjectionMatrix;

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

    MaterialAmbient = create2DArray(24);
    MaterialDiffuse = create2DArray(24);
    MaterialSpecular = create2DArray(24);
    MaterialShininess = create2DArray(24);

    setMaterialSphere();

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

    var wid = jgg_canvas.width / 4;
    var ht = jgg_canvas.height / 6;

    for (var x = 0; x < 4; x++) {
        for (var y = 0; y < 6; y++) {
            setViewports(x * wid, y * ht, wid, ht);

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

                if (keyPress == 1) {
                    LightPosition[0] = 0.0;
                    LightPosition[1] = Math.cos(rotationAngleX);
                    LightPosition[2] = Math.sin(rotationAngleX);
                    gl.uniform4fv(lightPositionUniform, LightPosition);
                } else if (keyPress == 2) {
                    LightPosition[0] = Math.cos(rotationAngleY);
                    LightPosition[1] = 0.0;
                    LightPosition[2] = Math.sin(rotationAngleY);
                    gl.uniform4fv(lightPositionUniform, LightPosition);
                } else if (keyPress == 3) {
                    LightPosition[0] = Math.cos(rotationAngleZ);
                    LightPosition[1] = Math.sin(rotationAngleZ);
                    LightPosition[2] = 0.0;
                    gl.uniform4fv(lightPositionUniform, LightPosition);
                }

                if (x == 0) {
                    if (y == 5) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[0]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[0]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[0]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[0]);
                    } else if (y == 4) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[1]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[1]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[1]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[1]);
                    } else if (y == 3) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[2]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[2]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[2]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[2]);
                    } else if (y == 2) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[3]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[3]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[3]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[3]);
                    } else if (y == 1) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[4]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[4]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[4]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[4]);
                    } else if (y == 0) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[5]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[5]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[5]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[5]);
                    }
                } else if (x == 1) {
                    if (y == 5) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[6]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[6]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[6]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[6]);
                    } else if (y == 4) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[7]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[7]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[7]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[7]);
                    } else if (y == 3) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[8]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[8]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[8]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[8]);
                    } else if (y == 2) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[9]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[9]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[9]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[9]);
                    } else if (y == 1) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[10]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[10]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[10]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[10]);
                    } else if (y == 0) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[11]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[11]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[11]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[11]);
                    }
                } else if (x == 2) {
                    if (y == 5) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[12]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[12]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[12]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[12]);
                    } else if (y == 4) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[13]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[13]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[13]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[13]);
                    } else if (y == 3) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[14]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[14]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[14]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[14]);
                    } else if (y == 2) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[15]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[15]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[15]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[15]);
                    } else if (y == 1) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[16]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[16]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[16]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[16]);
                    } else if (y == 0) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[17]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[17]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[17]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[17]);
                    }
                } else if (x == 3) {
                    if (y == 5) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[18]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[18]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[18]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[18]);
                    } else if (y == 4) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[19]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[19]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[19]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[19]);
                    } else if (y == 3) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[20]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[20]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[20]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[20]);
                    } else if (y == 2) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[21]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[21]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[21]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[21]);
                    } else if (y == 1) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[22]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[22]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[22]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[22]);
                    } else if (y == 0) {
                        gl.uniform3fv(kaUniform, MaterialAmbient[23]);
                        gl.uniform3fv(kdUniform, MaterialDiffuse[23]);
                        gl.uniform3fv(ksUniform, MaterialSpecular[23]);
                        gl.uniform1f(materialShininessUniform, MaterialShininess[23]);
                    }
                }
            } else {
                gl.uniform1i(lKeyPressedUniform, 0);
            }

            vao_sphere.draw();

            gl.useProgram(null);
        }
    }

    //animation loop
    update();

    requestAnimationFrame(draw, jgg_canvas);
}

function update() {
    rotationAngleX = rotationAngleX + 0.05;
    if (rotationAngleX >= 360.0) {
        rotationAngleX = 0.0;
    }

    rotationAngleY = rotationAngleY + 0.05;
    if (rotationAngleY >= 360.0) {
        rotationAngleY = 0.0;
    }

    rotationAngleZ = rotationAngleZ + 0.05;
    if (rotationAngleZ >= 360.0) {
        rotationAngleZ = 0.0;
    }
}

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

        case 88:
            keyPress = 1;
            rotationAngleX = 0.0;
            break;

        case 89:
            keyPress = 2;
            rotationAngleY = 0.0;
            break;

        case 90:
            keyPress = 3;
            rotationAngleZ = 0.0;
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

function setMaterialSphere() {
    //material 1 : Emerald
    MaterialAmbient[0].push(0.0215, 0.1745, 0.0215);
    MaterialDiffuse[0].push(0.07568, 0.61424, 0.07568);
    MaterialSpecular[0].push(0.633, 0.727811, 0.633);
    MaterialShininess[0].push(0.6 * 128.0);

    //material 2: Jade
    MaterialAmbient[1].push(0.135, 0.225, 0.1575);
    MaterialDiffuse[1].push(0.54, 0.89, 0.63);
    MaterialSpecular[1].push(0.316228, 0.316228, 0.316228);
    MaterialShininess[1].push(0.1 * 128.0);

    //material 3 : Obsidian
    MaterialAmbient[2].push(0.05375, 0.05, 0.06625);
    MaterialDiffuse[2].push(0.18275, 0.17, 0.22525);
    MaterialSpecular[2].push(0.332741, 0.328634, 0.346435);
    MaterialShininess[2].push(0.3 * 128.0);

    //material 4 : Pearl
    MaterialAmbient[3].push(0.25, 0.20725, 0.20725);
    MaterialDiffuse[3].push(1.0, 0.829, 0.829);
    MaterialSpecular[3].push(0.296648, 0.296648, 0.296648);
    MaterialShininess[3].push(0.088 * 128.0);

    //material 5 : Ruby
    MaterialAmbient[4].push(0.1745, 0.01175, 0.01175);
    MaterialDiffuse[4].push(0.61424, 0.04136, 0.04136);
    MaterialSpecular[4].push(0.727811, 0.626959, 0.626959);
    MaterialShininess[4].push(0.6 * 128.0);

    //material 6 : Turquoise
    MaterialAmbient[5].push(0.1, 0.18725, 0.1745);
    MaterialDiffuse[5].push(0.396, 0.74151, 0.69102);
    MaterialSpecular[5].push(0.297254, 0.30829, 0.306678);
    MaterialShininess[5].push(0.1 * 128.0);

    //---------------COLUMN 2 : METAL -----------------------------
    //material 1 : Brass
    MaterialAmbient[6].push(0.329412, 0.223529, 0.027451);
    MaterialDiffuse[6].push(0.780392, 0.568627, 0.113725);
    MaterialSpecular[6].push(0.992157, 0.941176, 0.807843);
    MaterialShininess[6].push(0.21794872 * 128.0);

    //material  2 : Bronze
    MaterialAmbient[7].push(0.2125, 0.1275, 0.054);
    MaterialDiffuse[7].push(0.714, 0.4284, 0.18144);
    MaterialSpecular[7].push(0.393548, 0.271906, 0.166721);
    MaterialShininess[7].push(0.2 * 128.0);

    //material 3 : Chrome
    MaterialAmbient[8].push(0.25, 0.25, 0.25);
    MaterialDiffuse[8].push(0.4, 0.4, 0.4);
    MaterialSpecular[8].push(0.774597, 0.774597, 0.774597);
    MaterialShininess[8].push(0.6 * 128.0);

    //material 4 : Copper
    MaterialAmbient[9].push(0.19125, 0.0735, 0.0225);
    MaterialDiffuse[9].push(0.7038, 0.27048, 0.0828);
    MaterialSpecular[9].push(0.256777, 0.1376222, 0.086014);
    MaterialShininess[9].push(0.1 * 128.0);

    //material 5 : Gold
    MaterialAmbient[10].push(0.24725, 0.1995, 0.0745);
    MaterialDiffuse[10].push(0.75164, 0.60648, 0.22648);
    MaterialSpecular[10].push(0.628281, 0.555802, 0.366065);
    MaterialShininess[10].push(0.4 * 128.0);

    //material 6 : Silver
    MaterialAmbient[11].push(0.19225, 0.19225, 0.19225);
    MaterialDiffuse[11].push(0.50754, 0.50754, 0.50754);
    MaterialSpecular[11].push(0.508273, 0.508273, 0.508273);
    MaterialShininess[11].push(0.4 * 128.0);

    //---------------COLUMN 3 : PLASTIC -----------------------------
    //material 1 : Black

    MaterialAmbient[12].push(0.0, 0.0, 0.0);
    MaterialDiffuse[12].push(0.0, 0.0, 0.0);
    MaterialSpecular[12].push(0.5, 0.5, 0.5);
    MaterialShininess[12].push(0.25 * 128.0);

    //material  2 : Cyan
    MaterialAmbient[13].push(0.0, 0.1, 0.06);
    MaterialDiffuse[13].push(0.0, 0.50980329, 0.50980329);
    MaterialSpecular[13].push(0.50196078, 0.50196078, 0.50196078);
    MaterialShininess[13].push(0.25 * 128.0);

    //material 3 : Green
    MaterialAmbient[14].push(0.0, 0.0, 0.0);
    MaterialDiffuse[14].push(0.1, 0.35, 0.1);
    MaterialSpecular[14].push(0.45, 0.55, 0.45);
    MaterialShininess[14].push(0.25 * 128.0);

    //material 4 : red
    MaterialAmbient[15].push(0.0, 0.0, 0.0);
    MaterialDiffuse[15].push(0.5, 0.0, 0.0);
    MaterialSpecular[15].push(0.7, 0.6, 0.6);
    MaterialShininess[15].push(0.25 * 128.0);

    //material 5 : white
    MaterialAmbient[16].push(0.0, 0.0, 0.0);
    MaterialDiffuse[16].push(0.55, 0.55, 0.55);
    MaterialSpecular[16].push(0.7, 0.7, 0.7);
    MaterialShininess[16].push(0.4 * 128.0);

    //material 6 : yellow
    MaterialAmbient[17].push(0.0, 0.0, 0.0);
    MaterialDiffuse[17].push(0.5, 0.5, 0.5);
    MaterialSpecular[17].push(0.6, 0.6, 0.5);
    MaterialShininess[17].push(0.35 * 128.0);

    //---------------COLUMN 4 : RUBBER -----------------------------
    //material 1 : Black
    MaterialAmbient[18].push(0.02, 0.02, 0.02);
    MaterialDiffuse[18].push(0.01, 0.01, 0.01);
    MaterialSpecular[18].push(0.4, 0.4, 0.4);
    MaterialShininess[18].push(0.078125 * 128.0);

    //material  2 : Cyan
    MaterialAmbient[19].push(0.0, 0.05, 0.05);
    MaterialDiffuse[19].push(0.4, 0.5, 0.5);
    MaterialSpecular[19].push(0.04, 0.7, 0.7);
    MaterialShininess[19].push(0.078125 * 128.0);

    //material 3 : Green
    MaterialAmbient[20].push(0.0, 0.05, 0.0);
    MaterialDiffuse[20].push(0.4, 0.5, 0.4);
    MaterialSpecular[20].push(0.04, 0.7, 0.04);
    MaterialShininess[20].push(0.078125 * 128.0);

    //material 4 : red
    MaterialAmbient[21].push(0.05, 0.0, 0.0);
    MaterialDiffuse[21].push(0.5, 0.4, 0.4);
    MaterialSpecular[21].push(0.7, 0.04, 0.04);
    MaterialShininess[21].push(0.078125 * 128.0);

    //material 5 : white
    MaterialAmbient[22].push(0.05, 0.05, 0.05);
    MaterialDiffuse[22].push(0.5, 0.5, 0.5);
    MaterialSpecular[22].push(0.7, 0.7, 0.7);
    MaterialShininess[22].push(0.078125 * 128.0);

    //material 6 : yellow
    MaterialAmbient[23].push(0.05, 0.05, 0.0);
    MaterialDiffuse[23].push(0.5, 0.5, 0.4);
    MaterialSpecular[23].push(0.7, 0.7, 0.04);
    MaterialShininess[23].push(0.078125 * 128.0);
}

function setViewports(x, y, width, height) {
    gl.viewport(x, y, width, height);

    //perspective
    mat4.perspective(perspectiveProjectionMatrix, 45.0, parseFloat(width) / parseFloat(height), 0.1, 100.0);
}

function create2DArray(row) {
    var arr = [];

    for (var i = 0; i < row; i++) {
        arr[i] = [];
    }

    return arr;
}
