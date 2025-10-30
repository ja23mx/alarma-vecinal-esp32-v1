#include "DataManager.h"

#include "Constantes.h"
#include "LogSistema.h"

bool DataManager::guardarRedWiFi(const char *ssid, const char *password)
{
    // Verificar si el espacio de nombres NVS está disponible
    if (!nvsData.begin(NAME_SPACE_WIFI, false))
    {
        controlesCnfFlag = false;
        LOG("\r\nError al abrir el espacio de redes wifi.");
        strcpy(rspMetodo, "Error. Al abrir el espacio de redes wifi");
        return false;
    }

    // Crear un nuevo JSON desde cero
    StaticJsonDocument<1024> doc;
    JsonArray networks = doc.createNestedArray("networks");

    // Agregar la nueva red (única)
    JsonObject newNetwork = networks.createNestedObject();
    newNetwork["rd"] = ssid;
    newNetwork["cl"] = password;

    // Serializar el JSON
    String updatedJsonStr;
    serializeJson(doc, updatedJsonStr);

    // Guardar el JSON en NVS
    if (!nvsData.putString("wifi_networks", updatedJsonStr))
    {
        LOG("\r\nError al guardar el JSON en NVS.");
        nvsData.end();
        strcpy(rspMetodo, "Error. Al guardar el JSON");
        return false;
    }

    nvsData.end();

    doc.clear();
    updatedJsonStr.clear();

    LOG("\r\nRed WiFi guardada correctamente.");
    strcpy(rspMetodo, "Ok. Red WiFi guardada correctamente");
    return true;
}

const char *DataManager::leerRedWiFi(const char *ssid)
{

    // Verificar si el espacio de nombres NVS está disponible
    if (!nvsData.begin(NAME_SPACE_WIFI, true))
    {
        controlesCnfFlag = false;                                // Indicar que no se cargaron las configuraciones
        LOG("\r\nError al abrir el espacio de config_control."); //
        return nullptr;                                          // Retornar nullptr si no se puede abrir NVS
    }

    // Verificar si la clave "wifi_networks" existe en NVS
    if (!nvsData.isKey("wifi_networks"))
    {
        LOG("\r\nNo hay redes almacenadas.");
        return nullptr; // Retornar nullptr si no hay redes almacenadas
    }

    // Leer el JSON almacenado en NVS
    String jsonStr = nvsData.getString("wifi_networks", "{}");

    nvsData.end(); // Cerrar el espacio de nombres NVS

    // Parsear el JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        LOG("\r\nError al parsear el JSON de redes WiFi.");
        return nullptr; // Retornar nullptr si hay un error al parsear
    }

    // Obtener el array de redes
    JsonArray networks = doc["networks"].as<JsonArray>();
    if (!networks)
    {
        LOG("\r\nNo hay redes almacenadas.");
        return nullptr; // Retornar nullptr si no hay redes
    }

    // Buffer estático para almacenar la red y la clave
    static char result[60]; // Asegúrate de que sea lo suficientemente grande para contener "SSID,clave"

    // Buscar la red por SSID
    for (JsonObject network : networks)
    {
        if (strcmp(network["rd"].as<const char *>(), ssid) == 0)
        {
            // LOGF("\r\nRed encontrada: %s", ssid);
            //  Construir la cadena "SSID,clave,"
            snprintf(result, sizeof(result), "%s,%s,",
                     network["rd"].as<const char *>(),
                     network["cl"].as<const char *>());

            doc.clear(); // Limpiar el documento JSON para liberar memoria

            return result; // Retornar la cadena concatenada
        }
    }

    doc.clear(); // Limpiar el documento JSON para liberar memoria

    LOGF("\r\nRed no encontrada: %s", ssid);
    return nullptr; // Retornar nullptr si no se encuentra la red
}

std::vector<RedGuardada> DataManager::leerTodasLasRedesGuardadas()
{
    std::vector<RedGuardada> redesGuardadas;

    // Verificar si la clave "wifi_networks" existe en NVS
    if (!nvsData.begin(NAME_SPACE_WIFI, true))
    {
        LOG("\r\nError al abrir el espacio de nombres NVS.");
        return redesGuardadas; // Retornar vacío si no se puede abrir NVS
    }

    if (!nvsData.isKey("wifi_networks"))
    {
        LOG("\r\nNo hay redes guardadas en NVS.");
        nvsData.end();
        return redesGuardadas; // Retornar vacío si no hay redes guardadas
    }

    // Leer el JSON almacenado en NVS
    String jsonStr = nvsData.getString("wifi_networks", "{}");
    nvsData.end(); // Cerrar el espacio de nombres NVS

    // LOGF("\r\n\r\n\r\n\r\nJSON leido de NVS: %s\r\n\r\n", jsonStr.c_str()); // Depuración

    // Parsear el JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        LOG("\r\nError al parsear el JSON de redes guardadas.");
        return redesGuardadas; // Retornar vacío si hay un error al parsear
    }

    // Obtener el array de redes
    JsonArray networks = doc["networks"].as<JsonArray>();
    if (!networks)
    {
        LOG("\r\nNo hay redes guardadas en el JSON.");
        return redesGuardadas; // Retornar vacío si no hay redes en el JSON
    }

    // Recorrer las redes guardadas y almacenarlas en el vector
    for (JsonObject network : networks)
    {
        if (network.containsKey("rd") && network.containsKey("cl"))
        {
            RedGuardada red;
            red.ssid = network["rd"].as<const char *>();
            red.password = network["cl"].as<const char *>();
            redesGuardadas.push_back(red);
            // LOGF("\r\nRed encontrada: %s, %s", red.ssid.c_str(), red.password.c_str()); // Depuración
        }
        else
        {
            // LOG("\r\nRed inválida encontrada en el JSON. Ignorando...");
        }
    }

    // LOGF("\r\nSe encontraron %d redes guardadas.", redesGuardadas.size());
    return redesGuardadas;
}

bool DataManager::borrar1RedWiFi(const char *ssid)
{

    // Verificar si la clave "wifi_networks" existe en NVS
    if (!nvsData.begin(NAME_SPACE_WIFI, false))
    {
        LOG("\r\nError al abrir el espacio de nombres NVS.");
        return false; // Retornar vacío si no se puede abrir NVS
    }

    if (!nvsData.isKey("wifi_networks"))
    {
        LOG("\r\nNo hay redes guardadas en NVS.");
        strcpy(rspMetodo, "Error. No hay redes guardadas");
        return false;
    }

    // Leer el JSON almacenado en NVS
    String jsonStr = nvsData.getString("wifi_networks", "{}");

    // Parsear el JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error)
    {
        LOG("\r\nError al parsear el JSON de redes WiFi.");
        strcpy(rspMetodo, "Error. Al parsear el JSON de redes WiFi");
        return false;
    }

    // LOG("\r\n\r\nJSON 1: "); // Imprimir el JSON leído para depuración
    // serializeJsonPretty(doc, Serial);
    //  LOGF("\r\nJSON 2: %s", jsonStr.c_str()); // Imprimir el JSON leído para depuración

    // Obtener el array de redes
    JsonArray networks = doc["networks"].as<JsonArray>();
    if (!networks)
    {
        LOG("\r\nNo hay redes guardadas en el JSON.");
        strcpy(rspMetodo, "Error. No hay redes guardadas en el JSON");
        nvsData.end();
        return false;
    }

    // Buscar y eliminar la red
    bool redEncontrada = false;
    for (size_t i = 0; i < networks.size(); ++i)
    { // busqar la red por el SSID
        JsonObject network = networks[i];
        if (String(network["rd"].as<const char *>()) == String(ssid))
        {
            networks.remove(i); // Eliminar la red del array
            redEncontrada = true;
            break;
        }
    }

    if (!redEncontrada)
    {
        LOGF("\r\nRed no encontrada: %s", ssid);
        strcpy(rspMetodo, "Error. Red no encontrada");
        nvsData.end();
        return false;
    }

    // Serializar el JSON actualizado
    String updatedJsonStr;
    serializeJson(doc, updatedJsonStr);

    // LOGF("\r\nJSON 3: %s", updatedJsonStr.c_str()); // Imprimir el JSON leído para depuración

    size_t res = nvsData.putString("wifi_networks", updatedJsonStr);
    if (res == 0)
    {
        LOG("\r\nError al guardar el JSON actualizado en NVS.");
        strcpy(rspMetodo, "Error. Al guardar el JSON actualizado");
        nvsData.end();
        return false;
    }

    nvsData.end(); // Cerrar el espacio de nombres

    // Eliminar la red del vector
    auto it = std::remove_if(redesGuardadas.begin(), redesGuardadas.end(),
                             [ssid](const RedGuardada &red)
                             {
                                 return red.ssid == ssid;
                             });
    if (it != redesGuardadas.end())
    {
        redesGuardadas.erase(it, redesGuardadas.end());
        LOGF("\r\nRed eliminada del vector: %s", ssid);
    }
    else
    {
        LOGF("\r\nRed no encontrada en el vector: %s", ssid);
        strcpy(rspMetodo, "Error. Red no encontrada en la RAM");
        return false; // Retornar falso si no se encuentra la red en el vector
    }

    /* LOG("\r\nRedes guardadas: " + String(redesGuardadas.size()) + "\r\n");
    for (const auto &red : redesGuardadas)
    {
        LOGF("\r\n\r\n%s\r\n%s", red.ssid.c_str(), red.password.c_str());
    } */

    LOG("\r\nRed WiFi eliminada correctamente.");
    strcpy(rspMetodo, "Ok. Red WiFi eliminada correctamente");
    return true;
}