#define MQTT_MAX_PACKET_SIZE 10240 // Aumenta el tamaño si el payload es grande (10KB)
#define MQTT_KEEPALIVE 60          // Asegura que no se cierre la conexión

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <ArduinoJson.h>

#include "ConfigSistema.h"
#include "LogSistema.h"
#include "WiFiTool.h"
#include "DataManager.h"
#include "MqttTools.h"
#include "TimeManager.h"

#include "wifi_mqtt_esp.h"
#include "VariablesGlobales.h"
#include "mqtt_cert.h"
#include "procesos_cmd.h"
#include "gestor_salidas.h"
#include "GestorCmd.h"
#include "voz_esp.h"
#include "tarea_neopixel.h"

extern WiFiTool WiFiTools; // Instancia de la clase WiFiTool

bool envio_async_mqtt_msg_ctrl_alarma = false; // Variable para verificar si se ha enviado un mensaje de control
String msg_ctrl_alarma;                        // Variable para almacenar el mensaje de control

// Crear una instancia del objeto MqttTools
MqttTools mqttManager(mqtt_server, mqtt_port, mqtt_user, mqtt_pass);

bool mqtt_loop()
{

    rev_msg_ctrl_alarma(); // mensaje de control de alarma

    if (mqttManager.loop())
    {
        if (mqttManager.nuevoPayload)
        {
            mqttManager.nuevoPayload = false;                                  // Reiniciar la variable de nuevo payload
            sync_evento_mqtt(mqttManager.payload);                             // Procesar el evento MQTT
            if (GestorCmd.rspJson)                                             // Si se debe enviar una respuesta JSON
            {                                                                  //
                GestorCmd.rspJson = false;                                     // Reiniciar la variable de respuesta JSON
                async_led_server_data();                                       // Indicar que hay datos recepcionados del servidor
                mqttManager.publishAck(GestorCmd.jsonBuffer);                  // Publicar el mensaje de error
                memset(GestorCmd.jsonBuffer, 0, sizeof(GestorCmd.jsonBuffer)); // Limpiar el buffer JSON
            }
        }
        return true; // Retornar verdadero si el loop de MQTT se ejecuta correctamente
    }

    return false; // Retornar falso si el loop de MQTT no se ejecuta correctamente
}

void envio_msg_init_broker(void)
{
    if (!mqttManager.conectado) // Si no está conectado a MQTT, salir
        return;

    Data.CmdSerial("mqttHandShakeRequest:");

    bool rsp = mqttManager.publish(TOPIC_STATUS, Data.jsonCMDSerial, 1); // Publicar el mensaje de inicialización al broker MQTT

    // Limpiar el buffer Data.jsonCMDSerial
    memset(Data.jsonCMDSerial, 0, sizeof(Data.jsonCMDSerial));

    // Con QoS 0 y retain=true, client.publish() puede retornar false incluso si el mensaje se envía
    // Lo importante es que el cliente siga conectado
    if (rsp)
        LOG("\r\n\r\nMQTT. Mensaje de inicialización enviado (client.publish() = true).");
    else if (mqttManager.conectado)
        LOG("\r\n\r\nMQTT. Mensaje de inicialización enviado (cliente sigue conectado, probablemente exitoso).");
}

void rev_msg_ctrl_alarma(void)
{

    if (!mqttManager.conectado) // Si no está conectado a MQTT, salir
        return;

    if (envio_async_mqtt_msg_ctrl_alarma)
    {
        String topic = String(TOPIC_SUS_1);
        topic += String(Data.numeroSerie);
        topic += String(TOPIC_SUS_3);
        Serial.print("\r\n\r\n async_mqtt_msg_ctrl_alarma. topic:" + topic);

        envio_async_mqtt_msg_ctrl_alarma = false;                                     // Reiniciar la variable de envío del mensaje de control
        bool rsp = mqttManager.publish(topic.c_str(), msg_ctrl_alarma.c_str(), true); // Publicar el heartbeat
        if (rsp)
            LOG("\r\n\r\nMQTT. Publicado heartbeat:\r\n" + msg_ctrl_alarma); // Publicar el heartbeat
        else
            LOG("\r\n\r\nMQTT. Error al publicar heartbeat:\r\n" + msg_ctrl_alarma); // Error al publicar el heartbeat
    }
}

void async_mqtt_msg_ctrl_alarma(void)
{
    if (Data.perifericos[1] == 0) // Si el periférico de wifi está deshabilitado
        return;                   // Salir sin hacer nada

    LOG("\r\n\r\nasync_mqtt_msg_ctrl_alarma.");
    LOG("\r\n\r\nMQTT. Enviando mensaje de control.");
    LOG("\r\n\r\nMQTT. Configuracion: " + String(modeloCtrlAVRx.configuracion));

    static bool estado_alarma_on = false;

    switch (modeloCtrlAVRx.configuracion) // Obtener la configuración del control
    {
    case 1: // alerta vecinal
        if (envio_async_mqtt_msg_ctrl_alarma)
            return; // Si el mensaje ya fue enviado, salir

        // Si la alarma ya está desactivada y se presionó el botón de desactivación, salir
        // no enviar mensaje redundante
        if (estado_alarma_on == false && (estadoCompRFAv.btnIndice + 1) == modeloCtrlAVRx.boton_desactivacion)
            return;

        (estadoCompRFAv.btnIndice + 1) == modeloCtrlAVRx.boton_desactivacion ? estado_alarma_on = false : estado_alarma_on = true;

        // Obtener datos de TimeManager
        TimeManager &tm = TimeManager::getInstance();
        String timestamp = tm.getTimeISO8601();
        String ntp_status = tm.ntp_status;

        Serial.print("\r\n\r\nestado_alarma_on: " + String(estado_alarma_on));

        // Construir mensaje con nueva estructura
        msg_ctrl_alarma = "{\"dsp\":\"" + String(Data.numeroSerie) +
                          "\",\"type\":\"ctrl-av-1\"" +
                          ",\"nm-ctrl\":" + String(estadoCompRFAv.control) +
                          ",\"btn-nm\":" + String(estadoCompRFAv.btnIndice + 1) +
                          ",\"btn-str\":\"" + String(estadoCompRFAv.btnStr) +
                          "\",\"tm\":\"" + timestamp +
                          "\",\"ntp_status\":\"" + ntp_status +
                          "\",\"estado-alarma\":" + String(estado_alarma_on) +
                          "}";

        envio_async_mqtt_msg_ctrl_alarma = true;                                  // Marcar como enviado
        LOG("\r\n\r\nMQTT. Publicado mensaje de control:\r\n" + msg_ctrl_alarma); // Publicar el mensaje de control
        break;
    }
}

bool gestionar_conexion_wifi(void)
{

    if (init_conexion_wifi())              // Si la conexión es exitosa
    {                                      //
        TimeManager::getInstance().init(); // Inicializar TimeManager
        if (mqttManager.init())
        {
            envio_msg_init_broker(); // Enviar el mensaje de inicialización al broker MQTT
            Serial.print("\r\n\r\nMQTT inicializado correctamente.");
            return true; // Retornar verdadero si la conexión es exitosa
        }
        else
        {
            Serial.print("\r\n\r\nError al inicializar MQTT.");
        }
    }

    return false; // Si no se pudo conectar a ninguna red conocida, retornar falso
}

bool init_conexion_wifi(void)
{

    if (Data.redesGuardadas.empty())        // Verificar si hay redes guardadas
    {                                       //
        LOG("\r\nNo hay redes guardadas."); // no hay redes guardadas
        return false;
    }

    if (!WiFiTools.scanInit()) // Iniciar el escaneo de redes WiFi
        return false;          // Si no redes disponibles, salir

    LOG("\r\n\r\ninit_conexion_wifi.");

    LOG("\r\nRedes guardadas:");
    for (const auto &red : Data.redesGuardadas)
    {
        LOGF("\r\n* SSID: %s\r\n  Clave: %s", red.ssid.c_str(), red.password.c_str());
    }

    // Obtener las redes disponibles
    const std::vector<RedWiFi> &redesDisponibles = WiFiTools.getRedesDisponibles();
    if (redesDisponibles.empty())
    {
        LOG("\r\nNo se encontraron redes disponibles.");
        return false;
    }

    // Comparar las redes disponibles con las guardadas
    for (const auto &redDisponible : redesDisponibles)
    {
        bool encontrada = false;

        for (const auto &redGuardada : Data.redesGuardadas)
        {
            if (redDisponible.ssid == redGuardada.ssid)
            {
                LOGF("\r\n\r\n\r\nRed Guard Dsp:\r\n %s", redDisponible.ssid.c_str());
                LOGF("\r\n%s", redGuardada.password.c_str());

                // Aquí puedes priorizar la conexión a esta red
                if (WiFiTools.startConnection(redGuardada.ssid.c_str(), redGuardada.password.c_str(), 15000))
                {
                    // LOGF("\r\nConectado a %s", redGuardada.ssid.c_str());
                    // LOGF("\r\nIP: %s", WiFi.localIP().toString().c_str()); //
                    return true; // Salir del bucle si se conecta exitosamente
                }
                else
                {
                    LOGF("\r\nNo se pudo conectar a %s", redGuardada.ssid.c_str());
                }

                encontrada = true;
                break;
            }
        }
    }

    return false; // Si no se pudo conectar a ninguna red conocida, retornar falso
}