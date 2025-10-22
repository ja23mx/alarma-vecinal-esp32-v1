#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <AESLib.h>
#include <LittleFS.h>

#include "EstructurasGlobales.h"
#include <vector>

struct CmdResponse
{
    uint8_t status;  // 0 = error, 1 = ok, 2 = advertencia
    String response; // Mensaje descriptivo
};

struct RedGuardada
{
    String ssid;
    String password;
};

class DataManager
{
public:
    DataManager(); // Inicializar el tamaño del JSON

    // Métodos de inicialización y configuración //////////////////////////////////////////////////
    // datamanager.cpp
    bool begin(void);
    void progSerial(void);
    void rstData(void);
    // datos de configuracion
    bool setNumSerie(const char *numeroDeSerie);
    bool setClaveDispositivo(uint16_t clave);
    bool setPerifericos(uint8_t tipo, uint8_t valor);
    bool setTimerAlarma(uint16_t timer);
    bool setTamper(uint8_t habilitacion);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Métodos de comando serial //////////////////////////////////////////////////////////////////
    // cmdserial.cpp
    void CmdSerial(const String &cmd);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Métodos de controles ///////////////////////////////////////////////////////////////////////
    // controles.cpp
    bool beginCnfCtrl(void);
    bool rstDataCnfCtrl(void);
    bool cargarModelosRF(bool log);
    void infoCtrlModeloLog(CTRL_MODELO_AV &control);
    bool guardarModeloCtrl(const char *json);
    bool revModeloGuardado(const char *modelo);
    bool existeDspRFRAM(uint16_t num_ctrl);
    bool borrar1DspRFAV(uint16_t num_ctrl);
    bool guardarDspRFLoop(uint8_t status, uint16_t num_ctrl, const char *nombre);
    bool guardarDspRFFull(const char *nombre, uint16_t num_ctrl, uint8_t status, unsigned long signal);
    bool cargarDatosDspRF(void);
    bool busqDspRF(unsigned long senalRF);
    uint16_t espacioVacioDspRF(void);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Métodos de archivos en LittleFS ////////////////////////////////////////////////////////////
    // fileslittlefs.cpp
    void listaJsonLF(uint8_t salida_datos);
    void listarArchivosRec(File dir, JsonArray &arr, const String &rutaBase);
    bool archivoExiste(const String &ruta); // Verifica si un archivo existe en el vector
    //////////////////////////////////////////////////////////////////////////////////////////////////

    // Métodos de configuración de red //////////////////////////////////////////////////////////
    // WiFi.cpp
    bool guardarRedWiFi(const char *ssid, const char *password);
    const char *leerRedWiFi(const char *ssid);
    std::vector<RedGuardada> leerTodasLasRedesGuardadas();
    bool borrar1RedWiFi(const char *ssid);
    /////////////////////////////////////////////////////////////////////////////////////////////////

    // Configuración del sistema //////////////////////////////////////////////////////////////////
    char numeroSerie[11];    // Número de Serie (hasta 10 caracteres)
    uint16_t pinDispositivo; // Clave de Dispositivo (4 dígitos)
    uint8_t perifericos[4];  // Configuración de Periféricos (RF, WIFI, VOZ, PERIFONEO)
    uint16_t timerAlarma;    // Timer de Alarma (0-600)
    uint8_t tamper;          // Habilitación de Tamper (0 o 1)
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Configuración de controles /////////////////////////////////////////////////////////////////
    // Vector para almacenar las configuraciones cargadas
    std::vector<CTRL_MODELO_AV> CtrlModelos;
    // Vector para almacenar las configuraciones de RF
    // char -> id_cnf_ctrl, uint16_t -> controlNumber, uint8_t -> status, unsigned long -> baseSignal
    std::vector<std::tuple<char, uint16_t, uint8_t, unsigned long>> controlValues;

    bool deteccionDispRF; // Bandera de detección de dispositivo RF

    /*
        @brief Bandera de nueva señal RF
        @details Esta bandera se activa cuando se recibe una nueva señal RF
        y se utiliza para indicar que se debe procesar la señal recibida.
        1 = Nueva señal RF recibida que no existe en la memoria
        2 = Nueva señal RF recibida que existe en la memoria, inforacion del control en rspCompRF
    */
    uint8_t detecConfigComp = false;

    unsigned long ctrlRfLoopSignal = 0; // Variable para almacenar la señal de control externo

    /*
        @brief Bandera de carga de controles RF
        @details Esta bandera se activa cuando se estan cargando los controles RF
    */
    bool cargandoCtrlRF = false;
    /*
        @brief Bandera de controles cargados
        @details Esta bandera indica que han sido cargados los controles, se activa al finalizar la carga.
        La memoria NVS de controles esta bien
    */
    bool controlesNVSok = false;
    /*
        @brief Bandera de configuracion de controles
        @details Esta bandera se activa cuando se ha confirmado la(s) configuración(es) de control(es)
    */
    bool controlesCnfFlag = false;
    uint16_t controlesCargadosSize = 0; // Tamaño de controles cargados
    unsigned long timerCargaCtrlRF = 0; // Timer de carga de controles RF
    CTRL_MODELO_AV controlRecepionado;  // Control recibido por RF
    ///////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<RedGuardada> redesGuardadas; // Vector para almacenar las redes WiFi guardadas
    std::vector<String> rutasArchivos;       // Vector para almacenar las rutas completas de los archivos
    static char jsonCMDSerial[2048];         // Buffer para el JSON final
    char rspMetodo[100];                     // respuesta de metodos

private:
    Preferences nvsData; // Instancia de Preferences

    // métodos de controles privados //////////////////////////////////////////////////////////////
    // controles.cpp
    const char *config_list_key = "config_list";
    void updateConfigList(const String &tipo);
    void loadConfigList(String *config_list, size_t &count);
    void addKey(const String &key);
    void removeKey(const String &key);
    bool cargar1ModeloControl(const char *tipo, CTRL_MODELO_AV &control);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // metodos de nvs privados ////////////////////////////////////////////////////////////////////
    // nvsdata.cpp
    bool cargarConfig(void);
    bool limpiarNamespace(const char *namespaceName);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Métodos de procesamiento de JSON //////////////////////////////////////////////////////////
    // json.cpp
    void procesarJsonRec(const JsonArray &arr, const String &rutaBase); // Método recursivo para procesar el JSON
    static char jsonLittleFS[2048];                                     // Buffer para almacenar el JSON de LittleFS
    //////////////////////////////////////////////////////////////////////////////////////////////
};

extern DataManager Data;

#endif // DATAMANAGER_H
