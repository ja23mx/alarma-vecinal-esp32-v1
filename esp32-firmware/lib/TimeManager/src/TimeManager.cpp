#include "TimeManager.h"

// Inicialización de la instancia estática
TimeManager *TimeManager::instance = nullptr;

/**
 * @brief Constructor privado del singleton TimeManager
 */
TimeManager::TimeManager()
    : initialized(false),
      current_status(NTP_NOT_INITIALIZED),
      last_sync_time(0),
      sync_check_interval(30000), // 30 segundos
      tm("1970-01-01T00:00:00Z"),
      ntp_status("not_initialized"),
      time_valid(false)
{
    // Constructor vacío - inicialización en init()
}

/**
 * @brief Obtiene la instancia única del TimeManager
 */
TimeManager &TimeManager::getInstance()
{
    if (!instance)
    {
        instance = new TimeManager();
    }
    return *instance;
}

/**
 * @brief Inicializa ezTime con timezone fijo GMT-6 para México
 */
bool TimeManager::init(uint8_t timeout_seconds)
{
    if (initialized)
    {
        return true; // Ya está inicializado
    }

    Serial.println("[TimeManager] Iniciando sincronización con NTP...");

    // Configurar timezone FIJO GMT-6 para México usando formato POSIX
    // CST-6: CST = Central Standard Time, -6 = GMT-6 (sin DST)
    bool posix_success = myTZ.setPosix("CST6");
    
    if (posix_success) {
        Serial.println("[TimeManager] Configurado timezone fijo GMT-6 para México (sin DST)");
    } else {
        Serial.println("[TimeManager] ERROR: No se pudo configurar timezone POSIX");
        return false;
    }

    // Esperar sincronización con timeout - función GLOBAL de ezTime
    bool sync_success = waitForSync(timeout_seconds);

    if (sync_success)
    {
        initialized = true;
        current_status = NTP_SUCCESS;
        last_sync_time = millis();
        Serial.println("[TimeManager] Sincronización NTP exitosa");
    }
    else
    {
        // Fallo en sincronización - usar tiempo local del ESP32
        initialized = true; // Marcamos como inicializado pero con error
        current_status = NTP_FAIL_LOCAL_TIME;
        Serial.println("[TimeManager] Fallo en sincronización NTP - usando tiempo local");
    }

    // Actualizar estado y variables públicas
    updateTimeStatus();

    return sync_success;
}

/**
 * @brief Obtiene timestamp actual en formato ISO8601
 */
String TimeManager::getTimeISO8601()
{
    if (!initialized)
    {
        tm = "1970-01-01T00:00:00Z";
        ntp_status = "not_initialized";
        time_valid = false;
        return tm;
    }

    // Verificar si necesitamos actualizar el estado
    if (millis() - last_sync_time > sync_check_interval)
    {
        updateTimeStatus();
    }

    // Generar timestamp según el estado actual
    if (timeStatus() == timeSet)
    {
        tm = myTZ.dateTime(ISO8601);
        time_valid = true;
    }
    else
    {
        // Usar tiempo local del ESP32 con advertencia
        tm = myTZ.dateTime(ISO8601);
        time_valid = false;

        // Verificar casos específicos de error
        if (!isTimeReasonable())
        {
            if (tm.startsWith("1970"))
            {
                current_status = NTP_FAIL_LOCAL_TIME;
            }
            else
            {
                current_status = NTP_CLOCK_DRIFT;
            }
        }
    }

    // Actualizar string de estado
    ntp_status = statusToString(current_status);

    return tm;
}

/**
 * @brief Obtiene timestamp actual como const char*
 */
const char *TimeManager::getTimeISO8601_cstr()
{
    getTimeISO8601(); // Actualiza tm
    return tm.c_str();
}

/**
 * @brief Obtiene hora local en formato legible
 */
String TimeManager::getLocalTime(const String &format)
{
    if (!initialized)
    {
        return "00:00:00";
    }

    return myTZ.dateTime(format);
}

/**
 * @brief Obtiene fecha actual en formato ISO
 */
String TimeManager::getDate()
{
    if (!initialized)
    {
        return "1970-01-01";
    }

    return myTZ.dateTime("Y-m-d");
}

/**
 * @brief Verifica si el TimeManager está inicializado
 */
bool TimeManager::isInitialized() const
{
    return initialized;
}

/**
 * @brief Verifica si el tiempo es confiable para logging crítico
 */
bool TimeManager::isTimeReliable() const
{
    return time_valid && (current_status == NTP_SUCCESS);
}

/**
 * @brief Fuerza una nueva sincronización con servidor NTP
 */
bool TimeManager::forceSync()
{
    if (!initialized)
    {
        return false;
    }

    Serial.println("[TimeManager] Forzando sincronización NTP...");

    // Intentar nueva sincronización - función GLOBAL de ezTime
    bool sync_success = waitForSync(15);

    if (sync_success)
    {
        current_status = NTP_SUCCESS;
        last_sync_time = millis();
        Serial.println("[TimeManager] Resincronización exitosa");
    }
    else
    {
        current_status = NTP_FAIL_LOCAL_TIME;
        Serial.println("[TimeManager] Fallo en resincronización");
    }

    updateTimeStatus();
    return sync_success;
}

/**
 * @brief Obtiene el código de estado actual
 */
NTPStatus TimeManager::getStatusCode() const
{
    return current_status;
}

/**
 * @brief Actualiza estado y retorna información para debugging
 */
String TimeManager::getDebugInfo()
{
    updateTimeStatus();

    String debug_info = "[TimeManager Debug]\n";
    debug_info += "Initialized: " + String(initialized ? "true" : "false") + "\n";
    debug_info += "Current Time: " + tm + "\n";
    debug_info += "NTP Status: " + ntp_status + "\n";
    debug_info += "Time Valid: " + String(time_valid ? "true" : "false") + "\n";
    debug_info += "Last Sync: " + String((millis() - last_sync_time) / 1000) + "s ago\n";
    debug_info += "Time Status: " + String(timeStatus() == timeSet ? "synced" : "not_synced") + "\n";

    return debug_info;
}

/**
 * @brief Evalúa el estado actual del tiempo y actualiza variables públicas
 */
void TimeManager::updateTimeStatus()
{
    if (!initialized)
    {
        current_status = NTP_NOT_INITIALIZED;
        return;
    }

    // Verificar si ezTime está sincronizado
    if (timeStatus() != timeSet)
    {
        current_status = NTP_FAIL_LOCAL_TIME;
        return;
    }

    // Verificar si el tiempo es razonable
    if (!isTimeReasonable())
    {
        String current_time = myTZ.dateTime(ISO8601);
        if (current_time.startsWith("1970"))
        {
            current_status = NTP_FAIL_LOCAL_TIME;
        }
        else
        {
            current_status = NTP_CLOCK_DRIFT;
        }
        return;
    }

    // Verificar timezone fijo GMT-6 para México
    String current_time = myTZ.dateTime(ISO8601);
    
    // Con offset fijo, debe ser siempre GMT-6 (-0600)
    if (!current_time.endsWith("-0600"))
    {
        current_status = NTP_WRONG_TIMEZONE;
        Serial.println("[TimeManager] ERROR: Timezone incorrecto");
        Serial.println("[TimeManager] Esperado: GMT-6 (-0600), Actual: " + current_time.substring(current_time.length() - 6));
        return;
    }

    // Con offset manual no hay DST - remover verificación de isDST()
    
    // Si llegamos aquí, todo está bien
    current_status = NTP_SUCCESS;
}

/**
 * @brief Convierte código de estado NTP a string descriptivo
 */
String TimeManager::statusToString(NTPStatus status)
{
    switch (status)
    {
    case NTP_SUCCESS:
        return "success";
    case NTP_FAIL_LOCAL_TIME:
        return "fail_local_time";
    case NTP_WRONG_TIMEZONE:
        return "wrong_timezone";
    case NTP_CLOCK_DRIFT:
        return "clock_drift";
    case NTP_NOT_INITIALIZED:
        return "not_initialized";
    default:
        return "unknown";
    }
}

/**
 * @brief Verifica si el tiempo actual es razonable
 */
bool TimeManager::isTimeReasonable()
{

    time_t current_time = myTZ.now();

    // Tiempo mínimo razonable: 1 de enero de 2020
    time_t min_reasonable_time = 1577836800; // 2020-01-01 00:00:00 UTC

    // Tiempo máximo razonable: 1 de enero de 2030
    time_t max_reasonable_time = 1893456000; // 2030-01-01 00:00:00 UTC

    return (current_time >= min_reasonable_time && current_time <= max_reasonable_time);
}