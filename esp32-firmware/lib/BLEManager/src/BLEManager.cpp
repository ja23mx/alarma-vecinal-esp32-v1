#include "BLEManager.h"

#include "ConfigSistema.h"
#include "LogSistema.h"
#include "VariablesGlobales.h"
#include "MonitorRAM.h"

#include "WiFiTool.h"

// Constructor
BLEManager::BLEManager(WiFiTool &wifiToolRef) : wifiTools(wifiToolRef),
                                                pServer(nullptr),
                                                pRxCharacteristic(nullptr),
                                                pTxCharacteristic(nullptr),
                                                _bleEnabled(false),
                                                _clientConnected(false) {}

// 📌 Callbacks personalizados para manejar conexión BLE y mostrar el MTU negociado
class CustomServerCallbacks : public NimBLEServerCallbacks
{
    BLEManager *bleManager;

public:
    CustomServerCallbacks(BLEManager *manager) : bleManager(manager) {}

    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
    {
        uint16_t mtu = NimBLEDevice::getMTU(); // 📌 Obtener el MTU negociado
        Serial.printf("\r\nCliente conectado");
        bleManager->setClientConnected(true);
    }
};

void BLEManager::setClientConnected(bool estado)
{
    _clientConnected = estado;
}

// 📌 Iniciar BLE
void BLEManager::begin(const char *deviceName)
{
    if (!_bleEnabled)
    {
        RAMStatus(0); // Imprimir la cantidad de RAM usada
        NimBLEDevice::init(deviceName);

        NimBLEDevice::getAdvertising()->setName(deviceName);

        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(new CustomServerCallbacks(this));

        NimBLEService *pService = pServer->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

        pRxCharacteristic = pService->createCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
                                                           NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
        pTxCharacteristic = pService->createCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
                                                           NIMBLE_PROPERTY::NOTIFY);

        pService->start();
        NimBLEDevice::getAdvertising()->addServiceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
        NimBLEDevice::getAdvertising()->start(0); // 0 indica que la publicidad es indefinida

        _bleEnabled = true;
        LOGF("\r\nBLE: ON. RED: %s", deviceName);

        RAMStatus(0);           // Imprimir la cantidad de RAM usada
        _beginTime = millis(); // Guardar el tiempo de inicio
    }
}

// 📌 Apagar BLE
void BLEManager::end()
{
    if (_bleEnabled)
    {
        NimBLEDevice::deinit(true);
        pRxCharacteristic = nullptr;
        pTxCharacteristic = nullptr;
        _bleEnabled = false;
        LOG("\r\nBLE: OFF y RAM liberada.");
        RAMStatus(0); // Imprimir la cantidad de RAM usada
    }
}

// 📌 Loop para manejar datos BLE
void BLEManager::loop()
{
    if (pRxCharacteristic)
    {
        std::string value = pRxCharacteristic->getValue();
        if (!value.empty())
        {
            LOG("\r\n\r\nBLE: RECIBIDO: ");
            LOG(value.c_str());
            paqueteXL(value);                // Verificar si es un paquete XL
            pRxCharacteristic->setValue(""); // Limpia buffer de recepción
        }
    }
}

// 📌 Verifica si hay un cliente conectado
bool BLEManager::isConnected()
{
    return _clientConnected;
}

void BLEManager::enviarRespuestaBLE()
{
    LOG("\r\nBLE: RESPUESTA: ");
    LOG(_rsp_cmd_ble);

    pTxCharacteristic->setValue(_rsp_cmd_ble);
    pTxCharacteristic->notify();
    pRxCharacteristic->setValue(""); // Limpia buffer de recepción
    delay(50);

    /* pTxCharacteristic->setValue(_rsp_cmd_ble);
    pTxCharacteristic->notify();
    pRxCharacteristic->setValue(""); // Limpia buffer de recepción */
}

void BLEManager::enviarStrBLE(String rsp)
{

    memset(_rsp_cmd_ble, 0x20, BLE_RESPONSE_SIZE - 1);                // Rellenar con espacios (0x20)
    snprintf(_rsp_cmd_ble, BLE_RESPONSE_SIZE - 1, "%s", rsp.c_str()); // Copia el contenido de `String`
    enviarRespuestaBLE();
}

// 📌 Enviar datos BLE segmentados con indicadores de inicio y fin
void BLEManager::enviarRespuestaBLE_segmentado()
{
    size_t totalLen = strlen(_rsp_cmd_ble); // Tamaño total del mensaje
    size_t offset = 0;
    char paquete[21] = {0}; // Espacio suficiente para contenido

    // 📌 Enviar primer paquete con _ini
    size_t chunkSize = std::min((size_t)16, totalLen - offset); // 16 bytes después de _ini
    snprintf(paquete, sizeof(paquete), "_ini%.16s", _rsp_cmd_ble + offset);
    pTxCharacteristic->setValue((uint8_t *)paquete, strlen(paquete));
    pTxCharacteristic->notify();
    delay(20);
    offset += chunkSize;

    size_t remainingBytes = totalLen - offset; // Bytes restantes después del primer paquete

    Serial.print("\r\n\r\nenviarRespuestaBLE_segmentado: ");
    Serial.print("\r\n");
    Serial.print(paquete);

    // 📌 Enviar paquetes intermedios y final
    while (remainingBytes > 0)
    {
        chunkSize = std::min((size_t)20, remainingBytes);
        memset(paquete, 0x20, 20); // Rellenar con espacios para garantizar 20 bytes

        if (remainingBytes <= 20) // Último paquete con `~`
        {
            snprintf(paquete, 21, "%.19s~", _rsp_cmd_ble + offset);
        }
        else
        {
            snprintf(paquete, 21, "%.20s", _rsp_cmd_ble + offset);
        }

        Serial.print("\r\n");
        Serial.print(paquete);

        pTxCharacteristic->setValue((uint8_t *)paquete, 20);
        pTxCharacteristic->notify();
        delay(50);

        offset += chunkSize;
        remainingBytes -= chunkSize;
    }

    pRxCharacteristic->setValue(""); // Limpia buffer de recepción
}

// 📌 Recibir y ensamblar datos BLE segmentados con timeout
bool BLEManager::paqueteXL(const std::string &data)
{
    static bool recibiendo = false;
    static std::string bufferRecepcion = "";
    static unsigned long tiempoInicio = 0;
    const unsigned long TIMEOUT_RECEPCION = 5000; // 5 segundos

    if (data == "_ini")
    {
        bufferRecepcion.clear(); // Limpiar buffer
        recibiendo = true;
        tiempoInicio = millis(); // Iniciar tiempo de espera
        Serial.println("\r\nInicio de recepcion BLE");
        return false;
    }

    if (data == "_fin")
    {
        recibiendo = false;
        Serial.printf("\r\n\r\nRecepcion completada: %s\n", bufferRecepcion.c_str());
        procesarComandoBLE(bufferRecepcion.c_str()); // Procesar la cadena ensamblada
        return false;
    }

    if (recibiendo)
    {
        // Verificar si ha pasado el timeout
        if (millis() - tiempoInicio > TIMEOUT_RECEPCION)
        {
            recibiendo = false;
            bufferRecepcion.clear();
            Serial.println("\r\n\r\n Error: Timeout de recepcion BLE alcanzado");
            return true; // acabo tiempo de espera de paquete XL, procesa la info directamente
        }

        bufferRecepcion += data;
        Serial.printf("\r\n\r\nRecibiendo fragmento: %s", data.c_str());
        recibiendo = false;
        return false;
    }

    procesarComandoBLE(data.c_str()); // no es un paquete XL
    return true;                      // no esta recibiendo datos de paquete XL
}
