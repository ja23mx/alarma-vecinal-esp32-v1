# Guía de Flasheo - ESP32 Git Template 1

Esta guía explica cómo flashear el firmware compilado al ESP32 usando **esptool**.

---

## 📋 Requisitos Previos

### Hardware
- ESP32 Dev Module (especificar modelo exacto)
- Cable USB (datos, no solo carga)
- PC con Windows/Linux/macOS

### Software
- **esptool.py** - [Descargar aquí](https://github.com/espressif/esptool/releases)

---

## 🔧 Instalación de esptool

### Windows
```cmd
pip install esptool
```

### Linux/macOS
```bash
pip3 install esptool
```

### Verificar instalación
```bash
esptool.py version
```

---

## 📦 Obtener el Firmware

### Opción 1: Desde GitHub Releases (Recomendado para Usuarios)
1. Ve a Releases.
2. Descarga el archivo `.bin` de la versión deseada.
3. Guarda en una carpeta conocida (ej: `C:\esp32-firmware\`)

### Opción 2: Compilar desde Código Fuente (Desarrolladores)
```bash
cd esp32-firmware
pio run
# El .bin estará en: .pio/build/esp32dev/firmware.bin
```

---

## 🔌 Conectar el ESP32

1. **Conectar ESP32 al PC** vía USB.
2. **Identificar el puerto COM/Serial**:

### Windows
```cmd
# Abrir el Administrador de Dispositivos
# Buscar en "Puertos (COM y LPT)"
# Anotar el puerto, ej: COM3
```

### Linux
```bash
ls /dev/ttyUSB*
# Resultado típico: /dev/ttyUSB0
```

### macOS
```bash
ls /dev/cu.*
# Resultado típico: /dev/cu.usbserial-0001
```

---

## 🚀 Flashear el Firmware

### Método 1: Flasheo Básico (Recomendado)

#### Windows
```cmd
esptool.py --chip esp32 --port COM3 --baud 921600 write_flash 0x10000 esp32-TEMPLATE-V0.0.1-B-001-T-11-10-2025-015448HRS.bin
```

#### Linux/macOS
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x10000 esp32-TEMPLATE-V0.0.1-B-001-T-11-10-2025-015448HRS.bin
```

### Método 2: Flasheo Completo (Borra toda la memoria)

#### Windows
```cmd
esptool.py --chip esp32 --port COM3 --baud 921600 erase_flash
esptool.py --chip esp32 --port COM3 --baud 921600 write_flash 0x10000 esp32-TEMPLATE-V0.0.1-B-001-T-11-10-2025-015448HRS.bin
```

#### Linux/macOS
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x10000 esp32-TEMPLATE-V0.0.1-B-001-T-11-10-2025-015448HRS.bin
```

---

## 📊 Parámetros Explicados

| Parámetro       | Descripción                                                  |
| --------------- | ------------------------------------------------------------ |
| `--chip esp32`  | Especifica el tipo de chip (ESP32, ESP32-S2, ESP32-C3, etc.) |
| `--port COM3`   | Puerto serial donde está conectado el ESP32                  |
| `--baud 921600` | Velocidad de transmisión (921600 es la más rápida)           |
| `write_flash`   | Comando para escribir firmware                               |
| `0x10000`       | Dirección de memoria donde se escribe el firmware            |
| `archivo.bin`   | Ruta al archivo binario del firmware                         |

### Velocidades de Baudrate Alternativas

Si `921600` da errores, prueba velocidades más bajas:
- `460800`
- `230400`
- `115200` (más lenta pero más estable)

Ejemplo:
```bash
esptool.py --chip esp32 --port COM3 --baud 115200 write_flash 0x10000 firmware.bin
```

---

## ✅ Verificación del Flasheo

### Salida Exitosa
Deberías ver algo como:
```plaintext
esptool.py v4.7.0
Serial port COM3
Connecting....
Chip is ESP32-D0WD-V3 (revision v3.0)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: xx:xx:xx:xx:xx:xx
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Flash will be erased from 0x00010000 to 0x000xxxxx...
Compressed 123456 bytes to 78901...
Wrote 123456 bytes (78901 compressed) at 0x00010000 in 2.5 seconds
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

### Monitor Serial (Opcional)

Para ver la salida del ESP32:

#### Con esptool:
```bash
esptool.py --port COM3 monitor
```

#### Con PlatformIO:
```bash
pio device monitor --port COM3 --baud 115200
```

---

## ❌ Solución de Problemas

### Error: "Serial port not found"
- **Causa**: Puerto incorrecto o driver USB no instalado
- **Solución**:
  - Verificar que el cable USB sea de datos (no solo carga)
  - Instalar driver CP210x o CH340 según el chip USB del ESP32
  - Verificar el puerto correcto en Administrador de Dispositivos

### Error: "Failed to connect"
- **Causa**: ESP32 no entra en modo bootloader
- **Solución**:
  1. Mantén presionado el botón **BOOT** del ESP32
  2. Presiona el botón **RESET**
  3. Suelta **RESET**, luego suelta **BOOT**
  4. Ejecuta el comando de flasheo nuevamente

### Error: "Hash of data verified failed"
- **Causa**: Archivo .bin corrupto o transmisión con errores
- **Solución**:
  - Reducir baudrate a `115200`
  - Descargar nuevamente el archivo .bin
  - Usar un cable USB de mejor calidad

### Error: "Permission denied" (Linux/macOS)
- **Causa**: Permisos insuficientes en el puerto serial
- **Solución**:
```bash
sudo usermod -a -G dialout $USER
# Cerrar sesión y volver a iniciar
```

---

Última actualización: 2025-10-11



























