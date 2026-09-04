# MQTT sobre WSS (Cloudflare) — Overview

Desde v0.1.0-beta.11 el firmware se conecta al broker MQTT vía **WebSocket Secure (wss)** en vez de TCP+TLS directo, porque la infraestructura del broker migró detrás de Cloudflare, que solo proxea HTTP(S)/WS (no expone un puerto TCP crudo tipo 8883).

## 1. Límite de confianza (importante)

```
ESP32 --wss (TLS público Cloudflare)--> Edge Cloudflare --túnel--> cloudflared (NAS) --TLS interno--> Mosquitto
        └──────────── esto es lo único que valida el firmware ────────────┘
```

El ESP32 **solo** valida el certificado público que sirve el edge de Cloudflare (Universal SSL). El salto interno `cloudflared → Mosquitto` usa una CA privada propia del servidor, ajena por completo al firmware — si el backend, el broker o esa CA privada cambian, **no requiere ningún cambio en el ESP32**, siempre que el hostname público, el puerto y el emisor del certificado de Cloudflare se mantengan dentro de lo que el firmware confía (ver sección 3).

## 2. Stack de transporte

Se reemplazó `PubSubClient` (que no soporta WebSocket) por **`esp_mqtt_client`**, el componente MQTT nativo de ESP-IDF incluido en el core `arduino-esp32` (no es una librería nueva en `platformio.ini`, ya viene con el framework).

| Antes (≤ beta.10) | Ahora (≥ beta.11) |
|---|---|
| `PubSubClient` + `WiFiClientSecure`/`SSLClient` | `esp_mqtt_client` (ESP-IDF) |
| TCP+TLS directo, puerto 8883 | WebSocket Secure (wss), puerto 443, path `/` |
| Reconexión manual bloqueante (`mqttReconnect()`) | Reconexión automática interna del cliente |
| Dual transporte: WiFi y Ethernet (W5500) | **Solo WiFi** (ver limitación abajo) |

### Limitación: sin soporte Ethernet

`esp_mqtt_client` transmite por sockets BSD sobre la pila **lwIP** del ESP32. El W5500 en este proyecto se maneja con `arduino-libraries/Ethernet`, que habla directo con el stack TCP/IP *interno del propio chip* (offload por hardware) — nunca pasa por lwIP. Ambos stacks son incompatibles entre sí, así que MQTT sobre Ethernet quedó fuera de alcance. `gestionar_conexion_wifi()` en [`src/wifi_mqtt_esp.cpp`](../../src/wifi_mqtt_esp.cpp) sigue intentando Ethernet primero (por si sirve para otra cosa, p. ej. NTP), pero solo inicializa MQTT cuando hay WiFi.

## 3. Certificados

`arduino_esp_crt_bundle_attach()` (el bundle de CAs de ESP-IDF) **no funciona** en el core `arduino-esp32` usado aquí: no trae un bundle embebido por defecto, requeriría generar y embeber un `.bin` propio (`gen_crt_bundle.py` + `board_build.embed_files`). En su lugar, se embeben directamente 4 certificados **raíz** públicos de larga vigencia en [`include/mqtt_cert_jlinfra_wss.h`](../../include/mqtt_cert_jlinfra_wss.h), concatenados en un solo buffer PEM (mbedtls acepta múltiples CAs en un mismo buffer como conjunto de anclas de confianza):

| CA raíz | Emisor típico | Vigente hasta |
|---|---|---|
| GTS Root R4 | Google Trust Services (ECC) | 22-06-2036 |
| GTS Root R1 | Google Trust Services (RSA) | 22-06-2036 |
| ISRG Root X1 | Let's Encrypt | 04-06-2035 |
| DigiCert Global Root CA | DigiCert | 10-11-2031 |

**Por qué 4 y no 1:** Cloudflare Universal SSL no usa un emisor fijo — rota automáticamente entre estos emisores al renovar certificados. Embeber solo el emisor actual (al momento de escribir esto, Google Trust Services) dejaría a toda la flota sin validar TLS ante una rotación futura, sin forma de recuperarse remotamente (ver sección 5). Con los 4 embebidos, una rotación dentro de ese pool no requiere firmware nuevo.

**Mantenimiento:** revisar este archivo antes de 2031 (vencimiento más próximo, DigiCert). Si Cloudflare empieza a usar un emisor fuera de este pool, la conexión fallará con `MQTT error TCP/TLS` en el log serie y habrá que agregar la CA correspondiente.

## 4. Configuración de conexión

| Parámetro | Valor | Definido en |
|---|---|---|
| Host | `mqtt-sltnas.jlinfra.online` | `src/VariablesGlobales.cpp` |
| Puerto | `443` | `src/VariablesGlobales.cpp` |
| Transporte | `MQTT_TRANSPORT_OVER_WSS` | `lib/MQTT/src/MqttTools.cpp` |
| Path | `/` | `lib/MQTT/src/MqttTools.cpp` |
| Usuario/contraseña | compartidos para todos los dispositivos | `src/VariablesGlobales.cpp` |

Topics y formato de mensajes MQTT no cambiaron (ver [`docs/mqtt-memory-rw/overview.md`](../mqtt-memory-rw/overview.md) para el flujo de comandos).

## 5. Gotcha: conexión asíncrona vs. arranque síncrono esperado

`esp_mqtt_client_start()` arranca una tarea FreeRTOS interna y **retorna de inmediato**, antes de que el handshake TLS/WS termine — a diferencia de `PubSubClient`, que bloqueaba hasta conectar o fallar.

`gestionar_conexion_wifi()` (en `wifi_mqtt_esp.cpp`) fue escrito asumiendo el comportamiento síncrono viejo: si no se ajusta, marca `conexion_wifi = false` prematuramente (porque `conectado` todavía es `false` en el primer chequeo), y ~30s después el `loop()` principal reinicializa WiFi/Ethernet **aunque MQTT ya esté conectado**, tumbando el socket en curso.

**Fix aplicado:** `MqttTools::init()` espera (bloqueante, timeout de 30s — el mismo que usaba el `mqttReconnect()` original) a que `conectado` se confirme antes de retornar. Si se vuelve a tocar `MqttTools::init()`, mantener este comportamiento o ajustar `gestionar_conexion_wifi()` para tolerar una conexión asíncrona.

## 6. Archivos involucrados

| Archivo | Rol |
|---|---|
| `lib/MQTT/src/MqttTools.h` / `.cpp` | Cliente MQTT (`esp_mqtt_client`), eventos, heartbeat, publish/subscribe |
| `include/mqtt_cert_jlinfra_wss.h` | CAs raíz embebidas para validar TLS |
| `src/VariablesGlobales.cpp` | Host/puerto/usuario/contraseña del broker |
| `src/wifi_mqtt_esp.cpp` | `gestionar_conexion_wifi()`, `mqtt_loop()` |
