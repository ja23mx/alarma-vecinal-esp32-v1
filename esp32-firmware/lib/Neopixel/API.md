# NeoPixelAV API Documentation

## Clase Principal: NeoPixelController

### Constructor
```cpp
NeoPixelController(uint8_t pin = NEOPIXEL_PIN, uint16_t numPixels = NUMPIXELS);
```
- **pin**: Pin GPIO donde están conectados los NeoPixels (por defecto: 19)
- **numPixels**: Número de LEDs en la tira (por defecto: 8)

### Métodos Públicos

#### Inicialización
```cpp
void init(void);
```
Inicializa la librería Adafruit_NeoPixel y configura el estado inicial.

#### Control Básico
```cpp
void apagar(void);
void setColor(uint32_t color);
void setAllLEDs(uint32_t color);
```
- **apagar()**: Apaga todos los LEDs
- **setColor()**: Establece un color fijo en todos los LEDs
- **setAllLEDs()**: Función interna para establecer color (más directa)

#### Sistema de Patrones
```cpp
void setPatron(PatronType_t patron, uint32_t color);
void loop(void);
```
- **setPatron()**: Activa un patrón específico con un color
- **loop()**: Debe llamarse continuamente para manejar el timing de los patrones

#### Consultas de Estado
```cpp
bool isConfigured(void) const;
PatronType_t getCurrentPattern(void) const;
```
- **isConfigured()**: Retorna true si el NeoPixel está inicializado
- **getCurrentPattern()**: Retorna el patrón actualmente activo

#### Colores Predefinidos (Métodos Estáticos)
```cpp
static uint32_t colorAzul(void);        // 0x0000FF
static uint32_t colorVerde(void);       // 0x00FF00
static uint32_t colorAmarillo(void);    // 0xFFFF00
static uint32_t colorTurquesa(void);    // 0x40E0D0
static uint32_t colorNaranja(void);     // 0xFFA500
static uint32_t colorRojo(void);        // 0xFF0000
static uint32_t colorVioleta(void);     // 0x800080
static uint32_t colorAzulClaro(void);   // 0x00BFFF
```

## Enumeraciones

### PatronType_t
```cpp
typedef enum {
    PATRON_APAGADO,     // LEDs apagados
    PATRON_FIJO,        // Color fijo continuo
    PATRON_RAPIDO_1,    // 3 pulsos de 0.1s ON/OFF en 0.6s
    PATRON_RAPIDO_2     // 2 pulsos dobles en 0.6s
} PatronType_t;
```

## Funciones de Conveniencia (para usar desde otras tareas)

```cpp
void neopixel_set_patron_rapido_1(uint32_t color);
void neopixel_set_patron_rapido_2(uint32_t color);
void neopixel_set_color_fijo(uint32_t color);
void neopixel_apagar_todos(void);
```

## Ejemplo de Uso Básico

```cpp
#include "NeoPixelAV.h"

// Crear instancia
NeoPixelController neopixel(19, 8);

void setup() {
    // Inicializar
    neopixel.init();
    
    // Color fijo azul
    neopixel.setColor(NeoPixelController::colorAzul());
    delay(1000);
    
    // Patrón rápido 1 con amarillo
    neopixel.setPatron(PATRON_RAPIDO_1, NeoPixelController::colorAmarillo());
}

void loop() {
    // Importante: llamar loop() para manejar patrones
    neopixel.loop();
    delay(10);
}
```

## Timing de Patrones

### PATRON_RAPIDO_1
- **Duración total**: 0.6 segundos
- **Secuencia**: 3 pulsos de 0.1s ON + 0.1s OFF
- **Representación**: `|■_|■_|■_|`

### PATRON_RAPIDO_2
- **Duración total**: 0.6 segundos
- **Secuencia**: 2 pulsos dobles (0.2s ON + 0.1s OFF + 0.2s ON + 0.1s OFF)
- **Representación**: `|■■|_|■■|_|`

## Compatibilidad

- **Plataforma**: ESP32
- **Framework**: Arduino
- **Dependencia**: Adafruit_NeoPixel ^1.10.0
- **FreeRTOS**: Compatible con tareas múltiples

## Consideraciones de Rendimiento

- La función `loop()` es no bloqueante
- Los patrones se ejecutan automáticamente una vez y luego se apagan
- El timing está optimizado para precisión de milisegundos
- Compatible con sistemas de tiempo real
