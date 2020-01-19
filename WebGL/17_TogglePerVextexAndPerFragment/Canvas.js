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

//pf
var gShaderProgramObject_pf;
var gVertexShaderObject_pf;
var gFragmentShaderObject_pf;

//pv
var gShaderProgramObject_pv;
var gVertexShaderObject_pv;
var gFragmentShaderObject_pv;

//pf
var modelUniform_pf;
var viewUniform_pf;
var projectionUniform_pf;

//pv
var modelUniform_pv;
var viewUniform_pv;
var projectionUniform_pv;

//pf
var laUniform_pf;
var ldUniform_pf;
var lsUniform_pf;
var lightPositionUniform_pf;
var kaUniform_pf;
var kdUniform_pf;
var ksUniform_pf;
var materialShininessUniform_pf;
var lKeyPressedUniform_pf;

//pv
var laUniform_pv;
var ldUniform_pv;
var lsUniform_pv;
var lightPositionUniform_pv;
var kaUniform_pv;
var kdUniform_pv;
var ksUniform_pv;
var materialShininessUniform_pv;
var lKeyPressedUniform_pv;

var LightAmbient = [0.0, 0.0, 0.0];
var LightDiffuse = [1.0, 1.0, 1.0];
var LightSpecular = [1.0, 1.0, 1.0];
var LightPosition = [100.0, 100.0, 100.0, 1.0];

var MaterialAmbient = [0.0, 0.0, 0.0];
var MaterialDiffuse = [1.0, 1.0, 1.0];
var MaterialSpecular = [1.0, 1.0, 1.0];
var MaterialShininess = 128.0;

var vao_sphere;

var gbLight = false;
var isFKeyPressed = false;
var isVKeyPressed = false;

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

    //***************** PER FRAGMENT *****************************************
    //vertex shader
    var vertexShaderSourceCode_pf =
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

    gVertexShaderObject_pf = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(gVertexShaderObject_pf, vertexShaderSourceCode_pf);
    gl.compileShader(gVertexShaderObject_pf);
    if (gl.getShaderParameter(gVertexShaderObject_pf, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(gVertexShaderObject_pf);
        if (error.length > 0) {
            alert("\nVertex Shader compilation log (Per Fragment): " + error);
            uninitialize();
        }
    }

    //fragment shader
    var fragmentShaderSourceCode_pf =
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

    gFragmentShaderObject_pf = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(gFragmentShaderObject_pf, fragmentShaderSourceCode_pf);
    gl.compileShader(gFragmentShaderObject_pf);
    if (gl.getShaderParameter(gFragmentShaderObject_pf, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(gFragmentShaderObject_pf);
        if (error.length > 0) {
            alert("\nFragment Shader compilation log (Per Fragment): " + error);
            uninitialize();
        }
    }

    //shader program
    gShaderProgramObject_pf = gl.createProgram();
    gl.attachShader(gShaderProgramObject_pf, gVertexShaderObject_pf);
    gl.attachShader(gShaderProgramObject_pf, gFragmentShaderObject_pf);

    //pre-linking
    gl.bindAttribLocation(gShaderProgramObject_pf, WebGLMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
    gl.bindAttribLocation(gShaderProgramObject_pf, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    //linking
    gl.linkProgram(gShaderProgramObject_pf);
    if (gl.getProgramParameter(gShaderProgramObject_pf, gl.LINK_STATUS) == false) {
        var error = gl.getProgramInfoLog(gShaderProgramObject_pf);
        if (error.length > 0) {
            alert("\nProgram linking log (Per Fragment): " + error);
            uninitialize();
        }
    }

    //get uniform location
    viewUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_view_matrix");
    modelUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_model_matrix");
    projectionUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_projection_matrix");
    lKeyPressedUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_lKeyPressed");

    laUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_la");
    ldUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_ld");
    lsUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_ls");
    lightPositionUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_light_position");

    kaUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_ka");
    kdUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_kd");
    ksUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_ks");
    materialShininessUniform_pf = gl.getUniformLocation(gShaderProgramObject_pf, "u_material_shininess");

    //***************** PER VERTEX *****************************************
    //vertex shader
    var vertexShaderSourceCode_pv =
        "#version 300 es" +
        "\n" +
        "precision mediump int;" +
        "in vec4 vPosition;" +
        "in vec3 vNormal;" +
        "uniform mat4 u_view_matrix;" +
        "uniform mat4 u_model_matrix;" +
        "uniform mat4 u_projection_matrix;" +
        "uniform int u_lKeyPressed;" +
        "uniform vec3 u_la;" +
        "uniform vec3 u_ld;" +
        "uniform vec3 u_ls;" +
        "uniform vec4 u_light_position;" +
        "uniform vec3 u_ka;" +
        "uniform vec3 u_kd;" +
        "uniform vec3 u_ks;" +
        "uniform float u_material_shininess;" +
        "out vec3 phong_ads_light;" +
        "void main(void)" +
        "{" +
        "if(u_lKeyPressed == 1)" +
        "{" +
        "vec4 eyeCoords = u_view_matrix * u_model_matrix * vPosition;" +
        "vec3 tNormal = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" +
        "vec3 light_direction = normalize(vec3(u_light_position - eyeCoords));" +
        "float tNorm_Dot_LightDirection = max(dot(light_direction, tNormal), 0.0);" +
        "vec3 reflection_vector = reflect(-light_direction, tNormal);" +
        "vec3 viewer_vector = normalize(vec3(-eyeCoords.xyz));" +
        "vec3 ambient = u_la * u_ka;" +
        "vec3 diffuse = u_ld * u_kd * tNorm_Dot_LightDirection;" +
        "vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector,viewer_vector), 0.0), u_material_shininess);" +
        "phong_ads_light = ambient + diffuse + specular;" +
        "}" +
        "else" +
        "{" +
        "phong_ads_light = vec3(1.0, 1.0, 1.0);" +
        "}" +
        "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" +
        "}";

    gVertexShaderObject_pv = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(gVertexShaderObject_pv, vertexShaderSourceCode_pv);
    gl.compileShader(gVertexShaderObject_pv);
    if (gl.getShaderParameter(gVertexShaderObject_pv, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(gVertexShaderObject_pv);
        if (error.length > 0) {
            alert("\nVertex Shader compilation log (Per Vertex): " + error);
            uninitialize();
        }
    }

    //fragment shader
    var fragmentShaderSourceCode_pv =
        "#version 300 es" +
        "\n" +
        "precision highp float;" +
        "in vec3 phong_ads_light;" +
        "out vec4 FragColor;" +
        "void main(void)" +
        "{" +
        "   FragColor = vec4(phong_ads_light,1.0);" +
        "}";

    gFragmentShaderObject_pv = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(gFragmentShaderObject_pv, fragmentShaderSourceCode_pv);
    gl.compileShader(gFragmentShaderObject_pv);
    if (gl.getShaderParameter(gFragmentShaderObject_pv, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(gFragmentShaderObject_pv);
        if (error.length > 0) {
            alert("\nFragment Shader compilation log (Per Vertex): " + error);
            uninitialize();
        }
    }

    //shader program
    gShaderProgramObject_pv = gl.createProgram();
    gl.attachShader(gShaderProgramObject_pv, gVertexShaderObject_pv);
    gl.attachShader(gShaderProgramObject_pv, gFragmentShaderObject_pv);

    //pre-linking
    gl.bindAttribLocation(gShaderProgramObject_pv, WebGLMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
    gl.bindAttribLocation(gShaderProgramObject_pv, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

    //linking
    gl.linkProgram(gShaderProgramObject_pv);
    if (gl.getProgramParameter(gShaderProgramObject_pv, gl.LINK_STATUS) == false) {
        var error = gl.getProgramInfoLog(gShaderProgramObject_pv);
        if (error.length > 0) {
            alert("\nProgram linking log (Per Vertex): " + error);
            uninitialize();
        }
    }

    //get uniform location
    viewUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_view_matrix");
    modelUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_model_matrix");
    projectionUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_projection_matrix");
    lKeyPressedUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_lKeyPressed");

    laUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_la");
    ldUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_ld");
    lsUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_ls");
    lightPositionUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_light_position");

    kaUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_ka");
    kdUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_kd");
    ksUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_ks");
    materialShininessUniform_pv = gl.getUniformLocation(gShaderProgramObject_pv, "u_material_shininess");

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

    if (isFKeyPressed == true) {
        gl.useProgram(gShaderProgramObject_pf);

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

        gl.uniformMatrix4fv(modelUniform_pf, false, modelMatrix);
        gl.uniformMatrix4fv(viewUniform_pf, false, viewMatrix);
        gl.uniformMatrix4fv(projectionUniform_pf, false, projectionMatrix);

        if (gbLight == true) {
            gl.uniform1i(lKeyPressedUniform_pf, 1);
            gl.uniform3fv(laUniform_pf, LightAmbient);
            gl.uniform3fv(ldUniform_pf, LightDiffuse);
            gl.uniform3fv(lsUniform_pf, LightSpecular);
            gl.uniform3fv(kaUniform_pf, MaterialAmbient);
            gl.uniform3fv(kdUniform_pf, MaterialDiffuse);
            gl.uniform3fv(ksUniform_pf, MaterialSpecular);
            gl.uniform1f(materialShininessUniform_pf, MaterialShininess);
            gl.uniform4fv(lightPositionUniform_pf, LightPosition);
        } else {
            gl.uniform1i(lKeyPressedUniform_pf, 0);
        }

        vao_sphere.draw();

        gl.useProgram(null);
    } else if (isVKeyPressed == true) {
        gl.useProgram(gShaderProgramObject_pv);

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

        gl.uniformMatrix4fv(modelUniform_pv, false, modelMatrix);
        gl.uniformMatrix4fv(viewUniform_pv, false, viewMatrix);
        gl.uniformMatrix4fv(projectionUniform_pv, false, projectionMatrix);

        if (gbLight == true) {
            gl.uniform1i(lKeyPressedUniform_pv, 1);
            gl.uniform3fv(laUniform_pv, LightAmbient);
            gl.uniform3fv(ldUniform_pv, LightDiffuse);
            gl.uniform3fv(lsUniform_pv, LightSpecular);
            gl.uniform3fv(kaUniform_pv, MaterialAmbient);
            gl.uniform3fv(kdUniform_pv, MaterialDiffuse);
            gl.uniform3fv(ksUniform_pv, MaterialSpecular);
            gl.uniform1f(materialShininessUniform_pv, MaterialShininess);
            gl.uniform4fv(lightPositionUniform_pv, LightPosition);
        } else {
            gl.uniform1i(lKeyPressedUniform_pv, 0);
        }

        vao_sphere.draw();

        gl.useProgram(null);
    }
    //animation loop

    requestAnimationFrame(draw, jgg_canvas);
}

function update() {}

function uninitialize() {
    if (vao_sphere) {
        vao_sphere.deallocate();
        vao_sphere = null;
    }

    if (gShaderProgramObject_pf) {
        if (gFragmentShaderObject_pf) {
            gl.detachShader(gShaderProgramObject_pf, gFragmentShaderObject_pf);
            gl.deleteShader(gFragmentShaderObject_pf);
            gFragmentShaderObject_pf = null;
        }
        if (gVertexShaderObject_pf) {
            gl.detachShader(gShaderProgramObject_pf, gVertexShaderObject_pf);
            gl.deleteShader(gVertexShaderObject_pf);
            gVertexShaderObject_pf = null;
        }

        gl.deleteProgram(gShaderProgramObject_pf);
        gShaderProgramObject_pf = null;
    }

    if (gShaderProgramObject_pv) {
        if (gFragmentShaderObject_pv) {
            gl.detachShader(gShaderProgramObject_pv, gFragmentShaderObject_pv);
            gl.deleteShader(gFragmentShaderObject_pv);
            gFragmentShaderObject_pv = null;
        }
        if (gVertexShaderObject_pv) {
            gl.detachShader(gShaderProgramObject_pv, gVertexShaderObject_pv);
            gl.deleteShader(gVertexShaderObject_pv);
            gVertexShaderObject_pv = null;
        }

        gl.deleteProgram(gShaderProgramObject_pv);
        gShaderProgramObject_pv = null;
    }
}

function keyDown(event) {
    switch (event.keyCode) {
        case 27:
            uninitialize();
            window.close();
            break;

        case 70:
            isFKeyPressed = true;
            isVKeyPressed = false;
            break;

        case 86:
            isVKeyPressed = true;
            isFKeyPressed = false;
            break;

        //L
        case 76:
            if (gbLight == false) {
                gbLight = true;
            } else {
                gbLight = false;
            }
            break;

        //s
        case 83:
            toggleFullScreen();
            break;
    }
}

function mouseDown() {}

function degToRad(degrees) {
    var d = Math.PI / 180;
    return degrees * d;
}
