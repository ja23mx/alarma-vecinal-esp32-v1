const char const_html_ccs_wifi[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi</title>
    <link rel="icon" href="data:,">

    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: #f8f9fa;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 10px;
        }

        .header {
            width: 100%;
            max-width: 500px;
            text-align: center;
        }

        .header h1 {
            color: #1c85f4;
            font-size: 23px;
            margin: 0;
        }

        .wifi-container {
            background-color: #fff;
            width: 100%;
            max-width: 500px;
            padding: 30px;
            border-radius: 10px 10px 10px 10px;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.15);
            margin-top: 10px;
        }

        .mensaje-reset {
            display: none;
            background-color: #ff9800;
            color: white;
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            margin-bottom: 20px;
            font-weight: bold;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {

            0%,
            100% {
                opacity: 1;
            }

            50% {
                opacity: 0.7;
            }
        }

        .form-group {
            margin-bottom: 20px;
            position: relative;
        }

        .custom-label {
            display: block;
            color: #333;
            font-size: 16px;
            font-weight: bold;
            margin-bottom: 8px;
        }

        .custom-textbox {
            width: 100%;
            padding: 12px 15px;
            font-size: 16px;
            border: 2px solid #ddd;
            border-radius: 8px;
            background-color: #f9f9f9;
            transition: all 0.3s ease;
            text-align: center;
        }

        .custom-textbox:focus {
            outline: none;
            border-color: #667eea;
            background-color: #fff;
        }

        .custom-textbox:disabled {
            background-color: #e0e0e0;
            cursor: not-allowed;
        }

        /* Autocompletado Dropdown */
        .autocomplete-items {
            position: absolute;
            border: 1px solid #ddd;
            border-top: none;
            z-index: 99;
            top: 100%;
            left: 0;
            right: 0;
            max-height: 200px;
            overflow-y: auto;
            background-color: white;
            border-radius: 0 0 8px 8px;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
        }

        .autocomplete-items div {
            padding: 12px;
            cursor: pointer;
            border-bottom: 1px solid #f0f0f0;
            transition: background-color 0.2s;
        }

        .autocomplete-items div:hover {
            background-color: #667eea;
            color: white;
        }

        .autocomplete-items div:last-child {
            border-bottom: none;
        }

        /* Botones */
        .button-container {
            display: flex;
            gap: 15px;
            margin-top: 30px;
        }

        .action-button {
            flex: 1;
            padding: 14px 20px;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
        }

        .btn-guardar {
            background-color: #4CAF50;
            color: white;
        }

        .btn-guardar:hover {
            background-color: #45a049;
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(76, 175, 80, 0.4);
        }

        .btn-guardar:disabled {
            background-color: #ccc;
            cursor: not-allowed;
            transform: none;
        }

        .btn-reset {
            background-color: #f44336;
            color: white;
        }

        .btn-reset:hover {
            background-color: #da190b;
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(244, 67, 54, 0.4);
        }

        .btn-reset:disabled {
            background-color: #ccc;
            cursor: not-allowed;
            transform: none;
        }

        /* Modal */
        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.5);
            animation: fadeIn 0.3s;
        }

        @keyframes fadeIn {
            from {
                opacity: 0;
            }

            to {
                opacity: 1;
            }
        }

        .modal-content {
            background-color: #fff;
            margin: 40% auto;
            padding: 30px;
            border-radius: 10px;
            width: 90%;
            max-width: 400px;
            text-align: center;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
            animation: slideDown 0.3s;
        }

        @keyframes slideDown {
            from {
                transform: translateY(-50px);
                opacity: 0;
            }

            to {
                transform: translateY(0);
                opacity: 1;
            }
        }

        .modal-content p {
            font-size: 18px;
            color: #333;
            margin-bottom: 25px;
        }

        .button-modal {
            padding: 12px 30px;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        .btn-ok {
            background-color: #4CAF50;
            color: white;
        }

        .btn-ok:hover {
            background-color: #45a049;
            transform: scale(1.05);
        }

        /* Responsive */
        @media (max-width: 600px) {
            .header h1 {
                font-size: 24px;
            }

            .wifi-container {
                padding: 20px;
            }

            .button-container {
                flex-direction: column;
            }

            .action-button {
                width: 100%;
            }
        }
    </style>
</head>

<body>
    <!-- Modal de Alerta -->
    <div id="modal_alerta" class="modal">
        <div class="modal-content">
            <p id="modal_texto"></p>
            <button class="button-modal btn-ok" onclick="cerrarModal()">OK</button>
        </div>
    </div>

    <!-- Header -->
    <div class="header">
        <h1>WiFi</h1>
    </div>

    <!-- Contenedor principal -->
    <div class="wifi-container">
        <!-- Mensaje de Reset -->
        <div id="mensaje_reset" class="mensaje-reset">
            Reiniciando dispositivo...
        </div>

        <!-- Formulario -->
        <form id="form_wifi" style="text-align: center;">
            <div class="form-group">
                <label class="custom-label" for="input_ssid">SSID</label>
                <input type="text" id="input_ssid" class="custom-textbox" placeholder="Nombre de la red WiFi"
                    autocomplete="off" maxlength="32">
                <div id="autocomplete_list" class="autocomplete-items"></div>
            </div>

            <div class="form-group">
                <label class="custom-label" for="input_password">PASSWORD</label>
                <input type="text" id="input_password" class="custom-textbox" placeholder="Contraseña WiFi"
                    maxlength="64">
            </div>

            <div class="button-container">
                <button type="button" class="action-button btn-guardar" id="btn_guardar" onclick="guardarWifi()">
                    Guardar
                </button>
                <button type="button" class="action-button btn-reset" id="btn_reset" onclick="resetearDispositivo()">
                    Reset
                </button>
            </div>
        </form>
    </div>

    <script>

        let redesDisponibles = [];
        let autocompleteActivo = false;

        // Cargar redes WiFi al iniciar
        window.addEventListener('DOMContentLoaded', function () {
            cargarRedesWifi();
            configurarAutocompletado();
        });

        // Cargar redes WiFi
        function cargarRedesWifi() {
            fetch('/api_data?get_info=1')
                .then(response => response.json())
                .then(data => {
                    // Cargar redes disponibles para autocompletado
                    if (data.escaneo_wifi && data.escaneo_wifi.redes) {
                        redesDisponibles = data.escaneo_wifi.redes.map(red => red.ssid);
                    }

                    // Cargar red guardada en los inputs
                    if (data.redes_guardadas && data.redes_guardadas.redes && data.redes_guardadas.redes.length > 0) {
                        const redGuardada = data.redes_guardadas.redes[0];
                        document.getElementById('input_ssid').value = redGuardada.ssid;
                        document.getElementById('input_password').value = redGuardada.password;
                    }
                })
                .catch(error => console.error('Error al cargar redes WiFi:', error));

        }

        // Configurar autocompletado
        function configurarAutocompletado() {
            const inputSsid = document.getElementById('input_ssid');
            const autocompleteList = document.getElementById('autocomplete_list');

            inputSsid.addEventListener('input', function () {
                const valor = this.value;
                cerrarAutocompletado();

                if (valor.length < 2) return;

                const coincidencias = redesDisponibles.filter(red =>
                    red.toLowerCase().includes(valor.toLowerCase())
                );

                if (coincidencias.length === 0) return;

                coincidencias.forEach(red => {
                    const item = document.createElement('div');
                    item.textContent = red;
                    item.addEventListener('click', function () {
                        inputSsid.value = red;
                        cerrarAutocompletado();
                    });
                    autocompleteList.appendChild(item);
                });

                autocompleteActivo = true;
            });

            // Cerrar autocompletado al hacer click fuera
            document.addEventListener('click', function (e) {
                if (e.target !== inputSsid) {
                    cerrarAutocompletado();
                }
            });
        }

        function cerrarAutocompletado() {
            const autocompleteList = document.getElementById('autocomplete_list');
            autocompleteList.innerHTML = '';
            autocompleteActivo = false;
        }

        // Validaciones
        function validarSSID(ssid) {
            if (ssid.trim() === '') {
                return { valido: false, mensaje: 'El SSID no puede estar vacío' };
            }

            // Validar caracteres especiales no permitidos
            const caracteresNoPermitidos = /[ñÑ\u{1F300}-\u{1F9FF}]/u;
            if (caracteresNoPermitidos.test(ssid)) {
                return { valido: false, mensaje: 'El SSID contiene caracteres no permitidos (ñ, emojis)' };
            }

            return { valido: true };
        }

        function validarPassword(password) {
            if (password.trim() === '') {
                return { valido: false, mensaje: 'La contraseña no puede estar vacía' };
            }

            if (password.length < 8) {
                return { valido: false, mensaje: 'La contraseña debe tener mínimo 8 caracteres' };
            }

            // Validar emojis
            const emojis = /[\u{1F300}-\u{1F9FF}]/u;
            if (emojis.test(password)) {
                return { valido: false, mensaje: 'La contraseña no puede contener emojis' };
            }

            return { valido: true };
        }

        // Guardar WiFi
        function guardarWifi() {
            const ssid = document.getElementById('input_ssid').value;
            const password = document.getElementById('input_password').value;

            // Validar SSID
            const validacionSSID = validarSSID(ssid);
            if (!validacionSSID.valido) {
                mostrarModal(validacionSSID.mensaje);
                return;
            }

            // Validar Password
            const validacionPassword = validarPassword(password);
            if (!validacionPassword.valido) {
                mostrarModal(validacionPassword.mensaje);
                return;
            }

            // Enviar al servidor
            fetch(`/apiWifi/guardar?red=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(password)}`)
                .then(response => response.json())
                .then(data => {
                    console.log('Respuesta del servidor:', data);
                    
                    // Verificar si hay respuesta exitosa
                    if (data.rsp && data.rsp.includes('Ok')) {
                        mostrarModal('Red WiFi guardada correctamente');
                    }
                    // Verificar si hay error en el campo 'rsp'
                    else if (data.rsp && data.rsp.includes('Error')) {
                        mostrarModal(data.rsp);
                    }
                    // Verificar si el error viene directamente en la descripción
                    else if (data.descripcion && data.descripcion.includes('Error')) {
                        mostrarModal(data.descripcion);
                    }
                    // Error genérico
                    else {
                        mostrarModal('Error al guardar la red WiFi');
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    mostrarModal('Error de comunicación con el dispositivo');
                });
        }

        // Resetear dispositivo
        function resetearDispositivo() {
            fetch('/apiWifi/rst')
                .then(response => response.json())
                .then(data => {
                    if (data.success === 'ok') {
                        // Mostrar mensaje de reset
                        document.getElementById('mensaje_reset').style.display = 'block';

                        // Deshabilitar campos y botones
                        document.getElementById('input_ssid').disabled = true;
                        document.getElementById('input_password').disabled = true;
                        document.getElementById('btn_guardar').disabled = true;
                        document.getElementById('btn_reset').disabled = true;
                    }
                })
                .catch(error => {
                    console.error('Error:', error);
                    mostrarModal('Error al resetear el dispositivo');
                });
        }

        // Mostrar modal
        function mostrarModal(mensaje) {
            document.getElementById('modal_texto').textContent = mensaje;
            document.getElementById('modal_alerta').style.display = 'block';
        }

        // Cerrar modal
        function cerrarModal() {
            document.getElementById('modal_alerta').style.display = 'none';
        }
    </script>
</body>

</html>
)rawliteral";