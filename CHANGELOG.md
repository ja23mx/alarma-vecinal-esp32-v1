# Changelog

Todos los cambios notables de este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/), y este proyecto adhiere a [Semantic Versioning](https://semver.org/).

---

## [Unreleased]

### 📋 Por Hacer

- [ ] Documentación completa de esquemas hardware
- [ ] Manual de configuración detallado  
- [ ] Optimizaciones de rendimiento
- [ ] Testing exhaustivo

---

## [v0.9.0] - 2025-10-22

### ✨ Implementado (Sistema Funcional 90%)

#### Core del Sistema
- Sistema de alarma vecinal completamente funcional
- Gestión de estados y procesos del sistema
- Sistema de tareas FreeRTOS optimizado
- Watchdog configurado para estabilidad

#### Conectividad
- **WiFi:** Conexión automática y gestión de redes
- **MQTT TLS:** Cliente seguro puerto 8883 para backend
- **Servidor Web:** Interfaz de configuración en http://192.168.4.1
- **RF 433MHz:** Recepción y procesamiento de controles remotos

#### Hardware Integrado
- **DFPlayer Mini (MP3-TF-16P):** Reproducción de audio personalizado
- **NeoPixel LEDs:** Sistema de notificación visual (1 embebido + 7 externos)
- **Amplificador:** Control de audio con MOSFET
- **Salidas digitales:** Control de actuadores externos

#### Librerías Desarrolladas
- `WiFiTools` - Gestión avanzada de conectividad
- `DataManager` - Persistencia de configuraciones
- `ServerWeb` - Servidor web de configuración
- `BLEManager` - Gestión Bluetooth (preparado)
- `MQTT` - Cliente MQTT con TLS
- `GestorCmd` - Procesamiento de comandos
- `Neopixel` - Control de LEDs de estado
- `TimeManager` - Gestión de tiempo del sistema

### 🔧 Configuración Sistema

- **Placa:** ESP32-WROOM-32 (8MB Flash)
- **Framework:** Arduino con FreeRTOS
- **Frecuencia CPU:** 240MHz
- **Velocidad Upload:** 921600 baud
- **Monitor Serial:** 115200 baud
- **Partición:** 8MB 
  
### 🔄 En Desarrollo

- [ ] Documentación técnica completa
- [ ] Esquemas de conexión hardware
- [ ] Manual de usuario final

---

## [v0.1.0] - 2025-01 

### ✨ Añadido (Desarrollo Inicial)

- Estructura inicial del proyecto
- Configuración base de PlatformIO para ESP32
- Sistema de carpetas organizado por función
- Sistema de versionado semántico
- Documentación base del proyecto
- `.gitignore` configurado para desarrollo ESP32

### 🔧 Configuración Base

- Configuración inicial ESP32 Dev Module
- Framework Arduino base
- Estructura de carpetas para desarrollo

---

## 📚 Estructura de Carpetas

```plaintext
/docs/                  - Documentación usuario final
/esp32-bin-files/       - Binarios compilados por versión
    /dev-test/          - Binarios de prueba (no versionados)
    COMO-FLASHEAR.md    - Guía de instalación
/examples/              - Proyectos de prueba y ejemplos
/esp32-firmware/        - Código fuente principal (PlatformIO)
    /lib/               - Librerías personalizadas del sistema
    /src/               - Código fuente principal
    /include/           - Headers del sistema
```

---

## 📝 Documentación

- **Técnica:** Documentación de código y APIs (en desarrollo)
- **Usuario:** Manual de instalación y configuración (pendiente)
- **Hardware:** Esquemas y conexiones (pendiente)
- **Desarrollo:** Guías para contribuidores

---

## 🔄 Workflow de Desarrollo

- Convención de commits ([Conventional Commits](https://www.conventionalcommits.org/))
- Versionado semántico (Semantic Versioning)
- Sistema de releases con GitHub
- Desarrollo con feature branches

---

## 🛠️ Tipos de Cambios

- ✨ **Añadido (Added):** Nuevas funcionalidades
- 🔧 **Cambiado (Changed):** Cambios en funcionalidad existente
- ⚠️ **Deprecado (Deprecated):** Funcionalidad que será removida
- 🗑️ **Removido (Removed):** Funcionalidad eliminada
- 🐛 **Corregido (Fixed):** Corrección de bugs
- 🔒 **Seguridad (Security):** Vulnerabilidades corregidas

---

Mantenido por: **Jorge** 

Última actualización: 2025-10-22