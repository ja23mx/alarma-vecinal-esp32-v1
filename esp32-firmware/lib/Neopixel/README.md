# NeoPixelAV Library

Librería para controlar NeoPixels con patrones de parpadeo específicos para el sistema AV NEXUS.

## Características

- ✅ **Patrones de parpadeo precisos** con timing exacto
- ✅ **Colores predefinidos** según especificaciones del sistema
- ✅ **Arquitectura orientada a objetos** escalable
- ✅ **Compatible con FreeRTOS** para sistemas multitarea
- ✅ **Basada en Adafruit_NeoPixel** para máxima compatibilidad

## Instalación

Esta librería está incluida en el proyecto AV NEXUS y depende de:
- `Adafruit_NeoPixel` (^1.10.0)
- `Arduino Framework`

## Archivos de la librería:

### 1. `src/NeoPixelAV.h`
- Declaración de la clase `NeoPixelController`
- Enumeraciones para patrones y estados
- Métodos estáticos para colores predefinidos

### 2. `src/NeoPixelAV.cpp`
- Implementación de la clase `NeoPixelController`
- Lógica para patrones de parpadeo rápido
- Manejo de timing preciso

### 3. `include/tarea_neopixel.h`
- Declaraciones de la tarea FreeRTOS
- Funciones de conveniencia para usar desde otras partes del código
- Handle de la tarea

### 4. `src/tarea_neopixel.cpp`
- Implementación de la tarea FreeRTOS
- Instancia global de la clase NeoPixel
- Funciones wrapper para facilitar el uso

## Arquitectura:

### Clase NeoPixelController:
```cpp
NeoPixelController neopixel(NEOPIXEL_PIN, NUMPIXELS);
```

### Patrones implementados:
- **PATRON_APAGADO**: LEDs apagados
- **PATRON_FIJO**: Color fijo continuo
- **PATRON_RAPIDO_1**: 3 pulsos de 0.1s ON/OFF en 0.6s total
- **PATRON_RAPIDO_2**: 2 pulsos dobles en 0.6s total

### Colores predefinidos:
- `colorAzul()`: 0x0000FF
- `colorVerde()`: 0x00FF00
- `colorAmarillo()`: 0xFFFF00
- `colorTurquesa()`: 0x40E0D0
- `colorNaranja()`: 0xFFA500
- `colorRojo()`: 0xFF0000
- `colorVioleta()`: 0x800080
- `colorAzulClaro()`: 0x00BFFF

## Configuración:
- **Pin**: 19
- **Cantidad de LEDs**: 8
- **Prioridad de tarea**: 4
- **Stack size**: 2048 palabras

## Funciones principales:

### Desde la clase:
- `neopixel.init()`: Inicializa el NeoPixel
- `neopixel.setPatron(PATRON_RAPIDO_1, color)`: Activa un patrón específico
- `neopixel.setColor(color)`: Establece color fijo
- `neopixel.apagar()`: Apaga todos los LEDs
- `neopixel.loop()`: Debe llamarse continuamente para manejar patrones

### Funciones de conveniencia:
- `neopixel_set_patron_rapido_1(color)`: Activa patrón rápido 1
- `neopixel_set_patron_rapido_2(color)`: Activa patrón rápido 2
- `neopixel_set_color_fijo(color)`: Establece color fijo
- `neopixel_apagar_todos()`: Apaga todos los LEDs

## Uso desde otras tareas:

```cpp
#include "tarea_neopixel.h"

// Activar patrón rápido 1 con color amarillo
neopixel_set_patron_rapido_1(NeoPixelController::colorAmarillo());

// Activar patrón rápido 2 con color verde
neopixel_set_patron_rapido_2(NeoPixelController::colorVerde());

// Color fijo azul
neopixel_set_color_fijo(NeoPixelController::colorAzul());

// Apagar
neopixel_apagar_todos();
```

## Integración en main.cpp:
1. Incluir `"tarea_neopixel.h"`
2. Llamar `crearTareaNeoPixel()` para crear la tarea
3. Usar `cambiar_estado_neopixel(0/1)` para pausar/reanudar la tarea

## Timing de patrones:

### Patrón Rápido 1 (PATRON_RAPIDO_1):
```
Tiempo (s): |0--0.1--0.2--0.3--0.4--0.5--0.6|
LED:        |■_|■_|■_|
```

### Patrón Rápido 2 (PATRON_RAPIDO_2):
```
Tiempo (s): |0--0.1--0.2--0.3--0.4--0.5--0.6|
LED:        |■■|_|■■|_|
```

## Nota importante:
Asegúrate de instalar la librería `Adafruit_NeoPixel` en tu proyecto PlatformIO.

## Escalabilidad:
La arquitectura está diseñada para ser fácilmente extensible:
- Agregar nuevos patrones: Crear nueva enumeración y método `ejecutarPatronNuevo()`
- Agregar nuevos colores: Crear método estático `colorNuevo()`
- Modificar timing: Cambiar constantes en los métodos de patrón
