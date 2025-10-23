const char const_html_ccs_cmd[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title>mi alarma</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        .main {
            padding-top: 1rem;
        }

        .css_input_texto {
            width: 350px;
            padding: 1px 4px;
            box-sizing: border-box;
            border: none;
            border-bottom: 2px solid #545252;
            font-family: Times New Roman, Times, serif;
            ;
            font-size: 1.3rem;
            font-weight: 400s;
        }

        .btn_azul {
            text-decoration: none;
            padding: 3px;
            font-weight: 40;
            font-size: 17px;
            font-family: Verdana, Helvetica;
            color: #ffffff;
            background-color: #2e86c1;
            border-radius: 6px;
            border: 2px solid #2e86c1;
            padding-left: 2rem;
            padding-right: 2rem;
        }
    </style>
</head>

<body>

    <div id="pantalla_principal">
        <main class='main' style='text-align:center;'>
            <input type='text' class='css_input_texto' id='caja_texto' minlength='5' maxlength='80'>
            <br><br>
            <input class='btn_azul' type='submit' onclick='guardar_datos()' value='GUARDAR'>
            <br><br><br>
            <span id="ajax_etiqueta" style="font-family: Arial; font-size: 1.3rem;"></span>
        </main>
    </div>

</body>

<script>

    function guardar_datos() {
        var datos, w1;

        w1 = document.getElementById("caja_texto").value;
        datos = "/cmdSerial?cmd=";
        datos += w1;
        //alert(datos);
        open_direcion(datos);
    }

    function open_direcion(dir_esp) { // 
        var peticion;

        peticion = new XMLHttpRequest();
        peticion.onreadystatechange = function () {
            if (peticion.readyState == 4 && peticion.status == 200) {
                alert(peticion.responseText);
            }
        }
        peticion.open("GET", dir_esp, true); // true for asynchronous 
        peticion.send(null);
    }

</script>

</html>

)rawliteral";