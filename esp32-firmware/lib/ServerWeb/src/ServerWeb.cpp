#include <LittleFS.h>
#include "esp_task_wdt.h"
#include "esp_wifi.h"

#include "ConfigSistema.h"
#include "ServerWeb.h"
#include "DataManager.h"
#include "LogSistema.h"
#include "MonitorRAM.h"
#include "VariablesGlobales.h"

#include <Update.h>
#define U_PART U_SPIFFS
size_t content_len;

#include <map>

#include "Constantes.h"
#include "../html_files/html_css_ota.h"
#include "../html_files/index-html.h"
#include "../html_files/html_css_cmd.h"

#include <DNSServer.h>

DNSServer dnsServer;
const byte DNS_PORT = 53;

/**
 * @brief Constructor de la clase ServerWeb.
 * @param wifiToolRef Referencia a la instancia de WiFiTools.
 */
ServerWeb::ServerWeb(WiFiTool &wifiToolRef)
    : server(nullptr), WiFiTools(wifiToolRef) {}

/**
 * @brief Inicializa el servidor web
 **/
uint8_t ServerWeb::begin(uint8_t origenConfig)
{

  init_prog_ble = true; // Inicializar la programación BLE

  if (PROG_LOCAL == false) // parpadeo inicial para indicar que esta iniciando server
  {
    for (size_t i = 0; i < 3; i++)
    {
      digitalWrite(LED_STATUS, LED_ST_ON);
      delay(100);
      digitalWrite(LED_STATUS, LED_ST_OFF);
      delay(100);
    }

    digitalWrite(LED_STATUS, LED_ST_ON); // Encender el LED de estado
  }

  // deshabilitacion del escaneo WiFi
  WiFiTools.scanInit();
  escaneoWiFi = WiFiTools.getEscaneoStatus(!PROG_LOCAL);

  /* LOGF_SERVER("\r\n\r\n\r\nRedes Totales: %d\n", escaneoWiFi.redesTotales);
  for (uint8_t i = 0; i < escaneoWiFi.redesTotales; i++)
  {
    LOGF_SERVER("\r\n%d: %s \r\nRSSI: %d - Canal: %d\n",
                i + 1,
                escaneoWiFi.ssid[i].c_str(),
                escaneoWiFi.rssi[i],
                escaneoWiFi.canal[i]);
  }
  LOG("\r\nCanal AP: " + String(escaneoWiFi.canalAP));*/

  // Configurar la conexion WiFi segun el modo de programa.
  if (PROG_LOCAL)
  { // Modo normal: conectar a la red WiFi como estacion (STA).
    LOG("\r\n\r\nPROG_LOCAL\r\nWIFI CONECTANDO...");
    WiFi.mode(WIFI_STA);
    WiFi.begin("REDALERTA", "@TLWN_7200NDMX2148"); // Conectar a la red
    // "REDLOCAL", "TLWN7200NDMX2148"
    delay(2000);
    // Esperar hasta conectar
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
    }

    LOG("\r\n\r\n\r\nIP LOCAL: ");
    LOG(WiFi.localIP());
  }
  else
  {

    // Configurar el ESP32 como Access Point y modo estacion (AP+STA).
    WiFi.mode(WIFI_AP_STA);
    // Configurar la red AP con ssid, password, canal, oculto (0=no oculto) y maximo de clientes (2)
    // Obtener los últimos 6 dígitos de la MAC address
    String macAddress = WiFi.macAddress();
    macAddress.replace(":", "");                // Remover los dos puntos
    String macSuffix = macAddress.substring(6); // Últimos 6 caracteres
    // Crear el SSID con el formato MN-AV1-{MAC}
    String ssid = "MN-AV1-" + macSuffix;

    // Configurar el Access Point con la nueva configuración
    WiFi.softAP(ssid.c_str(), "admin123456", escaneoWiFi.canalAP, 0, 2); //
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);

    LOGF("\r\n\r\nAP: %s\r\n\r\n", ssid);

    // RAMStatus(0);                         // Imprimir la cantidad de RAM usada
    delay(500);

    if (digitalRead(BTN_PROG) == LOW)
    {
      while (digitalRead(BTN_PROG) == LOW)
        delay(100);
    }
  }

  // Generar el JSON y almacenarlo en jsonLittleFS
  // Data.listaJsonLF(2); // 2: Almacena el JSON en jsonLittleFS

  // Iniciar el servidor web.
  initServer();

  LOG("\r\n\r\n\r\n\r\n\r\nCICLO SERVER \r\n\r\n\r\n\r\n\r\n");

  unsigned long timerRF = millis(); // Temporizador para el control de integrador, evitar rebotes

  while (1)
  {

    if (digitalRead(BTN_PROG) == LOW)
    {
      delay(250);
      if (digitalRead(BTN_PROG) == LOW)
      {
        origenConfig = 0; // Boton de programacion
        LOG("\r\n\r\n\r\nPROG_LOCAL BOTON FIN\r\n");
        break; // Salir del bucle y continuar con la configuración
      }
    }

    // Si se inicia la programación por control de integrador
    // y llego senal RF de control integrador
    if (origenConfig == 1 && init_prog_rf == false)
    {
      // paso mas de x tiempo desde que se inicio la programacion
      // y si recibio una señal RF
      if (millis() - timerRF > 10000)
      {
        LOG("\r\n\r\n\r\nPROG_LOCAL CTRL INTEGRADOR FIN\r\n");
        break;
      }
      else
      {
        // reset de la variable de inicio de programación RF aunque haya
        // llegado una señal RF, no se toma como fin de programación
        init_prog_rf = true;
        LOG("\r\n\r\n\r\nREBOTE SENAL RF INTEGRADOR\r\n");
      }
    }

    delay(1); // Esperar 100ms para evitar rebotes del botón
  }

  /* delay(1000);
  ESP.restart(); // Reiniciar el ESP32 */

  return origenConfig; // Retornar el origen de la configuración
}

/**
 * @brief Inicializa el servidor web general y registra las rutas.
 */
void ServerWeb::initServer()
{
  // Crear el objeto servidor asyncrono con el puerto especificado.
  server = new AsyncWebServer(80);

  // Registrar rutas especificas de la seccion RF.
  // registerRFRoutes();

  // registro de API's
  url_api_data();          // Registrar la ruta para la API de datos
  url_setDetectarDispRF(); // Registrar la ruta para la API de configuracion
  url_revDetecSenalRF();   // Registrar la ruta para la API de deteccion de senal RF
  url_getCtrlModelos();    // Registrar la ruta para obtener los modelos de control y espacio vacio
  url_setCnfNvRF();        // Registrar la ruta para el guardado el control RF
  url_borrar1Ctrl();       // Registrar la ruta para borrar un control
  url_info1Ctrl();         // Registrar la ruta para obtener la info de un control

  url_borrar1RedWifi(); // Registrar la ruta para borrar una red WiFi

  url_cmdSerial(); // Registrar la ruta para el comando serial

  url_ota(); // Registrar la ruta OTA

  // Páginas adicionales
  server->on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send_P(200, "text/html", const_html_ccs_cmd); });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
             {
               Serial.print("\r\n\r\nPAG INICIO");
               request->send_P(200, "text/html", const_html_ccs_index); });

  /* server->serveStatic("/css/", LittleFS, "/web/css/");
  server->serveStatic("/js/", LittleFS, "/web/js/");
  server->serveStatic("/images/", LittleFS, "/web/images/");
  server->serveStatic("/favicon.ico", LittleFS, "/web/favicon.ico"); */
  // Configurar Captive Portal
  // configurarCaptivePortal();

  // Iniciar el servidor.
  server->begin();
  LOG("\r\nServidor web iniciado en el puerto 80");
}

void ServerWeb::configurarCaptivePortal()
{
  // Función lambda para redirección (definida localmente)
  auto redirectToIndex = [](AsyncWebServerRequest *request) { // "<meta http-equiv='refresh' content='0;url=/'>"
    String html = "<!DOCTYPE html><html><head>"
                  "</head><body>Redirecting...</body></html>";

    /* request->redirect("/");  */
  };

  // Respuestas para detección de captive portal

  // Android (múltiples versiones)
  /* server->on("/generate_204", HTTP_GET, redirectToIndex);
  server->on("/gen_204", HTTP_GET, redirectToIndex);
  server->on("/connectivitycheck.gstatic.com/generate_204", HTTP_GET, redirectToIndex);

  // iOS (múltiples versiones)
  server->on("/hotspot-detect.html", HTTP_GET, redirectToIndex);
  server->on("/library/test/success.html", HTTP_GET, redirectToIndex);
  server->on("/captive.apple.com/hotspot-detect.html", HTTP_GET, redirectToIndex);

  // Windows
  server->on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send(200, "text/plain", "Microsoft Connect Test"); }); */

  // Ruta raíz que sirve tu index
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
             {
               Serial.print("\r\n\r\nPAG INICIO");
               request->send_P(200, "text/html", const_html_ccs_index);

               // request->send(LittleFS, "/web/index.html", "text/html");
             });

  // Manejar solicitudes no encontradas
  server->onNotFound([](AsyncWebServerRequest *request)
                     {
        String path = request->url();
        String host = request->host();
        
        LOG("Solicitud no encontrada: " + path + " Host: " + host);
        
        // Si es detección de captive portal
        if (host.indexOf("apple.com") >= 0 || host.indexOf("google.com") >= 0 || 
            host.indexOf("microsoft.com") >= 0 || host.indexOf("connectivitycheck") >= 0) {
            request->send(200, "text/html", "<html><body>Success</body></html>");
            return;
        }
        
        // Si solicita archivos estáticos que no existen
        if (path.startsWith("/css/") || path.startsWith("/js/") || path.startsWith("/images/")) {
            request->send(404, "text/plain", "Archivo no encontrado: " + path);
            return;
        }
        
        // Para todo lo demás, redirigir a la página principal
        request->redirect("/"); });
}

// Definir un mapa para manejar las opciones de "get_info"
std::map<String, std::function<void(AsyncWebServerRequest *)>> getInfoHandlers = {
    {"1", [](AsyncWebServerRequest *request)
     {
       Data.CmdSerial("infoInitServer:");
       // Responder con el estado del sistema
       request->send(200, "application/json", Data.jsonCMDSerial);
       // Limpiar el buffer Data.jsonCMDSerial
       memset(Data.jsonCMDSerial, 0, sizeof(Data.jsonCMDSerial));
     }},
    {"3", [](AsyncWebServerRequest *request)
     {
       // Responder con información adicional
       request->send(200, "application/json", "{\"info\":\"Información adicional\"}");
     }}

};

void ServerWeb::url_api_data()
{
  server->on("/api_data", HTTP_GET, [](AsyncWebServerRequest *request)
             {
    // Verificar si el parámetro "get_info" está presente
    if (request->hasParam("get_info"))
    {
        String getInfo = request->getParam("get_info")->value();

        // Buscar el manejador correspondiente en el mapa
        auto it = getInfoHandlers.find(getInfo);
        if (it != getInfoHandlers.end())
        {
            // Llamar al manejador correspondiente
            it->second(request);
        }
        else
        {
            // Responder con un error si el valor no es válido
            request->send(400, "application/json", "{\"error\":\"Parámetro get_info no válido\"}");
        }
    }
    else
    {
        // Responder con un error si falta el parámetro "get_info"
        request->send(400, "application/json", "{\"error\":\"Falta el parámetro get_info\"}");
    } });
}

void ServerWeb::url_cmdSerial()
{
  server->on("/cmdSerial", HTTP_GET, [this](AsyncWebServerRequest *request)
             {
               request->hasParam("cmd") == true ?                                                    // busque el parametro "cmd" en la URL
                   cmdSerialFast(request, request->getParam("cmd")->value())                         // Ejecutar el comando
                                                :                                                    //
                   request->send(400, "application/json", "{\"error\":\"Falta el parámetro cmd\"}"); // Responder con un error si falta el parámetro "cmd"
             });
}

void ServerWeb::url_ota(void)
{
  server->on(URL_OTA, HTTP_GET, [](AsyncWebServerRequest *request)
             {
               LOG("\r\n\r\nPAG OTA");
               request->send_P(200, "text/html", const_html_ccs_ota); });

  server->on(URL_UPDATE, HTTP_POST, [](AsyncWebServerRequest *request) {}, [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
             {
               if (!index)
               {
                 LOG("\r\n\r\nUpdate init");
                 content_len = request->contentLength();
                 int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;

                 if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
                   Update.printError(Serial);
               }

               if (Update.write(data, len) != len)
                 Update.printError(Serial);

               if (final)
               {
                 request->send(200, "text/plain", "OK");
                 if (!Update.end(true))
                 {
                   Update.printError(Serial);
                 }
                 else
                 {
                   LOG("\r\n\r\nUpdate fin");
                   delay(100);
                   ESP.restart();
                 }
               } });
}

void ServerWeb::cmdSerialFast(AsyncWebServerRequest *request, const String &comando)
{
  // Ejecutar el comando
  Data.CmdSerial(comando);

  // Enviar la respuesta al cliente
  request->send(200, "application/json", Data.jsonCMDSerial);

  // Limpiar el buffer Data.jsonCMDSerial
  memset(Data.jsonCMDSerial, 0, sizeof(Data.jsonCMDSerial));
}