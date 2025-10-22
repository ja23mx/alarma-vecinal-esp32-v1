#include <Arduino.h>

#include <LogSistema.h>
#include <BLEManager.h> //
#include <map>          // 🔹 Asegura que std::map esté disponible

void BLEManager::manejarWiFiBLE(const String &parametros)
{
    static const std::map<String, void (BLEManager::*)(const String &)> wifiBLECommands = {
        {"scaninit", &BLEManager::wifiBLEScanInit},
        {"scanfin", &BLEManager::wifiBLEScanFin},
        {"ci", &BLEManager::wifiBLEInitConexion},
        {"cf", &BLEManager::wifiBLEStatusConexion}};

    Serial.printf("\r\n📥 Comando WiFi recibido: %s\n", parametros.c_str());

    // 📌 Extraer el comando y los argumentos
    String command = parametros;
    String args = "";

    if (parametros.startsWith("ci"))
    {
        command = "ci";
        args = parametros.substring(2); // Extraer desde después de "cnxini"
    }

    LOGF("\r\nProcesando comando: %s | Argumentos: %s\n", command.c_str(), args.c_str());

    auto it = wifiBLECommands.find(command);
    if (it != wifiBLECommands.end())
    {
        (this->*it->second)(args);
    }
    else
    {
        LOG("\r\nComando WiFi BLE no reconocido");
    }
}

void BLEManager::wifiBLEScanInit(const String &parametros)
{
    wifiTools.scanAsyncInit(); // Llama al método de escaneo asíncrono en WiFiTool
    enviarStrBLE("ok");
}
void BLEManager::wifiBLEScanFin(const String &parametros)
{

    if (!wifiTools.scanAsyncStatus()) // Verifica si el escaneo asíncrono ha terminado
    {
        LOG("\r\nEscaneo WiFi en progreso...");
        snprintf(_rsp_cmd_ble, BLE_RESPONSE_SIZE, "0");
        enviarRespuestaBLE();
    }

    LOG("\r\nEscaneo WiFi finalizado");
    generarResumenEscaneo();
}

void BLEManager::generarResumenEscaneo()
{
    const std::vector<RedWiFi> &redes = wifiTools.getRedesDisponibles();
    size_t numRedes = redes.size();

    //memset(_rsp_cmd_ble, 0, BLE_RESPONSE_SIZE); // Limpia el buffer antes de escribir
    //int offset = 0;

    memset(_rsp_cmd_ble, 0, BLE_RESPONSE_SIZE);
    snprintf(_rsp_cmd_ble, BLE_RESPONSE_SIZE - 1, "WFSC:["); // Agregar prefijo
    int offset = strlen(_rsp_cmd_ble);                     // Ajustar offset después de "WFSC"

    for (size_t i = 0; i < numRedes; i++)
    {
        const RedWiFi &red = redes[i];
        offset += snprintf(_rsp_cmd_ble + offset, BLE_RESPONSE_SIZE - offset, "%s[%d,%d,%s]", (i == 0) ? "" : ",", red.rssiNivel, red.seguridad, red.ssid.c_str());
    }
    offset += snprintf(_rsp_cmd_ble + offset, BLE_RESPONSE_SIZE - offset, "]");

    Serial.printf("\r\nResumen de redes: %s\n", _rsp_cmd_ble);

    size_t totalLen = strlen(_rsp_cmd_ble); // Tamaño total del mensaje

    if (totalLen < 21)
    {
        enviarRespuestaBLE();
    }
    else
    {
        enviarRespuestaBLE_segmentado();
    }
}

void BLEManager::wifiBLEInitConexion(const String &parametros)
{
    int pos = parametros.indexOf(',');
    if (pos == -1)
    {
        Serial.println("\r\nError: Formato incorrecto en wifiBLEInitConexion");
        return;
    }

    int numRed = parametros.substring(0, pos).toInt();
    String clave = parametros.substring(pos + 1);

    Serial.printf("\r\nConexión a red %d con clave %s\n", numRed, clave.c_str());

    const std::vector<RedWiFi> &redes = wifiTools.getRedesDisponibles();
    if (numRed < 0 || numRed >= redes.size())
    {
        Serial.println("\r\nError: Índice de red fuera de rango");
        return;
    }

    String ssid = redes[numRed].ssid;
    Serial.printf("\r\n\r\nIntentando conectar a %s con clave %s\n", ssid.c_str(), clave.c_str());

    wifiTools.startConnection(ssid.c_str(), clave.c_str(), 12000); // Timeout
}

void BLEManager::wifiBLEStatusConexion(const String &parametros)
{
    //uint8_t estado = wifiTools.verificarEstadoConexion(); // Verificar estado de conexión
    //enviarStrBLE("cf" + String(estado));                  // Enviar respuesta por BLE
}