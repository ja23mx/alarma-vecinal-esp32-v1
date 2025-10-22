#include "DataManager.h"

// Definición del typedef para un puntero a función de comando.
// Cada función recibe un puntero a DataManager y los parámetros del comando,
// la informacio de la operacion esta en: jsonCMDSerial
typedef void (*CmdFunc)(DataManager *self, const String &params);

// Declaración de la función cmdSetNS
// Esta función es estática y solo es visible en este archivo

// Funciones para los comandos de escritura
static void cmdSetNS(DataManager *self, const String &params);
static void cmdSetClave(DataManager *self, const String &params);
static void cmdSetPeriferico(DataManager *self, const String &params);
static void cmdSetTimer(DataManager *self, const String &params);
static void cmdSetTamper(DataManager *self, const String &params);

static void cmdmqttHandShakeRequest(DataManager *self, const String &params);
static void cmdinfoInitServer(DataManager *self, const String &params);
static void cmdgetCtrlModelos(DataManager *self, const String &params);
static void cmdsetDetectarDispRF(DataManager *self, const String &params);
static void cmdsetCnfNvRF(DataManager *self, const String &params);
static void cmdgetInfoDspRFLoop(DataManager *self, const String &params);
static void cmdborrar1Ctrl(DataManager *self, const String &params);
static void cmdinfo1Ctrl(DataManager *self, const String &params);
static void cmdborrar1RedWifi(DataManager *self, const String &params);
static void cmdset1RedWifi(DataManager *self, const String &params);
static void cmdgetRedesWifi(DataManager *self, const String &params);

bool serializeJsonToBuffer(void);
bool jsonRspHeader(const char *cod_respuesta, const char *descripcion, const char *rsp);
void jsonRspCmd(const char *cod_respuesta, const char *descripcion);