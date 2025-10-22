# BLEManager

BLEManager es una librería para ESP32 que permite gestionar conexiones BLE de manera eficiente utilizando la librería **NimBLE-Arduino**.

## Características

- **Gestión de Bluetooth Low Energy (BLE)** con **ESP32**.
- **Compatibilidad con NimBLE-Arduino**, optimizado para bajo consumo de memoria.
- **Soporte para recepción y envío de datos** con características **Rx y Tx**.
- **Callbacks de conexión y desconexión** para manejar eventos BLE.

## Instalación

### 1️⃣ **Instalación en PlatformIO**
1. Agrega la librería a tu proyecto en PlatformIO creando la carpeta `lib/BLEManager/` y colocando los archivos:
   - `BLEManager.h`
   - `BLEManager.cpp`
   - `library.json`
2. Reinicia PlatformIO para detectar la librería.
3. PlatformIO instalará automáticamente la dependencia **NimBLE-Arduino**.

### 2️⃣ **Instalación en Arduino IDE**
1. Descarga los archivos de la librería desde GitHub.
2. Coloca la carpeta en el directorio `libraries/` de Arduino IDE.
3. Agrega manualmente la dependencia **NimBLE-Arduino** en el **Gestor de Librerías**.

## Uso Básico

### 📌 **Ejemplo de Uso**
```cpp
#include <Arduino.h>
#include "BLEManager.h"

BLEManager ble;

void setup() {
    Serial.begin(115200);
    ble.begin("ESP32_BLE");
}

void loop() {
    ble.loop();
}
