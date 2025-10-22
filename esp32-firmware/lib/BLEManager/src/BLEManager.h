// BLEManager.h
#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H
#ifdef dump
#undef dump
#endif
#include <NimBLEDevice.h>

#include <Arduino.h>

#include <WiFiTool.h>

#define BLE_RESPONSE_SIZE 512 // Ajustable según el tamaño esperado

class BLEManager
{
public:
    BLEManager(WiFiTool &WiFiTools);

    void setClientConnected(bool estado);

    void begin(const char *deviceName); // Iniciar BLE
    void end();                         // Apagar BLE
    void loop();                        // Procesar datos BLE
    bool isConnected();                 // Saber si hay un cliente conectado

    String dataReceived; // Datos recibidos

private:
    bool usuarioAutenticado = false;

    // Metodos privados de BLEManager
    void enviarRespuestaBLE();
    void enviarStrBLE(String rsp);
    void enviarRespuestaBLE_segmentado();
    bool paqueteXL(const std::string &data);

    void procesarComandoBLE(const String &comando);
    void manejarLogin(const String &parametros);
    void manejarWiFiBLE(const String &parametros);

    bool verificarLogin(const String &pin);

    void wifiBLEScanInit(const String &parametros);
    void wifiBLEScanFin(const String &parametros);
    void generarResumenEscaneo();
    void wifiBLEInitConexion(const String &parametros);
    void wifiBLEStatusConexion(const String &parametros);

    // metodos de BLEManager con RF ///////////////////////////////////////////////////////////////
    void manejarWiFiRF(const String &cmd);
    void rfBLEnvctrl(const String &parametros);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    NimBLEServer *pServer;
    NimBLECharacteristic *pRxCharacteristic;
    NimBLECharacteristic *pTxCharacteristic;
    bool _bleEnabled;
    bool _clientConnected;

    class ServerCallbacks : public NimBLEServerCallbacks
    {
    public:
        ServerCallbacks(BLEManager *manager) : bleManager(manager) {}
        void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo);
        void onDisconnect(NimBLEServer *pServer);

    private:
        BLEManager *bleManager;
    };

    unsigned long _beginTime;
    char _rsp_cmd_ble[BLE_RESPONSE_SIZE]; // Buffer de respuesta BLE

private:
    WiFiTool &wifiTools; // Referencia al objeto WiFiTool
};

#endif
