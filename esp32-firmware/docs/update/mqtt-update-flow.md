# MQTT Update Flow

Protocolo de comunicación MQTT para operaciones de actualización de firmware (OTA) entre servidor y dispositivo AV.

> **Nota:** Esta sesión cubre únicamente el flujo de payloads y respuestas MQTT. La ejecución real del proceso OTA no está implementada aún.

---

## Estructura general de payloads

| Campo   | Dirección         | Descripción                                 |
|---------|-------------------|---------------------------------------------|
| `dsp`   | server → device   | MAC / número de serie del dispositivo destino |
| `tipo`  | server → device   | Tipo de mensaje. Para este flujo: `update`  |
| `accion`| server → device   | Subtipo de acción: `init` o `status`        |
| `url`   | server → device   | URL del binario de firmware                 |

---

## Acciones disponibles

### 1. `init` — Iniciar actualización

El servidor indica al dispositivo que debe iniciar una actualización de firmware.

**Server → Device**
```json
{
  "dsp": "<mac_dispositivo>",
  "tipo": "update",
  "accion": "init",
  "url": "http://server.com/firmware.bin"
}
```

**Device → Server**
```json
{
  "dsp": "<mac_dispositivo>",
  "type": "ack-update",
  "modelo": "AV-SLT-V1.0",
  "firmware": "AV-RF-WF",
  "version": "1.0",
  "build": "BETA",
  "fecha": "Jun  9 2026 12:00:00"
}
```

> El ACK confirma recepción del comando e informa la versión **actual** (antes de actualizar).

---

### 2. `status` — Consultar versión

El servidor consulta la versión de firmware instalada en el dispositivo.

**Server → Device**
```json
{
  "dsp": "<mac_dispositivo>",
  "tipo": "update",
  "accion": "status"
}
```

**Device → Server**
```json
{
  "dsp": "<mac_dispositivo>",
  "type": "ack-update",
  "modelo": "AV-SLT-V1.0",
  "firmware": "AV-RF-WF",
  "version": "1.0",
  "build": "BETA",
  "fecha": "Jun  9 2026 12:00:00"
}
```

> La respuesta es idéntica a `init`. El servidor diferencia el contexto por la acción que envió.

---

## Flujo de comunicación

```
Servidor                                Dispositivo
   |                                        |
   |-- tipo:update, accion:init ----------> |
   |                                        | processUpdateJson()
   |                                        | generateAckUpdate()
   | <------------- type:ack-update --------|  (versión actual, antes de actualizar)
   |                                        |
   |                                        | [reset — por OTA exitoso o error]
   |                                        |
   |   Topic: AV/{PROD|DEV}/CONTROLADOR/HB/ |
   | <------------- tipo:hb ----------------|  (señal de que el dispositivo volvió)
   |                                        |
   |-- tipo:update, accion:status --------> |
   |                                        | processUpdateJson()
   |                                        | generateAckUpdate()
   | <------------- type:ack-update --------|  (versión instalada tras el reset)
```

> El servidor usa el mensaje `hb` como señal para disparar `accion:status` y confirmar
> si la actualización fue exitosa comparando la versión antes y después del reset.

---

## Manejo de errores

| Condición                  | Respuesta                                      |
|----------------------------|------------------------------------------------|
| Campo `accion` ausente     | `generateAckResponse("ack-update", ERROR_FORMATO_INVALIDO)` |
| `accion` desconocida       | `generateAckResponse("ack-update", ERROR_PARAMETROS_INVALIDOS)` |

---

## Campos del ACK (`ack-update`)

| Campo      | Fuente                  | Descripción                        |
|------------|-------------------------|------------------------------------|
| `dsp`      | `Data.numeroSerie`      | Identificador del dispositivo      |
| `type`     | literal `"ack-update"`  | Tipo de respuesta                  |
| `modelo`   | `BOARD_MODELO`          | Modelo de la placa (`CnfTarjeta.h`)|
| `firmware` | `SISTEMA_FIRMWARE`      | Nombre del firmware                |
| `version`  | `SISTEMA_VERSION`       | Versión del firmware               |
| `build`    | `SISTEMA_BUILD`         | Etiqueta de build (ej. `BETA`)     |
| `fecha`    | `SISTEMA_DATE`          | Fecha y hora de compilación        |

---

## Implementación

- **Entrada:** `GestorCMD::datosJSON()` → rama `tipo == "update"`
- **Procesador:** `GestorCMD::processUpdateJson()` — `lib/GestorCmd/src/CmdJson.cpp`
- **Generador de respuesta:** `GestorCMD::generateAckUpdate()` — `lib/GestorCmd/src/GestorCmd.cpp`
- **Constantes de versión:** `include/CnfTarjeta.h`
