#include "ServerWeb.h"

#include "DataManager.h"
#include "LogSistema.h"

void ServerWeb::url_borrar1RedWifi()
{
    server->on("/borrar1RedWifi", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String red;
                   request->hasParam("red") == true ? red = request->getParam("red")->value() : red = "";
                   this->cmdSerialFast(request, "borrar1RedWifi:" + red);
               });
}