# WiFiTool - Librería para ESP32

## 📌 Descripción
**WiFiTool** es una librería para ESP32 que permite:
✅ Escanear redes WiFi de manera **sincrónica** y **asíncrona**.  
✅ Conectarse a una red WiFi de manera eficiente.  
✅ Obtener el estado de conexión y dirección IP.  
✅ Generar JSON con las redes WiFi detectadas.

---

## 📂 Estructura de Archivos
WiFiLibrary/ 
│ ├── 📂 include/ 
│ ├── WiFiTool.h # Declaración de métodos de escaneo y conexión 
│ ├── 📂 src/ 
│ ├── WiFiScan.cpp # Implementación del escaneo sincrónico 
│ ├── WiFiAsyncScan.cpp # Implementación del escaneo asíncrono 
│ ├── WiFiConexion.cpp # Implementación de la conexión WiFi 
│ └── 📂 examples/ 
├── wifi_example.ino # Código de prueba

---

## 🚀 Instalación
1. **Clonar el repositorio**  
   ```sh
   git clone https://github.com/TuUsuario/WiFiTool.git
2. Añadir la librería a PlatformIO
    pio lib install WiFiTool
2. O copiar los archivos a lib/ en tu proyecto de PlatformIO.

🔍 Métodos Principales

Escaneo Sincrónico

    wifiTool.scanInit();  // Bloquea hasta completar (~6.7s)
    Serial.println(wifiTool.scanJSON());  // Imprime redes en JSON

Escaneo Asíncrono

    wifiTool.scanAsyncInit();  // Inicia el escaneo sin bloquear
    while (!wifiTool.scanAsyncStatus()) {
        Serial.println("Escaneando...");
        delay(500);
    }
    Serial.println(wifiTool.scanJSON());

Conexión WiFi

    wifiTool.startConnection("REDLOCAL", "TLWN7200NDMX2148", 10000);
    if (wifiTool.isConnected()) {
        Serial.print("Conectado. IP: ");
        Serial.println(wifiTool.getLocalIP());
    }


📜 Generación de JSON

    String json = wifiTool.scanJSON();
    Serial.println(json);


📌 Ejemplo de salida JSON:

    {
      "redes": [
        {"ssid": "MiRed", "rssi": -45, "rssiNivel": 1, "seguridad": 2, "canal": 6},
        {"ssid": "OtraRed", "rssi": -70, "rssiNivel": 2, "seguridad": 3, "canal": 11}
      ]
    }


🛠️ Compatibilidad

    ✔ ESP32
    ✔ PlatformIO con Arduino Framework

📝 Licencia
    📄 MIT License - Puedes usar, modificar y distribuir libremente.


📌 Contribuciones
    ¡Pull requests y sugerencias son bienvenidas! 😃

    Fork este repositorio.
    Crea una rama (git checkout -b feature-nueva).
    Haz tus cambios y commitea (git commit -m "Añadir nueva funcionalidad").
    Sube los cambios (git push origin feature-nueva).
