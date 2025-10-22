#include "WiFiTool.h"
#include <LogSistema.h>

/**
 * @brief Inicia un escaneo WiFi de manera asíncrona.
 */
void WiFiTool::scanAsyncInit()
{
    static unsigned long lastScanTime = 0;

    if (millis() - lastScanTime < 3000)
    {
        LOG_WIFI("⌛ Esperando antes de iniciar un nuevo escaneo...");
        return;
    }

    LOG("\r\nIniciando escaneo WiFi asincrono...");
    redesDisponibles.clear();
    WiFi.scanNetworks(true); // Escaneo en segundo plano
    lastScanTime = millis();
    scanInProgress = true; // 🔹 Marcar que el escaneo ha comenzado
}

/**
 * @brief Verifica si el escaneo asíncrono ha terminado y almacena los resultados.
 * @return true si el escaneo terminó, false si aún está en progreso.
 */
bool WiFiTool::scanAsyncStatus()
{
    if (!scanInProgress) //
        return false;    // 🔹 Si no hay escaneo en curso, no hacer nada

    int scanResult = WiFi.scanComplete();

    if (scanResult == WIFI_SCAN_RUNNING)
    {
        return false;
    }

    scanInProgress = false; // 🔹 Marcar que el escaneo terminó

    if (scanResult > 0)
    {
        LOGF("\r\nEscaneo finalizado. Se encontraron %d redes.\n", scanResult);

        for (int i = 0; i < scanResult; i++)
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
        //WiFi.scanDelete(); // Limpia los datos de escaneo en el WiFi
        return true;
    }

    return false;
}