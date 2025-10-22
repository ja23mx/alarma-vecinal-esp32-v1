#ifndef COMMAND_CONTROLLER_H
#define COMMAND_CONTROLLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#include "EstructurasGlobales.h"

// Estructura para la información de cada comando procesado
struct ComandoProcesadoInfo
{
    String rsp_cmd;
    int error;
    String msg;
};

class GestorCMD
{
public:
    // Constructor
    GestorCMD();

    // Metodo para activacion de perifericos via ctrl remoto AV
    void CtrlAv(uint8_t tipoCtrl);

    // Método principal para procesar un comando entrante
    void process(const String &payload, CmdOrigen origenInfo);

    bool playPistaLedAtm(uint16_t pista, uint8_t config, uint8_t cargaPeriferico);

    bool separarComando(const String &comando, DATOS_INSTRUCCION &data);
    bool separarArrNumeros(const String &input, char delimitador, std::vector<long> &numerosArray);

    /*
    variable para indicar si se debe enviar una respuesta JSON, tambien significa que
    se han procesado por lo menos un comando de salida o audio.
    */
    bool rspJson = false;  
    char jsonBuffer[2048]; // Buffer para almacenar la respuesta JSON

private:
    // Gestor de comandos //////////////////////////////////////////////////////////////////////////
    bool revisarCMD(const String &cmd, ComandoProcesadoInfo &info);
    void setErrorInfo(ComandoProcesadoInfo &info, const String &cmdStr, int errorCode, const String &errorMsg);
    void generateDeviceJsonResponse(const std::vector<ComandoProcesadoInfo> &responses);
    
    // Método para obtener mensaje de error basado en código
    const char* getErrorMessage(uint8_t errorCode);
    
    // Método genérico para generar respuestas ACK
    void generateAckResponse(const String &responseType, uint8_t errorCode);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Método para procesar un comando D2D  ///////////////////////////////////////////////////////
    void datosD2D(void);
    void procesarCmd(const String &payload);
    void separarInstrucciones(const String &input);
    bool validarFormato(const String &input, const String &tipo);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Método para procesar un comando JSON ///////////////////////////////////////////////////////
    void datosJSON(void);
    void processEmgCoDi(void);
    void processHandShakeRspJson(void);
    void processCmdJson(void);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // CmdCtrl.cpp ////////////////////////////////////////////////////////////////////////////////
    void desactivacionAud1Sal1(void);
    void procesarInstrucciones(uint16_t numInstrucciones, String *instruccionesTot);
    void procesarControlAV(const String &modelo, int btn);
    void procesarControlGuardian(const String &modelo, int btn);
    void procesarControlIntegrador(const String &modelo, int btn);
    void cmd1PistaLedAtm(uint16_t pista, uint16_t ledFade);
    void logInformacionControl();
    void ejecutarInstrucciones(const String &instruccionSalida, const String &instruccionAudio);
    
    // Función para generar respuesta de handshake
    void generateHandShakeAckResponse(uint8_t status);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    String lastPayload;               // el último payload recibido
    StaticJsonDocument<4096> docJson; // Usa el tamaño de tu jsonBuffer

    std::vector<String> cmd_totales;

    CMD_DATA cmdDatos = {}; // Estructura para almacenar los datos del comando
};

// Declaración de la instancia global
extern GestorCMD GestorCmd;

#endif // COMMAND_CONTROLLER_H