#include "DataManager.h"
#include "LogSistema.h"

#include "CnfTarjeta.h"
#include "ConfigSistema.h"
#include "VariablesGlobales.h"
#include "CmdSerial.h"
#include "TimeManager.h"

#include <map>

DynamicJsonDocument datosJson(2048); // Variable global para los datos JSON

// Estructura que mapea un comando a su función
struct CmdEntry
{
    const char *command;
    CmdFunc function;
};

// Tabla de mapeo de comandos a funciones
static const CmdEntry cmdTable[] = {
    {"setNS", cmdSetNS},
    {"setClDs", cmdSetClave},
    {"setPerif", cmdSetPeriferico},
    {"setTimer", cmdSetTimer},
    {"setTamper", cmdSetTamper},
    {"mqttHandShakeRequest", cmdmqttHandShakeRequest},
    {"infoInitServer", cmdinfoInitServer},
    {"getCtrlModelos", cmdgetCtrlModelos},
    {"setDetectarDispRF", cmdsetDetectarDispRF},
    {"setCnfNvRF", cmdsetCnfNvRF},
    {"getInfoDspRFLoop", cmdgetInfoDspRFLoop},
    {"borrar1Ctrl", cmdborrar1Ctrl},
    {"info1Ctrl", cmdinfo1Ctrl},
    {"borrar1RedWifi", cmdborrar1RedWifi},
    {"set1RedWifi", cmdset1RedWifi},
    {"getRedesWifi", cmdgetRedesWifi}};

static const int numCommands = sizeof(cmdTable) / sizeof(cmdTable[0]);

void DataManager::CmdSerial(const String &cmd)
{

    // Separa el comando en nombre y parámetros usando ':' como delimitador
    int delimIndex = cmd.indexOf(':');
    String commandName = (delimIndex != -1) ? cmd.substring(0, delimIndex) : cmd;
    String parameters = (delimIndex != -1) ? cmd.substring(delimIndex + 1) : "";
    commandName.trim();
    parameters.trim();

    // LOG("\r\n\r\nComando: " + commandName + " Parametros: " + parameters);

    // Busca en la tabla el comando y ejecuta la función asociada
    for (int i = 0; i < numCommands; i++)
    {
        if (commandName.equalsIgnoreCase(cmdTable[i].command))
        {
            return cmdTable[i].function(this, parameters);
        }
    }
}

/*
 * @brief Función para el comando "setNS"
 * @param self Puntero a la instancia de DataManager
 * @param params numero de serie 0-9, a-f, A-F
 * @return Respuesta del comando
 */
static void cmdSetNS(DataManager *self, const String &params)
{
    CmdResponse res;

    if (params.length() > 10)
    {
        res.status = 0;
        res.response = "Error. En el numero de serie";
    }
    else if (self->setNumSerie(params.c_str()))
    {
        res.status = 1;
        res.response = "Ok. Numero de Serie actualizado a " + params;
    }
    else
    {
        res.status = 0;
        res.response = "Error. Al actualizar el numero de serie";
    }
}

/*
 * @brief Función para el comando "setNS"
 * @param self Puntero a la instancia de DataManager
 * @param params numero de serie 0-9, a-f, A-F
 * @return Respuesta del comando
 */
static void cmdSetClave(DataManager *self, const String &params)
{
    CmdResponse res;

    uint16_t clave = params.toInt();

    if (clave > 9999)
    {
        res.status = 0;
        res.response = "Error. En la clave de dispositivo";
    }
    else if (self->setClaveDispositivo(clave))
    {
        res.status = 1;
        res.response = "Ok. Clave de Dispositivo actualizado a " + params;
    }
    else
    {
        res.status = 0;
        res.response = "Error. Al actualizar la clave de dispositivo";
    }
}

/*
 * @brief Función para el comando "setPerif"
 * @param self Puntero a la instancia de DataManager
 * @param params datos del periférico en formato "tipo,valor"
 * @return Respuesta del comando
 */
/*
 * @brief Función para el comando "setPerif"
 * @param self Puntero a la instancia de DataManager
 * @param params datos del periférico en formato "tipo,valor"
 * @return Respuesta del comando
 */
static void cmdSetPeriferico(DataManager *self, const String &params)
{
    CmdResponse res;

    // Separa el tipo de periférico y el valor usando ',' como delimitador
    int delimIndex = params.indexOf(',');
    if (delimIndex == -1)
    {
        res.status = 0;
        res.response = "Error. Formato de parámetros inválido.";
        return;
    }

    String tipo = params.substring(0, delimIndex); // Obtener la subcadena del tipo
    tipo.trim();                                   // Trim el tipo

    String valorStr = params.substring(delimIndex + 1); // Obtener la subcadena del valor
    valorStr.trim();                                    // Trim el valor

    uint8_t valor = valorStr.toInt();

    // Verifica que el valor es un dígito y está dentro del rango permitido
    if (valorStr.length() != 1 || !isDigit(valorStr[0]) || (valor > 2))
    {
        res.status = 0;
        res.response = "Error. Valor del periférico inválido.";
    }

    // Mapa de tipos de periféricos a sus índices
    static const std::map<String, int> perifericoMap = {
        {"RF", 0},
        {"WIFI", 1},
        {"VOZ", 2},
        {"PRF", 3}};

    auto it = perifericoMap.find(tipo);
    if (it == perifericoMap.end())
    {
        res.status = 0;
        res.response = "Error. Tipo de periférico no reconocido.";
        return;
    }

    // Actualiza la configuración del periférico
    bool success = self->setPerifericos(it->second, valor);

    // Responde basado en el éxito de la operación
    res.status = success ? 1 : 0;
    res.response = success ? "Ok. config perif " + tipo + " a: " + valorStr
                           : "Error. Al actualizar perif " + tipo;
}

/*
 * @brief Función para el comando "setTimer"
 * @param self Puntero a la instancia de DataManager
 * @param params valor del temporizador en segundos
 * @return Respuesta del comando
 */
static void cmdSetTimer(DataManager *self, const String &params)
{
    CmdResponse res;

    // Verifica que el parámetro es un número válido
    if (!params.length() || !params.toInt())
    {
        res.status = 0;
        res.response = "Error. Valor del temporizador inválido.";
        return;
    }

    uint16_t valor = params.toInt();

    // Valida el valor del temporizador
    if (valor > 600)
    {
        res.status = 0;
        res.response = "Error. Valor del temporizador fuera de rango.";
        return;
    }

    // Actualiza la configuración del temporizador de alarma
    bool success = self->setTimerAlarma(valor);

    // Responde basado en el éxito de la operación
    res.status = success ? 1 : 0;
    res.response = success ? "Ok. Temporizador de alarma actualizado a " + params
                           : "Error. Al actualizar el temporizador de alarma";
}

/*
 * @brief Función para el comando "setTamper"
 * @param self Puntero a la instancia de DataManager
 * @param params valor de habilitación del tamper (0 o 1)
 * @return Respuesta del comando
 */
static void cmdSetTamper(DataManager *self, const String &params)
{
    CmdResponse res;

    uint8_t valor = params.toInt();

    // Verifica que el parámetro es un número válido
    if (params.length() != 1 || !isDigit(params[0]) || (valor > 1))
    {
        res.status = 0;
        res.response = "Error. Valor de habilitación del tamper inválido.";
        return;
    }

    // Actualiza la configuración de habilitación del tamper
    bool success = self->setTamper(valor);

    // Responde basado en el éxito de la operación
    res.status = success ? 1 : 0;
    res.response = success ? "Ok. Habilitación del tamper actualizada a " + params
                           : "Error. Al actualizar la habilitación del tamper";

    return;
}

static void cmdmqttHandShakeRequest(DataManager *self, const String &params)
{
    datosJson.clear();                           // Limpiar el documento JSON global antes de usarlo
    datosJson["dsp"] = String(Data.numeroSerie); //
    datosJson["type"] = "hd-req";                // Tipo de mensaje

    // Crear un objeto JSON dentro de "datos"
    JsonObject datos = datosJson.createNestedObject("datos");
    datos["mdl"] = String(BOARD_MODELO);
    datos["frm"] = String(SISTEMA_FIRMWARE);
    datos["vrs"] = String(SISTEMA_VERSION);
    datos["bld"] = String(SISTEMA_BUILD);
    datos["tm-bl"] = String(SISTEMA_DATE);

    datos["tm"] = TimeManager::getInstance().getTimeISO8601();
    datos["ntp_status"] = TimeManager::getInstance().ntp_status;

    LOG("\r\n\r\ncmdmqttHandShakeRequest Respuesta:\r\n");
    serializeJsonPretty(datosJson, Serial);

    serializeJsonToBuffer();
}

static void cmdinfoInitServer(DataManager *self, const String &params)
{

    jsonRspCmd("infoInitServer", "Info de sistema");

    // Crear un objeto JSON dentro de "datos"
    JsonObject datos = datosJson.createNestedObject("datos");

    // Agregar los campos al objeto "datos"
    datos["modelo"] = String(BOARD_MODELO);
    datos["firmware"] = String(SISTEMA_FIRMWARE);
    datos["version"] = String(SISTEMA_VERSION);
    datos["build"] = String(SISTEMA_BUILD);
    datos["fecha"] = String(SISTEMA_DATE);
    datos["numSerie"] = self->numeroSerie;
    datos["tamper"] = self->tamper;
    datos["timer"] = self->timerAlarma;
    datos["lm_ctrl"] = String(CTRL_LIMITE_CTRL);

    // Crear un objeto JSON dentro de "escaneo_wifi"
    JsonObject escaneo_wifi = datosJson.createNestedObject("escaneo_wifi");

    // Crear un nuevo apartado para redes guardadas disponibles
    JsonObject red_gd_dsp = datosJson.createNestedObject("red_gd_dsp");
    JsonArray redesGuardadasDisponibles = red_gd_dsp.createNestedArray("redes");
    uint16_t redesGuardadasDisponiblesTot = 0;
    uint16_t redesDisponiblesTot = 0;

    if (escaneoWiFi.redesTotales > 0)
    {
        // Crear un array JSON para las redes WiFi
        JsonArray redesArray = escaneo_wifi.createNestedArray("redes");

        // Recorrer el vector de redes disponibles
        for (size_t i = 0; i < escaneoWiFi.redesTotales; i++)
        {
            bool esGuardada = false;

            // Verificar si la red está en redes guardadas
            for (const auto &redGuardada : self->redesGuardadas)
            {
                // Comparar el SSID y el nivel de señal
                // Si la red guardada tiene un nivel de señal menor o igual a 2, agregarla a "red_gd_dsp"
                if (String(escaneoWiFi.ssid[i]) == redGuardada.ssid && escaneoWiFi.rssiNivel[i] <= 2)
                {
                    // Agregar la red a "red_gd_dsp"
                    JsonObject redObj = redesGuardadasDisponibles.createNestedObject();
                    redObj["ssid"] = escaneoWiFi.ssid[i];
                    redObj["rssi"] = escaneoWiFi.rssi[i];
                    redObj["rssi_nivel"] = escaneoWiFi.rssiNivel[i];
                    redObj["seguridad"] = escaneoWiFi.seguridad[i];
                    redObj["canal"] = escaneoWiFi.canal[i];
                    redesGuardadasDisponiblesTot++;
                    esGuardada = true;
                    break;
                }
            }

            // Si no es una red guardada, agregarla a "escaneo_wifi"
            // Si la red no está guardada y su nivel de señal es menor o igual a 2, agregarla a "redes"
            if (!esGuardada && escaneoWiFi.rssiNivel[i] <= 2)
            {
                JsonObject redObj = redesArray.createNestedObject();
                redObj["ssid"] = escaneoWiFi.ssid[i];
                redObj["rssi"] = escaneoWiFi.rssi[i];
                redObj["rssi_nivel"] = escaneoWiFi.rssiNivel[i];
                redObj["seguridad"] = escaneoWiFi.seguridad[i];
                redObj["canal"] = escaneoWiFi.canal[i];
                redesDisponiblesTot++;
            }
        }
    }

    // Agregar los campos al objeto "escaneo_wifi"
    // Actualizar el total de redes disponibles
    escaneo_wifi["redes_tot"] = redesDisponiblesTot;
    // Actualizar el total de redes guardadas disponibles
    red_gd_dsp["redes_tot"] = redesGuardadasDisponiblesTot;

    // Crear un objeto JSON dentro de "redes_guardadas"
    JsonObject redes_guardadas = datosJson.createNestedObject("redes_guardadas");
    redes_guardadas["redes_tot"] = self->redesGuardadas.size(); // Total de redes guardadas
    // redesGuardadas es un vector de estructuras RedGuardada
    // Crear un array JSON para las redes WiFi guardadas
    JsonArray redesArray = redes_guardadas.createNestedArray("redes");
    // Recorrer el vector de redes guardadas
    for (const auto &red : self->redesGuardadas)
    {
        // Crear un objeto JSON para cada red guardada
        JsonObject redObj = redesArray.createNestedObject();
        redObj["ssid"] = red.ssid;         // Nombre de la red WiFi guardada
        redObj["password"] = red.password; // Contraseña de la red WiFi guardada
    }

    LOG("\r\n\r\ncmdinfoInitServer Respuesta:\r\n");
    serializeJsonPretty(datosJson, Serial);

    serializeJsonToBuffer();

    // LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdgetCtrlModelos(DataManager *self, const String &params)
{

    jsonRspCmd("getCtrlModelos", "Lista de modelos y espacio vacio");

    // Crear un array JSON para los modelos
    JsonArray datosArray = datosJson.createNestedArray("datos");

    // Recorrer el vector de controles cargados
    for (const auto &control : self->CtrlModelos)
    {
        // Crear un objeto JSON para cada modelo
        JsonObject modeloObj = datosArray.createNestedObject();
        modeloObj["modelo"] = control.nombre; // Nombre del modelo
    }

    JsonObject modeloObj = datosArray.createNestedObject(); //
    modeloObj["espacio_vacio"] = self->espacioVacioDspRF(); // Agregar espacio vacío al JSON

    serializeJsonToBuffer();

    LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdsetDetectarDispRF(DataManager *self, const String &params)
{

    uint8_t config = params.toInt(); // Obtener el número de control del parámetro

    if (config > 1)
    {
        return;
    }

    self->deteccionDispRF = config; // Actualiza la bandera de detección de dispositivo RF
    self->detecConfigComp = 0;      // Reinicia la bandera de configuración de control
    bool respuesta_fnc = jsonRspHeader("setDetectarDispRF", "Inicio de deteccion de dispositivo RF", "ok");

    if (respuesta_fnc == false)
    {
        return;
    }

    // LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdsetCnfNvRF(DataManager *self, const String &params)
{

    // Separar los campos usando ',' como delimitador
    int delimIndex = params.indexOf(','); // Encuentra la posición del primer delimitador
    if (delimIndex == -1)
    {
        LOG("Error: Formato de parámetros inválido. Falta , el delimitador.");
        return;
    }

    // Extraer el primer campo (status)
    String status_str = params.substring(0, delimIndex);
    status_str.trim();                        // Elimina espacios en blanco
    uint16_t status_int = status_str.toInt(); // Convertir a número entero

    // Buscar el delimitador para el segundo campo
    int secondDelimIndex = params.indexOf(',', delimIndex + 1);
    if (secondDelimIndex == -1)
    {
        // Manejar error si no se encuentra el segundo delimitador
        LOG("Error: Formato de parámetros inválido. Falta el segundo delimitador.");
        return;
    }

    // Extraer el segundo campo (nm_ctrl)
    String nm_ctrl = params.substring(delimIndex + 1, secondDelimIndex);
    nm_ctrl.trim();                          // Elimina espacios en blanco
    uint16_t num_ctrl_int = nm_ctrl.toInt(); // Convertir a número entero

    // Extraer el tercer campo (mdl)
    String mdl = params.substring(secondDelimIndex + 1, params.length() - 1);
    mdl.trim(); // Elimina espacios en blanco

    LOG("\r\nmdl: " + mdl);
    LOG("\r\nstatus_int: " + String(status_int));
    LOG("\r\nnum_ctrl_int: " + String(num_ctrl_int));

    bool rsp = self->guardarDspRFLoop(status_int, num_ctrl_int, mdl.c_str());

    jsonRspHeader("setCnfNvRF", "Guardado de dispositivo RF", rsp == true ? "ok" : "error");

    LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdgetInfoDspRFLoop(DataManager *self, const String &params)
{

    jsonRspCmd("getInfoDspRFLoop", "Informacion del dispositivo RF de Loop");

    // Crear un objeto JSON dentro de "datos"
    JsonObject datos = datosJson.createNestedObject("datos");
    // Agregar los campos al objeto "datos"
    datos["nombre"] = modeloCtrlAVRx.nombre; // Nombre del control

    String configs[CTRL_TP_DP_LIMITE] = {CTRL_TP_DSP_RF_1, CTRL_TP_DSP_RF_2, CTRL_TP_DSP_RF_3};

    // Tipo de control
    if (modeloCtrlAVRx.configuracion > 0 && modeloCtrlAVRx.configuracion <= CTRL_TP_DP_LIMITE)
        datos["tipo"] = configs[modeloCtrlAVRx.configuracion - 1];
    else
        datos["tipo"] = "Error";

    datos["codificador"] = modeloCtrlAVRx.codificador; // Codificador del control
    datos["control"] = estadoCompRFAv.control;         // Número de control del dispositivo RF
    datos["btn_presionado"] = estadoCompRFAv.btnStr;   // Botón presionado del control

    // Serializar el JSON en el buffer global
    serializeJsonToBuffer();

    LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdborrar1Ctrl(DataManager *self, const String &params)
{

    // Verifica que el parámetro es un número válido
    uint16_t num_ctrl = params.toInt(); // Obtener el número de control del parámetro
    if (num_ctrl > CTRL_LIMITE_CTRL)
    {
        jsonRspHeader("borrar1Ctrl", "Borrado de 1 control", "Error. numero de control invalido o fuera de rango");
        LOG("\r\ncmdborrar1Ctrl. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    if (self->existeDspRFRAM(num_ctrl) == false)
    {

        jsonRspHeader("borrar1Ctrl", "Borrado de 1 control", "Error. Control no existe en memoria.");
        LOG("\r\ncmdborrar1Ctrl. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    if (!self->borrar1DspRFAV(num_ctrl))
    {
        jsonRspHeader("borrar1Ctrl", "Borrado de 1 control", "Error. Al borrar el control");
        LOG("\r\ncmdborrar1Ctrl. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    jsonRspHeader("borrar1Ctrl", "Borrado de 1 control", "Ok. Control borrado de memoria.");

    LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdinfo1Ctrl(DataManager *self, const String &params)
{

    // Verifica que el parámetro es un número válido
    uint16_t num_ctrl = params.toInt(); // Obtener el número de control del parámetro
    if (num_ctrl > CTRL_LIMITE_CTRL)
    {
        jsonRspHeader("info1Ctrl", "Info de 1 control", "Error. Número de control inválido o fuera de rango");
        LOG("\r\ncmdinfo1Ctrl. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    if (self->existeDspRFRAM(num_ctrl) == false)
    {
        jsonRspHeader("info1Ctrl", "Info de 1 control", "Error. Control no existe en memoria.");
        LOG("\r\ncmdinfo1Ctrl. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    jsonRspCmd("info1Ctrl", "Resultado de info de 1 control");

    CTRL_MODELO_AV modeloCtrl1Info = {}; // Variable para almacenar el control encontrado

    for (const auto &controlValue : self->controlValues)
    {
        uint16_t controlNumber = std::get<1>(controlValue);
        if (controlNumber == num_ctrl)
        {
            char id_cnf_ctrl = std::get<0>(controlValue);
            for (const auto &control : self->CtrlModelos)
            {
                if (control.id_cnf_ctrl == id_cnf_ctrl)
                {
                    modeloCtrl1Info = control; // Asignar el control encontrado a la variable global
                    break;                     // Salir del bucle una vez encontrado el control
                }
            }
            break;
        }
    }

    // Crear un objeto JSON dentro de "datos"
    JsonObject datos = datosJson.createNestedObject("datos");
    // Agregar los campos al objeto "datos"
    datos["nombre_modelo"] = modeloCtrl1Info.nombre;    // Nombre del modelo del control
    datos["codificador"] = modeloCtrl1Info.codificador; // Codificador del control
    datos["num_control"] = num_ctrl;                    // Número de control del dispositivo RF

    // Serializar el JSON en el buffer global
    serializeJsonToBuffer();

    LOG("\r\nRespuesta: " + String(self->jsonCMDSerial));
}

static void cmdborrar1RedWifi(DataManager *self, const String &params)
{

    if (params.length() == 0)
    {
        jsonRspHeader("borrar1RedWifi", "Borrado de red WiFi", "Error. Falta el nombre de la red WiFi.");
        LOG("\r\ncmdborrar1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    if (!self->borrar1RedWiFi(params.c_str())) // Llama a la función para borrar la red WiFi
    {
        jsonRspHeader("borrar1RedWifi", "Borrado de red WiFi", self->rspMetodo);
        LOG("\r\ncmdborrar1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
    }
    else
    {
        jsonRspHeader("borrar1RedWifi", "Borrado de red WiFi", "Ok. Red WiFi borrada.");
        LOG("\r\ncmdborrar1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
    }
}

static void cmdset1RedWifi(DataManager *self, const String &params)
{

    if (params.length() == 0)
    {
        jsonRspHeader("set1RedWifi", "Guardado de 1 red WiFi", "Error. Info vacia");
        LOG("\r\ncmdset1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    // Separar los campos usando ',' como delimitador
    int delimIndex = params.indexOf(','); // Encuentra la posición del primer delimitador
    if (delimIndex == -1)
    {
        jsonRspHeader("set1RedWifi", "Guardado de 1 red WiFi", "Error. Formato de parámetros inválido. Falta , el delimitador.");
        LOG("\r\ncmdset1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    // Extraer el primer campo (status)
    String red_x = params.substring(0, delimIndex);

    // Buscar el delimitador para el segundo campo
    int secondDelimIndex = params.indexOf(',', delimIndex + 1);
    if (secondDelimIndex == -1)
    {
        // Manejar error si no se encuentra el segundo delimitador
        jsonRspHeader("set1RedWifi", "Guardado de 1 red WiFi", "Error. Falta el segundo delimitador.");
        LOG("\r\ncmdset1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    String clave_x = params.substring(delimIndex + 1, secondDelimIndex);

    self->guardarRedWiFi(red_x.c_str(), clave_x.c_str()); // Llama a la función para borrar la red WiFi
    jsonRspHeader("set1RedWifi", "Guardado de 1 red WiFi", self->rspMetodo);
    LOG("\r\ncmdborrar1RedWifi. Respuesta: " + String(self->jsonCMDSerial));
}

static void cmdgetRedesWifi(DataManager *self, const String &params)
{
    String info;

    if (self->redesGuardadas.size() == 0)
    {
        jsonRspHeader("getRedesWifi", "Redes WiFi guardadas", "Error. No hay redes guardadas.");
        LOG("\r\ncmdgetRedesWifi. Respuesta: " + String(self->jsonCMDSerial));
        return;
    }

    info = "Redes Guardadas: ";                  // Inicializa la cadena de información
    info += String(self->redesGuardadas.size()); // Agrega el número total de redes

    for (const auto &red : self->redesGuardadas)
    {
        info += "\r\nRed: ";                                   //
        info += String((&red - &self->redesGuardadas[0]) + 1); // Calcula el índice de la red
        info += "\r\nSSID: ";                                  // Agrega el encabezado de SSID
        info += red.ssid;                                      // Agrega el SSID de la red
        info += "\r\nPassword: ";                              // Agrega el encabezado de contraseña
        info += red.password;                                  // Agrega la contraseña de la red

        LOGF("\r\n\r\n%s\r\n%s", red.ssid.c_str(), red.password.c_str());
    }
    LOG("\r\n\r\n");

    jsonRspHeader("getRedesWifi", "Guardado de 1 red WiFi", info.c_str());
    LOG("\r\ncmdgetRedesWifi. Respuesta: " + String(self->jsonCMDSerial));
}

/**
 * @brief Función auxiliar para serializar un documento JSON en el buffer global.
 * @return true si la serialización fue exitosa, false en caso contrario.
 */
bool serializeJsonToBuffer(void)
{
    // Limpiar el buffer antes de usarlo
    memset(DataManager::jsonCMDSerial, 0, sizeof(DataManager::jsonCMDSerial));

    // Serializar el JSON en el buffer
    size_t len = serializeJson(datosJson, DataManager::jsonCMDSerial, sizeof(DataManager::jsonCMDSerial));

    datosJson.clear(); // Limpiar el documento JSON después de usarlo

    // Verificar si el JSON se serializó correctamente
    if (len == 0)
    {
        LOG("\r\nError: No se pudo serializar el JSON en el buffer.");
        return false;
    }
    return true;
}

/**
 * @brief Genera un JSON con el encabezado y lo almacena en jsonCMDSerial.
 * @param cod_respuesta Código único para identificar la respuesta.
 * @param descripcion Mensaje descriptivo sobre la operación.
 * @param rsp Respuesta adicional (opcional).
 */
bool jsonRspHeader(const char *cod_respuesta, const char *descripcion, const char *rsp)
{
    datosJson.clear(); // Limpiar el documento JSON global antes de usarlo

    datosJson["cod_respuesta"] = cod_respuesta;
    datosJson["descripcion"] = descripcion;
    if (rsp)
    {
        datosJson["rsp"] = rsp;
    }

    // Serializar el JSON en el buffer global
    if (serializeJsonToBuffer())
        return true;
    else
        return false; // Retornar false si la serialización falla
}

/**
 * @brief Genera un JSON con el encabezado y los datos, y lo almacena en jsonCMDSerial.
 * @param cod_respuesta Código único para identificar la respuesta.
 * @param descripcion Mensaje descriptivo sobre la operación.
 */
void jsonRspCmd(const char *cod_respuesta, const char *descripcion)
{

    datosJson.clear();                          // Limpiar el documento JSON global antes de usarlo
    datosJson["cod_respuesta"] = cod_respuesta; //
    datosJson["descripcion"] = descripcion;
}
