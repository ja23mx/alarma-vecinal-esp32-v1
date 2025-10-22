#ifndef SERVER_WEB_H
#define SERVER_WEB_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "WiFiTool.h"

#define DEBUG_SERVER_WEB // Activa los logs de depuración

#ifdef DEBUG_SERVER_WEB
#define LOG_SERVER(x) Serial.print(x)
#define LOGF_SERVER(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define LOG_SERVER(x)
#define LOGF_SERVER(fmt, ...)
#endif

/**
 * @brief Clase principal para gestionar el servidor web e integrar la seccion RF.
 */
class ServerWeb
{
public:
    /**
     * @brief Constructor de la clase ServerWeb.
     * @param WiFiToolRef Referencia a la instancia de WiFiTool.
     */
    ServerWeb(WiFiTool &WiFiTools);

    /**
     * @brief Inicializa el servidor web y la seccion RF.
     * @param origenConfig origen de la configuración (0: Boton Prog, 1: Control Integrador).
     * @return uint8_t Retorna 0: fin por boton, 1: fin por control integrador.
     * @details Esta función configura el servidor web y establece las rutas para manejar las solicitudes HTTP
     */
    uint8_t begin(uint8_t origenConfig);

    void url_api_data();

    // controles rf seccion  ////////////
    void url_setDetectarDispRF();
    void url_revDetecSenalRF();
    void url_getCtrlModelos();
    void url_setCnfNvRF();
    void url_borrar1Ctrl();
    void url_info1Ctrl();
    /////////////////////////////////////

    // wifi seccion /////////////////////
    void url_borrar1RedWifi();
    /////////////////////////////////////

    void url_cmdSerial();

    void url_ota(void);
    void initServer();

private:
    AsyncWebServer *server; // Puntero al servidor asyncrono.
    WiFiTool &WiFiTools;    // Referencia a la instancia de WiFiTool

    void cmdSerialFast(AsyncWebServerRequest *request, const String &comando);
    void configurarCaptivePortal();
};

#endif // SERVER_WEB_H
