
#include "ServerWeb.h"

#include "DataManager.h"
#include "LogSistema.h"

void ServerWeb::url_setDetectarDispRF()
{
    server->on("/setDetectarDispRF", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String config;
                   request->hasParam("config") == true ? config = request->getParam("config")->value() : config = "";
                   this->cmdSerialFast(request, "setDetectarDispRF:" + config); });
}

void ServerWeb::url_revDetecSenalRF()
{
    server->on("/revDetecSenal", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   // 1 = control existente, 2 = control no existente
                   if (Data.detecConfigComp == 0)
                   {
                       request->send(200, "application/json", "{\"cod_respuesta\":\"sin_senal\",\"response\":\"sin recepcion de senal\"}");
                   }
                   else if (Data.detecConfigComp == 1)
                   {
                       request->send(200, "application/json", "{\"cod_respuesta\":\"nv_ctrl\",\"response\":\"el dispositivo  no esta guardado\"}");
                   }
                   else if (Data.detecConfigComp == 2)
                   {
                       this->cmdSerialFast(request, "getInfoDspRFLoop:");
                   }
                   //LOG("\r\n\r\nRev Detec Senal RF. ctrl: " + String(Data.detecConfigComp) + "\r\n");
                   if (Data.detecConfigComp != 0)  // resetear la variable de deteccion de control RF
                       Data.detecConfigComp = 255; // en caso de que no haya control RF, se reseteara a 255 para que no se
                                                   // vuelva a enviar la señal de configuracion
               });
}

void ServerWeb::url_getCtrlModelos()
{
    server->on("/getCtrlModelos", HTTP_GET, [this](AsyncWebServerRequest *request)
               { this->cmdSerialFast(request, "getCtrlModelos:"); });
}

void ServerWeb::url_setCnfNvRF()
{
    server->on("/setCnfNvRF", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String nm_ctrl;
                   request->hasParam("nm_ctrl") == true ? nm_ctrl = request->getParam("nm_ctrl")->value() : nm_ctrl = "";
                   String mdl;
                   request->hasParam("mdl") == true ? mdl = request->getParam("mdl")->value() : mdl = "";
                   String status;
                   request->hasParam("status") == true ? status = request->getParam("status")->value() : status = "";

                   this->cmdSerialFast(request, "setCnfNvRF:" + status + "," + nm_ctrl + "," + mdl + ","); });
}

void ServerWeb::url_borrar1Ctrl()
{
    server->on("/borrar1Ctrl", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String nm_ctrl = "";
                   request->hasParam("nm_ctrl") == true ? nm_ctrl = request->getParam("nm_ctrl")->value() : nm_ctrl = "0";

                   LOG("\r\n\r\nBorrar 1 Control. ctrl: " + nm_ctrl + "\r\n");

                   // senal RF inexistente, senal RF ya existe en la memoria
                   if (nm_ctrl != "")
                   {
                       this->cmdSerialFast(request, "borrar1Ctrl:" + nm_ctrl);
                   }
                   else // no se recibió nueva señal RF
                   {
                       request->send(200, "application/json", "{\"cod_respuesta\":\"borrar1Ctrl\",\"descripcion\":\"Borrado de 1 control\",\"rsp\":\"Error. sintaxis error num ctrl\"}");
                   } });
}

void ServerWeb::url_info1Ctrl()
{
    server->on("/info1Ctrl", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   String nm_ctrl = "";
                   request->hasParam("nm_ctrl") == true ? nm_ctrl = request->getParam("nm_ctrl")->value() : nm_ctrl = "0";

                   LOG("\r\n\r\ninfo1Ctrl. ctrl: " + nm_ctrl + "\r\n");

                   // senal RF inexistente, senal RF ya existe en la memoria
                   if (nm_ctrl != "")
                   {
                       this->cmdSerialFast(request, "info1Ctrl:" + nm_ctrl);
                   }
                   else // no se recibió nueva señal RF
                   {
                       request->send(200, "application/json", "{\"cod_respuesta\":\"info1Ctrl\",\"descripcion\":\"Info de 1 control\",\"rsp\":\"Error. sintaxis error num ctrl\"}");
                   } });
}
