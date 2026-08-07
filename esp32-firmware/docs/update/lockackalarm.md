# Candado de Activación con Ack de Guardián

Mecanismo que impide reactivaciones repetidas/espontáneas de la alarma después de un
incidente, hasta que un control Guardián reconozca (ack) la activación previa.

> **Estado:** implementado y probado en hardware (v0.1.0-beta.6). No probado en campo real todavía.

---

## Problemática que resuelve

Se reportaron (dos veces) incidentes de **"activación espontánea e indefinida"**: la alarma
se enciende y permanece activa por un periodo largo (~1 hora) sin que el usuario logre
apagarla, hasta que actúa el timer de auto-apagado.

Investigación en hardware (logs `[RF-DIAG]`, monitor serie COM7) confirmó que el dedup
gate en `procesos_cmd.cpp` descarta **silenciosamente** cualquier botón de activación
repetido mientras la alarma ya está armada — sin dar ningún feedback al usuario. Si un
vecino confundido sostiene o repite un botón que no es el de desactivación (D) pensando
que está apagando la alarma, el sistema simplemente ignora cada intento.

El usuario final no admite haber presionado el botón equivocado — culpa al dispositivo.
Se decidió, de forma consciente, priorizar una defensa dura contra este patrón de soporte
por encima de la disponibilidad total del sistema (ver [Decisiones de diseño](#decisiones-de-diseño-explícitas)).

**Se investigó y descartó como causa raíz:** un hardcode en `rf_esp.cpp` que asumía
`btnIndice != 3` como criterio de activación en vez de usar `modeloCtrlAVRx.boton_desactivacion`
dinámicamente. Se corrigió por consistencia con el resto del código, pero no explicaba el
incidente de campo (todos los controles instalados son el mismo modelo, con
`boton_desactivacion=4` en ambas plantillas existentes).

---

## Flujo conceptual

```
Vecino activa (A/B/C)                    Alarma suena
        |                                     |
        v                                     v
g_estadoAlarma: 0 -> 1          g_alarmaPendienteAck: false -> true (persistido en NVS)
                                             |
                            [CANDADO ARMADO desde aquí]
                                             |
        Vecino desactiva (D normal, o auto-apagado por timer)
                                             |
                                             v
                          g_estadoAlarma: 1 -> 0   (g_alarmaPendienteAck sigue en true)
                                             |
              Cualquier nuevo intento de activación (A/B/C, mismo u otro vecino)
                                             |
                                             v
                    BLOQUEADO — filtrado=true, bloqueadoPorCandado=true
                    (sin importar el valor de g_estadoAlarma)
                                             |
                           ... se repite indefinidamente ...
                                             |
                        Guardián presiona D de su control físico
                                             |
                                             v
                   ack_alarma_pendiente() -> g_alarmaPendienteAck: true -> false
                                (persistido en NVS, candado liberado)
                                             |
                    Siguiente activación (A/B/C) vuelve a funcionar normalmente
                              y re-arma el candado desde cero
```

**Regla clave:** el candado bloquea únicamente **activaciones** (`estadoAlarma==1`).
La **desactivación** (D normal o auto-apagado) nunca se bloquea — el candado protege
contra volver a encender, no contra apagar.

---

## Componentes involucrados

| Archivo | Responsabilidad |
|---|---|
| `src/procesos_cmd.cpp` | Estado en RAM (`g_alarmaPendienteAck`), lógica del candado (`rev_async_evento_ctrl_av`, `sync_evento_mqtt`), liberación (`ack_alarma_pendiente`), siembra al arrancar (`crearTareaProcesosCmd`) |
| `include/procesos_cmd.h` | Declaraciones públicas: `get_alarma_pendiente_ack()`, `ack_alarma_pendiente()` |
| `lib/DataManager/src/DataManager.h` / `.cpp` | Persistencia NVS: `Data.alarmaPendienteAck`, `setAlarmaPendienteAck()`, `getAlarmaPendienteAck()` (get-or-create) |
| `lib/DataManager/src/NvsData.cpp` | `cargarConfig()` — **no** incluye `alarmaAck` en su tabla genérica a propósito (ver [Persistencia NVS](#persistencia-nvs)) |
| `lib/GestorCmd/src/CmdCtrl.cpp` | `CtrlAv()` — rama compartida de desactivación AV/Guardián (`desactivacionAud1Sal1()`) |
| `src/rf_esp.cpp` | `rf_esp_nv_dato()` — clasificación activación/desactivación (case 1, AV) y despacho del botón D del Guardián (case 2) |

---

## Lógica del candado (`procesos_cmd.cpp`)

Duplicada en dos puntos de entrada — RF (`rev_async_evento_ctrl_av`) y MQTT
(`sync_evento_mqtt`) — con la misma estructura:

```cpp
if (estadoAlarma == 1)              // intento de activación
{
    if (g_alarmaPendienteAck)       // candado armado
        filtrado = true;            // bloqueado, sin importar g_estadoAlarma
    else if (g_estadoAlarma == 1)
        filtrado = true;            // dedup normal (ya estaba activa)
    else
    {
        g_estadoAlarma = 1;
        g_alarmaPendienteAck = true;  // arma el candado
        nuevaActivacion = true;       // dispara persistencia a NVS fuera del mutex
    }
}
else                                 // intento de desactivación — nunca bloqueado
{
    if (g_estadoAlarma == 0)
        filtrado = true;
    else
        g_estadoAlarma = 0;
}
```

### Ack del Guardián — desacoplado del filtro de estado físico

**Bug encontrado y corregido durante las pruebas:** originalmente `ack_alarma_pendiente()`
vivía dentro de `GestorCMD::CtrlAv()` (`CmdCtrl.cpp`), en la rama `btnDesactivacion`. Pero
esa rama solo se alcanza si el evento **no fue filtrado** — y si `g_estadoAlarma` ya estaba
en `0` (por ejemplo, porque el candado bloqueó toda reactivación desde el arranque), el
evento del Guardián se filtraba como "ya está desactivada, nada que hacer" **antes** de
llegar a `CtrlAv()`, y el ack nunca se ejecutaba. El candado quedaba irreleasable.

**Fix:** en `rev_async_evento_ctrl_av`, el ack ahora se dispara siempre que `tipoCtrl ==
AL_CTRL_TP_GUARDIAN`, **antes** del `if (filtrado) return;` — independiente de si hubo o
no cambio físico de estado:

```cpp
if (tipoCtrl == AL_CTRL_TP_GUARDIAN)
    ack_alarma_pendiente();   // siempre, sin importar filtrado

if (filtrado)
    return;
```

La llamada equivalente en `CmdCtrl.cpp::CtrlAv()` se quitó (quedaba redundante y solo se
alcanzaba en el subconjunto de casos no filtrados).

---

## Despacho del botón D del Guardián (`rf_esp.cpp`)

**Segundo bug encontrado:** el caso 2 (`configuracion == 2`, control Guardián) tenía la
llamada a `async_evento_ctrl_av(AL_CTRL_TP_GUARDIAN)` **comentada** — código muerto,
nunca completado (ni siquiera tenía el segundo parámetro `estadoAlarma`). Esto significaba
que ninguna señal RF del Guardián llegaba jamás al flujo de `procesos_cmd.cpp`, incluyendo D.

**Fix**, acotado únicamente al botón D (los botones 0/1/2 del Guardián — pistas 70/71/72 —
quedan sin conectar a propósito, `procesarControlGuardian()` permanece intacto/inalcanzable):

```cpp
case 2: // GUARDIAN
    if ((estadoCompRFAv.btnIndice + 1) == modeloCtrlAVRx.boton_desactivacion)
        async_evento_ctrl_av(AL_CTRL_TP_GUARDIAN, 0);
    return true;
```

---

## Persistencia NVS

El candado sobrevive a reinicios — campo `Data.alarmaPendienteAck` (`uint8_t`, 0/1),
clave `"alarmaAck"` en `NAME_SPACE_CNF`, mismo patrón que `tamper`/`timerAlarma`.

**Rollout seguro a flota ya instalada:** un campo nuevo agregado a NVS no existe en
dispositivos ya desplegados hasta que algo lo escriba. Dos mecanismos cubren esto:

1. `DataManager::rstData()` llama `setAlarmaPendienteAck(0)` — cubre dispositivos nuevos
   / reseteados de fábrica (mismo patrón que el resto de los campos).
2. `DataManager::getAlarmaPendienteAck()` — patrón **get-or-create**: si la clave no
   existe en NVS (dispositivo ya instalado, actualizado vía OTA, que nunca pasó por
   `rstData()`), la crea con `0` en ese mismo boot. `crearTareaProcesosCmd()` la llama
   para sembrar `g_alarmaPendienteAck` en RAM.

Por esto se **excluyó** `alarmaAck` de la tabla genérica `ConfigEntry` en
`NvsData.cpp::cargarConfig()` — evita el log de "clave no encontrada" en cada arranque
de un dispositivo que nunca ha tenido un incidente, y evita doble manejo del mismo campo.

Verificado en hardware: reinicio con candado pendiente (antes de ack) → sigue en `true`
tras el arranque. Reinicio simulando dispositivo de campo sin la clave → se crea en `0`
sin errores.

---

## Decisiones de diseño explícitas

| Decisión | Alternativa descartada | Razón |
|---|---|---|
| Candado duro (bloquea reactivación hasta ack manual) | Cooldown corto (10-30s) auto-expirable | El objetivo explícito es evitar que se repita el reporte de "activación espontánea e indefinida" — un cooldown acotado no da la garantía de que el desarrollador necesita frente a un usuario final que no admite responsabilidad. Se documenta el riesgo de disponibilidad como trade-off consciente. |
| Ack solo vía RF, control Guardián, botón D | Ack también vía MQTT/app | Explícitamente fuera de alcance esta ronda — se puede agregar después sin tocar la lógica del candado (ver Futuro). |
| El candado nunca bloquea desactivación | Bloquear todo mientras el candado esté armado | Bloquear la desactivación dejaría la alarma sonando sin forma de silenciarla — inaceptable. |
| No tocar timers de auto-apagado ni el dedup gate existente | Modificar la lógica de filtrado original | Riesgo de reintroducir bugs ya vistos (`74191ea`). El candado es una capa nueva encima, no una reescritura. |
| Solo se persiste el booleano (`alarmaPendienteAck`) | Guardar timestamp/control/botón de origen | Simplicidad — el registro MQTT (`msgPayloadCtrlMqtt`) ya envía esa información al backend en cada evento; no se duplicó en NVS. |

---

## Pruebas realizadas en hardware (2026-08-03)

- Activación (A/B/C) arma el candado — confirmado con `[CANDADO-DIAG]`.
- Reactivación con candado armado → `bloqueadoPorCandado:true`, sin cambio de estado, para A, B y C.
- Desactivación con D normal (AV) → silencia la salida, **no** limpia el candado.
- Botón D del Guardián → `ack_alarma_pendiente. habiaPendiente:true` → candado liberado.
- Reactivación posterior al ack → funciona normalmente y re-arma el candado.
- Persistencia: reinicio con candado pendiente → sigue en `true` tras arrancar.
- Auto-apagado por timer con candado ya armado → comportamiento consistente (no lo libera; solo D del Guardián lo hace).
- Simulación de dispositivo de campo sin la clave NVS → `getAlarmaPendienteAck()` la crea en `0` sin error.

---

## Seguimiento — beta.9 (2026-08-07)

Se detectó que `async_mqtt_msg_ctrl_alarma()` (`wifi_mqtt_esp.cpp`) notificaba la activación
(`"estado-alarma":1`) al topic `.../ACK/` **antes** de que el candado pudiera bloquearla — la
notificación se dispara de forma síncrona en `rf_esp.cpp::rf_esp_nv_dato()` al recibir la señal
RF, mientras que el filtro real del candado (`g_alarmaPendienteAck`) solo corre después, de forma
asíncrona, en `rev_async_evento_ctrl_av()` (tarea `tareaProcesosCmd`). El resultado: aunque el
dispositivo se bloqueaba correctamente en local, igual anunciaba al backend una activación que
nunca ocurrió — y como ese topic ACK es el que el backend usa para propagar la alerta a otros
dispositivos de la misma red vecinal, terminaba activando equipos vecinos por una alarma que
localmente estaba bloqueada.

**Fix:** `async_mqtt_msg_ctrl_alarma()` ahora consulta `get_alarma_pendiente_ack()` antes de
publicar una notificación de activación — si el candado está armado, omite el envío (con log)
en vez de anunciar una activación falsa. La rama de desactivación no se modificó — nunca fue
parte del problema.

Confirmado en hardware: build v0.1.0-beta.9, OTA exitoso sobre WiFi en EF4934.

## Posibles mejoras futuras (no implementadas)

- **Cooldown configurable** en vez de (o adicional a) candado duro — ventana corta y
  acotada tras una desactivación, con posible bypass del Guardián. Quedó descartado para
  esta versión por priorizar la garantía dura sobre la disponibilidad.
- **Ack vía MQTT/app** — permitiría liberar el candado remotamente sin depender de que el
  Guardián esté físicamente presente con su control RF.
- **Feedback audible/visual cuando se bloquea un intento** — hoy el bloqueo queda solo en
  el log (`[CANDADO-DIAG]`); el usuario que presiona un botón bloqueado no recibe ninguna
  señal de que su intento fue rechazado.
- **Metadata del incidente** (timestamp, control, botón de origen) persistida junto al
  booleano, para trazabilidad completa incluso si el backend/MQTT no estuvo disponible en
  el momento del evento.
- Habilitar los botones 0/1/2 del control Guardián (`procesarControlGuardian`) si en algún
  momento se necesita esa funcionalidad — hoy quedan intactos pero inalcanzables desde RF.

---

## Logs de diagnóstico

Todos gateados por `DEBUG_MODE` (`LogSistema.h`), no requieren remoción manual para build
de producción:

- `[RF-DIAG]` (`rf_esp.cpp`) — clasificación de botón, estado del candado, predicción de filtrado, en cada señal RF del control AV.
- `[CANDADO-DIAG]` (`procesos_cmd.cpp`, `rf_esp.cpp`) — resultado real del gate (`filtrado`, `bloqueadoPorCandado`), detección del botón D del Guardián, resultado de `ack_alarma_pendiente()`.
