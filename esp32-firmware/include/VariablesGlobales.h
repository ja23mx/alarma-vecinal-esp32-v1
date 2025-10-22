#ifndef VARIABLES_GLOBALES_H
#define VARIABLES_GLOBALES_H

#include "EstructurasGlobales.h"

extern STATE_MP3 mp3State;
extern RF_COMP_AV estadoCompRFAv;
extern ESTADO_ALARMA estadoAlarma;    // Estructura global para el estado de la alarma
extern CTRL_MODELO_AV modeloCtrlAVRx; // Estructura global para el modelo del control recibido por RF
extern ESCANEO_STATUS escaneoWiFi;    // Estructura global para el escaneo de WiFi

/*
    @brief variable global para almacenar el estado de la red BLE
    @param 0 = deshabilitada, 1 = habilitada
*/
extern bool init_prog_ble;

/* 
    @brief inicio programacion via RF
    @param 0 = deshabilitada, 1 = habilitada
    @note Variable global que se utiliza para indicar que inicio programacion via RF, usada en el loop principal
*/
extern bool init_prog_rf; 

///// MQTT VARIABLES //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
extern const char *mqtt_server;
extern uint16_t mqtt_port;
extern const char *mqtt_user;
extern const char *mqtt_pass;

extern std::vector<String> cmdLocal;
extern std::vector<String> cmdExterno;
extern std::vector<String> cmdSalidas;
extern String cmdAudio;
extern bool cmdSalidasPendiente, cmdSalidasBusy;
extern bool cmdAudioPendiente, cmdAudioBusy;

extern std::vector<String> emgCoDi; // Lista de mensajes de emergencia recibidos por MQTT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // VARIABLES_GLOBALES_H
