#include "MqttTools.h"

#include "ConfigSistema.h"
#include "LogSistema.h"
#include "DataManager.h"
#include "EstructurasGlobales.h"
#include "TimeManager.h"

#include "mqtt_cert.h"
#include "VariablesGlobales.h"

bool MqttTools::dato_mqtt_callback = false;
String MqttTools::payload = "";

MqttTools::MqttTools(const char *server, int port, const char *user, const char *pass)
    : client(espClient)
{
    client.setServer(server, port);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    MqttTools::dato_mqtt_callback = true; // Indicar que hay un nuevo payload

    MqttTools::payload = "";
    for (int i = 0; i < length; i++)
        MqttTools::payload += (char)payload[i]; //
    MqttTools::payload += '\0';                 // Agregar el terminador nulo al final del payload

    Serial.print("\r\n\r\n\r\nMQTT NV MSG\r\nTOPIC:\r\n" + String(topic) + "\r\nPAYLOAD:\r\n" + MqttTools::payload + "\r\n\r\n\r\n");
}

bool MqttTools::init()
{
    topic_cmd = TOPIC_SUS_1 + String(Data.numeroSerie) + TOPIC_SUS_2;
    topic_pub_ack = TOPIC_SUS_1 + String(Data.numeroSerie) + TOPIC_SUS_3;
    id_cliente = "AV-" + String(Data.numeroSerie);

    LOG("\r\n\r\nMQTT init.");
    LOG("\r\ntopic_cmd: " + topic_cmd);
    LOG("\r\ntopic_pub_ack: " + topic_pub_ack);
    LOG("\r\nid_cliente: " + id_cliente);

    espClient.setCACert(mqtt_crt);
    client.setCallback(mqtt_callback);

    return mqtt_suscripcion();
}

bool MqttTools::loop()
{

    if (!client.connected())
    {
        return mqtt_reconnect();
    }

    if (dato_mqtt_callback)
    {
        dato_mqtt_callback = false;
        nuevoPayload = true; // Indicar que hay un nuevo payload
        // Serial.print("\r\n\r\n\r\ndato_mqtt_callback\r\nPAYLOAD:\r\n" + payload + "\r\n\r\n\r\n");

        // RSP_EVALUACION payloadRsp = payloadEvaluacion(payload); // Evaluar el payload recibido
        //  error en el formato del payload
        /* if (payloadRsp.error > 0)
        {
            // Publicar el mensaje de error
            publish(TOPIC_STATUS, json, true);
        } */
    }

    client.loop();
    rev_hb_timer();
    return true;
}

bool MqttTools::publishAck(const String &payload)
{
    if (!conectado)
    {
        LOG("\r\nNo se puede publicar, no conectado a MQTT.");
        return false;
    }

    bool result = client.publish(topic_pub_ack.c_str(), payload.c_str(), true);
    return result;
}

bool MqttTools::publish(const char *topic, const String &payload, bool retain)
{
    if (!conectado)
    {
        LOG("\r\nNo se puede publicar, no conectado a MQTT.");
        return false;
    }

    // Información de debug antes de publicar
    /* LOG("\r\nIntentando publicar mensaje:");
    LOG("\r\nTopic: " + String(topic));
    LOG("\r\nPayload size: " + String(payload.length()) + " bytes");
    LOG("\r\nRetain: " + String(retain ? "true" : "false"));*/

    bool result = client.publish(topic, payload.c_str(), retain);

    /* if (result)
    {
        LOG("\r\nMensaje publicado correctamente (client.publish() = true)");
    }
    else
    {
        LOG("\r\nWarning: client.publish() = false (pero puede haberse enviado)");
        LOG("\r\nEstado cliente post-publish: " + String(client.connected() ? "conectado" : "desconectado"));
        LOG("\r\nCódigo estado MQTT: " + String(client.state()));
    } */

    return result;
}

void MqttTools::rev_hb_timer()
{
    if (!conectado)
        return;

    if (!mqtt_hb_tm_init)
    {
        mqtt_hb_tm_init = true;
        ti_hb = millis();
    }

    if (millis() - ti_hb > MQTT_CNF_TM_HB_SG * 1000)
    {
        env_hb_mqtt();
    }
}

void MqttTools::env_hb_mqtt()
{
    if (!conectado)
        return;

    ti_hb = millis();
    // Obtener datos de TimeManager
    TimeManager &tm = TimeManager::getInstance();
    String timestamp = tm.getTimeISO8601();
    String ntp_status = tm.ntp_status;

    // Construir payload JSON con timestamp y ntp_status
    String payload = "{\"dsp\":\"" + String(Data.numeroSerie) +
                     "\",\"tipo\":\"hb\"" +
                     ",\"tm\":\"" + timestamp +
                     "\",\"ntp_status\":\"" + ntp_status + "\"}";

    client.publish(TOPIC_PUB, payload.c_str(), true);
}

bool MqttTools::mqtt_reconnect()
{
    unsigned long start = millis();
    while (!client.connected())
    {
        LOG("\r\nIntentando conectar a MQTT...");
        if (client.connect(id_cliente.c_str(), mqtt_user, mqtt_pass))
        {
            LOG("\r\nConexión exitosa.");
            return mqtt_suscripcion();
        }
        if (millis() - start > 30000)
        {
            LOG("\r\nTimeout de conexión.");
            return false;
        }
        delay(5000);
    }
    return false;
}

bool MqttTools::mqtt_suscripcion()
{
    if (!client.connect(id_cliente.c_str(), mqtt_user, mqtt_pass))
        // Intentar reconectar al broker MQTT
        return mqtt_reconnect();

    if (client.subscribe(topic_cmd.c_str()))
    {
        conectado = true;
        env_hb_mqtt();
        return true;
    }
    return false;
}
