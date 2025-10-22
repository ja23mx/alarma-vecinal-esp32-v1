# Documentación de bin_build_manager.py y bin_build_config.py

## Descripción

Estos archivos automatizan la generación y gestión del archivo binario (.bin) del firmware ESP32, asegurando un nombre estructurado y la copia a una carpeta destino configurable.
Por defecto la creación del nuevo archivo sera en la locacion `/esp32-bin-files/dev-test`.

```

/esp32-bin-files/    - Binarios compilados organizados por versión
    /dev-test/       - Destino de archivos .bin generados, esta carpeta no se sube a git
    /v0.0.1/         - Firmware versión 0.0.1, esta carpeta si sube a git
    COMO-FLASHEAR.md - Guía de instalación con esptool y como subir archivos .bin

/esp32-firmware/     - Código fuente del firmware (PlatformIO) 
    /bin-tools-py/   - Scripts y herramientas para gestionar binarios y configuraciones de builds 
    ...

/examples
    ...

/docs/
    ...    

/meta
    ...

.gitignore           
OPEN_WORKSPACE.txt   
```

---

## Archivos

### 1. `bin_build_config.py`

Contiene las variables de configuración:
- **NAME_BASE**: Nombre base del firmware, por ejemplo, si el proyecto es: esp32-git-template-1 su nombre base es: esp32-GT-TMP-1.
- **VERSION**: Versión del firmware.
- **DEST_DIR**: Carpeta destino donde se copiará el archivo .bin generado.

Modifica estos valores para personalizar el nombre y la ubicación del archivo binario.

---

### 2. `bin_build_manager.py`

Script principal que:
- Lee las variables de configuración desde `bin_build_config.py`.
- Calcula el número de build incremental.
- Genera el nombre del archivo .bin con formato:  
  `esp32-GIT-TMP-1-V0.0.1-B-<build_number>-T-<timestamp>.bin`
- Copia el archivo .bin generado a la carpeta destino.
- Se ejecuta automáticamente como script extra en PlatformIO (`extra_scripts`).

---

## Uso

1. Configura los valores en `bin_build_config.py` según tus necesidades.
2. Asegúrate de que `platformio.ini` incluya la línea:
   ```
   extra_scripts = ./bin-tools-py/bin_build_manager.py
   ```
3. Al compilar el proyecto, el archivo .bin se generará y copiará automáticamente a la carpeta destino con el nombre estructurado.

---

## Notas

- El número de build se calcula de forma incremental leyendo y actualizando el valor almacenado en el archivo `build_number.txt` ubicado en la misma carpeta que los scripts, asegurando que cada compilación tenga un identificador único y consecutivo.
- Si cambias la versión, actualiza también la carpeta destino para mantener la organización.
- Ambos archivos deben permanecer en la carpeta `bin_tools_py` para mantener la estructura recomendada.

---