# Documentación de bin_build_manager.py y bin_build_config.py

## Descripción

Automatizan la generación y gestión del archivo binario (.bin) del firmware ESP32.
La versión se lee directamente desde `include/CnfTarjeta.h` (fuente única de verdad),
eliminando la necesidad de mantener la versión en múltiples lugares.

Cada compilación genera un `.bin` con nombre estructurado, lo copia a `ota/` (último build)
y archiva una copia permanente en `releases/v{version}/`.

```
/esp32-bin-files/
    /ota/               - Último build compilado (se sobreescribe en cada compilación)
    /releases/
        /v0.1.0-beta.1/ - Historial de binarios por versión liberada
        /v0.1.0/
        ...
    COMO-FLASHEAR.md    - Guía de instalación con esptool

/esp32-firmware/
    /bin-tools-py/      - Scripts de gestión de binarios
    /include/
        CnfTarjeta.h    - Fuente única de versión del firmware
    ...
```

---

## Archivos

### 1. `bin_build_config.py`

Contiene las rutas de configuración (relativas a `esp32-firmware/`):

- **HEADER_PATH**: Ruta al header `CnfTarjeta.h` desde donde se leen `SISTEMA_FIRMWARE`, `SISTEMA_VERSION` y `SISTEMA_ETAPA`.
- **OTA_DIR**: Carpeta destino del último build. Se sobreescribe en cada compilación.
- **RELEASES_DIR**: Carpeta de historial. Se crea una subcarpeta por versión (`v{VERSION}-{ETAPA}`).

---

### 2. `bin_build_manager.py`

Script principal que se ejecuta automáticamente al compilar. Realiza:

1. Parsea `CnfTarjeta.h` con regex para extraer `SISTEMA_FIRMWARE`, `SISTEMA_VERSION` y `SISTEMA_ETAPA`.
2. Genera el nombre del binario con el formato:
   ```
   {FIRMWARE}-v{VERSION}-{ETAPA}-T.DD.MM.YYYY.HHMMSS-ota.bin
   ```
   Ejemplo: `esp32.av-v0.1.0-beta.1-T.11.06.2026.153546HRS-ota.bin`
3. Elimina el `.bin` anterior en `ota/` y copia el nuevo.
4. Archiva una copia en `releases/v{VERSION}-{ETAPA}/`.
5. Genera `manifest.json` en ambas carpetas con los campos:
   ```json
   {
     "firmware": "esp32.av",
     "version": "0.1.0.beta.1",
     "fecha": "T.11.06.2026.153546HRS",
     "file": "esp32.av-v0.1.0-beta.1-T.11.06.2026.153546HRS-ota.bin"
   }
   ```

---

## Para cambiar la versión

Editar únicamente `include/CnfTarjeta.h`:

```c
#define SISTEMA_FIRMWARE "esp32.av"  // Nombre del firmware
#define SISTEMA_VERSION  "0.1.0"     // Versión semver
#define SISTEMA_ETAPA    "beta.1"    // Etapa: alpha, beta.1, rc.1, stable
```

El script toma los valores automáticamente en la siguiente compilación. No hay que modificar ningún otro archivo.

---

## Uso

1. Asegurarse de que `platformio.ini` incluya:
   ```ini
   extra_scripts = ./bin-tools-py/bin_build_manager.py
   ```
2. Compilar el proyecto:
   ```bash
   pio run
   ```
3. El `.bin` y `manifest.json` se generan automáticamente en `ota/` y `releases/`.

---

## Notas

- El campo `version` en `manifest.json` usa punto como separador (`0.1.0.beta.1`).
- El nombre del archivo usa guión entre versión y etapa (`v0.1.0-beta.1`).
- Los tags de git siguen la convención `v{VERSION}-{ETAPA}` (ej. `v0.1.0-beta.1`).
- La carpeta `ota/` siempre contiene solo el binario más reciente.
- La carpeta `releases/` acumula versiones y no se sobreescribe.
