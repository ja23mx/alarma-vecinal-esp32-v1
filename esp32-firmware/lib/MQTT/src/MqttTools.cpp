/**
 * @file MqttTools.cpp
 * @brief Implementación de MqttTools.
 */
#include "MqttTools.h"

#include "ConfigSistema.h"
#include "LogSistema.h"
#include "DataManager.h"
#include "EstructurasGlobales.h"
#include "TimeManager.h"
#include "VariablesGlobales.h"

#include "mqtt_cert_jlinfra_wifi.h"
#include "mqtt_cert_jlinfra_ethernet.h"

// ---------------------------------------------------------------------------
// Estáticos públicos
// ---------------------------------------------------------------------------

bool MqttTools::dato_mqtt_callback = false;
String MqttTools::payload = "";

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

MqttTools::MqttTools(const char *server, int port, const char *user, const char *pass)
    : _wifiMqtt(_wifiClient),
      _sslEthClient(_ethClient, TAs, TAs_NUM, W5500_ENTROPY_PIN),
      _ethMqtt(_sslEthClient),
      _activeClient(nullptr),
      _useEthernet(false),
      _hbTmInit(false),
      _tiHb(0)
{
    _wifiMqtt.setBufferSize(2048);
    _wifiMqtt.setServer(server, port);
    _ethMqtt.setBufferSize(2048);
    _ethMqtt.setServer(server, port);
}

// ---------------------------------------------------------------------------
// Callback (free function requerida por PubSubClient)
// ---------------------------------------------------------------------------

/**
 * @brief Callback de PubSubClient para mensajes MQTT entrantes.
 *
 * Copia el payload al buffer estático y activa el flag para procesamiento
 * diferido desde loop(). No realizar operaciones bloqueantes aquí.
 *
 * @param topic   Tópico del mensaje recibido.
 * @param payload Puntero al payload (no null-terminated).
 * @param length  Longitud del payload en bytes.
 *
 * @warning No llamar funciones bloqueantes ni delay() desde este callback.
 */
static void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    MqttTools::dato_mqtt_callback = true;
    MqttTools::payload = "";
    for (unsigned int i = 0; i < length; i++)
        MqttTools::payload += (char)payload[i];
    MqttTools::payload += '\0';

    Serial.print("\r\n\r\n\r\nMQTT NV MSG\r\nTOPIC:\r\n" + String(topic) +
                 "\r\nPAYLOAD:\r\n" + MqttTools::payload + "\r\n\r\n\r\n");
}

// ---------------------------------------------------------------------------
// Métodos públicos
// ---------------------------------------------------------------------------

bool MqttTools::init(bool ethernet)
{
    _useEthernet = ethernet;
    _activeClient = ethernet ? &_ethMqtt : &_wifiMqtt;

    _topicCmd = TOPIC_SUS_1 + String(Data.numeroSerie) + TOPIC_SUS_2;
    _topicPubAck = TOPIC_SUS_1 + String(Data.numeroSerie) + TOPIC_SUS_3;
    _clientId = "AV-" + String(Data.numeroSerie);

    LOG("\r\n\r\nMQTT init. Medio: " + String(ethernet ? "Ethernet" : "WiFi"));
    LOG("\r\ntopicCmd: " + _topicCmd);
    LOG("\r\ntopicPubAck: " + _topicPubAck);
    LOG("\r\nclientId: " + _clientId);

    if (!ethernet)
        _wifiClient.setCACert(mqtt_crt_wifi);

    _activeClient->setCallback(mqttCallback);
    return mqttSuscripcion();
}

bool MqttTools::loop()
{
    if (!_activeClient->connected())
        return mqttReconnect();

    if (dato_mqtt_callback)
    {
        dato_mqtt_callback = false;
        nuevoPayload = true;
    }

    _activeClient->loop();
    revHbTimer();
    return true;
}

bool MqttTools::publishAck(const String &msg)
{
    if (!conectado)
    {
        LOG("\r\nNo se puede publicar, no conectado a MQTT.");
        return false;
    }
    return _activeClient->publish(_topicPubAck.c_str(), msg.c_str(), true);
}

bool MqttTools::publish(const char *topic, const String &msg, bool retain)
{
    if (!conectado)
    {
        LOG("\r\nNo se puede publicar, no conectado a MQTT.");
        return false;
    }
    return _activeClient->publish(topic, msg.c_str(), retain);
}

// ---------------------------------------------------------------------------
// Métodos privados
// ---------------------------------------------------------------------------

void MqttTools::revHbTimer()
{
    if (!conectado)
        return;

    if (!_hbTmInit)
    {
        _hbTmInit = true;
        _tiHb = millis();
    }

    if (millis() - _tiHb > MQTT_CNF_TM_HB_SG * 1000)
        envHbMqtt();
}

void MqttTools::envHbMqtt()
{
    if (!conectado)
        return;

    _tiHb = millis();

    TimeManager &tm = TimeManager::getInstance();
    String timestamp = tm.getTimeISO8601();
    String ntpStatus = tm.ntp_status;

    String msg = "{\"dsp\":\"" + String(Data.numeroSerie) +
                 "\",\"tipo\":\"hb\"" +
                 ",\"tm\":\"" + timestamp +
                 "\",\"ntp_status\":\"" + ntpStatus + "\"}";

    _activeClient->publish(TOPIC_PUB, msg.c_str(), true);
}

bool MqttTools::mqttReconnect()
{
    // Ethernet: link caído → limpiar error SSL y salir rápido para habilitar fallback a WiFi
    if (_useEthernet && Ethernet.linkStatus() != LinkON)
    {
        _sslEthClient.stop();
        LOG("\r\nEthernet: link caído, cancelando reconexión MQTT.");
        return false;
    }

    unsigned long start = millis();
    while (!_activeClient->connected())
    {
        if (_useEthernet && Ethernet.linkStatus() != LinkON)
        {
            _sslEthClient.stop();
            LOG("\r\nEthernet: link caído durante reconexión MQTT.");
            return false;
        }

        LOG("\r\nIntentando conectar a MQTT...");

        // Fix SSLClient: stop() limpia m_write_error antes de cada intento TLS
        if (_useEthernet)
            _sslEthClient.stop();
        else
            _wifiClient.stop();

        if (_activeClient->connect(_clientId.c_str(), mqtt_user, mqtt_pass))
        {
            LOG("\r\nConexión exitosa.");
            return mqttSuscripcion();
        }
        if (millis() - start > 30000)
        {
            LOG("\r\nTimeout de conexión MQTT.");
            return false;
        }
        delay(5000);
    }
    return false;
}

bool MqttTools::mqttSuscripcion()
{
    if (!_activeClient->connected())
    {
        if (!_activeClient->connect(_clientId.c_str(), mqtt_user, mqtt_pass))
            return mqttReconnect();
    }

    if (_activeClient->subscribe(_topicCmd.c_str()))
    {
        conectado = true;
        envHbMqtt();
        return true;
    }
    return false;
}
