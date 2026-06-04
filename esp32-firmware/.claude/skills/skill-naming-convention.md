# skill-naming-convention — Convenciones de Nomenclatura

Proyecto: Gestor E/S MF v1.0 (ESP32 / PlatformIO / Arduino + FreeRTOS)

## Tabla de Referencia Rápida

| Tipo de elemento | Convención | Ejemplo |
|---|---|---|
| Variable local | camelCase | `retryCount`, `sensorValue`, `lastMillis` |
| Variable miembro de clase | camelCase con prefijo `_` | `_volume`, `_isConnected`, `_inputConfig` |
| Función / método | camelCase, verbo descriptivo | `getVolume()`, `handleInput()`, `publishHeartbeat()` |
| Clase | PascalCase | `ConfigManager`, `InputManager`, `StatusLED` |
| Constante / define | UPPER_SNAKE_CASE | `MAX_RETRIES`, `WATCHDOG_TIMEOUT`, `DEFAULT_VOLUME` |
| Enum (tipo) | PascalCase con `enum class` | `AudioMode`, `InputType`, `SystemState` |
| Enum (valor) | UPPER_SNAKE_CASE | `AudioMode::FIXED`, `SystemState::CONNECTED` |
| Namespace NVS | snake_case | `wifi`, `mqtt`, `input`, `audio`, `ap` |
| Clave NVS | snake_case (máx. 15 chars) | `e1_enabled`, `hb_interval`, `ssid` |
| Archivo de módulo | PascalCase | `ConfigManager.h`, `AudioManager.cpp` |
| Archivo de config | snake_case | `config.h`, `types.h` |
| Tarea FreeRTOS (tipo/handle) | PascalCase con prefijo Task | `TaskNetwork`, `TaskIO`, `TaskMonitor` |
| Función de tarea FreeRTOS | camelCase con sufijo Func | `taskNetworkFunc`, `taskIOFunc` |
| Cola FreeRTOS | camelCase con prefijo queue | `queueMQTTPublish`, `queueAudioCmd`, `queueConfigUpdate` |
| Mutex | camelCase con sufijo Mutex | `configMutex` |
| GPIO (constante) | UPPER_SNAKE_CASE con prefijo GPIO_ | `GPIO_INPUT_E1`, `GPIO_LED_STATUS`, `GPIO_BTN_AP` |
| Tópico MQTT | kebab-case con separador `/` | `gestor-es-v1/nodo/{id}/cmd/` |
| Handle de tarea | camelCase con sufijo Handle | `taskNetworkHandle`, `taskIOHandle` |

---

## Detalle por Categoría

### Variables Locales — camelCase

```cpp
uint32_t lastMillis = 0;
uint8_t  retryCount = 0;
bool     isConnected = false;
char     payloadBuffer[256];
```

### Variables Miembro — camelCase con prefijo `_`

```cpp
class AudioManager {
private:
    uint8_t _volume;
    bool    _isAvailable;
    uint32_t _lastCommandTime;
};
```

### Métodos — camelCase, verbos descriptivos

```cpp
// Getters: get + Nombre
uint8_t getVolume() const;
InputConfig getInputConfig(uint8_t inputNumber) const;

// Setters: set + Nombre
bool setVolume(uint8_t volume);
bool setInputConfig(uint8_t inputNumber, const InputConfig& config);

// Acciones: verbo descriptivo
void init();
void handle();
void deinit();
void publishHeartbeat();
bool isPlaying() const;
void reloadConfig(uint8_t inputNumber);
```

### Constantes en `config.h` — UPPER_SNAKE_CASE

```cpp
// GPIOs
constexpr uint8_t GPIO_BTN_AP        = 0;
constexpr uint8_t GPIO_LED_STATUS    = 2;
constexpr uint8_t GPIO_DFPLAYER_BUSY = 4;
constexpr uint8_t GPIO_INPUT_E1      = 12;
constexpr uint8_t GPIO_INPUT_E2      = 14;
constexpr uint8_t GPIO_INPUT_E3      = 27;
constexpr uint8_t GPIO_DFPLAYER_TX   = 16;
constexpr uint8_t GPIO_DFPLAYER_RX   = 17;

// Defaults
constexpr uint8_t  DEFAULT_VOLUME       = 15;
constexpr uint16_t DEFAULT_HB_INTERVAL  = 60;
constexpr uint16_t DEFAULT_DEBOUNCE_MS  = 50;
constexpr uint16_t DEFAULT_COOLDOWN_MS  = 500;

// Watchdog
constexpr uint32_t WATCHDOG_TIMEOUT = 30000;
constexpr uint8_t  MAX_RETRIES      = 3;

// Colas
constexpr uint8_t QUEUE_MQTT_SIZE   = 10;
constexpr uint8_t QUEUE_AUDIO_SIZE  = 5;
constexpr uint8_t QUEUE_CONFIG_SIZE = 5;

// Tareas
constexpr uint16_t TASK_NETWORK_STACK = 8192;
constexpr uint16_t TASK_IO_STACK      = 4096;
constexpr uint16_t TASK_MONITOR_STACK = 2048;
constexpr uint8_t  TASK_NETWORK_PRIO  = 2;
constexpr uint8_t  TASK_IO_PRIO       = 3;
constexpr uint8_t  TASK_MONITOR_PRIO  = 1;

// Versión
constexpr char FW_VERSION[] = "1.0.0";
```

### Enums en `types.h` — `enum class` PascalCase, valores UPPER_SNAKE_CASE

```cpp
enum class AudioMode : uint8_t {
    FIXED  = 0,
    RANDOM = 1
};

enum class InputType : uint8_t {
    BUTTON = 0,
    SWITCH = 1
};

enum class EdgeType : uint8_t {
    FALLING = 0,
    RISING  = 1
};

enum class AudioAction : uint8_t {
    PLAY = 0,
    STOP = 1
};

enum class MessageType : uint8_t {
    EVENTO = 0,
    ACK    = 1,
    ALERTA = 2
};

enum class ConfigTarget : uint8_t {
    INPUT_E1 = 0,
    INPUT_E2 = 1,
    INPUT_E3 = 2,
    AUDIO    = 3,
    ALL      = 4
};

enum class SystemState : uint8_t {
    CONNECTED    = 0,
    WIFI_ONLY    = 1,
    AP_ACTIVE    = 2,
    CONNECTING   = 3,
    OFFLINE      = 4,
    ERROR        = 5,
    TASK_FAILURE = 6
};
```

### Tareas y Colas FreeRTOS

```cpp
// Handles de tareas
TaskHandle_t taskNetworkHandle = nullptr;
TaskHandle_t taskIOHandle      = nullptr;
TaskHandle_t taskMonitorHandle = nullptr;

// Colas
QueueHandle_t queueMQTTPublish  = nullptr;
QueueHandle_t queueAudioCmd     = nullptr;
QueueHandle_t queueConfigUpdate = nullptr;

// Mutex
SemaphoreHandle_t configMutex = nullptr;

// Función de tarea (static dentro de la clase o en main.cpp)
static void taskNetworkFunc(void* pvParameters);
static void taskIOFunc(void* pvParameters);
static void taskMonitorFunc(void* pvParameters);
```

---

## Buenos vs Malos Nombres

| Malo | Bueno | Por qué |
|---|---|---|
| `vol` | `_volume` | Miembro de clase, nombre completo |
| `flag` | `_isConnected` | Descriptivo, prefijo `is` para booleanos |
| `handle_input` | `handleInput()` | camelCase, no snake_case |
| `GPIO2` | `GPIO_LED_STATUS` | La constante expresa función, no número |
| `q1` | `queueMQTTPublish` | Nombre completo con prefijo |
| `task1` | `taskNetworkHandle` | Expresa qué tarea y que es un handle |
| `mode` | `AudioMode::FIXED` | Enum con contexto de tipo |
| `initialized` | `_initialized` | Variable miembro requiere prefijo `_` |
