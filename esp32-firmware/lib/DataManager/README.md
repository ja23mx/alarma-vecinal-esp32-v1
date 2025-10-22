# DataManager

DataManager es una librería para ESP32 que permite gestionar configuraciones usando LittleFS y JSON. Incluye soporte para recibir comandos desde diversas interfaces (Serial, Web, MQTT) y actualizarlos de manera centralizada. La librería también ofrece módulos específicos, como el control de RF, y funciones CRUD para el manejo del ID y otros parámetros.

## Características

- **Gestión de configuraciones:** Guarda y recupera parámetros como ID, estados de RF, WIFI, y perifoneo.
- **Soporte para JSON:** Permite actualizar la configuración completa mediante cadenas JSON.
- **Manejo de comandos:** Función centralizada (`CmdGral`) para procesar comandos de entrada y devolver respuestas estructuradas.
- **Módulos específicos:** Ejemplo de módulo para RF integrado, con métodos para habilitar, deshabilitar o bloquear RF.
- **Plataforma:** Diseñada para ESP32 utilizando el framework Arduino y el sistema de archivos LittleFS.

## Estructura del Proyecto
    DataManager 
    ├── src/ │ 
    ├── ConstantesClase.h // Definiciones globales y valores por defecto 
    │ ├── DataManager.h // Declaración de la clase DataManager y métodos principales 
    │ ├── DataManager.cpp // Implementación de la funcionalidad central de DataManager 
    │ ├── DataManagerCmd.cpp // Implementación del procesamiento de comandos (CmdGral) 
    │ ├── ControlesRF.h // Declaración de métodos específicos para controlar RF 
    │ ├── ControlesRF.cpp // Implementación de métodos específicos para RF 
    ├── examples/ 
    │ ├── BasicUsage/ │ 
    │ └── BasicUsage.ino // Ejemplo básico de uso de la librería 
    │ └── UsoCmdGral/ 
    │ └── UsoCmdGral.ino // Ejemplo de procesamiento de comandos vía Serial 
    ├── library.json // Archivo de configuración de la librería para PlatformIO 
    ├── README.md // Este archivo 
    └── LICENSE // Archivo de licencia

    
## Instalación

1. Descarga o clona el repositorio.
2. Coloca la carpeta **DataManager** dentro de la carpeta `lib/` de tu proyecto PlatformIO o Arduino.
3. Reinicia PlatformIO/Arduino IDE para que se reconozca la nueva librería.

## Uso Básico

### Inicialización

Incluye la librería en tu sketch:

main.cpp
#include <DataManager.h>

void setup() {
  Serial.begin(115200);
  Data.begin();
  Data.rstConfig(); // Restablece la configuración a los valores por defecto
}

