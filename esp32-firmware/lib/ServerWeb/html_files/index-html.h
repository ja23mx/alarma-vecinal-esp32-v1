const char const_html_ccs_index[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Alarma</title>
    <link rel="icon" href="data:,">
    <!-- <link rel="stylesheet" href="/css/style.css"> -->

    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f8f9fa;
        }

        .header {
            background-color: #f8f9fa;
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 43px;
            margin-top: 8px;
        }

        .header h1 {
            color: #1c85f4;
            font-size: 23px;
            margin: 0;
        }

        .menu {
            width: 0;
            height: 100vh;
            background-color: #343a40;
            color: #f3f4fb;
            overflow: hidden;
            position: fixed;
            top: 0;
            left: 0;
            transition: width 0.3s ease;
        }

        .menu a {
            display: flex;
            align-items: center;
            color: white;
            padding: 15px;
            text-decoration: none;
            font-size: 23px;
            transition: background-color 0.3s ease;
            margin-top: 17px;
        }

        .menu a img {
            width: 32px;
            height: 32px;
            margin-right: 10px;
            filter: invert(1);
        }

        .menu a span {
            margin-top: 3px;
            margin-left: 10px;
        }

        .menu a:hover {
            background-color: #575757;
        }

        .menu-btn {
            font-size: 35px;
            color: #000000;
            background-color: #f8f9fa;
            border: none;
            cursor: pointer;
            position: absolute;
            top: 1px;
            left: 10px;
        }

        .content {
            margin-left: 20px;
            padding: 20px;
        }

        .content h1 {
            color: #333;
        }

        .menu.open {
            width: 250px;
        }

        .menu-contenedor-2,
        .menu-contenedor {
            padding: 13px;
            margin-right: 10px;
            margin-left: 10px;
            background-color: #f9f9f9;
            border-radius: 15px;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
            height: calc(100vh - 90px);
            overflow: hidden;
        }

        .menu-contenedor-2 {
            height: auto;
            margin-top: 23px;
            padding: 5px;
            border-radius: 10px;
        }

        .menu-contenedor-2 .texto {
            font-size: 20px;
        }

        .menu-contenedor .texto {
            font-size: 23px;
        }

        .menu-contenedor .texto {
            font-size: 23px;
            text-align: center;
        }

        /* Contenedor principal */
        .info-container {
            max-width: 600px;
            margin: 20px auto;
            padding: 20px;
            background-color: #f9f9f9;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            font-family: 'Arial', sans-serif;
        }

        /* Título de la sección */
        .info-container h2 {
            text-align: center;
            color: #333;
            margin-bottom: 20px;
            font-size: 24px;
        }

        /* Elementos individuales */
        .info-item {
            display: flex;
            flex-direction: column;
            /* Coloca el campo y la información en filas */
            margin-bottom: 17px;
        }

        /* Etiquetas de información */
        .info-label {
            font-weight: bold;
            color: #555;
            margin-bottom: 5px;
            /* Espacio entre la etiqueta y el valor */
        }

        /* Valores de información */
        .info-value {
            color: #007BFF;
            font-weight: normal;
            font-size: 20px;
        }

        /* Spinner animado */
        .spinner-submenu-rf-1,
        .spinner-pnt-main,
        .spinner {
            width: 60px;
            height: 60px;
            border: 6px solid #f3f3f3;
            /* Color del borde */
            border-top: 6px solid #3498db;
            /* Color del borde superior */
            border-radius: 50%;
            /* Hace que sea un círculo */
            animation: spin 1s linear infinite;
            /* Animación de giro */
            position: fixed;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            /* Centra el spinner */
            z-index: 1000;
            /* Asegura que esté por encima de otros elementos */
        }

        .spinner-submenu-rf-1 {
            top: 50%;
        }

        .spinner-pnt-main {
            top: 40%;
        }

        /* Animación de giro */
        @keyframes spin {
            0% {
                transform: translate(-50%, -50%) rotate(0deg);
            }

            100% {
                transform: translate(-50%, -50%) rotate(360deg);
            }
        }

        /* SUBEMENU 3434*/
        .header-submenu {
            position: relative;
            /* Cambiado a relative */
            top: 0;
            left: 0;
            right: 0;
            height: 70px;
            display: flex;
            align-items: center;
            justify-content: center;
            /* Centra el contenido horizontalmente */
        }

        .header-submenu .flecha {
            position: absolute;
            /* Mueve la flecha fuera del flujo normal */
            left: 5px;
            /* Coloca la flecha a la izquierda */
            width: 32px;
            height: 32px;
            cursor: pointer;
            transition: transform 0.2s, fill 0.2s;
            color: #007bff;
        }

        .header-submenu .texto {
            font-size: 23px;
            text-align: center;
        }

        .submenu-fila-contenedor {
            display: flex;
            /* Alinea los elementos horizontalmente */
            align-items: center;
            /* Centra verticalmente los elementos dentro del contenedor */
            padding: 10px 0px 10px 0px;
            /* Espaciado interno */
            position: relative;
            /* Necesario para posicionar elementos internos */
            height: auto;
            /* Asegura que el contenedor se ajuste al contenido */
        }

        .submenu-fila-contenedor::after {
            content: '';
            position: absolute;
            bottom: 0;
            /* Coloca la línea en la parte inferior del contenedor */
            left: 35px;
            /* Ajusta este valor para que coincida con el inicio del texto */
            right: 0;
            /* Extiende la línea hasta el borde derecho */
            height: 1px;
            /* Grosor de la línea */
            background-color: #ddd;
            /* Color de la línea */
            z-index: 0;
            /* Asegura que la línea quede detrás de los elementos */
        }

        .submenu-fila-contenedor .img {
            position: relative;
            /* Cambiado a relative para respetar el flujo */
            margin-right: 10px;
            /* Espaciado entre la imagen y el texto */
            width: 28px;
            /* Tamaño fijo de la imagen */
            height: 28px;
        }

        .submenu-fila-contenedor .img-2 {
            position: relative;
            /* Cambiado a relative para respetar el flujo */
            margin-right: 10px;
            /* Espaciado entre la imagen y el texto */
            width: 26px;
            /* Tamaño fijo de la imagen */
            height: 26px;
        }

        .submenu-fila-contenedor .texto,
        .txt-2 {
            position: relative;
            /* Mantiene el flujo normal */
            font-size: 20px;
            white-space: nowrap;
            /* Evita que el texto se divida en varias líneas */
            overflow: hidden;
            /* Oculta el texto que exceda el contenedor */
            text-overflow: ellipsis;
            /* Agrega puntos suspensivos si el texto es demasiado largo */
            margin-left: 0px;
            /* Ajusta la distancia entre la imagen y el texto */
            flex-grow: 1;
            /* Permite que el texto ocupe el espacio disponible */
            display: flex;
            /* Asegura que el texto esté alineado verticalmente */
            align-items: center;
            /* Centra el texto verticalmente */
        }

        .txt-2 {
            margin-left: 36px;
        }

        .submenu-fila-contenedor .flecha-derecha,
        .img-derecha {
            position: relative;
            /* Cambiado a relative para respetar el flujo */
            margin-left: auto;
            /* Empuja la flecha hacia el extremo derecho */
            width: 24px;
            /* Tamaño fijo de la flecha */
            height: 24px;
            cursor: pointer;
            color: #007bff;
            /* Color de la flecha */
        }


        /* Estilo del select */
        .custom-select {
            width: 70%;
            /* Ajusta el ancho automáticamente al contenido */
            padding: 10px;
            font-size: 16px;
            color: #333;
            background-color: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 5px;
            appearance: none;
            -webkit-appearance: none;
            -moz-appearance: none;
            position: relative;
            text-align: center;
            margin-bottom: 20px;
            margin-top: 20px;
            /* Espacio entre el select y el botón */
        }

        .custom-select:focus {
            outline: none;
            border-color: #007bff;
            box-shadow: 0 0 5px rgba(0, 123, 255, 0.5);
        }

        /* Flecha del select */
        .custom-select-container {
            position: relative;
            width: 100%;
            display: flex;
            justify-content: center;
        }

        .custom-select-container::after {
            content: '▼';
            position: absolute;
            top: 51%;
            right: calc(16%);
            /* Ajusta la posición de la flecha */
            transform: translateY(-50%);
            pointer-events: none;
            color: #007bff;
        }

        /* Estilo del textbox */
        .custom-textbox {
            position: absolute;
            /* Posición absoluta */
            margin-top: 30px;
            /* Espacio superior */
            left: 50%;
            /* Centrado horizontalmente dentro del contenedor padre */
            transform: translate(-50%, -50%);
            /* Ajuste para centrar completamente */
            width: 20%;
            /* Igual ancho que el select */
            padding: 10px;
            font-size: 16px;
            color: #333;
            background-color: #f9f9f9;
            border: 1px solid #ddd;
            border-radius: 5px;
            margin-bottom: 20px;
            /* Espacio entre elementos */
            text-align: center;
        }

        .custom-textbox:focus {
            outline: none;
            border-color: #007bff;
            box-shadow: 0 0 5px rgba(0, 123, 255, 0.5);
        }

        /* Estilo para las etiquetas */
        .custom-label {
            font-size: 20px;
            font-weight: bold;
            color: #333;
            text-align: center;
            margin-bottom: 10px;
            /* Espacio entre la etiqueta y el elemento */
            display: block;
            /* Asegura que ocupe toda la línea */
        }



        /* Botón de acción */
        .action-button {
            position: absolute;
            margin-top: 110px;
            /* Ajusta la posición vertical */
            left: 50%;
            /* Centrado horizontalmente */
            transform: translate(-50%, -50%);
            /* Ajuste para centrar completamente */
            width: auto;
            /* Ajusta el ancho al contenido */
            padding: 10px 20px;
            /* Espaciado interno horizontal y vertical */
            font-size: 16px;
            color: #fff;
            background-color: #007bff;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            text-align: center;
            white-space: nowrap;
            /* Evita que el texto se divida en varias líneas */
        }

        /* Estilo para la tabla moderna */
        .modern-table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            font-size: 16px;
            text-align: left;
            background-color: #f9f9f9;
            border: 2px solid #464545;
            border-radius: 2px;
            overflow: hidden;
        }

        .modern-table th,
        .modern-table td {
            padding: 12px 15px;
            border: 1px solid #ddd;
        }

        .modern-table th {
            color: white;
            font-weight: bold;
            text-align: center;
        }

        .modern-table td:nth-child(2) {
            text-align: center;
            /* Centra la columna "Valor" */
            width: 70%;
            /* Ajusta el ancho de la columna "Valor" al 70% */
        }

        .modern-table tr:nth-child(even) {
            background-color: #f2f2f2;
        }

        .modern-table tr:hover {
            background-color: #ddd;
        }

        /* FIN SUBMENU */

        /* ALERTA */

        /* El fondo de la ventana emergente (modal) */
        .modal {
            display: none;
            /* Oculto por defecto */
            position: fixed;
            z-index: 1;
            /* Asegura que la ventana esté por encima del contenido */
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            overflow: auto;
            /* Si el contenido es mayor que la ventana, se puede desplazar */
            background-color: rgba(0, 0, 0, 0.7);
            /* Fondo oscuro con opacidad */
        }

        /* El contenido de la ventana emergente */
        .modal-content {
            background-color: white;
            margin: 30% auto;
            padding: 20px;
            border-radius: 5px;
            width: 80%;
            /* Ancho de la ventana */
            max-width: 400px;
            /* Máximo tamaño de la ventana */
            text-align: center;
            /* Centrar todo el contenido dentro de la ventana */
        }

        /* Estilo base para los botones modales */
        .button-modal {
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
            color: white;
            border: none;
            border-radius: 5px;
        }

        /* Estilo específico para el botón "OK" */
        .btn-ok {
            background-color: #4CAF50;
        }

        .btn-ok:hover {
            background-color: #45a049;
        }

        /* Estilo específico para el botón "No" */
        .btn-no {
            background-color: #eb5a55;
        }

        .btn-no:hover {
            background-color: #d9534f;
            /* Ajuste para diferenciar el hover */
        }

        /* Centrando los botones */
        .button-modal-container {
            display: flex;
            justify-content: space-between;
            margin-top: 20px;
        }

        .button-modal-container button {
            width: 45%;
            /* Ocupa el 45% del ancho del contenedor */
        }

        /* FIN ALERTA */
    </style>
</head>

<body>

    <div id="alerta_1_opcion" class="modal" style="display: none;">
        <div class="modal-content">
            <p id="alerta_1_opcion_p"></p>
            <br>
            <button id="btn_alerta_1_opcion" class="button-modal btn-ok"
                onclick="close_alerta_1_opcion()">Cerrar</button>
        </div>
    </div>

    <div id="alerta_nv_senal" class="modal" style="display: none;">
        <!-- Contenido de la ventana emergente -->
        <div class="modal-content">
            <p>Señal RF recibida, el dispositivo no está guardado<br><br>¿Guardar?</p>
            <br>
            <div class="button-modal-container">
                <button class="button-modal btn-ok" onclick="proceso_nueva_senal(1)">Sí</button>
                <button class="button-modal btn-no" onclick="proceso_nueva_senal(0)">No</button>
            </div>
        </div>
    </div>

    <div id="alerta_ctrl_ram_detectado" class="modal" style="display: none;">
        <!-- Contenido de la ventana emergente -->
        <div class="modal-content">
            <p id="alerta_ctrl_ram_detectado_p"></p>
            <br>
            <button class="button-modal btn-ok" onclick="close_alerta_ctrl_ram_detectado()">Cerrar</button>
        </div>
    </div>

    <div id="alerta_ctrl_inf_1_borrado" class="modal" style="display: none;">
        <!-- Contenido de la ventana emergente -->
        <div class="modal-content">
            <p id="alerta_ctrl_inf_1_borrado_p"></p>
            <div class="button-modal-container">
                <button class="button-modal btn-ok" onclick="ejec_borrado_1_ctrl()">Sí</button>
                <button class="button-modal btn-no" onclick="close_alerta_ctrl_inf_1_borrado()">No</button>
            </div>
        </div>
    </div>

    <div id="alerta_nv_ctrl_dif_nm" class="modal" style="display: none;">
        <!-- Contenido de la ventana emergente -->
        <div class="modal-content">
            <p>El número de control es diferente al sugerido, si hay un control en esa posición será remplazado
                <br><br>
                ¿Continuar?
            </p>
            <br>
            <div class="button-modal-container">
                <button class="button-modal btn-ok" onclick="guardar_ctrl_nm_dif()">Sí</button>
                <button class="button-modal btn-no" onclick="close_alerta_nv_ctrl_dif_nm()">No</button>
            </div>
        </div>
    </div>

    <div id="alerta_doble_opcion" class="modal" style="display: none;">
        <!-- Contenido de la ventana emergente -->
        <div class="modal-content">
            <p id="alerta_doble_opcion_p">
            </p>
            <div class="button-modal-container" id="btn-contendor-alerta_doble_opcion">
                <button id="test-ok-al-db-opc" class="button-modal btn-ok"></button>
                <button id="test-cancel-al-db-opc" class="button-modal btn-no"></button>
            </div>
        </div>
    </div>

    <div id="menu_principal">
        <button class="menu-btn" onclick="toggleMenu()" id="btn-menu" style="display: none;">☰</button>

        <div class="header">
            <h1>Alarma Vecinal</h1>
        </div>

        <div class="menu" id="myMenu">
            <a onclick="submenu_init(1)"><span>Dispositivos RF</span></a>
        </div>
    </div>

    <div id="info-pantalla-principal" class="menu-contenedor">
        <div id="datos_pant_inicial" style="font-size: 23px; text-align: center; display: block; margin-top: 40px;">
            <span class="texto">Cargando Datos...<br>Espere Unos Segundos</span>
            <div class="spinner-pnt-main"></div>
        </div>
        <div id="datos_pant_main" style="display: none;">
            <div class="info-item">
                <span class="info-label">Número de Serie:</span>
                <span id="info-num-serie" class="info-value">Cargando...</span>
            </div>
            <div class="info-item">
                <span class="info-label">Modelo:</span>
                <span id="info-modelo" class="info-value">Cargando...</span>
            </div>
            <div class="info-item">
                <span class="info-label">Versión:</span>
                <span id="info-version" class="info-value">Cargando...</span>
            </div>
            <div class="info-item">
                <span class="info-label">Fecha Compilación:</span>
                <span id="info-build" class="info-value">Cargando...</span>
            </div>
        </div>
    </div>


    <div id="submenu_rf" style="display: none;" class="submenu-div-container">

        <div class="header-submenu"><!-- 3434 -->
            <svg class="flecha" onclick="back_submenu()" viewBox="0 0 24 24" fill="none">
                <path d="M20 25L9 12L20 1" stroke="currentColor" stroke-width="3" stroke-linecap="round"
                    stroke-linejoin="round" />
            </svg>
            <div class="texto">Dispositivos RF</div>
        </div>

        <div class="menu-contenedor">
            <div id="subm-rf-0" style="display: none;" class="subm-contenedor-rf">
                <div class="submenu-fila-contenedor" onclick="init_deteccion_rf()">
                    <div class="texto">Recepcion Señal</div>
                    <!-- Flecha derecha dibujada con SVG -->
                    <svg class="flecha-derecha" viewBox="0 0 24 24" fill="none" style="margin-right: -15px;">
                        <path d="M1 18L7 12L1 6" stroke="currentColor" stroke-width="2" stroke-linecap="round"
                            stroke-linejoin="round" />
                    </svg>
                </div>
                <div class="submenu-fila-contenedor" onclick="init_borrado_1_ctrl()">
                    <div class="texto">Borrar Dispositivo(s)</div>
                    <!-- Flecha derecha dibujada con SVG -->
                    <svg class="flecha-derecha" viewBox="0 0 24 24" fill="none" style="margin-right: -15px;">
                        <path d="M1 18L7 12L1 6" stroke="currentColor" stroke-width="2" stroke-linecap="round"
                            stroke-linejoin="round" />
                    </svg>
                </div>
            </div>

            <div id="subm-rf-1" style="display: none;" class="subm-contenedor-rf">
                <br><br>
                <label for="modelos-select-rf" class="custom-label">Seleccione el Tipo de Control</label>
                <div class="custom-select-container">
                    <select id="modelos-select-rf" class="custom-select">
                    </select>
                </div>
                <br>
                <label for="nv-ctrl-num" class="custom-label">Número de control</label>
                <input type="number" id="nv-ctrl-num" class="custom-textbox">
                <button class="action-button" onclick="configurar_num_ctrl_modelo()">GUARDAR</button>
            </div>

            <div id="subm-rf-2" style="display: none;" class="subm-contenedor-rf">
                <br><br>
                <label for="nv-ctrl-rx" class="custom-label">
                    Listo para recibir señal RF
                    <br><br>
                    Por favor, presione cualquier botón del dispositivo.
                </label>
                <div id="spinner-rf-subm-rf-2" class="spinner-submenu-rf-1" style="display: none;"></div>
            </div>

            <div id="subm-rf-3" style="display: none;" class="subm-contenedor-rf">
                <br><br>
                <label for="nv-ctrl-rx" class="custom-label">
                    Dispositivo Guardado Exitosamente
                    <br><br>
                </label>
                <button class="action-button" onclick="init_deteccion_rf()">VOLVER</button>
            </div>

            <div id="subm-rf-5" style="display: none;" class="subm-contenedor-rf">
                <br>
                <label for="lab-borrar-1-ctrl-num" class="custom-label">Borrado de 1 control de Alarma
                    Vecinal</label>
                <br><br>
                <input type="number" id="borrar-1-ctrl-num" class="custom-textbox" value="1">
                <button class="action-button" onclick="init_borrar_1_ctrl()">BORRAR</button>
            </div>

            <div id="subm-rf-6" style="display: none;" class="subm-contenedor-rf">
                <br><br>
                <label for="modelos-select-rf" class="custom-label">Número de Control</label>
                <br>
                <input type="number" id="borr-1-ctrl-num" class="custom-textbox" value="1">
                <button class="action-button" id="btn-borrar" onclick="solic_info_borr_1_ctrl()">BORRAR</button>
            </div>
        </div>
    </div>

    <script>
        let submenu_nivel = 0; // Variable para controlar el nivel del submenú
        let submenu_opcion = 0; // Variable para controlar la opción del submenú
        let redes_escaneadas_global = []; // Variable global para almacenar las redes WiFi
        let redes_g_dsp = []; // Variable global para almacenar las redes WiFi detectadas y guardadas
        let redes_guardadas = []; // Variable para almacenar las redes WiFi guardadas 
        let limite_controles_rf = 0; // Variable para controlar el límite de controles RF

        let archivos_totales_cargados = false; // Variable para controlar si los archivos CSS han sido cargados

        function mostrar_alerta_1_opcion(textoParrafo, textoBoton) {
            // Mostrar el modal
            const modal = document.getElementById('alerta_1_opcion');
            modal.style.display = 'block';

            // Establecer el texto del párrafo
            const parrafo = document.getElementById('alerta_1_opcion_p');
            parrafo.textContent = textoParrafo;

            // Establecer el texto del botón
            const boton = document.getElementById('btn_alerta_1_opcion');
            boton.textContent = textoBoton;
        }

        function mostrarAlertaDobleOpcion(texto, funcionOk, funcionError, textoOk, textoError) {
            // Mostrar el modal
            const alerta = document.getElementById("alerta_doble_opcion");
            if (!alerta) {
                console.error("El modal 'alerta_doble_opcion' no existe en el DOM.");
                return;
            }
            alerta.style.display = "block";

            // Insertar el texto en el párrafo
            const parrafo = document.getElementById("alerta_doble_opcion_p");
            if (parrafo) {
                parrafo.innerHTML = texto;
            }

            // Obtener los botones ya creados
            const botonOk = document.getElementById("test-ok-al-db-opc");
            const botonError = document.getElementById("test-cancel-al-db-opc");

            if (!botonOk || !botonError) {
                console.error("Los botones no existen en el DOM.");
                return;
            }

            // Asignar las funciones y textos a los botones
            botonOk.textContent = textoOk; // Modificar el texto del botón "OK"
            botonOk.onclick = funcionOk; // Asignar la función directamente

            botonError.textContent = textoError; // Modificar el texto del botón "Cancelar"
            botonError.onclick = funcionError; // Asignar la función directamente
        }


        function toggleMenu() {
            const menu = document.getElementById('myMenu');
            menu.classList.toggle('open');
        }

        document.addEventListener('click', function (event) {
            const menu = document.getElementById('myMenu');
            const menuBtn = document.querySelector('.menu-btn');
            if (!menu.contains(event.target) && event.target !== menuBtn) {
                menu.classList.remove('open');
            }
        });

        function fin_carga_archivos() {

            document.getElementById("datos_pant_inicial").style.display = "none";
            document.getElementById("datos_pant_main").style.display = "block";
            document.getElementById("btn-menu").style.display = "block";
        }

        function consultarApiData() { // 3434
            fetch('/api_data?get_info=1')
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Error al consultar la API');
                    }
                    return response.json();
                })
                .then(data => {
                    console.log("Datos recibidos de la API:", data);
                    actualizarInformacionSistema(data.datos); // Actualizar la interfaz con los datos recibidos
                    fin_carga_archivos();
                })
                .catch(error => {
                    console.error("Error al consultar la API:", error);
                });
        }

        function actualizarInformacionSistema(datos) {

            console.log("Actualizando información del sistema...");
            //console.log("Datos a mostrar:", datos);

            // Acceder a los datos anidados dentro de "datos"
            const info = datos;

            //console.log("info: ", datos.numSerie, datos.modelo, datos.firmware, datos.version, datos.build, datos.fecha);

            limite_controles_rf = info.lm_ctrl || 0; // Asignar el límite de controles RF

            // Actualizar los elementos en la interfaz
            document.getElementById('info-num-serie').textContent = info.numSerie || "No disponible";
            document.getElementById('info-modelo').textContent = info.modelo || "No disponible";
            document.getElementById('info-version').textContent = `${info.firmware} ${info.version} ${info.build}` || "No disponible";
            document.getElementById('info-build').textContent = info.fecha || "No disponible";
        }

        function submenu_init(opcion) {

            console.log("Inicializando submenu...");
            console.log("Opción seleccionada:", opcion); /* 3434 */

            // Ocultar todos los submenús con la clase "submenu-div-container"
            const submenus = document.querySelectorAll('.submenu-div-container');
            submenus.forEach(submenu => {
                submenu.style.display = 'none';
            });

            // muestra el memu principal si la opción es 0
            // y oculta los submenus
            if (opcion == 0) {
                document.getElementById('menu_principal').style.display = 'block'; // oculta el menu principal
                document.getElementById('info-pantalla-principal').style.display = 'block'; // oculta el contenido de principal
                return; // salir de la función
            } else {
                document.getElementById('menu_principal').style.display = 'none'; // oculta el menu principal
                document.getElementById('info-pantalla-principal').style.display = 'none'; // 
            }

            toggleMenu(); // Cerrar el menú principal

            submenu_nivel = 1;       // Cambiar el nivel del submenú a 1
            submenu_opcion = opcion; // Guardar la opción seleccionada

            switch (opcion) {
                case 1:
                    console.log("Opción 1 seleccionada: Controles RF");
                    cnf_screen_rf(0); // Inicializa submenu inicial de controles RF
                    break;
                case 2:
                    console.log("Opción 1 seleccionada: Wifi");
                    //config_screen_wifi(0); // Inicializa submenu inicial de controles RF
                    break;
            }
        }

        function back_submenu() {

            switch (submenu_opcion) {
                case 1: // RF
                    if (submenu_nivel == 1) { // Si estamos en el primer nivel del submenú
                        console.log("Volviendo al menú principal...");
                        submenu_nivel = 0; // Reiniciar el nivel del submenú
                        submenu_opcion = 0; // Reiniciar la opción del submenú
                        submenu_init(0); // Llamar a la función para mostrar el menú principal
                    } else if (submenu_nivel == 2) { // Si estamos en el segundo nivel del submenú
                        console.log("Volviendo al menú de controles RF...");
                        submenu_nivel = 1; // Cambiar el nivel del submenú a 1
                        cnf_screen_rf(0); // Inicializa submenu inicial de controles RF
                    }
                    break
            }

        }
        // **************************************************************************************
        // fin config init
        // **************************************************************************************

        // **** Funciones para la detección de dispositivos RF ****
        let timer_revDetecSenal = null;
        let timer_revVerificacionCtrl = null;
        let buffer_nuevo_num_ctrl = null;

        function cnf_screen_rf(opcion) { // 0=inicial

            // Ocultar todos los submenús con la clase "submenu_div_container"
            const submenus = document.querySelectorAll('.subm-contenedor-rf');
            submenus.forEach(submenu => {
                submenu.style.display = 'none';
            });

            ocultarCargando(); // Ocultar el canvas de carga

            if (opcion == 0) {
                submenu_nivel = 1; // Cambiar el nivel del submenú a 1
                realizarFetch(
                    '/setDetectarDispRF?&config=0', // config=0 para inicializar la detección
                    (data) => {
                        console.log("Datos:", data);
                    },
                    (error) => {
                        console.error("Error al procesar los datos:", error);
                    }
                );
            }
            else if (opcion > 0)
                submenu_nivel = 2; // Cambiar el nivel del submenú a 2

            clearTimeout(timer_revDetecSenal);

            document.getElementById('subm-rf-' + opcion).style.display = 'block'; // muestra el submenu seleccionado
            document.getElementById('submenu_rf').style.display = 'block'; // muestra el submenu seleccionado
        }

        function realizarFetch(url, onSuccess, onError) {
            fetch(url)
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Error al consultar la API');
                    }
                    return response.json();
                })
                .then(data => {
                    //console.log(`Datos recibidos de la API (${url}):`, data);
                    if (onSuccess) {
                        onSuccess(data); // Ejecutar la función de éxito
                    }
                })
                .catch(error => {
                    //console.error(`Error al consultar la API (${url}):`, error);
                    if (onError) {
                        onError(error); // Ejecutar la función de error
                    }
                });
        }

        function init_deteccion_rf() {
            console.log("init_deteccion_rf...");
            realizarFetch(
                '/setDetectarDispRF?&config=1',
                (data) => {
                    console.log("Datos:", data);
                    if (data.rsp == "ok") {
                        console.log("Dispositivo RF detectado correctamente.");
                        cnf_screen_rf(2); // Inicializa submenu deteccion rf
                        mostrarCargando();
                        init_timer_revDetecSenal(); // Inicia el temporizador para la detección de señal
                    }
                },
                (error) => {
                    console.error("Error al procesar los datos:", error);
                }
            );
        }

        function init_timer_revDetecSenal() {
            timer_revDetecSenal = setTimeout(() => {
                revDetecSenal();
            }, 100);
        }

        function revDetecSenal() {
            realizarFetch(
                '/revDetecSenal',
                (data) => {
                    if (data.cod_respuesta != "sin_senal")
                        console.log("Datos:", data);
                    if (data.cod_respuesta == "sin_senal") {
                        init_timer_revDetecSenal(); // Reinicia el temporizador para la detección de señal
                    } else if (data.cod_respuesta == "nv_ctrl") {
                        ocultarCargando(); // Ocultar el spinner de carga
                        clearTimeout(timer_revDetecSenal); // Detiene el temporizador
                        console.log("Nuevo Dispositivo RF detectado");
                        document.getElementById('alerta_nv_senal').style.display = 'block'; // Oculta el submenú de detección RF
                    } else if (data.cod_respuesta == "getInfoDspRFLoop") {
                        clearTimeout(timer_revDetecSenal); // Detiene el temporizador
                        ocultarCargando();                 // Ocultar el spinner de carga
                        mostrar_ctrl_ram_detectado(data.datos); // Muestra la alerta de verificación del control
                    }
                },
                (error) => {
                    console.error("Error al procesar los datos:", error);
                }
            );
        }

        function proceso_nueva_senal(opcion) {

            document.getElementById('alerta_nv_senal').style.display = 'none'; // Oculta el submenú de detección RF

            if (opcion == 1) {
                realizarFetch(
                    '/getCtrlModelos',
                    (data) => {
                        console.log("Datos:", data);
                        if (data.cod_respuesta == "getCtrlModelos")
                            datos_modelos_ctrl(data.datos); // Llama a la función para procesar los datos
                    },
                    (error) => {
                        console.error("Error al procesar los datos:", error);
                    }
                );
            } else {
                mostrarCargando(); // Muestra el spinner de carga
                init_timer_revDetecSenal(); // Inicia el temporizador para la detección de señal
            }
        }

        function datos_modelos_ctrl(datos) {

            console.log("datos_modelos_ctrl...");

            // Acceder al array de datos dentro de "datos"
            const info = datos;

            //console.log("info:", info);

            // Inicializar variables para almacenar los modelos y el espacio vacío
            const modelos = [];
            let espacioVacio = null;

            // Recorrer el array para separar los modelos y el espacio vacío
            info.forEach(item => {
                if (item.modelo) {
                    modelos.push(item.modelo); // Agregar el modelo al array
                } else if (item.espacio_vacio !== undefined) {
                    espacioVacio = item.espacio_vacio; // Guardar el espacio vacío
                }
            });

            // Mostrar los resultados en la consola
            console.log("Modelos:", modelos);
            console.log("Espacio vacío:", espacioVacio);
            cnf_screen_rf(1); // Inicializa submenu cnf modelo

            const select = document.getElementById('modelos-select-rf');

            // Limpiar todas las opciones existentes
            while (select.firstChild) {
                select.removeChild(select.firstChild);
            }

            modelos.forEach(modelo => {
                const option = document.createElement('option');
                option.value = modelo;
                option.textContent = modelo;
                select.appendChild(option);
            });

            buffer_nuevo_num_ctrl = espacioVacio; // Guardar el espacio vacío en la variable global
            document.getElementById('nv-ctrl-num').value = espacioVacio || ""; // Asignar el espacio vacío al campo correspondiente
        }

        function configurar_num_ctrl_modelo() {

            let modelo = document.getElementById('modelos-select-rf').value;
            let num_ctrl = document.getElementById('nv-ctrl-num').value;

            if (num_ctrl == "" || parseInt(num_ctrl) <= 0 || parseInt(num_ctrl) > limite_controles_rf) {
                mostrar_alerta_1_opcion("Numero de Control erroneo, debe ser mayor a 1 y menor o igual a " + limite_controles_rf, "Cerrar");
                return; // Salir de la función si el número de control es inválido
            }

            if (num_ctrl != buffer_nuevo_num_ctrl) {
                document.getElementById('alerta_nv_ctrl_dif_nm').style.display = 'block';
                return; // Salir de la función si el número de control no coincide
            }

            ejec_g_ctrl(num_ctrl, modelo); // Llama a la función para ejecutar el guardado del control
        }

        function ejec_g_ctrl(num_ctrl, modelo) { /* 3434 */
            realizarFetch(
                '/setCnfNvRF?nm_ctrl=' + num_ctrl + '&mdl=' + modelo + "&status=1",
                (data) => {
                    console.log("Datos:", data);
                    if (data.rsp == "ok") {
                        mostrar_alerta_1_opcion("Control guardado exitosamente", "Cerrar");
                        cnf_screen_rf(3);
                    }
                },
                (error) => {
                    console.error("Error al procesar los datos:", error);
                    mostrar_alerta_1_opcion("Error inesperado en la respuesta", "Cerrar");
                }
            );
        }

        function guardar_ctrl_nm_dif() {

            let num_ctrl = document.getElementById('nv-ctrl-num').value;
            let modelo = document.getElementById('modelos-select-rf').value;

            close_alerta_nv_ctrl_dif_nm(); // Cierra la alerta de número de control diferente
            ejec_g_ctrl(num_ctrl, modelo); // Llama a la función para ejecutar el guardado del control
        }

        function close_alerta_nv_ctrl_dif_nm() {
            document.getElementById('alerta_nv_ctrl_dif_nm').style.display = 'none';
        }

        function mostrar_ctrl_ram_detectado(info) {

            document.getElementById('alerta_ctrl_ram_detectado').style.display = 'block';

            // Establecer el texto del párrafo
            const parrafo = document.getElementById('alerta_ctrl_ram_detectado_p');
            parrafo.innerHTML = `
            <table class="modern-table">
                <tr>
                    <td>Categoría</td>
                    <td>${info.tipo}</td>
                </tr>
                <tr>
                    <td>Tipo</td>
                    <td>${info.nombre}</td>
                </tr>
                <tr>
                    <td>Codificador</td>
                    <td>${info.codificador}</td>
                </tr>
                <tr>
                    <td>Control</td>
                    <td>${info.control}</td>
                </tr>
                <tr>
                    <td>Botón</td>
                    <td>${info.btn_presionado}</td>
                </tr>
            </table>
            `;
        }

        function close_alerta_ctrl_ram_detectado() {
            mostrarCargando(); // Muestra el spinner de carga
            document.getElementById('alerta_ctrl_ram_detectado').style.display = 'none'; // Oculta la alerta de control RAM detectado
            clearTimeout(timer_revVerificacionCtrl); // Detiene el temporizador de verificación del control
            init_deteccion_rf();                     // Reinicia la detección de RF
        }

        function mostrarCargando() {
            const spinner = document.getElementById('spinner-rf-subm-rf-2');
            spinner.style.display = 'block'; // Mostrar el spinner
        }

        function ocultarCargando() {
            const spinner = document.getElementById('spinner-rf-subm-rf-2');
            spinner.style.display = 'none'; // Ocultar el spinner
        }

        function init_borrado_1_ctrl() {
            cnf_screen_rf(6); // Inicializa submenu borrado 1 ctrl
        }

        let flag_init_borrado_1_ctrl = 0; // Variable para controlar la inicialización del borrado de 1 control

        function solic_info_borr_1_ctrl() {

            if (flag_init_borrado_1_ctrl) {
                return; // Si ya se ha inicializado, no hacer nada
            }

            let num_ctrl = document.getElementById('borr-1-ctrl-num').value;
            if (num_ctrl == "" || parseInt(num_ctrl) <= 0 || parseInt(num_ctrl) > limite_controles_rf) {
                mostrar_alerta_1_opcion("Numero de Control erroneo, debe ser mayor a 1 y menor o igual a " + limite_controles_rf, "Cerrar");
                return; // Salir de la función si el número de control es inválido
            }

            flag_init_borrado_1_ctrl = 1; // Cambia el estado de inicialización a 1
            const btnBorrar = document.getElementById('btn-borrar');
            btnBorrar.style.setProperty('opacity', '1', 'important');

            btnBorrar.style.opacity = '0.3'; // hace invisible el botón de borrar

            realizarFetch(
                '/info1Ctrl?nm_ctrl=' + num_ctrl,
                (data) => {
                    console.log("Datos:", data);
                    let info = data.rsp || ""; // Usa la propiedad 'descripcion' o una cadena vacía si no existe

                    if (typeof info === "string") {

                        let error = info.indexOf("Error"); // Verifica si hay un error en la respuesta
                        console.log("info:", info, " error:", error);

                        if (error >= 0) {
                            flag_init_borrado_1_ctrl = 0; // Cambia el estado de inicialización a 1
                            btnBorrar.style.opacity = '1'; // hace visible el botón de borrar
                            mostrar_alerta_1_opcion(info, "Cerrar");
                            return; // Salir de la función si el número de control es inválido 
                        }

                        info = data.datos; // Asigna los datos a la variable info
                        console.log("info: ", info);

                        document.getElementById('alerta_ctrl_inf_1_borrado').style.display = 'block';

                        // Establecer el texto del párrafo
                        const parrafo = document.getElementById('alerta_ctrl_inf_1_borrado_p');
                        parrafo.innerHTML = `
                        <table class="modern-table">
                            <tr>
                                <td>Tipo</td>
                                <td>${info.nombre_modelo}</td>
                            </tr>
                            <tr>
                                <td>Codificador</td>
                                <td>${info.codificador}</td>
                            </tr>
                            <tr>
                                <td>Control</td>
                                <td>${info.num_control}</td>
                            </tr>
                        </table>
                        <p>¿Está seguro de que desea borrar el control?</p>
                        `;
                        flag_init_borrado_1_ctrl = 0; // Cambia el estado de inicialización a 1
                        btnBorrar.style.opacity = '1'; // hace visible el botón de borrar
                    } else {
                        flag_init_borrado_1_ctrl = 0; // Cambia el estado de inicialización a 1
                        btnBorrar.style.opacity = '1'; // hace visible el botón de borrar
                        console.error("La respuesta no contiene un valor válido para 'info':", info);
                        mostrar_alerta_1_opcion("Error inesperado en la respuesta", "Cerrar");
                        return;
                    }
                },
                (error) => {
                    flag_init_borrado_1_ctrl = 0; // Cambia el estado de inicialización a 1

                    document.getElementById('btn-borrar').style.opacity = '1'; // hace visible el botn de borrar
                    console.error("Error al procesar los datos:", error);
                }
            );
        }

        function ejec_borrado_1_ctrl() {

            document.getElementById('alerta_ctrl_inf_1_borrado').style.display = 'none';

            realizarFetch(
                '/borrar1Ctrl?nm_ctrl=' + document.getElementById('borr-1-ctrl-num').value,
                (data) => {
                    console.log("Datos:", data);
                    if (data.rsp == "Ok. Control borrado de memoria.") {
                        cnf_screen_rf(6); // Inicializa submenu inicial
                        mostrar_alerta_1_opcion("Control borrado correctamente", "Cerrar");
                        document.getElementById('borr-1-ctrl-num').value = 1; // Limpia el campo de texto
                        return; // Salir de la función si el número de control es válido
                    }
                    mostrar_alerta_1_opcion("Error inesperado en la respuesta", "Cerrar");
                },
                (error) => {
                    mostrar_alerta_1_opcion("Error inesperado en la respuesta", "Cerrar");
                    console.error("Error al procesar los datos:", error);
                }
            );
        }
        // **************************************************************************************

        function close_alerta_1_opcion() {
            // Ocultar el modal
            const modal = document.getElementById('alerta_1_opcion');
            modal.style.display = 'none';
        }

        function close_alerta_ctrl_inf_1_borrado() {
            document.getElementById('alerta_ctrl_inf_1_borrado').style.display = 'none'; // Oculta la alerta de información del control
            document.getElementById('alerta_ctrl_inf_1_borrado_p').innerHTML = ""; // Limpia el contenido del párrafo
        }

        function closeAlertaDobleOpcion() {
            // Mostrar el modal
            const alerta = document.getElementById("alerta_doble_opcion");
            alerta.style.display = "none";
            // Insertar el texto en el párrafo
            const parrafo = document.getElementById("alerta_doble_opcion_p");
            parrafo.innerHTML = "";
        }

        // **************************************************************************************
        consultarApiData();
    </script>
</body>

</html>
)rawliteral";