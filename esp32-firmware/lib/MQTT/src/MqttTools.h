/**
 * @file MqttTools.h
 * @brief Gestor MQTT con soporte dual WiFi y Ethernet (W5500).
 *
 * Encapsula PubSubClient con selección dinámica de transporte TLS:
 * WiFiClientSecure para conexiones inalámbricas y SSLClient+EthernetClient
 * para W5500. El medio activo se selecciona en init().
 *
 * @author AV-RF-WF
 * @version 1.1.0
 * @date 2026
 */
#pragma once

#include <WiFiClientSecure.h>
#include <Ethernet.h>
#include <SSLClient.h>
#include <PubSubClient.h>

#include "EstructurasGlobales.h"

/**
 * @class MqttTools
 * @brief Gestor MQTT TLS con soporte dual WiFi (WiFiClientSecure) y Ethernet W5500 (SSLClient).
 *
 * Instanciar globalmente. Llamar init(bool ethernet) una vez que el medio de red
 * esté disponible. Llamar loop() periódicamente desde el loop principal para
 * mantener la conexión y procesar mensajes entrantes.
 */
class MqttTools
{
private:
    // --- Transporte WiFi ---
    WiFiClientSecure _wifiClient;
    PubSubClient     _wifiMqtt;

    // --- Transporte Ethernet (W5500) ---
    EthernetClient _ethClient;
    SSLClient      _sslEthClient;
    PubSubClient   _ethMqtt;

    // --- Selección de medio activo ---
    PubSubClient* _activeClient;
    bool          _useEthernet;

    // --- Estado MQTT ---
    String        _topicCmd;
    String        _topicPubAck;
    String        _clientId;
    bool          _hbTmInit;
    unsigned long _tiHb;

    /** @brief Verifica el timer y dispara el heartbeat periódico. */
    void revHbTimer();

    /** @brief Construye y publica el payload JSON de heartbeat. */
    void envHbMqtt();

    /** @brief Reintenta la conexión MQTT hasta timeout de 30 s. */
    bool mqttReconnect();

    /** @brief Conecta al broker y suscribe al topic de comandos. */
    bool mqttSuscripcion();

public:
    /**
     * @brief Construye el gestor MQTT. Configura servidor en ambos clientes internos.
     *
     * @param server Dirección del broker MQTT.
     * @param port   Puerto del broker (normalmente 8883 para TLS).
     * @param user   Usuario MQTT.
     * @param pass   Contraseña MQTT.
     */
    MqttTools(const char* server, int port, const char* user, const char* pass);

    /**
     * @brief Inicializa el cliente MQTT sobre el medio seleccionado.
     *
     * Configura el certificado TLS apropiado, registra el callback y realiza
     * la primera suscripción al broker. Debe llamarse después de que el medio
     * de red (WiFi o Ethernet) esté conectado.
     *
     * @param ethernet true  = SSLClient + EthernetClient (W5500, TrustAnchors BearSSL).
     *                 false = WiFiClientSecure (certificado PEM).
     * @return true si la suscripción al broker fue exitosa.
     */
    bool init(bool ethernet = false);

    /**
     * @brief Ejecuta el loop MQTT: reconexión automática, despacho de callback y heartbeat.
     *
     * @return true si el cliente está conectado al broker al finalizar el ciclo.
     */
    bool loop();

    /**
     * @brief Publica un mensaje en el topic de ACK del dispositivo (retain = true).
     *
     * @param payload Payload JSON a publicar.
     * @return true si la publicación fue aceptada por PubSubClient.
     */
    bool publishAck(const String& payload);

    /**
     * @brief Publica un mensaje en un topic arbitrario.
     *
     * @param topic   Topic MQTT destino.
     * @param payload Payload a publicar.
     * @param retain  true para mensaje retenido en el broker.
     * @return true si la publicación fue aceptada por PubSubClient.
     */
    bool publish(const char* topic, const String& payload, bool retain);

    bool conectado    = false; ///< true si el cliente está conectado al broker.
    bool nuevoPayload = false; ///< true cuando hay un nuevo payload sin procesar.

    static bool   dato_mqtt_callback; ///< Flag seteado por el callback de PubSubClient.
    static String payload;            ///< Payload del último mensaje MQTT recibido.
};
