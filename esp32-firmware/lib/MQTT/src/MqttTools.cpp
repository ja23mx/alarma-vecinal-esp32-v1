/**
 * @file MqttTools.cpp
 * @brief Implementación de MqttTools sobre esp_mqtt_client (wss).
 */
#include "MqttTools.h"

#include "ConfigSistema.h"
#include "LogSistema.h"
#include "DataManager.h"
#include "EstructurasGlobales.h"
#include "TimeManager.h"
#include "VariablesGlobales.h"
#include "mqtt_cert_jlinfra_wss.h"

// NOTA: arduino_esp_crt_bundle_attach() (bundle de CAs de ESP-IDF) NO sirve aquí:
// el core arduino-esp32 usado en este proyecto no trae un bundle embebido por defecto
// (requeriría generar y embeber un .bin propio vía board_build.embed_files). En su lugar
// se validan varias CAs raíz públicas de larga vigencia embebidas en mqtt_crt_wss
// (ver mqtt_cert_jlinfra_wss.h), para tolerar que Cloudflare rote de emisor sin dejar
// la flota sin validar TLS.

// Buffer de topic usado solo para el log de depuración al reconstruir mensajes fragmentados.
static String s_topicBuf;

// ---------------------------------------------------------------------------
// Estáticos públicos
// ---------------------------------------------------------------------------

bool MqttTools::dato_mqtt_callback = false;
String MqttTools::payload = "";

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

MqttTools::MqttTools(const char *server, int port, const char *user, const char *pass)
    : _client(nullptr),
      _config(),
      _hbTmInit(false),
      _tiHb(0)
{
    _config.host = server;
    _config.port = port;
    _config.username = user;
    _config.password = pass;
}

// ---------------------------------------------------------------------------
// Manejo de eventos (ejecuta en la tarea interna de esp_mqtt_client)
// ---------------------------------------------------------------------------

void MqttTools::eventHandlerStatic(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)base;
    (void)event_id;
    static_cast<MqttTools *>(handler_args)->handleEvent(static_cast<esp_mqtt_event_handle_t>(event_data));
}

void MqttTools::handleEvent(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        LOG("\r\n\r\nMQTT (wss) conectado.");
        conectado = true;
        esp_mqtt_client_subscribe(_client, _topicCmd.c_str(), 0);
        _hbTmInit = false; // fuerza envío de heartbeat en el próximo loop()
        break;

    case MQTT_EVENT_DISCONNECTED:
        LOG("\r\n\r\nMQTT desconectado.");
        conectado = false;
        break;

    case MQTT_EVENT_DATA:
        // Un mensaje puede llegar fragmentado en varios eventos si supera el buffer interno.
        if (event->current_data_offset == 0)
        {
            payload = "";
            s_topicBuf = "";
            if (event->topic_len > 0)
                s_topicBuf.concat(event->topic, event->topic_len);
        }
        payload.concat(event->data, event->data_len);

        if (event->current_data_offset + event->data_len >= event->total_data_len)
        {
            Serial.print("\r\n\r\n\r\nMQTT NV MSG\r\nTOPIC:\r\n" + s_topicBuf +
                         "\r\nPAYLOAD:\r\n" + payload + "\r\n\r\n\r\n");
            dato_mqtt_callback = true;
        }
        break;

    case MQTT_EVENT_ERROR:
        LOG("\r\n\r\nMQTT error.");
        if (event->error_handle && event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            LOGF("\r\nMQTT error TCP/TLS: esp_err=0x%x sock_errno=%d",
                 event->error_handle->esp_tls_last_esp_err,
                 event->error_handle->esp_transport_sock_errno);
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// Métodos públicos
// ---------------------------------------------------------------------------

bool MqttTools::init(bool ethernet)
{
    (void)ethernet; // este cliente solo soporta WiFi (ver MqttTools.h)

    // esp_mqtt_client se reconecta solo; evitar recrearlo en cada llamada de
    // gestionar_conexion_wifi() (se reintenta cada MQTT_CNF_TM_INT_RECONEXION mientras no hay red).
    if (_client != nullptr)
        return true;

    _topicCmd = TOPIC_SUS_1 + String(Data.numeroSerie) + TOPIC_SUS_2;
    _topicPubAck = TOPIC_SUS_1 + String(Data.numeroSerie) + TOPIC_SUS_3;
    _clientId = "AV-" + String(Data.numeroSerie);

    LOG("\r\n\r\nMQTT init (wss). Medio: WiFi");
    LOG("\r\ntopicCmd: " + _topicCmd);
    LOG("\r\ntopicPubAck: " + _topicPubAck);
    LOG("\r\nclientId: " + _clientId);

    _config.transport = MQTT_TRANSPORT_OVER_WSS;
    _config.path = "/";
    _config.client_id = _clientId.c_str();
    _config.buffer_size = 2048;
    _config.cert_pem = mqtt_crt_wss;

    _client = esp_mqtt_client_init(&_config);
    if (_client == nullptr)
    {
        LOG("\r\n\r\nMQTT: error al crear el cliente (esp_mqtt_client_init).");
        return false;
    }

    esp_mqtt_client_register_event(_client, MQTT_EVENT_ANY, eventHandlerStatic, this);

    if (esp_mqtt_client_start(_client) != ESP_OK)
    {
        LOG("\r\n\r\nMQTT: error al iniciar el cliente (esp_mqtt_client_start).");
        return false;
    }

    return true;
}

bool MqttTools::loop()
{
    revHbTimer();
    return conectado;
}

bool MqttTools::publishAck(const String &msg)
{
    if (!conectado)
    {
        LOG("\r\nNo se puede publicar, no conectado a MQTT.");
        return false;
    }
    return esp_mqtt_client_publish(_client, _topicPubAck.c_str(), msg.c_str(), msg.length(), 0, true) != -1;
}

bool MqttTools::publish(const char *topic, const String &msg, bool retain)
{
    if (!conectado)
    {
        LOG("\r\nNo se puede publicar, no conectado a MQTT.");
        return false;
    }
    return esp_mqtt_client_publish(_client, topic, msg.c_str(), msg.length(), 0, retain) != -1;
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
        envHbMqtt();
        return;
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

    esp_mqtt_client_publish(_client, TOPIC_PUB, msg.c_str(), msg.length(), 0, true);
}
