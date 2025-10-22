#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <vector>
#include <string>

#include "EstructurasGlobales.h"

class MqttTools
{
private:
    WiFiClientSecure espClient;
    PubSubClient client;

    String topic_cmd;
    String topic_pub_ack;
    String id_cliente;

    bool mqtt_hb_tm_init = false;
    unsigned long ti_hb = 0;

    void rev_hb_timer();
    void env_hb_mqtt();
    bool mqtt_reconnect();
    bool mqtt_suscripcion();

public:
    MqttTools(const char *server, int port, const char *user, const char *pass);
    bool init();
    bool loop();
    bool publishAck(const String &payload);
    bool publish(const char *topic, const String &payload, bool retain);

    bool conectado = false;
    bool nuevoPayload = false;
    static bool dato_mqtt_callback; 
    static String payload;
};