#include <LittleFS.h>
#include <Preferences.h>

#include <ConfigSistema.h>
#include "DataManager.h"
#include "LogSistema.h"
#include "Constantes.h"

DataManager Data; // Instancia global

// Definiciones de las variables estáticas
char DataManager::jsonLittleFS[2048];
char DataManager::jsonCMDSerial[2048];

DataManager::DataManager()
{
    // Inicializar los buffers de datos
    memset(jsonCMDSerial, 0, sizeof(jsonCMDSerial));
    memset(jsonLittleFS, 0, sizeof(jsonLittleFS));
    memset(numeroSerie, 0, sizeof(numeroSerie));
    pinDispositivo = 0;
    memset(perifericos, 0, sizeof(perifericos));
    timerAlarma = 0;
    tamper = 0;
}

// Inicializa LittleFS y otras configuraciones necesarias
bool DataManager::begin(void)
{

    LOG("\r\n\r\nDataManager init");

    cargarConfig(); // Cargar configuraciones desde NVS

    redesGuardadas = leerTodasLasRedesGuardadas(); // Leer todas las redes guardadas
    LOG("\r\n\r\n\r\nRedes guardadas: " + String(redesGuardadas.size()));
    for (const auto &red : redesGuardadas)
    {
        LOGF("\r\n\r\n%s\r\n%s", red.ssid.c_str(), red.password.c_str());
    }
    LOG("\r\n\r\n");

    beginCnfCtrl();
    return true;
}

bool presion_btn_prog()
{
    uint8_t contador;

    contador = 0;
    if (digitalRead(BTN_PROG) == 0)
    {
        while (digitalRead(BTN_PROG) == 0)
        {
            delay(50);
            contador++;
            if (contador >= 60) // botón fue presionado
            {
                /* Serial.print("\r\n\r\nBTN"); */
                return true;
            }
        }
    }

    return false;
}

void DataManager::progSerial(void)
{
    unsigned long tiempoInicio = millis();
    bool ledEstado = LED_ST_ON;

    // Esperar hasta que se presione el botón o se agote el tiempo
    while (!presion_btn_prog())
    {
        if (millis() - tiempoInicio >= 2000)
        {
            return; // Salir si no se presiona el botón en 2 segundos
        }
    }

    LOG("\r\n\r\nDATA MANAGER progSerial\r\n\r\n");

    // Encender el LED de estado
    digitalWrite(LED_STATUS, LED_ST_ON);

    while (true)
    {
        // Leer comandos desde el puerto serie
        if (Serial.available())
        {
            String cmd = Serial.readString();
            if (cmd == "rst")
            {
                LOG("\r\n\r\nrestableciendo datos...");
                rstData();
                rstDataCnfCtrl();
            }
            else
            {
                CmdSerial(cmd);
            }
        }

        // Alternar el estado del LED cada 500 ms
        if (millis() - tiempoInicio >= 500)
        {
            tiempoInicio = millis();
            ledEstado = !ledEstado;
            digitalWrite(LED_STATUS, ledEstado);
        }

        delay(1); // Reducir la carga del procesador
    }
}

void DataManager::rstData(void)
{
    // Obtener la dirección MAC del ESP32
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);

    // Formatear los últimos 3 bytes de la MAC en un string hexadecimal
    char macSuffix[7];                                                              // 6 caracteres + null terminator
    snprintf(macSuffix, sizeof(macSuffix), "%02X%02X%02X", mac[3], mac[4], mac[5]); // por ejemplo: "A1B2C3"

    // LOGF("\r\nSufijo de la MAC: %s", macSuffix);

    setNumSerie(macSuffix);    // Usar el sufijo de la MAC como número de serie
    setClaveDispositivo(1234); // Clave de dispositivo
    setPerifericos(1, 1);      //    1 -> rf
    setPerifericos(2, 1);      //    2 -> wifi
    setPerifericos(3, 1);      //    3 -> voz
    setPerifericos(4, 0);      //    4 -> perifoneo
    setTimerAlarma(30);        // 30 segundos timer alarma
    setTamper(0);              // Tamper deshabilitado

    LOG("\r\n\r\nDatos de configuracion restablecidos a valores predeterminados.");
    LOGF("\r\nNumero de serie: %s", numeroSerie);
    LOGF("\r\nClave de dispositivo: %d", pinDispositivo);
    LOGF("\r\nPerifericos: %d, %d, %d, %d", perifericos[0], perifericos[1], perifericos[2], perifericos[3]);
    LOGF("\r\nTimer de alarma: %d", timerAlarma);
    LOGF("\r\nTamper: %d", tamper);

    if (!limpiarNamespace(NAME_SPACE_WIFI))
    {
        LOG("\r\nError al limpiar el espacio de config_control.");
    }
    else
    {
        guardarRedWiFi("REDLOCAL", "TLWN7200NDMX2148");        // Guardar red WiFi de ejemplo
        guardarRedWiFi("INTERNETLOCAL", "T3mp0r4l2512230014"); // Guardar red WiFi de ejemplo
    }

    rstDataCnfCtrl(); // Restablecer datos de configuración de controles

    LOG("\r\n\r\n\r\nr\nRST DATOS FINALIZADO\r\n\r\n");
}

/**
 * @brief Configura el número de serie del dispositivo.
 *
 * @param numeroSerie Cadena hexadecimal de hasta 10 dígitos.
 * @return true si la configuración fue exitosa, false si hubo un error.
 */
bool DataManager::setNumSerie(const char *numeroSerie)
{
    if (strlen(numeroSerie) > 10)
    {
        LOG("\r\nError: El número de serie excede los 10 caracteres.");
        return false;
    }
    for (size_t i = 0; i < strlen(numeroSerie); i++)
    {
        if (!isxdigit(numeroSerie[i]))
        {
            LOG("\r\nError: El número de serie contiene caracteres inválidos.");
            return false;
        }
    }
    strcpy(this->numeroSerie, numeroSerie);

    if (!nvsData.begin(NAME_SPACE_CNF, false))
    {
        LOG("\r\nError al abrir el espacio config_sistema");
        return false;
    }

    nvsData.putString("numeroSerie", this->numeroSerie);
    // Cerrar el espacio de nombres después de guardar los datos
    nvsData.end();

    return true;
}

/**
 * @brief Configura la clave del dispositivo.
 *
 * @param clave Número de 4 dígitos.
 * @return true si la configuración fue exitosa, false si hubo un error.
 */
bool DataManager::setClaveDispositivo(uint16_t clave)
{
    if (clave < 1 || clave > 9999)
    {
        LOG("\r\nError: La clave del dispositivo debe estar entre 1 y 9999.");
        return false;
    }
    this->pinDispositivo = clave;

    if (!nvsData.begin(NAME_SPACE_CNF, false))
    {
        LOG("\r\nError al abrir el espacio config_sistema");
        return false;
    }

    nvsData.putUInt("pinDispositivo", this->pinDispositivo);
    // Cerrar el espacio de nombres después de guardar los datos
    nvsData.end();

    return true;
}

/**
 * @brief Configura un parámetro del dispositivo.
 *
 * @param tipo Identificador del tipo de configuración (1 -> rf, 2 -> wifi, 3 -> voz, 4 -> perifoneo, 5 -> claveDispositivo).
 * @param valor Valor a configurar.
 * @return true si la configuración fue exitosa, false si hubo un error.
 */
bool DataManager::setPerifericos(uint8_t tipo, uint8_t valor)
{
    // Constantes para nombres de los parámetros en NVS
    const char *nombres[] = {"cnfRF", "cnfWIFI", "cnfVOZ", "confPER"};

    // Validar tipo
    if (tipo > 4)
    {
        LOG("\r\nError: Tipo de configuración desconocido.");
        return false;
    }

    // Validar valor
    if (valor > 2)
    {
        LOG("\r\nError: El valor de configuración debe estar entre 0 y 2.");
        return false;
    }

    // Asegúrate de cerrar cualquier espacio de nombres abierto antes de abrir uno nuevo
    if (!nvsData.begin(NAME_SPACE_CNF, false))
    {
        LOG("\r\nError al abrir el espacio config_sistema");
        return false;
    }

    // Configurar periférico
    this->perifericos[tipo - 1] = valor;
    nvsData.putUChar(nombres[tipo - 1], valor);

    // Cerrar el espacio de nombres después de guardar los datos
    nvsData.end();

    return true;
}

/**
 * @brief Configura el temporizador de alarma del dispositivo.
 *
 * @param timer Valor del temporizador de alarma (0-600).
 * @return true si la configuración fue exitosa, false si hubo un error.
 */
bool DataManager::setTimerAlarma(uint16_t timer)
{
    if (timer > 600)
    {
        LOG("\r\nError: El valor del temporizador de alarma debe estar entre 0 y 600.");
        return false;
    }
    this->timerAlarma = timer;

    // Asegúrate de cerrar cualquier espacio de nombres abierto antes de abrir uno nuevo
    if (!nvsData.begin(NAME_SPACE_CNF, false))
    {
        LOG("\r\nError al abrir el espacio config_sistema.");
        return false;
    }

    nvsData.putUInt("timerAlarma", this->timerAlarma);

    // Cerrar el espacio de nombres después de guardar los datos
    nvsData.end();

    return true;
}

/**
 * @brief Configura la habilitación del tamper del dispositivo.
 *
 * @param habilitacion Valor de habilitación del tamper (0 o 1).
 * @return true si la configuración fue exitosa, false si hubo un error.
 */
bool DataManager::setTamper(uint8_t habilitacion)
{
    if (habilitacion > 1)
    {
        LOG("\r\nError: El valor de habilitación del tamper debe ser 0 o 1.");
        return false;
    }
    this->tamper = habilitacion;

    // Asegúrate de cerrar cualquier espacio de nombres abierto antes de abrir uno nuevo
    if (!nvsData.begin(NAME_SPACE_CNF, false))
    {
        LOG("\r\nError al abrir el espacio de nombres.");
        return false;
    }

    nvsData.putUChar("enTamper", this->tamper);

    // Cerrar el espacio de nombres después de guardar los datos
    nvsData.end();

    return true;
}