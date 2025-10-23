# Guía de Flasheo - Sistema Alarma Vecinal ESP32

Esta guía explica cómo flashear el firmware compilado al ESP32 del sistema de **Alarma Vecinal** usando **ESP32 Flash Download Tool**.

---

## 📋 Requisitos Previos

### Hardware
- **ESP32 Dev Module con 8MB Flash** (configurado en el proyecto)
- Cable USB de datos (no solo carga)
- PC con Windows

### Software
- **ESP32 Flash Download Tool** - Herramienta gráfica oficial de Espressif

---

## 🔧 Herramienta de Programación: ESP32 Flash Download Tool

### Descarga e Instalación

1. **Descargar Flash Download Tool:**
   - Ve a: [Espressif Flash Download Tools](https://www.espressif.com/en/support/download/other-tools)
   - Buscar **"Flash Download Tools"**
   - Descargar: `flash_download_tool_vX.X.X.zip`

2. **Extraer archivos:**
   ```
   Crear carpeta: C:\ESP32-FlashTool\
   Extraer el contenido del ZIP ahí
   ```

3. **Estructura esperada:**
   ```
   C:\ESP32-FlashTool\
   ├── flash_download_tool_3.9.9_R2.exe (o versión más reciente)
   ├── configure/
   └── (otros archivos)
   ```

### Verificación de la Instalación

1. **Ejecutar la herramienta:**
   - Navegar a `C:\ESP32-FlashTool\`
   - Doble clic en `flash_download_tool_3.9.9_R2.exe`

2. **Pantalla inicial:**
   ```
   Debería aparecer una ventana preguntando el tipo de chip
   Seleccionar: ESP32
   Seleccionar: develop
   Hacer clic en "OK"
   ```

3. **Ventana principal:**
   - Se abre la interfaz gráfica principal
   - Listo para usar

### ⚠️ Problemas Comunes en esta Etapa

| Error                       | Causa                             | Solución                                                 |
| --------------------------- | --------------------------------- | -------------------------------------------------------- |
| `No se abre la aplicación`  | Archivo corrupto o antivirus      | Descargar nuevamente, desactivar antivirus temporalmente |
| `Error de Windows Defender` | Falso positivo de seguridad       | Agregar excepción en Windows Defender                    |
| `Falta archivo .dll`        | Dependencias de Windows faltantes | Instalar Visual C++ Redistributable                      |

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

---

## 🚀 Flashear el Firmware

### Preparar el ESP32 para Programar

Antes de usar el Flash Download Tool, debes poner el ESP32 en **modo de programación (bootloader)**:

#### Método Manual (Para ESP32 sin auto-reset)

1. **Localizar los botones:**
   - **BOOT/PROG** (conectado a GPIO0)
   - **EN/RST** (botón de reset)

2. **Secuencia de botones:**
   - **Paso 1:** Mantener presionado el botón **BOOT/PROG**
   - **Paso 2:** Mientras mantienes BOOT, presionar y soltar **EN/RST**
   - **Paso 3:** Soltar el botón **BOOT/PROG**
   - **Resultado:** ESP32 entra en modo bootloader

3. **Verificar modo bootloader:**
   - Conectar ESP32 al PC
   - Abrir monitor serie (115200 baud)
   - Debería aparecer: `waiting for download` o similar

#### Método Automático (Para ESP32 con auto-reset)

Algunos ESP32 Dev Board tienen **auto-reset automático**:
- Solo conectar al PC
- El Flash Download Tool maneja automáticamente la entrada al bootloader
- **Si falla:** usar el método manual arriba

#### ⚠️ Problemas Comunes

| Problema                              | Causa                       | Solución                                |
| ------------------------------------- | --------------------------- | --------------------------------------- |
| `No response from device`             | ESP32 no en modo bootloader | Repetir secuencia de botones            |
| `Timed out waiting for packet header` | Cable USB defectuoso        | Usar cable de datos (no solo carga)     |
| `Serial port busy`                    | Puerto COM ocupado          | Cerrar otros programas usando el puerto |

---

### Configurar el Flash Download Tool

1. **Abrir la herramienta:**
   - Ejecutar `flash_download_tool_3.9.9_R2.exe`
   - Seleccionar: **ESP32**
   - Seleccionar: **develop**
   - Hacer clic en **"OK"**

2. **Configurar el archivo .bin:**
   - Marcar la primera casilla (checkbox) ☑️
   - Hacer clic en **"..."** para buscar el archivo
   - Seleccionar tu archivo `.bin` (ej: `esp32-ALARMA-VECINAL-v1.0.bin`)
   - En la dirección, escribir: **`0x10000`**

3. **Configurar SPI SPEED:**
   - Seleccionar: **40MHz** (recomendado)

4. **Configurar SPI MODE:**
   - Seleccionar: **DIO** (recomendado)

5. **Configurar puerto COM:**
   - En **COM:** seleccionar tu puerto (ej: **COM4**)
   - En **BAUD:** seleccionar **115200** (estable) o **921600** (rápido)

6. **Iniciar flasheo:**
   - **IMPORTANTE:** ESP32 debe estar en modo bootloader
   - Hacer clic en **"START"**
   - El botón se pondrá verde y dirá **"等待"** (esperando)

### Durante el Flasheo

**Progreso esperado:**
1. Estado: **"IDLE"** (inicial)
2. Estado: **"等待"** (esperando) - botón verde
3. Estado: **"Download"** (descargando) - color azul
4. Estado: **"FINISH"** (terminado) - color azul
5. Mensaje en la ventana inferior: **"完成"** (completado)

**Tiempo estimado:** 30-60 segundos dependiendo del tamaño del firmware

### Flasheo Exitoso ✅

**Indicadores de éxito:**
- Estado final: **"FINISH"**
- Mensaje: **"完成"** (completado en chino)
- Información del chip mostrada en el panel derecho:
  - **flash vendor:** (fabricante del flash)
  - **flash devID:** (ID del dispositivo)
  - **crystal:** 40 Mhz

**Información adicional mostrada:**
- **AP:** Dirección MAC del Access Point
- **STA:** Dirección MAC de la estación
- **BT:** Dirección MAC del Bluetooth
- **ETHERNET:** Dirección MAC Ethernet

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



























