# skill-doxygen — Estándar de Documentación Doxygen

Proyecto: Gestor E/S MF v1.0 (ESP32 / PlatformIO / Arduino + FreeRTOS)

## Regla General

Toda función o método **público** debe tener bloque Doxygen completo.
Toda función o método **privado** debe tener al menos `@brief`.
Structs, enums y clases deben documentarse siempre.

---

## Encabezado de Archivo

### Archivo `.h`

```cpp
/**
 * @file ConfigManager.h
 * @brief Capa de acceso a NVS para toda la configuración del sistema.
 *
 * Proporciona getters y setters tipados por subsistema (wifi, mqtt, input,
 * audio, ap). Mantiene una copia en RAM para lecturas rápidas y persiste
 * cambios en NVS. Protegido por mutex para acceso seguro desde ambos cores.
 *
 * @author Gestor E/S MF
 * @version 1.0.0
 * @date 2025
 */
```

### Archivo `.cpp`

```cpp
/**
 * @file ConfigManager.cpp
 * @brief Implementación de ConfigManager.
 */
```

---

## Documentación de Clase

```cpp
/**
 * @class ConfigManager
 * @brief Módulo pasivo de persistencia. Centraliza toda la configuración del sistema.
 *
 * No tiene tarea propia. Es llamado desde MQTTManager, APManager y main (setup).
 * El acceso concurrente desde TaskNetwork (Core 0) y TaskIO (Core 1) está
 * protegido por mutex asignado externamente vía setMutex().
 */
class ConfigManager {
```

---

## Documentación de Métodos

### Método público — forma completa

```cpp
/**
 * @brief Inicializa ConfigManager: abre namespaces NVS y carga configuración.
 *
 * En primer arranque escribe todos los valores por defecto y persiste el flag
 * `initialized`. En arranques posteriores carga los valores existentes desde NVS
 * a los structs internos en RAM. Imprime log por Serial de cada namespace cargado.
 *
 * @note Debe llamarse desde setup() antes de crear las tareas FreeRTOS.
 *       En ese momento no hay concurrencia, por lo que el mutex aún no es necesario.
 * @warning No llamar desde una tarea FreeRTOS sin haber asignado el mutex primero.
 */
void init();
```

```cpp
/**
 * @brief Retorna la configuración de una entrada digital desde RAM.
 *
 * @param inputNumber Número de entrada: 1 (E1), 2 (E2), 3 (E3).
 * @return InputConfig Struct con todos los parámetros de la entrada solicitada.
 *         Si inputNumber es inválido, retorna struct con valores por defecto.
 */
InputConfig getInputConfig(uint8_t inputNumber) const;
```

```cpp
/**
 * @brief Actualiza la configuración de una entrada digital en NVS y en RAM.
 *
 * Valida los rangos antes de escribir:
 * - track: 1–255
 * - rndStart < rndEnd
 * - rndEnd <= 255
 *
 * Si la validación falla, no modifica NVS ni RAM y retorna false.
 *
 * @param inputNumber Número de entrada: 1 (E1), 2 (E2), 3 (E3).
 * @param config      Struct con la nueva configuración a aplicar.
 * @return true  Si la validación y escritura en NVS fueron exitosas.
 * @return false Si inputNumber es inválido o la configuración no pasa validación.
 */
bool setInputConfig(uint8_t inputNumber, const InputConfig& config);
```

### Método privado — solo `@brief`

```cpp
/**
 * @brief Carga los valores por defecto en NVS y en los structs internos.
 */
void _loadDefaults();
```

---

## Documentación de Structs

```cpp
/**
 * @struct InputConfig
 * @brief Parámetros de configuración de una entrada digital.
 */
struct InputConfig {
    bool    enabled;    ///< true si la entrada está habilitada.
    uint8_t edge;       ///< Flanco activo: 0 = descendente, 1 = ascendente.
    uint8_t type;       ///< Tipo: 0 = botón, 1 = switch.
    uint8_t audioMode;  ///< Modo de audio: 0 = fijo, 1 = aleatorio.
    uint8_t track;      ///< Número de track en modo fijo (1–255).
    uint8_t rndStart;   ///< Inicio del rango aleatorio (1–255).
    uint8_t rndEnd;     ///< Fin del rango aleatorio (1–255). Debe ser > rndStart.
};
```

```cpp
/**
 * @struct MQTTMessage
 * @brief Mensaje encolado en queueMQTTPublish para publicación MQTT.
 *
 * Producido por TaskIO (InputManager), consumido por TaskNetwork (MQTTManager).
 */
struct MQTTMessage {
    uint8_t type;        ///< Tipo de mensaje: EVENTO, ACK, ALERTA (ver MessageType).
    char    topic[64];   ///< Subtópico destino (ej: "evt/", "ack/").
    char    payload[256];///< Payload JSON serializado.
};
```

---

## Documentación de Enums

```cpp
/**
 * @enum AudioMode
 * @brief Modo de reproducción de audio asignado a una entrada digital.
 */
enum class AudioMode : uint8_t {
    FIXED  = 0, ///< Reproduce siempre el mismo track (campo track de InputConfig).
    RANDOM = 1  ///< Reproduce un track aleatorio en el rango [rndStart, rndEnd].
};
```

---

## Documentación de Funciones de Tarea FreeRTOS

```cpp
/**
 * @brief Función de tarea FreeRTOS para la gestión de red.
 *
 * Se ejecuta en Core 0, prioridad 2. Gestiona WiFiManager y MQTTManager.
 * Actualiza networkLastAlive en cada ciclo para el watchdog de TaskMonitor.
 * Se suspende 50 ms al final de cada ciclo con vTaskDelay().
 *
 * @param pvParameters Puntero a contexto de tarea (no usado, puede ser NULL).
 */
static void taskNetworkFunc(void* pvParameters);
```

---

## Documentación de Callbacks

```cpp
/**
 * @brief Callback de PubSubClient para mensajes MQTT entrantes.
 *
 * Llamado por PubSubClient dentro de client.loop(). No procesa el mensaje
 * directamente: copia el payload a un buffer local y lo encola en
 * queueMQTTPublish o lo procesa de forma diferida para evitar bloqueos
 * dentro del callback.
 *
 * @param topic   Tópico del mensaje recibido.
 * @param payload Puntero al payload del mensaje (no null-terminated).
 * @param length  Longitud del payload en bytes.
 *
 * @warning No llamar funciones bloqueantes ni delay() desde este callback.
 */
void onMQTTMessage(char* topic, byte* payload, unsigned int length);
```

---

## Resumen de Tags Usados

| Tag | Uso |
|---|---|
| `@brief` | Descripción corta (obligatorio en todos) |
| `@param` | Cada parámetro de entrada con su descripción |
| `@return` | Valor de retorno (omitir en void) |
| `@note` | Información adicional importante |
| `@warning` | Advertencia de uso incorrecto o efecto lateral grave |
| `@file` | Encabezado de archivo (nombre + descripción) |
| `@author` | Autoría del archivo |
| `@version` | Versión del módulo |
| `@date` | Fecha o año de creación |
| `@class` | Descripción de clase (en el `.h`) |
| `@struct` | Descripción de struct |
| `@enum` | Descripción de enum |
| `///` | Documentación inline de campos de struct/enum |
