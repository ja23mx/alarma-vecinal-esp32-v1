#include <littleFS.h>
#include <ArduinoJson.h>

#include "LogSistema.h"
#include "DataManager.h"
#include "DataManager.h"
#include "LogSistema.h"

/**
 * @brief Genera un listado de todos los archivos en LittleFS y llena un vector con las rutas completas.
 *        El resultado se almacena en el vector `rutasArchivos`.
 * @param salida_datos 1: Imprimir en Serial, 2: Almacenar en un buffer interno.
 */
void DataManager::listaJsonLF(uint8_t salida_datos)
{
    // Limpiar el vector antes de llenarlo
    rutasArchivos.clear();

    // Ajustar el tamaño del JSON dinámico según el tamaño esperado
    DynamicJsonDocument doc(4096); // Ajusta el tamaño según el JSON esperado
    JsonArray arr = doc.createNestedArray("archivos");

    // Abrir el directorio raíz
    File root = LittleFS.open("/", "r");
    if (!root)
    {
        LOG("\r\nError al abrir el directorio raíz de LittleFS");
        return;
    }
    if (!root.isDirectory())
    {
        LOG("\r\nEl archivo raíz no es un directorio");
        root.close();
        return;
    }

    // Llamar al método recursivo para listar archivos y directorios
    listarArchivosRec(root, arr, "");
    root.close();

    // Serializar el JSON generado en el buffer interno (opcional)
    if (salida_datos == 2)
    {
        memset(jsonLittleFS, 0, sizeof(jsonLittleFS));
        serializeJson(doc, jsonLittleFS, sizeof(jsonLittleFS));
    }

    // Procesar el JSON para llenar el vector de rutas completas
    procesarJsonRec(arr, "");

    //LOG("\r\n\r\nListado de archivos en LittleFS (Formato legible):\r\n");
    //serializeJsonPretty(doc, Serial);

    // Imprimir el JSON en formato legible si se solicita
    if (salida_datos == 1)
    {
        LOG("\r\n\r\nListado de archivos en LittleFS (Formato legible):\r\n");
        serializeJsonPretty(doc, Serial);
        Serial.println(); // Agregar un salto de línea después del JSON
    }
}

/**
 * @brief Función recursiva que recorre un directorio y agrega a un arreglo JSON la información de los archivos.
 *        También llena el vector `rutasArchivos` con las rutas completas de los archivos.
 * @param dir Directorio a recorrer.
 * @param arr Arreglo JSON donde se agregará la información.
 * @param rutaBase Ruta base acumulada durante la recursión.
 */
void DataManager::listarArchivosRec(File dir, JsonArray &arr, const String &rutaBase)
{
    File file = dir.openNextFile();
    while (file)
    {
        String nombre = file.name();
        String rutaCompleta = rutaBase + "/" + nombre;

        if (file.isDirectory())
        {
            // Crear un objeto JSON para el directorio
            JsonObject dirObj = arr.createNestedObject();
            dirObj["nombre"] = nombre;
            dirObj["tipo"] = "directorio";

            // Crear un arreglo para los archivos dentro del directorio
            JsonArray subArr = dirObj.createNestedArray("archivos");

            // Llamada recursiva para listar subdirectorios
            listarArchivosRec(file, subArr, rutaCompleta);
        }
        else
        {
            // Crear un objeto JSON para el archivo
            JsonObject fileObj = arr.createNestedObject();
            fileObj["nombre"] = nombre;
            fileObj["tamanio"] = file.size();

            // Agregar la ruta completa al vector de rutas
            rutasArchivos.push_back(rutaCompleta);
            //LOGF("\r\nArchivo encontrado: %s", rutaCompleta.c_str());
        }

        file.close();
        file = dir.openNextFile();
    }
    file.close();
}

/**
 * @brief Método recursivo que procesa un arreglo JSON y llena el vector de rutas completas.
 * @param arr Arreglo JSON a procesar.
 * @param rutaBase Ruta base acumulada durante la recursión.
 */
void DataManager::procesarJsonRec(const JsonArray &arr, const String &rutaBase)
{
    for (JsonObject obj : arr)
    {
        String nombre = obj["nombre"].as<String>();
        String rutaCompleta = rutaBase + "/" + nombre;

        // Si es un archivo, agregar la ruta completa al vector
        if (!obj["tipo"].is<const char *>())
        {
            rutasArchivos.push_back(rutaCompleta);
            LOGF("\r\nArchivo encontrado: %s", rutaCompleta.c_str());
        }

        // Si es un directorio, procesar recursivamente
        if (obj["tipo"] == "directorio")
        {
            JsonArray subArchivos = obj["archivos"];
            if (subArchivos)
            {
                procesarJsonRec(subArchivos, rutaCompleta);
            }
        }
    }
}

/**
 * @brief Verifica si un archivo existe en el vector de rutas completas.
 * @param ruta Ruta completa del archivo a buscar.
 * @return true si el archivo existe, false en caso contrario.
 */
bool DataManager::archivoExiste(const String &ruta)
{
    for (const String &archivo : rutasArchivos)
    {
        if (archivo == ruta)
        {
            return true;
        }
    }
    return false;
}