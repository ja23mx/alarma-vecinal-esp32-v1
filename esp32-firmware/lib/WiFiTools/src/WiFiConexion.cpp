#include "WiFiTool.h"
#include "LogSistema.h"

#include "ConfigSistema.h"

// Constructor (asigna la instancia actual)
WiFiTool::WiFiTool()
{
}

/**
 * @brief Inicia la conexión WiFi con un tiempo límite.
 * @param ssid Nombre de la red WiFi.
 * @param password Clave de la red WiFi.
 * @param timeout Tiempo máximo para intentar la conexión (ms).
 */
bool WiFiTool::startConnection(const char *ssid, const char *password, unsigned long timeout)
{

    LOG("\r\n\r\nIniciando conexion WiFi...");

    unsigned long startTime = millis();

    WiFi.begin(ssid, password); // Iniciar conexión WiFi

    for (size_t i = 0; i < 4; i++)
    {
        digitalWrite(LED_STATUS, LED_ST_ON); // Encender LED de estado
        delay(500);
        digitalWrite(LED_STATUS, LED_ST_OFF); // Apagar LED de estado
        delay(500);
    }

    bool ledStatus = false; // Variable para almacenar el estado del LED

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - startTime > timeout)
        {
            LOG("\r\nTiempo de espera agotado. Cancelando conexion."); //
            cancelConnection();                                        // Cancelar conexión si se agota el tiempo
            return false;                                              // Indicar que no se pudo conectar
        }
        else if (WiFi.status() == WL_CONNECT_FAILED)                  // Verificar si la conexión falló
        {                                                             //
            LOG("\r\nConexion fallida. Verificando credenciales..."); // Si la conexión falla, verificar credenciales
            cancelConnection();                                       // Cancelar conexión
            return false;                                             // Indicar que no se pudo conectar
        }
        ledStatus = !ledStatus;                                       // Cambiar el estado del LED
        digitalWrite(LED_STATUS, ledStatus ? LED_ST_ON : LED_ST_OFF); // Cambiar el estado del LED de estado
        delay(500);                                                   // Esperar un segundo antes de volver a verificar
    }

    digitalWrite(LED_STATUS, LED_ST_OFF);                       // Apagar LED de estado
                                                                //
    LOGF("\r\n\r\n\r\nConectado a %s", ssid);                   // Indicar que se ha conectado
    LOGF("\r\nIP: %s", WiFi.localIP().toString().c_str());      // Mostrar la IP local asignada
    LOGF("\r\nTiempo Conexion: %lu\r\n", millis() - startTime); // Mostrar la dirección MAC local asignada

    return true; // Indicar que se ha conectado correctamente
}

/**
 * @brief Cancela la conexión WiFi manualmente.
 */
void WiFiTool::cancelConnection()
{
    LOG("\r\n\r\nCancelando conexión WiFi...");

    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA)
    {
        WiFi.disconnect(); // Desconecta del router
        LOG("\r\n\r\nWiFi STA desconectado.");
    }

    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
    {
        LOG("\r\n\r\nModo AP sigue activo. Clientes pueden seguir conectados.");
    }
    else
    {
        WiFi.mode(WIFI_OFF); // Solo apagamos WiFi si no hay AP activo
        LOG("\r\n\r\nWiFi apagado completamente.");
    }
}