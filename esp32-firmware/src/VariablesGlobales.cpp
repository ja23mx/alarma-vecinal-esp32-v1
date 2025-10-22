#include "EstructurasGlobales.h"
#include <vector>

STATE_MP3 mp3State;            // Estructura global para el estado del MP3
RF_COMP_AV estadoCompRFAv;     // Estructura global para la respuesta de comparación de RF
ESTADO_ALARMA estadoAlarma;    // Estructura global para el estado de la alarma
CTRL_MODELO_AV modeloCtrlAVRx; // Estructura global para el modelo del control recibido por RF
                               //
bool init_prog_ble = false;    // Variable global para inicializar la programación BLE
bool init_prog_rf = false;     // Variable global para inicializar la programación via RF
                               //
ESCANEO_STATUS escaneoWiFi;    // Estructura global para el escaneo de WiFi

///// MQTT VARIABLES //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
const char *mqtt_server = "meniam.virtualizate.com"; // Cambia por tu broker
uint16_t mqtt_port = 8883;                           // Puerto seguro MQTT
const char *mqtt_user = "mqttuser";                  // Usuario MQTT
const char *mqtt_pass = ".123qweasdzxc";             // Contraseña MQTT

std::vector<String> cmdLocal;
std::vector<String> cmdExterno;
std::vector<String> cmdSalidas; 
String cmdAudio; 
bool cmdSalidasPendiente, cmdSalidasBusy;
bool cmdAudioPendiente, cmdAudioBusy;

std::vector<String> emgCoDi; // Lista de mensajes de emergencia recibidos por MQTT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
