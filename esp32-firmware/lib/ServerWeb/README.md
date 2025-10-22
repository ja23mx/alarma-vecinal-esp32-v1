# ServerWeb

Libreria para servidor web async en ESP32 que integra la seccion RF.

## Caracteristicas

- Gestion de rutas web utilizando ESPAsyncWebServer.
- Registro modular de rutas para la seccion RF.
  - `/rf_init`: Responde con "ok" en formato text/plain.
  - `/rf_ajax`: Lee el argumento `numerox` y responde con una pagina HTML.
- Configuracion y uso sencillo para proyectos con PlatformIO y Arduino.

## Instalacion

1. Clona o descarga este repositorio.
2. Coloca la carpeta en el directorio de librerias de tu proyecto PlatformIO.
3. Las dependencias se gestionan automaticamente desde `library.json`.

## Uso

Un ejemplo basico se encuentra en la carpeta `examples/BasicExample`.  
Por ejemplo, en el archivo `main.cpp`:

```cpp
#include <Arduino.h>
#include "ServerWeb.h"

ServerWeb serverWeb;

void setup() {
  Serial.begin(115200);
  // Inicializa el servidor web en el puerto 80.
  serverWeb.begin(80);
}

void loop() {
  // Loop vacio, la gestion se realiza de forma interna.
}
