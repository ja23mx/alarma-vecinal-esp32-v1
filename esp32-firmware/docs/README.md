# Documentación Técnica - ESP32 Firmware

Documentación técnica del código fuente para desarrolladores.

## 📋 Índice

- [Arquitectura del Proyecto](#🏗️-arquitectura-del-proyecto)
- [Estructura de Código](#📁-estructura-de-código)
- [Módulos y Componentes](#🧩-módulos-y-componentes)
- [Configuración](#⚙️-configuración)
- [Desarrollo](#🛠️-desarrollo)
- [Testing](#🧪-testing)
- [Debugging](#🐛-debugging)

## 🏗️ Arquitectura del Proyecto

### Diagrama de Componentes

```
┌─────────────────────────────────────┐
│         main.cpp (Entry Point)      │
└──────────────┬──────────────────────┘
               │
    ┌──────────┴──────────┐
    │                     │
┌───▼────┐         ┌──────▼──────┐
│ Setup  │         │    Loop     │
└───┬────┘         └──────┬──────┘
    │                     │
    │              ┌──────▼──────────┐
    │              │  Task Manager   │
    │              └──────┬──────────┘
    │                     │
    └─────────┬───────────┘
              │
    ┌─────────▼──────────┐
    │   Hardware Layer   │
    │  (GPIO, WiFi, etc) │
    └────────────────────┘
```

### Patrón de Diseño

- **Arquitectura**: Modular basada en componentes
- **Patrón principal**: Event-driven con FreeRTOS tasks
- **Comunicación**: Message passing entre tareas

## 📁 Estructura de Código

```
/esp32-firmware/
├── src/
│   └── main.cpp              # Punto de entrada principal
├── include/
│   └── [headers personalizados]
├── lib/
│   └── [librerías personalizadas]
├── test/
│   └── [pruebas unitarias]
├── docs/
│   └── README.md             # Este archivo
└── platformio.ini            # Configuración del proyecto
```

### Descripción de Carpetas

| Carpeta    | Propósito                                |
| ---------- | ---------------------------------------- |
| `src/`     | Código fuente principal (.cpp)           |
| `include/` | Archivos de cabecera (.h) personalizados |
| `lib/`     | Librerías personalizadas del proyecto    |
| `test/`    | Pruebas unitarias con PlatformIO Unity   |
| `docs/`    | Documentación técnica                    |

## 🧩 Módulos y Componentes

### Estado Actual (v0.0.1)

#### main.cpp
- **Función**: Punto de entrada del programa
- **Responsabilidades**:
  - Inicialización del hardware
  - Configuración de periféricos
  - Loop principal

```cpp
void setup() {
    // Inicialización
    Serial.begin(115200);
    // ... más configuración
}

void loop() {
    // Loop principal
}
```

### Módulos Planificados

#### WiFi Manager (Próxima versión)
- Gestión de conexión WiFi
- Modo AP para configuración
- Reconnect automático

#### Config Manager (Próxima versión)
- Persistencia de configuración
- Sistema de archivos (SPIFFS/LittleFS)
- API de lectura/escritura

#### Logger (Próxima versión)
- Sistema de logging multinivel
- Salida a Serial y archivo
- Timestamps y categorías

## ⚙️ Configuración

### platformio.ini

Configuración actual del proyecto:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

### Opciones de Compilación

Para modificar la configuración:

```ini
# Habilitar optimizaciones
build_flags = 
    -O2
    -DCORE_DEBUG_LEVEL=3

# Cambiar partición
board_build.partitions = huge_app.csv

# Frecuencia CPU
board_build.f_cpu = 240000000L
```

## 🛠️ Desarrollo

### Configurar Entorno

```bash
# Clonar proyecto
git clone https://github.com/ja23mx/esp32-git-template-1.git
cd esp32-git-template-1/esp32-firmware

# Compilar
pio run

# Subir a ESP32
pio run --target upload

# Monitor serial
pio device monitor
```

### Crear Nueva Funcionalidad

1. **Crear rama feature**:
```bash
git checkout -b feature/nueva-funcionalidad
```

2. **Desarrollar código**:
   - Agregar archivos en `src/` o `lib/`
   - Crear headers en `include/`
   - Documentar en código

3. **Compilar y probar**:
```bash
pio run
pio run --target upload
```

4. **Commit y push**:
```bash
git add .
git commit -m "feat: Descripción de la funcionalidad"
git push origin feature/nueva-funcionalidad
```

5. **Abrir Pull Request en GitHub**

### Convenciones de Código

#### Nombres de Variables

```cpp
// Variables locales: camelCase
int sensorValue = 0;

// Constantes: UPPER_CASE
#define MAX_BUFFER_SIZE 256
const int LED_PIN = 13;

// Clases: PascalCase
class WiFiManager {
    // ...
};
```

#### Comentarios

```cpp
// Comentarios de una línea para explicaciones breves

/**
 * Comentarios de bloque para funciones
 * @param nombre Descripción del parámetro
 * @return Descripción del retorno
 */
int miFuncion(int nombre) {
    // ...
}
```

## 🧪 Testing

### Ejecutar Pruebas

```bash
# Compilar y ejecutar tests
pio test

# Test específico
pio test -f test_nombre
```

### Crear Nuevo Test

Crear archivo en `/test/test_ejemplo.cpp`:

```cpp
#include <unity.h>

void test_suma() {
    TEST_ASSERT_EQUAL(5, 2 + 3);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_suma);
    UNITY_END();
}

void loop() {
    // Nada aquí
}
```

## 🐛 Debugging

### Monitor Serial

```bash
# Monitor básico
pio device monitor

# Con filtro de nivel de log
pio device monitor --filter esp32_exception_decoder
```

### Niveles de Debug

Modificar en `platformio.ini`:

```ini
build_flags = 
    -DCORE_DEBUG_LEVEL=5  # 0=None, 5=Verbose
```

En código:

```cpp
#ifdef CORE_DEBUG_LEVEL
    Serial.println("DEBUG: Mensaje de debug");
#endif
```

### Debug con JTAG (Avanzado)

Requiere hardware adicional (JTAG debugger).

Configuración en `platformio.ini`:

```ini
debug_tool = esp-prog
debug_init_break = tbreak setup
```

### bin-tools-py

La carpeta `/bin-tools-py/` contiene scripts y herramientas para gestionar binarios y configuraciones de builds.  
Para más detalles, consulta la documentación específica en [`bin_tools_py_README.md`](../bin-tools-py/bin_tools_py_README.md).

## 📚 Referencias

- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [ESP32 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [FreeRTOS Docs](https://www.freertos.org/Documentation/RTOS_book.html)

---

**Última actualización**: 2025-10-11  
**Mantenido por**: [@ja23mx](https://github.com/ja23mx)

