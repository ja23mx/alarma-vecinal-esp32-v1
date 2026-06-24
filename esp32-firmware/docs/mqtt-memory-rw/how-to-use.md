# MQTT Memory R/W — How to Use

Topics:
- **Enviar al dispositivo:** `AV/<ENTORNO>/NODO/<num_serie>/CMD/`
- **Recibir respuesta:** `AV/<ENTORNO>/NODO/<num_serie>/ACK/`

Reemplazar `<ENTORNO>` con `DEV` o `PROD` según el entorno activo en `ConfigSistema.h`.
Reemplazar `<num_serie>` con el número de serie del dispositivo (ej. `7058B0`).

---

## Leer configuración

**Request:**
```json
{ "dsp": "A1B2C3", "tipo": "mem", "accion": "read", "target": "config" }
```

**Response:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "ack-mem",
  "target": "config",
  "status": 0,
  "datos": {
    "num_serie": "A1B2C3",
    "pin": 1234,
    "timer": 30,
    "tamper": 0,
    "per": { "rf": 1, "wifi": 1, "voz": 1, "per": 0 }
  }
}
```

---

## Escribir configuración (actualización parcial)

Solo se actualizan los campos presentes. Todos son opcionales.

**Request:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "mem",
  "accion": "write",
  "target": "config",
  "datos": {
    "timer": 60,
    "tamper": 1,
    "pin": 4321,
    "rf": 1,
    "wifi": 1,
    "voz": 1,
    "per": 0
  }
}
```

| Campo | Descripción | Rango |
|-------|-------------|-------|
| `timer` | Timer de alarma en segundos | 0–600 |
| `tamper` | Habilitación tamper | 0 o 1 |
| `pin` | Clave del dispositivo | 1–9999 |
| `rf` | Periférico RF habilitado | 0–2 |
| `wifi` | Periférico WiFi habilitado | 0–2 |
| `voz` | Periférico Voz habilitado | 0–2 |
| `per` | Periférico Perifoneo habilitado | 0–2 |

**Response:**
```json
{ "dsp": "A1B2C3", "tipo": "ack-mem", "target": "config", "status": 0 }
```

---

## Leer redes WiFi

**Request:**
```json
{ "dsp": "A1B2C3", "tipo": "mem", "accion": "read", "target": "wifi" }
```

**Response:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "ack-mem",
  "target": "wifi",
  "status": 0,
  "datos": [
    { "ssid": "MiRed", "pass": "clave123" },
    { "ssid": "RedasegundaRed", "pass": "otralave" }
  ]
}
```

---

## Agregar red WiFi

**Request:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "mem",
  "accion": "write",
  "target": "wifi",
  "op": "add",
  "datos": { "ssid": "NuevaRed", "pass": "contraseña" }
}
```

**Response:**
```json
{ "dsp": "A1B2C3", "tipo": "ack-mem", "target": "wifi", "status": 0 }
```

---

## Borrar red WiFi

**Request:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "mem",
  "accion": "write",
  "target": "wifi",
  "op": "del",
  "datos": { "ssid": "NuevaRed" }
}
```

---

## Leer controles RF (página 0)

**Request:**
```json
{ "dsp": "A1B2C3", "tipo": "mem", "accion": "read", "target": "rf" }
```

`page` es opcional, default 0.

**Response:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "ack-mem",
  "target": "rf",
  "status": 0,
  "page": 0,
  "pages": 2,
  "total": 25,
  "modelos": [
    { "id_ct": "A", "nombre": "CTRL BLANCO 4T AV M1" },
    { "id_ct": "B", "nombre": "CTRL BLANCO 4T AV M2" },
    { "id_ct": "C", "nombre": "CTRL BLANCO 4T GUARDIAN" },
    { "id_ct": "D", "nombre": "CTRL BLANCO 4T INTEGRADOR" }
  ],
  "datos": [
    { "num": 1, "id_ct": "A", "status": 1, "sig": 13255936 },
    { "num": 2, "id_ct": "A", "status": 1, "sig": 13500001 }
  ]
}
```

- `modelos`: aparece solo en página 0; lista los modelos disponibles con su `id_ct`
- `datos`: máximo 20 controles por página
- `num`: número de control (usado para borrar)
- `sig`: señal RF base almacenada

**Solicitar página siguiente:**
```json
{ "dsp": "A1B2C3", "tipo": "mem", "accion": "read", "target": "rf", "page": 1 }
```

---

## Registrar controles RF

Envía uno o más grupos por modelo (`id_ct`). Los números de control se asignan automáticamente.
Registrar múltiples señales del mismo modelo en un solo mensaje:

**Request:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "mem",
  "accion": "write",
  "target": "rf",
  "op": "add",
  "datos": [
    { "id_ct": "A", "sig": [13255936, 13500001, 14000000, 14100000] },
    { "id_ct": "B", "sig": [9000000] }
  ]
}
```

- `id_ct`: letra del modelo (obtener desde `read rf` → campo `modelos`)
- `sig`: array de señales RF base (hasta 100 señales en total por mensaje)
- El `status` se asigna en `1` (habilitado) automáticamente

**Response:**
```json
{ "dsp": "A1B2C3", "tipo": "ack-mem", "target": "rf", "status": 0 }
```

---

## Borrar controles RF

**Request:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "mem",
  "accion": "write",
  "target": "rf",
  "op": "del",
  "datos": [1, 2, 5, 10]
}
```

- `datos`: array de números de control (`num` del read rf)

**Response:**
```json
{ "dsp": "A1B2C3", "tipo": "ack-mem", "target": "rf", "status": 0 }
```

---

## Actualizar controles RF

Modifica la señal y/o el status de controles existentes identificados por su `num`.

**Request:**
```json
{
  "dsp": "A1B2C3",
  "tipo": "mem",
  "accion": "write",
  "target": "rf",
  "op": "update",
  "datos": [
    { "num": 1, "sig": 99887766 },
    { "num": 5, "sig": 11223344, "status": 0 }
  ]
}
```

- `num`: requerido — número de control a modificar (obtenido del `read rf`)
- `sig`: opcional — nueva señal RF base
- `status`: opcional — habilita (`1`) o deshabilita (`0`) el control
- `id_ct`: opcional — si el `num` no existe y se provee `id_ct` + `sig`, inserta el control en ese slot (upsert)
- Si el `num` no existe y no viene `id_ct`, responde `status: 1`
- `id_ct` no se modifica en controles existentes; para cambiar de modelo usar `del` + `add`
- Soporta array para actualizar varios controles en un solo mensaje

**Response:**
```json
{ "dsp": "A1B2C3", "tipo": "ack-mem", "target": "rf", "status": 0 }
```

`status: 1` si algún `num` no existe o falla la escritura en NVS.

---

## Códigos de status

| status | Significado |
|--------|-------------|
| `0` | Operación exitosa |
| `1` | Error en operación (NVS, modelo no encontrado, op inválida) |
| `2` | Formato inválido (campos `accion` o `target` ausentes) |
| `3` | Parámetros inválidos (target u op desconocidos) |
