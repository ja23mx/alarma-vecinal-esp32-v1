#include "WiFiTool.h"
#include <ArduinoJson.h>

#include <VariablesGlobales.h>

// 🔹 Inicia el escaneo de redes WiFi
bool WiFiTool::scanInit(void)
{
    LOG_WIFI("\r\n\r\nIniciando escaneo WiFi...");
    redesDisponibles.clear();
    int numNetworks = 0;

    // 🔹 Escaneo estándar (escanea todos los canales disponibles)
    // LOG_WIFI("\r\nEscaneo en todos los canales...");
    numNetworks = WiFi.scanNetworks();

    if (numNetworks > 0)
    {
        for (int i = 0; i < numNetworks; i++)
        {
            RedWiFi red;
            red.ssid = WiFi.SSID(i);
            red.rssi = WiFi.RSSI(i);
            red.rssiNivel = calcularRssiNivel(red.rssi);
            red.seguridad = WiFi.encryptionType(i);
            red.canal = WiFi.channel(i);

            redesDisponibles.push_back(red);
        }

        eliminarDuplicados();
        ordenarPorIntensidad();
        LOG_WIFI("\r\n\r\nEscaneo finalizado");

        escaneoWiFi = getEscaneoStatus(false); // Obtener el estado del escaneo de WiFi
        // LOGF_WIFI("\r\nEscaneo finalizado\r\nSe encontraron %d redes.\n", redesDisponibles.size());
        return true; // Retorna verdadero si se encontraron redes
    }

    LOG_WIFI("\r\nNo se encontraron redes WiFi.");
    return false; // Retorna falso si no se encontraron redes
}

// 🔹 Retorna la lista de redes escaneadas
const std::vector<RedWiFi> &WiFiTool::getRedesDisponibles() const
{
    return redesDisponibles;
}

/**
 * @brief Obtiene el estado del último escaneo WiFi en una estructura con información detallada.
 *
 * @param analisis_ap Si es `true`, calculará el canal menos saturado para usarlo en la red AP.
 *                    Si es `false`, no se realizará el análisis y `canalAP` será 0.
 *
 * @return ESCANEO_STATUS Estructura que contiene:
 *         - `redesTotales`: Número total de redes detectadas.
 *         - `ssid`: Lista de nombres de redes detectadas.
 *         - `rssi`: Intensidad de señal en dBm para cada red.
 *         - `rssiNivel`: Nivel de señal categorizado (1 = fuerte, 2 = media, 3 = débil).
 *         - `seguridad`: Tipo de seguridad de la red (0 = abierta, 1 = WEP, 2 = WPA, etc.).
 *         - `canal`: Canal en el que opera cada red detectada.
 *         - `canalAP`: Canal menos saturado recomendado para crear la red AP (si `analisis_ap` es `true`).
 */
ESCANEO_STATUS WiFiTool::getEscaneoStatus(bool analisis_ap)
{
    ESCANEO_STATUS status;
    status.redesTotales = redesDisponibles.size();

    for (const auto &red : redesDisponibles)
    {
        status.ssid.push_back(red.ssid);
        status.rssi.push_back(red.rssi);
        status.rssiNivel.push_back(static_cast<uint8_t>(red.rssiNivel));
        status.seguridad.push_back(static_cast<uint8_t>(red.seguridad));
        status.canal.push_back(static_cast<uint8_t>(red.canal));
    }

    status.canalAP = busquedaCanalAp(); // 🔹 Método privado calcula el canal óptimo

    return status;
}

// 🔹 Genera JSON con redes detectadas
String WiFiTool::scanJSON() const
{
    // 🔹 Calcular el tamaño del JSON dinámicamente
    size_t jsonSize = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(redesDisponibles.size());

    for (size_t i = 0; i < redesDisponibles.size(); i++)
    {
        jsonSize += JSON_OBJECT_SIZE(5); // Cada red tiene 5 campos
    }

    DynamicJsonDocument doc(jsonSize);
    JsonArray array = doc.createNestedArray("redes");

    for (const auto &red : redesDisponibles)
    {
        JsonObject obj = array.createNestedObject();
        obj["ssid"] = red.ssid;
        obj["rssi"] = red.rssi;
        obj["rssiNivel"] = red.rssiNivel;
        obj["seguridad"] = red.seguridad;
        obj["canal"] = red.canal;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

// 🔹 Genera JSON con estado de las redes y conexión actual
String WiFiTool::scanStatus() const
{
    size_t jsonSize = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(redesDisponibles.size());

    for (size_t i = 0; i < redesDisponibles.size(); i++)
    {
        jsonSize += JSON_OBJECT_SIZE(5);
    }

    DynamicJsonDocument doc(jsonSize);
    doc["conectado"] = WiFi.isConnected();
    doc["ip"] = WiFi.isConnected() ? WiFi.localIP().toString() : "0.0.0.0";
    doc["redes"] = scanJSON();

    String output;
    serializeJson(doc, output);
    return output;
}

// 🔹 Filtra redes repetidas con el mismo SSID y canal
void WiFiTool::eliminarDuplicados()
{
    std::vector<RedWiFi> filtradas;
    for (const auto &red : redesDisponibles)
    {
        bool existe = false;
        for (const auto &f : filtradas)
        {
            if (f.ssid == red.ssid)
            {
                existe = true;
                break;
            }
        }
        if (!existe)
            filtradas.push_back(red);
    }
    redesDisponibles = filtradas;
}

// 🔹 Ordena las redes por intensidad de señal (RSSI mayor primero)
void WiFiTool::ordenarPorIntensidad()
{
    std::sort(redesDisponibles.begin(), redesDisponibles.end(), [](const RedWiFi &a, const RedWiFi &b)
              { return a.rssi > b.rssi; });
}

// 🔹 Determina el nivel de señal basado en el RSSI
int WiFiTool::calcularRssiNivel(int rssi) const
{
    if (rssi >= -50) //
        return 1;    // Fuerte
    if (rssi >= -70) //
        return 2;    // Media
    return 3;        // Débil
}

/**
 * @brief Encuentra el canal WiFi óptimo basado en escaneo de redes
 * @return Canal WiFi recomendado (1, 6 u 11 preferentemente)
 */
uint8_t WiFiTool::busquedaCanalAp(void)
{
    int canales[14] = {0};                       // Ponderación de interferencia por canal
    const uint8_t canalesIdeales[] = {1, 6, 11}; // Canales no solapados

    // 🔹 Ponderar cada red según su intensidad de señal
    for (const auto &red : redesDisponibles)
    {
        if (red.canal >= 1 && red.canal <= 13)
        {
            // Red más fuerte = más interferencia (valor absoluto de RSSI)
            int peso = (red.rssi < -90) ? 1 : // Redes muy débiles casi no afectan
                           (red.rssi < -80) ? 2
                                            : // Redes débiles
                           (red.rssi < -70) ? 4
                                            : // Redes moderadas
                           8;                 // Redes fuertes
            canales[red.canal] += peso;

            // Considerar solapamiento de canales adyacentes (+/- 3)
            for (int i = -3; i <= 3; i++)
            {
                if (i != 0 && red.canal + i >= 1 && red.canal + i <= 13)
                {
                    canales[red.canal + i] += peso / 2;
                }
            }
        }
    }

    // 🔹 Buscar mejor canal entre los ideales (1, 6, 11)
    uint8_t mejorCanal = 1;
    int menorInterferencia = INT_MAX;

    for (uint8_t canal : canalesIdeales)
    {
        if (canales[canal] < menorInterferencia)
        {
            menorInterferencia = canales[canal];
            mejorCanal = canal;
        }
    }

    // 🔹 Si todos los canales ideales están muy saturados, buscar otro
    if (menorInterferencia > 20)
    { // Umbral empírico
        for (uint8_t canal = 1; canal <= 13; canal++)
        {
            if (canales[canal] < menorInterferencia)
            {
                menorInterferencia = canales[canal];
                mejorCanal = canal;
            }
        }
    }

    LOGF_WIFI("Canal seleccionado: %d (Interferencia: %d)\n", mejorCanal, menorInterferencia);
    return mejorCanal;
}