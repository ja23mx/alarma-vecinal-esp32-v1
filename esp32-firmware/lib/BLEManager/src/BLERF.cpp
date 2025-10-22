#include <DataManager.h>
#include <LogSistema.h>

#include <BLEManager.h>
#include <map>

void BLEManager::manejarWiFiRF(const String &cmd)
{
    static const std::map<String, void (BLEManager::*)(const String &)> rfBLECommands = {
        {"nvctrl", &BLEManager::rfBLEnvctrl}};

    int delimIndex = cmd.indexOf(':');
    String commandName = (delimIndex != -1) ? cmd.substring(0, delimIndex) : cmd;
    String parameters = (delimIndex != -1) ? cmd.substring(delimIndex + 1) : "";
    commandName.trim();
    parameters.trim();

    LOG("\r\n\r\nComando: " + commandName + " Parametros: " + parameters);

    auto it = rfBLECommands.find(commandName);
    if (it != rfBLECommands.end())
    {
        (this->*it->second)(parameters);
    }
    else
    {
        LOG("\r\nComando WiFi BLE no reconocido");
    }
}

void BLEManager::rfBLEnvctrl(const String &parametros)
{

    
}