# Firmware v0.0.1

Versión inicial del template ESP32.

---

## 📦 Archivos en esta Carpeta

Esta carpeta contiene los binarios compilados de la versión v0.0.1 listos para flashear al ESP32.

| Archivo                                                  | Descripción                            |
| -------------------------------------------------------- | -------------------------------------- |
| `esp32-TEMPLATE-V0.0.1-B-001-T-DD-MM-YYYY-HHMMSSRHS.bin` | Firmware compilado listo para flashear |

**Nota:** Los archivos `.bin` solo se agregan en releases oficiales. Durante desarrollo esta carpeta puede estar vacía.

---

## 🚀 Cómo Flashear

Ver la guía completa: [COMO-FLASHEAR.md](../COMO-FLASHEAR.md)

### Comando rápido (Windows):
```cmd
esptool.py --chip esp32 --port COM3 --baud 921600 write_flash 0x10000 esp32-TEMPLATE-V0.0.1-B-001-T-DD-MM-YYYY-HHMMSSRHS.bin
```

### Comando rápido (Linux/macOS):
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x10000 esp32-TEMPLATE-V0.0.1-B-001-T-DD-MM-YYYY-HHMMSSRHS.bin
```

---

## 📜 Changelog - v0.0.1

### ✨ Nuevas Características

- Configuración inicial del proyecto
- Estructura de carpetas base
- Template PlatformIO para ESP32
- Sistema de versionado semántico

### 🔧 Configuración

- Placa: ESP32 Dev Module
- Framework: Arduino
- Frecuencia CPU: 240MHz
- Velocidad Upload: 921600 baud
- Velocidad Monitor: 115200 baud

### 📚 Librerías Incluidas

Ninguna (versión base)

### 🔌 Hardware Requerido

- ESP32 Dev Module (cualquier modelo compatible)
- Cable USB (datos, no solo carga)

### ✅ Funcionalidad Implementada

Esta versión incluye:
- Estructura básica del proyecto
- Configuración PlatformIO
- Sistema de documentación
- Workflow de desarrollo con Git

Pendiente para próximas versiones:
- Funcionalidad WiFi
- Sistema de logging
- Configuración OTA (Over-The-Air updates)
- Interfaz web de configuración

### 🐛 Problemas Conocidos

Ninguno reportado en esta versión.

---

## 📝 Notas de Desarrollo

- Primera versión del template
- Código base mínimo para iniciar desarrollo
- Sin funcionalidad específica implementada aún

---

## 🔗 Enlaces Relacionados

## 🔗 Enlaces Relacionados

- [Changelog completo del proyecto](../../CHANGELOG.md)
- [Documentación técnica](../../esp32-firmware/docs/README.md)
- [Issues y bugs](https://github.com/ja23mx/esp32-git-template-1/issues)

---

**Fecha de release:** 2025-01-11  
**Build:** 001  
**Autor:** @ja23mx

