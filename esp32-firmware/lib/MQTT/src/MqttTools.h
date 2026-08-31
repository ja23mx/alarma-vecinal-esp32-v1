/**
 * @file MqttTools.h
 * @brief Gestor MQTT sobre WebSocket Secure (wss) usando esp_mqtt_client (ESP-IDF).
 *
 * Reemplaza el transporte TCP+TLS directo (PubSubClient) por MQTT sobre WSS,
 * requerido por el broker actual (Cloudflare solo proxea HTTP(S)/WS). Soporta
 * únicamente el medio WiFi: el transporte Ethernet (W5500 vía arduino-libraries/Ethernet)
 * no se integra a la pila lwIP que usa esp_mqtt_client, por lo que queda fuera de
 * alcance de este cliente.
 *
 * @author AV-RF-WF
 * @version 2.0.0
 * @date 2026
 */
#pragma once

#include <Arduino.h>
#include "mqtt_client.h"

#include "EstructurasGlobales.h"

/**
 * @class MqttTools
 * @brief Gestor MQTT sobre wss:// usando el cliente esp_mqtt_client de ESP-IDF.
 *
 * Instanciar globalmente. Llamar init() una vez que WiFi esté conectado.
 * Llamar loop() periódicamente desde el loop principal para procesar el
 * heartbeat y el flag de nuevo payload (la reconexión la maneja esp_mqtt_client
 * internamente).
 */
class MqttTools
{
private:
    esp_mqtt_client_handle_t _client;
    esp_mqtt_client_config_t _config;

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

    /** @brief Maneja los eventos del cliente esp_mqtt (conectado, dato, desconectado, error). */
    void handleEvent(esp_mqtt_event_handle_t event);

    /** @brief Callback libre requerido por esp_event, reenvía al método de instancia. */
    static void eventHandlerStatic(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

public:
    /**
     * @brief Construye el gestor MQTT. No conecta todavía.
     *
     * @param server Dirección del broker MQTT (host, sin esquema).
     * @param port   Puerto del broker (443 para wss).
     * @param user   Usuario MQTT.
     * @param pass   Contraseña MQTT.
     */
    MqttTools(const char* server, int port, const char* user, const char* pass);

    /**
     * @brief Inicializa y arranca el cliente MQTT sobre wss:// (WiFi).
     *
     * Configura URI, credenciales, certificado (bundle de CAs públicas de ESP-IDF)
     * y registra el manejador de eventos. Debe llamarse después de que WiFi esté conectado.
     *
     * @param ethernet Sin efecto, se mantiene por compatibilidad de firma con el call site.
     *                 Este cliente solo soporta WiFi.
     * @return true si el cliente se arrancó correctamente.
     */
    bool init(bool ethernet = false);

    /**
     * @brief Ejecuta el loop MQTT: heartbeat y despacho de nuevo payload.
     *
     * @return true si el cliente está conectado al broker al finalizar el ciclo.
     */
    bool loop();

    /**
     * @brief Publica un mensaje en el topic de ACK del dispositivo (retain = true).
     *
     * @param payload Payload JSON a publicar.
     * @return true si la publicación fue aceptada.
     */
    bool publishAck(const String& payload);

    /**
     * @brief Publica un mensaje en un topic arbitrario.
     *
     * @param topic   Topic MQTT destino.
     * @param payload Payload a publicar.
     * @param retain  true para mensaje retenido en el broker.
     * @return true si la publicación fue aceptada.
     */
    bool publish(const char* topic, const String& payload, bool retain);

    bool conectado    = false; ///< true si el cliente está conectado al broker.
    bool nuevoPayload = false; ///< true cuando hay un nuevo payload sin procesar.

    static bool   dato_mqtt_callback; ///< Flag seteado por el evento MQTT_EVENT_DATA.
    static String payload;            ///< Payload del último mensaje MQTT recibido.
};
