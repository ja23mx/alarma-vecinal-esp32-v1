#include "DataManager.h"
#include "LogSistema.h"
#include "Constantes.h"

struct ConfigEntry
{
    const char *key;
    void *variable;
    size_t size;
    bool isString;
};


bool DataManager::cargarConfig(void)
{
    LOG("\r\n\r\nCargando configuracion del sistema...");

    if (!nvsData.begin(NAME_SPACE_CNF, false))
    {
        LOG("\r\nError al abrir el espacio de config_sistema.");
        return false;
    }

    // Array de estructuras para mapear las claves y sus correspondientes variables
    ConfigEntry entries[] = {
        {"numeroSerie", this->numeroSerie, sizeof(this->numeroSerie), true},
        {"pinDispositivo", &this->pinDispositivo, sizeof(this->pinDispositivo), false},
        {"cnfRF", &this->perifericos[0], sizeof(this->perifericos[0]), false},
        {"cnfWIFI", &this->perifericos[1], sizeof(this->perifericos[1]), false},
        {"cnfVOZ", &this->perifericos[2], sizeof(this->perifericos[2]), false},
        {"confPER", &this->perifericos[3], sizeof(this->perifericos[3]), false},
        {"timerAlarma", &this->timerAlarma, sizeof(this->timerAlarma), false},
        {"enTamper", &this->tamper, sizeof(this->tamper), false}};

    // Cargar configuraciones desde NVS
    for (const auto &entry : entries)
    {
        if (!nvsData.isKey(entry.key))
            LOGF("\r\nError: Clave de configuración no encontrada: %s", entry.key);

        if (entry.isString)
        {
            nvsData.getString(entry.key, static_cast<char *>(entry.variable), entry.size);
        }
        else
        {
            if (entry.size == sizeof(uint8_t))
                *static_cast<uint8_t *>(entry.variable) = nvsData.getUChar(entry.key);
            else if (entry.size == sizeof(uint16_t))
                *static_cast<uint16_t *>(entry.variable) = nvsData.getUInt(entry.key);
        }
    }

    LOG("\r\nConfiguracion cargada correctamente.");
    LOGF("\r\nNumero de serie: %s", numeroSerie);
    LOGF("\r\nClave de dispositivo: %d", pinDispositivo);
    LOGF("\r\nPerifericos: %d, %d, %d, %d", perifericos[0], perifericos[1], perifericos[2], perifericos[3]);
    LOGF("\r\nTimer de alarma: %d", timerAlarma);
    LOGF("\r\nTamper: %d", tamper);
    // Cerrar Preferences
    nvsData.end();
    return true;
}

bool DataManager::limpiarNamespace(const char *namespaceName)
{
    // Inicializar NVS
    if (!nvsData.begin(namespaceName, false))
    {
        LOG("\r\n\r\nError al abrir NVS para limpiar");
        return false;
    }

    // Borrar todos los datos en el namespace
    bool result = nvsData.clear();
    /* if (result)
        LOGF("\r\n\r\nNamespace: %s limpio correctamente\n", namespaceName);
    else
        LOGF("\r\n\r\nError al limpiar namespace %s: %s\n", namespaceName, esp_err_to_name(result)); */

    // Cerrar NVS
    nvsData.end();

    return true;
}