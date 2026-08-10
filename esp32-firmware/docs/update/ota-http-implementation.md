# OTA via HTTP — Implementación

Proceso de actualización de firmware disparado por MQTT y ejecutado
via HTTP sobre el medio de red activo (Ethernet W5500 o WiFi).

## Archivos involucrados

| Archivo | Rol |
|---|---|
| `include/VariablesGlobales.h` / `src/VariablesGlobales.cpp` | `otaUrl[256]`, `otaPendiente` |
| `include/ota_esp.h` / `src/ota_esp.cpp` | Implementación del proceso OTA |
| `include/wifi_mqtt_esp.h` / `src/wifi_mqtt_esp.cpp` | Trigger y getter de medio activo |
| `lib/GestorCmd/src/CmdJson.cpp` | Extrae URL y activa `otaPendiente` |

## Flujo completo

```
MQTT accion:init (con campo "url")
  └─ processUpdateJson()
       ├─ otaUrl = url, otaPendiente = true
       └─ generateAckUpdate() → ACK con versión actual

mqtt_loop()
  ├─ publishAck(jsonBuffer)           ← ACK enviado al servidor
  ├─ mqttManager.loop() + delay(500)  ← garantía de envío
  └─ ejecutarOTA(otaUrl)
       ├─ parseUrl() → host, port, path
       ├─ getUsingEthernet()?
       │    Sí → otaEthernet(): EthernetClient manual HTTP/1.0
       │    No → otaWiFi(): HTTPClient + WiFiClient
       ├─ Update.begin(UPDATE_SIZE_UNKNOWN)
       ├─ Descarga en chunks + Update.write()
       ├─ Update.end() → _verifyEnd() → esp_ota_set_boot_partition()
       └─ ESP.restart() (siempre, éxito o error)

Tras reset
  └─ HB publicado → servidor envía accion:status → ACK con versión nueva
```

## Variables globales

```cpp
char otaUrl[256];   // URL del binario recibida vía MQTT
bool otaPendiente;  // flag: OTA debe ejecutarse en el próximo ciclo de mqtt_loop()
```

## Requisito de tabla de particiones

Se requieren **dos slots OTA** (`ota_0` y `ota_1`). Con un solo slot,
`esp_ota_set_boot_partition()` falla con `assert()` → `abort()`.

Usar `config_particion.csv` (no `huge_app.csv`):

| Partición | Tipo | Tamaño |
|---|---|---|
| nvs | data/nvs | 20 KB |
| otadata | data/ota | 8 KB |
| app0 (ota_0) | app | 3312 KB |
| app1 (ota_1) | app | 3312 KB |
| spiffs | data/spiffs | 1536 KB |
| coredump | data/coredump | 64 KB |

> Al cambiar la tabla de particiones se requiere borrado completo del flash
> (`esptool.py erase_flash`) antes de reflashear.

## Manejo de errores MQTT

| Condición | Respuesta |
|---|---|
| Campo `accion` ausente | `ack-update` + `ERROR_FORMATO_INVALIDO` |
| `accion` desconocida | `ack-update` + `ERROR_PARAMETROS_INVALIDOS` |
| URL vacía en `init` | ACK enviado, `otaPendiente` no se activa |
| Fallo en descarga/flash | `ESP.restart()` de todas formas |

## Troubleshooting

### HTTP 301 al descargar el binario

Síntoma en log: `OTA WiFi: HTTP GET falló. Código: 301`, seguido de reinicio sin flashear
(el firmware reinicia siempre tras un fallo de descarga, ver tabla de arriba).

**No es un bug de `otaWiFi()`/`otaEthernet()`** — ninguno de los dos sigue redirecciones,
por diseño, así que un 301 real del servidor siempre se reporta como fallo. Antes de tocar
el firmware, descartar el servidor: probar la URL exacta del binario con `curl -D -` desde
otra máquina. Si responde `200 OK` limpio ahí pero el ESP32 recibe 301, la causa está en la
infraestructura de `update-ssh` (servidor Python efímero en `127.0.0.1:9091` + proxy nginx) —
caso confirmado (2026-08-07, beta.9): el propio servidor devolvía 301 en vez de proxyar
correctamente al backend temporal.
