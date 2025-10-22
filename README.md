# ESP32 Alarma Vecinal

Sistema de alarma vecinal con ESP32, recepción RF 433MHz, conectividad MQTT TLS y interfaz web para configuración.

---

## 📋 Descripción

Sistema de alarma vecinal basado en ESP32 que permite:
- ✅ Activación/desactivación mediante controles RF 433MHz
- ✅ Reproducción de audio personalizado (MP3) 
- ✅ Conectividad MQTT TLS (puerto 8883) para integración con backend
- ✅ Interfaz web para configurar controles RF y red WiFi
- ✅ Indicadores LED NeoPixel para estados del sistema
- ✅ Salidas para actuadores externos

**Estado del proyecto:** 90% completo - Funcional, en fase de documentación.

---

## 🔧 Hardware Requerido

### Componentes Principales
- **ESP32-WROOM-32** (8MB Flash)
- **Módulo RF 433MHz** (receptor)
- **DFPlayer Mini** (MP3-TF-16P)
- **Amplificador de audio** (necesario)
- **NeoPixel LEDs** (1 embebido + 7 en tira WS2812B)
- **MOSFET** para control de LED externo
- **Tarjeta microSD** con archivos de audio MP3

### Conexiones
- Conversor USB a puerto COM. Comunicacion a 115200 baudios
- Acceso al menú web: **http://192.168.4.1**

---

## 📦 Estructura del Proyecto

```
/docs/               - Documentación usuario final

/esp32-bin-files/    - Binarios compilados organizados por versión
    /dev-test/       - Binarios de prueba (NO se suben a Git)
    /v0.9.0/         - Firmware versión 0.9.0 listo para testing
    COMO-FLASHEAR.md - Guía de instalación con esptool

/esp32-firmware/     - Código fuente del firmware (PlatformIO) 
    /bin-tools-py/   - Scripts y herramientas para gestionar binarios
    /docs/           - Documentación técnica del código 
    /src/            - Código fuente principal 
    /include/        - Archivos de cabecera (.h) 
    /lib/            - Librerías personalizadas del sistema
        /BLEManager/   - Gestión Bluetooth (futuro)
        /DataManager/  - Gestión de configuraciones
        /GestorCmd/    - Procesamiento de comandos y datos
        /MQTT/         - Cliente MQTT con TLS
        /Neopixel/     - Control de LEDs de estado
        /ServerWeb/    - Servidor web de configuración
        /TimeManager/  - Gestión de tiempo del sistema
        /WiFiTools/    - Herramientas de conectividad WiFi
    /test/           - Pruebas unitarias 
    platformio.ini   - Configuración del proyecto PlatformIO

/examples/           - Proyectos de prueba y ejemplos
    README.md        - Guía de uso de ejemplos

/docs/               - Documentación para usuario final 
    /fuentes/        - Enlaces a documentos editables 
    /publicados/     - PDFs finales (en releases)

.gitignore           - Configuración de exclusión para Git
CHANGELOG.md         - Historial de cambios del proyecto
LICENSE              - Licencia del proyecto
README.md            - Documentación general del proyecto
```

---

## 🚀 Inicio Rápido

### Para Usuarios (Instalar Firmware)

1. Descarga el binario de la [última release](../../releases).
2. Consulta [`esp32-bin-files/COMO-FLASHEAR.md`](esp32-bin-files/COMO-FLASHEAR.md).
3. Conecta el hardware según esquemas (pendiente documentar).
4. Accede al menú web en **http://192.168.4.1** para configurar WiFi y controles RF.

---

### Para Desarrolladores

#### Requisitos Previos

- [PlatformIO Core](https://platformio.org/) o PlatformIO IDE
- Git
- Visual Studio Code (recomendado)

#### Configuración del Entorno

```bash
# Clonar repositorio
git clone [https://github.com/ja23mx/alarma-vecinal-esp32-v1.git]
cd alarma-vecinal-esp32-v1

# Abrir proyecto principal en VS Code
code esp32-firmware/

# Compilar
cd esp32-firmware
pio run

# Subir a ESP32
pio run --target upload

# Monitor serial
pio device monitor
```

---

## 📚 Documentación

- **Usuario Final**: Ver carpeta `/docs/publicados/` (en desarrollo).
- **Desarrolladores**: Ver `/esp32-firmware/docs/` (en desarrollo).
- **Instalación Firmware**: Ver `/esp32-bin-files/COMO-FLASHEAR.md`.

---

## 🏷️ Versionado

Este proyecto usa [Semantic Versioning](https://semver.org/):
- **MAJOR.MINOR.PATCH** (ej: `v0.9.0`)
- Ver [Releases](../../releases) para historial de versiones.

---

## 📝 Convención de Nomenclatura de Binarios

`esp32-ALARMA-VECINAL-V[VERSION]-B-[BUILD]-T-[TIMESTAMP].bin`

Ejemplo: `esp32-ALARMA-VECINAL-V0.9.0-B-001-T-22-10-2025-143012HRS.bin`

---

## 🗺️ Roadmap

### ✅ Completado (90%)
- Core del sistema de alarma
- Recepción RF 433MHz 
- Reproducción de audio MP3
- Conectividad MQTT TLS
- Servidor web de configuración
- Control de LEDs NeoPixel
- Gestión de salidas

### 🔄 En Progreso
- [ ] Documentación completa
- [ ] Esquemas de conexión hardware
- [ ] Manual de configuración detallado
- [ ] Guías de instalación paso a paso

### 📋 Pendiente
- [ ] Optimizaciones de rendimiento
- [ ] Funciones adicionales menores
- [ ] Testing exhaustivo

---

## 🤝 Contribución

Este es un proyecto en desarrollo activo. Las contribuciones son bienvenidas:

1. Fork del proyecto.
2. Crear rama feature (`git checkout -b feature/nueva-funcionalidad`).
3. Commit cambios (`git commit -m 'feat: Descripción del cambio'`).
4. Push a la rama (`git push origin feature/nueva-funcionalidad`).
5. Abrir Pull Request.

### Commits Convencionales

- `feat:` - Nueva funcionalidad.
- `fix:` - Corrección de bugs.
- `docs:` - Cambios en documentación.
- `refactor:` - Refactorización de código.
- `test:` - Agregar o modificar tests.

---

## 📄 Licencia

[Pendiente especificar licencia]

---

## 👥 Autores

- **Jorge Aguilar** - *Desarrollo principal* 

---

## 📅 Historial

- **2025-10-22**: Proyecto al 90% - Preparando documentación para GitHub.
- **2025**: Desarrollo del sistema core de alarma vecinal.

---

## 📞 Contacto

[Pendiente agregar información de contacto]