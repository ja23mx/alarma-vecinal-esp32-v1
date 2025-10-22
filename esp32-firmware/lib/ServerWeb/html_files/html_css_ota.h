const char const_html_ccs_ota[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title>OTA</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="shortcut icon" href="#">
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            background-color: #1f3048eb;
        }

        .login-box {
            width: 320px;
            height: 520px;
            background: #2E2E2E;
            color: #fff;
            top: 50%;
            left: 50%;
            position: absolute;
            transform: translate(-50%, -50%);
            box-sizing: border-box;
            padding: 30px 30px;
        }

        .info {
            font-size: 1.2rem;
            line-height: 1.3rem;
        }

        div#div_file {
            background-image: url("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAB2AAAAdgB+lymcgAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAAQRSURBVHic7ZpfaBxFHMe/M5u7WJPUUxBEIwSsgmlBi/ogBhSMhfRFKMTW+mCK2tgHX0wr1dZ2MWIkoYIgRFJEG/xDeslVFCxNLgaaDd5DoX+Sa6A+ScESK/5Nbm//zc8HTdqee5q73Z057X4eZ3a+893vzM3Ozi0QExMTExMTExMTI5+M8SzGZx9UaYEp6VWfqkMrm4dtrcNtdwDARzDWPwedCdlWuOwOQcSxnp+Fba27qrQLbfkPQCR9QOQHkJ7OwSq2+tR04eu5d2XbkRvA6EkDtvlQ2XpiL2FyrleiI4kBjJ78HEXzkX+9jrAf2fyrEhwBkBXA2PRRFM0nV9+A3kI23xOdoStEH8CYcRim2Vl5QxpANr8zfEPXEm0Ao8YAzKXnAaqmNQNoEBP57WHbuproAsjMvAlraXdAFQ6GYUzmq5hBq+0gCo7NvAJzcV91A18KaSD6GJP5jjDUSgk/gDGjG4XC2+Hc/ApJEI1i/Nyjoaoi7ADGjK2wCoMgEcWO7kZw/gWyc+X3EVUQXgAZYwss8zOISLezawGMYyK/MSzBcALIGE+gaKYhIhn5UlLgOIGJ2XvDEAsewJe5NhSLxyGEvF0l0a1gbBxT8y1BpYKbLhSOQ3haYJ3KaYbnfRJUJHgAjtMYWKN67gsqEDyAZP3ZwBrVQjgcVCL4okXEcSy3E/DuKXuNbb0A1/WfKX+eCPkJ58D4O2U1PfoemzbMVGLVDzknMEenLsK2m33rygaANNo3PBWZp7+QfyJUY8QBqDagmjq/ws4evTXBeLsg0RJGJxnHabArbLPWXGrecqA/lNdg8vgCF/b8h32vXS6tu2YRfHp3b7fn2X3CdW8Oo+Nlshsfxi/Jev/KMotgy+VLeOz8mTBteARMEXBguHfPN8uFKz+BbS8fPOFYhffDvvkaQmNAOweMrtf79y4XcgDY1qOPuI69SZ03qXCA9e3Y378LAPj2vW884DlW5M/bWoMYO7Rj36E7ubDFexTu6c1/hTVCE7u48Nz7VTtRBSNs5oK8Msvz/x8C7uIgRX+R1wAMaLzud4JSAqhmijFJC7OUAFKFpYrb3GRW3qYapATQ+t23YGz18yDpurj70sUIHV1BSgBriiYeP38GDZ4H/EMQCc/D7T/9iI7TOTRYRRnW/N8GoyD128/oODX9t/LG1C2yLPgSPwVUG1BNHIBqA6qJA1BtQDVxAKq+l64RiDMm/wvt2oF+4Ezjv6q2oQoCm+Ua175SbUQVHGyEu014kXPuqDajgAuJhaYjPK3ri3XJ5DNM2hFETfC70NA5NNTtcAD4dOBgOnFD/VbO2fUwEy4IDW3D+p5zQMkzsFPXG+sWMShcb7MQXoqIIt8nyHkdpgUCm+VgI4mFpiNDQ90rA/0HkHVMubg2Z58AAAAASUVORK5CYII=");
            height: 64px;
            width: 64px;
            background-position: center;
            background-repeat: no-repeat;
            background-size: cover;
            position: relative;
            margin: auto;
        }

        input#myfile {
            position: absolute;
            top: -5px;
            left: 0px;
            right: 0px;
            bottom: 50px;
            width: 100%;
            height: 100%;
            opacity: 0;
        }

        .box {

            width: 320px;
            height: 400px;
            background: white;
            color: black;
            top: 45%;
            left: 50%;
            position: absolute;
            transform: translate(-50%, -50%);
            box-sizing: border-box;
            padding: 30px 30px;
        }
    </style>
</head>

<body>

    <div class="box">
        <br><br>

        <div style="text-align: center;">
            <span class="info" id="info_archivo">Seleccione un archivo <strong>.bin</strong></span>
        </div>
        <br><br>

        <form id="upload_form" enctype="multipart/form-data" method="post">
            <div id="div_file">
                <input type="file" name="myfile" id="myfile" onchange="subir_archivo()" accept=".bin;">
            </div>
            <div style="text-align: center; display: none;" id="info_subida">
                <progress id="progressBar" value="0" max="100" style="width:230px; height: 17px;"></progress>
                <h4 id="status"></h4>
            </div>
            <br><br><br>
        </form>
    </div>


</body>
<script>
   
    function subir_archivo() {
        var file, formdata, peticion, x1, txt, respuesta, tamano, i, cx, w_char, int_var;

        file = document.getElementById("myfile").files[0];
        console.log(file.name + " | " + file.size);

        x1 = file.name.search(".bin");
        if (x1 < 0) {

            txt = "Sólo archivos .bin";
        } else {

            txt = file.name + "  ";
            tamano = file.size / 1000;
            int_var = Math.floor(tamano);
            txt += int_var;
            txt += " kB"
            console.log("txt 0: " + txt);
            cx = "";
            for (i = 0; i < txt.length; i++) {
                w_char = txt.charAt(i);
                cx += w_char;
                if (i == 19 || i == 39 || i == 59 || i == 79)
                    cx += "<br>";
            }
            txt = cx;
            console.log("txt : " + txt);
        }

        document.getElementById("info_archivo").innerHTML = txt;

        if (x1 < 0)
            return;

        document.getElementById("div_file").style.display = "none";
        document.getElementById("info_subida").style.display = "block";

        formdata = new FormData();
        formdata.append("myfile", file);

        peticion = new XMLHttpRequest();
        peticion.upload.addEventListener("progress", progressHandler, false);
        peticion.addEventListener("load", completeHandler, false);
        peticion.addEventListener("error", errorHandler, false);
        peticion.addEventListener("abort", abortHandler, false);

        peticion.onreadystatechange = function () {
            if (peticion.readyState == 4 && peticion.status == 200) {
                respuesta = peticion.responseText;
                console.log("resp: " + respuesta);
                if (respuesta == "OK") {
                    document.getElementById("status").innerHTML = "Archivo subido. Reiniciando...";
                } else {
                    document.getElementById("status").innerHTML = "Error. Vuelva a intertarlo";
                }
            }
        }

        peticion.open("POST", "/update");
        peticion.send(formdata);
    }

    function progressHandler(event) {
        var percent;

        percent = (event.loaded / event.total) * 100;
        document.getElementById("progressBar").value = Math.round(percent);
        document.getElementById("status").innerHTML = "Subido: " + Math.round(percent) + "% ";
    }

    function completeHandler(event) {

        document.getElementById("progressBar").value = 100;                               // al finalizar se queda la barra llena
    }

    function errorHandler(event) {

        document.getElementById("status").innerHTML = "Subida fallida";
    }

    function abortHandler(event) {

        document.getElementById("status").innerHTML = "Subida fallida";
    }
</script>

</html>
)rawliteral";