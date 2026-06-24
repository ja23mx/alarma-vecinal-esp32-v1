# MQTT Memory R/W — Overview

Permite leer y escribir la memoria del dispositivo (configuración, redes WiFi, controles RF) mediante mensajes MQTT en formato JSON.

## Flujo de datos

```
Broker → topic cmd → MqttTools::loop()
  → GestorCmd.process()
    → GestorCMD::datosJSON()      (tipo == "mem")
      → processMemJson()           dispatcher por accion + target
        → read/writeMemConfig/Wifi/RF()
          → serializa respuesta en jsonBuffer
          → rspJson = true
→ mqtt_loop() detecta rspJson
  → mqttManager.publishAck(jsonBuffer)
    → Broker ← topic ack
```

## Archivos involucrados

| Archivo                               | Rol                                                                                       |
| ------------------------------------- | ----------------------------------------------------------------------------------------- |
| `lib/GestorCmd/src/CmdMem.cpp`        | Dispatcher `processMemJson()` + handlers de lectura/escritura                             |
| `lib/GestorCmd/src/CmdJson.cpp`       | Detecta `tipo == "mem"` y delega a `processMemJson()`                                     |
| `lib/GestorCmd/src/GestorCmd.h`       | Declaraciones de los 8 métodos privados de CmdMem                                         |
| `lib/DataManager/src/DataManager.cpp` | Setters de config: `setTimerAlarma`, `setTamper`, `setClaveDispositivo`, `setPerifericos` |
| `lib/DataManager/src/Controles.cpp`   | RF: `guardarDspRFFull`, `borrar1DspRFAV`, `espacioVacioDspRF`                             |
| `lib/DataManager/src/WiFi.cpp`        | WiFi: `guardarRedWiFi`, `borrar1RedWiFi`, `leerTodasLasRedesGuardadas`                    |
| `src/wifi_mqtt_esp.cpp`               | `mqtt_loop()` publica el ACK cuando `rspJson == true`                                     |

## Targets disponibles

| Target   | Fuente en RAM                                                                   | Persistencia NVS       |
| -------- | ------------------------------------------------------------------------------- | ---------------------- |
| `config` | `Data.numeroSerie`, `pinDispositivo`, `perifericos[4]`, `timerAlarma`, `tamper` | `NAME_SPACE_CNF`       |
| `wifi`   | `Data.redesGuardadas` (recargado tras escritura)                                | `NAME_SPACE_WIFI`      |
| `rf`     | `Data.controlValues`                                                            | `NAME_SPACE_CTRL_DATA` |

## Topics MQTT

| Dirección             | Topic                                      |
| --------------------- | ------------------------------------------ |
| Enviar al dispositivo | `AV/<ENTORNO>/NODO/<num_serie>/CMD/`       |
| Recibir respuesta     | `AV/<ENTORNO>/NODO/<num_serie>/ACK/`       |

`<ENTORNO>` = `DEV` o `PROD` según `ConfigSistema.h`.

## Formato del ACK

```json
{ "dsp": "<num_serie>", "tipo": "ack-mem", "target": "<target>", "status": <0|1>, "datos": {...} }
```

- `status 0` = operación exitosa
- `status != 0` = error (campo faltante, target/op desconocido, fallo de NVS)
- `datos` presente solo en operaciones de lectura

## Lectura RF — paginación

Para soportar hasta 100 controles sin exceder el buffer MQTT de 2048 bytes, la lectura RF devuelve **20 controles por página**.

- Campo `page` en el request (0-based, default 0)
- Respuesta incluye `page`, `pages`, `total`
- Página 0 incluye además el array `modelos` con los tipos disponibles

## RF write — cómo funciona

### op: add
1. El payload envía `id_ct` (letra A–Z asignada al modelo en NVS)
2. `writeMemRF()` busca el `nombre` del modelo en `Data.CtrlModelos` por `id_ct`
3. Para cada señal: `Data.espacioVacioDspRF()` asigna el número de control libre
4. `Data.guardarDspRFFull(nombre, num, 1, signal)` persiste en NVS y actualiza `controlValues` en RAM
5. El `status` al registrar siempre es `1` (habilitado)

Los `id_ct` disponibles se obtienen leyendo RF (campo `modelos` en página 0).

### op: update
1. El payload envía un array de objetos con `num` (requerido), `sig` y/o `status` (opcionales)
2. `writeMemRF()` localiza el control en `Data.controlValues` por `num`
3. Si `sig` está presente, reemplaza la señal; si `status` está presente, actualiza el flag
4. Persiste con `Data.actualizarDspRF()` — modifica el tuple en RAM y sobreescribe en NVS sin `addKey`
5. `status: 1` en el ACK si algún `num` no existe o falla la escritura en NVS

## WiFi write — sincronización RAM

Tras `guardarRedWiFi` o `borrar1RedWiFi`, se recarga `Data.redesGuardadas` llamando a `leerTodasLasRedesGuardadas()` para mantener la coherencia entre NVS y RAM.
