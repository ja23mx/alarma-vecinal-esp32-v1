#include "ServerWeb.h"

#include "DataManager.h"
#include "LogSistema.h"

#include "../html_files/html_css_wifi.h"

void ServerWeb::url_borrar1RedWifi()
{
    server->on("/borrar1RedWifi", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String red;
                   request->hasParam("red") == true ? red = request->getParam("red")->value() : red = "";
                   this->cmdSerialFast(request, "borrar1RedWifi:" + red); });
}

void ServerWeb::gestionWifi()
{
    server->on("/wifi", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   LOG("\r\n\r\nPAG WIFI");
                   request->send_P(200, "text/html", const_html_ccs_wifi); });

    server->on("/apiWifi/rst", HTTP_GET, [this](AsyncWebServerRequest *request)
               { 
                Serial.println("\r\nResetear Dispositivo WiFi");
                request->send(200, "application/json", "{\"success\":\"ok\"}");
                delay(2000);
                ESP.restart(); });

    server->on("/apiWifi/guardar", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String red, pass;
                   request->hasParam("red") == true ? red = request->getParam("red")->value() : red = "";
                   request->hasParam("pass") == true ? pass = request->getParam("pass")->value() : pass = "";

                   Serial.printf("\r\nGuardar WiFi SSID: %s PASS: %s", red.c_str(), pass.c_str());

                   if (red != "" && pass != "")
                   {
                       /// request->send(200, "application/json", "{\"success\":\"ok\"}");
                       this->cmdSerialFast(request, "set1RedWifi:" + red + "," + pass + ",");
                   }
                   else
                   {
                       request->send(400, "application/json", "{\"error\":\"Faltan parámetros red o pass\"}");
                   } });
}