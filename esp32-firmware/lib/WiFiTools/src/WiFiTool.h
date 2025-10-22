#ifndef WIFITOOL_H
#define WIFITOOL_H

#include <WiFi.h>
#include <vector>

#include "EstructurasGlobales.h"

#define DEBUG_WIFI_TOOLS // Activa los logs de depuración

#ifdef DEBUG_WIFI_TOOLS
#define LOG_WIFI(x) Serial.print(x)
#define LOGF_WIFI(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define LOG_WIFI(x)
#define LOGF_WIFI(fmt, ...)
#endif

// 🔹 Estructura para almacenar información de redes WiFi
struct RedWiFi
{
    String ssid;
    int rssi;
    int rssiNivel;
    int seguridad;
    int canal;
};

class WiFiTool
{
public:
    WiFiTool();

    // 🔹 Métodos de escaneo de redes (Implementados en WiFiScan.cpp)
    bool scanInit(void);
    const std::vector<RedWiFi> &getRedesDisponibles() const;
    ESCANEO_STATUS getEscaneoStatus(bool analisis_ap);
    String scanJSON() const;
    String scanStatus() const;

    // Métodos de escaneo asíncrono (Implementados en WiFiAsyncScan.cpp)
    void scanAsyncInit();   // Inicia el escaneo asíncrono
    bool scanAsyncStatus(); // Verifica si el escaneo ha terminado

    // 🔹 Métodos de conexión WiFi (Implementados en WiFiConexion.cpp)
    bool startConnection(const char *ssid, const char *password, unsigned long timeout = 10000);
    void cancelConnection();

    // 🔹 Variables de escaneo
    std::vector<RedWiFi> redesDisponibles;

private:
    // 🔹 Variables de escaneo
    void eliminarDuplicados();
    void ordenarPorIntensidad();
    int calcularRssiNivel(int rssi) const;
    uint8_t busquedaCanalAp(void);

    bool scanInProgress = false; // Indica si hay un escaneo en curso
};

#endif // WIFITOOL_H
