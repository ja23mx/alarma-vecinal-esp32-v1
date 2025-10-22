#include "BLEManager.h"
#include "DataManager.h"

#include "BLEManager.h"
#include "DataManager.h"

// 📌 Método para procesar los comandos BLE
void BLEManager::procesarComandoBLE(const String &comando)
{

    // Extraer la categoría del comando
    int pos1 = comando.indexOf(':');
    if (pos1 == -1)
    {
        Serial.println("\r\nError: Formato invalido");
        return;
    }

    String categoria = comando.substring(0, pos1);
    String parametros = comando.substring(pos1 + 1);

    // Procesar según la categoría
    if (categoria.equals("login"))
    {
        manejarLogin(parametros);
    }
    else if (categoria.equals("wifi"))
    {
        /* if (usuarioAutenticado) */
        manejarWiFiBLE(parametros);
        /* else
            _rsp_cmd_ble = "Error: Usuario no autenticado"; */
    }
    else if (categoria.equals("set"))
    {
        SettingsResponse result;
        Data.CmdSerial(comando);
        enviarStrBLE(result.error == true ? "ok" : "error");
    }
    else
    {
        SettingsResponse result;

        // Verificar si el comando contiene 'set' o 'get'
        if (comando.indexOf("set") != -1)
        {
            Data.CmdSerial(comando);
            enviarStrBLE(result.error == 1 ? "ok" : "error");
        }
        else if (comando.indexOf("get") != -1)
        {
            Data.CmdSerial(comando);
            enviarStrBLE(result.cmd);
        }
        else
        {
            Serial.println("\r\n\r\nCategoria no reconocida en BLE.");
        }
    }
}

// 📌 Manejar autenticación de usuario
void BLEManager::manejarLogin(const String &parametros)
{
}

// 📌 Método para verificar el login BLE
bool BLEManager::verificarLogin(const String &pin)
{
    /* if (String(Data.claveDispositivo) == pin)
    {
        Serial.println("✅ Autenticación exitosa vía BLE");
        return true;
    } */

    Serial.println("❌ Error: Autenticación fallida vía BLE");
    return false;
}
