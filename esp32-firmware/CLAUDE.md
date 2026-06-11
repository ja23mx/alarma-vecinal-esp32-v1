# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build commands

```bash
# Compilar firmware
pio run

# Compilar y flashear al ESP32
pio run --target upload

# Monitor serie
pio device monitor --baud 115200

# Compilar + flashear + monitorear
pio run --target upload && pio device monitor --baud 115200

# Limpiar build
pio run --target clean
```

El post-build script (`bin-tools-py/bin_build_manager.py`) se ejecuta automáticamente al compilar y genera el binario en `../esp32-bin-files/ota/` y una copia en `../esp32-bin-files/releases/v{version}/`.

## Configuración del entorno

**Antes de compilar, verificar en `include/ConfigSistema.h`:**
- `ENTORNO`: `1` = DEV, `2` = PROD — cambia los topics MQTT (`AV/DEV/...` vs `AV/PROD/...`)
- `PROG_LOCAL`: `1` = inicia servidor web local al arrancar, `0` = modo Access Point
- `TARJETA_SLT_V1`: define el pinout activo (solo hay una variante actualmente)
- `board_build.partitions` en `platformio.ini`: usar `huge_app.csv` (1 slot OTA, estable). Las tablas con 2 slots OTA causan boot loop en este hardware — no cambiar sin resolver ese issue primero.

**Versión del firmware — fuente única de verdad: `include/CnfTarjeta.h`**
```c
#define SISTEMA_FIRMWARE "esp32.av"
#define SISTEMA_VERSION  "0.1.0"
#define SISTEMA_ETAPA    "beta.1"   // alpha / beta.N / rc.N / stable
```
No editar `VERSION` en `bin-tools-py/bin_build_config.py` — el script la lee del `.h` automáticamente.

## Arquitectura

### Capas del sistema

```
main.cpp
├── Loop principal: MQTT + botón de programación + serial
├── FreeRTOS tasks (creadas en setup()):
│   ├── crearTareaNeoPixel()      — animaciones LED ring
│   ├── crearTareaProcesos()      — máquina de estados de alarma
│   ├── crearTareaProcesosCmd()   — despacho de comandos de audio/salidas
│   └── crearTareaGestionSalida() — control PWM/fade de la salida física
│
├── lib/DataManager   — persistencia (NVS + LittleFS), controles RF, redes WiFi
├── lib/GestorCmd     — parser y dispatcher de comandos (JSON y D2D)
├── lib/MQTT          — cliente MQTT TLS dual WiFi/Ethernet
├── lib/WiFiTools     — gestión de conexión WiFi y AP
├── lib/BLEManager    — configuración inicial vía BLE
├── lib/ServerWeb     — servidor web de configuración (AsyncWebServer)
├── lib/TimeManager   — sincronización NTP (ezTime)
└── lib/Neopixel      — patrones de animación para WS2812B
```

### Flujo de comandos MQTT

1. `MqttTools::loop()` recibe mensaje → llama `GestorCmd.process(payload, CmdOrigen::SRV_EXT)`
2. `GestorCMD::process()` detecta formato: JSON (`datosJSON()`) o D2D (`datosD2D()`)
3. En JSON, el campo `tipo` enruta a: `emgCoDi`, `handshake`, `cmd`, o `update`
4. El resultado se acumula en `GestorCmd.jsonBuffer` con flag `rspJson = true`
5. `mqtt_loop()` en `wifi_mqtt_esp.cpp` publica el ACK si `rspJson == true`

Los comandos que afectan salidas o audio se encolan en vectores globales (`cmdLocal`, `cmdExterno`, `cmdSalidas`, `cmdAudio`) y las tareas FreeRTOS los consumen de forma asíncrona.

### Selección de red (WiFi vs Ethernet W5500)

`get_config_red()` lee el pin ADC `PIN_CNF_RED_ADC` con antirrebote para determinar `config_red`:
- `1` = Ethernet W5500 (default)
- `2` = WiFi

`MqttTools::init(bool ethernet)` instancia el transporte correcto: `SSLClient` + `EthernetClient` para W5500, o `WiFiClientSecure` para WiFi. El certificado TLS está en `include/mqtt_cert_jlinfra_ethernet.h` / `include/mqtt_cert_jlinfra_wifi.h`.

### Persistencia

- **NVS (Preferences)**: número de serie, PIN, periféricos habilitados, timer de alarma, tamper, redes WiFi, señales RF
- **LittleFS**: modelos de controles RF en JSON, configuraciones extendidas
- `DataManager Data` es la instancia global singleton que agrupa toda la persistencia

### Logging

`LOG(x)` y `LOGF(fmt, ...)` definidos en `include/LogSistema.h`. Controlados por `#define DEBUG_MODE 1` — comentar para builds de producción silenciosos.

## Estructura de releases

```
esp32-bin-files/
  ota/               ← último build (sobreescribe en cada compilación)
  releases/
    v0.1.0-beta.1/   ← historial inmutable por versión
```

Para liberar una versión: actualizar `CnfTarjeta.h` → compilar → commit → merge a `main` → `git tag v{VERSION}-{ETAPA}` → push.
