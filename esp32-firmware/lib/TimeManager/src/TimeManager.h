#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <ezTime.h>

/**
 * @brief Enumeración de estados posibles para la sincronización NTP
 */
enum NTPStatus {
    NTP_SUCCESS,           // Sincronización exitosa con NTP
    NTP_FAIL_LOCAL_TIME,   // Fallo NTP, usando tiempo local del ESP32
    NTP_WRONG_TIMEZONE,    // NTP sincronizado pero timezone incorrecto
    NTP_CLOCK_DRIFT,       // Reloj interno del ESP32 desincronizado
    NTP_NOT_INITIALIZED    // TimeManager no ha sido inicializado
};

/**
 * @brief Clase singleton para manejo centralizado de tiempo con ezTime
 * 
 * Proporciona una interfaz única para acceso a tiempo sincronizado con NTP,
 * manejo de errores y estados de sincronización para todo el proyecto.
 */
class TimeManager {
private:
    static TimeManager* instance;   // Instancia única del singleton
    Timezone myTZ;                  // Objeto ezTime para timezone CDMX
    bool initialized;               // Estado de inicialización
    NTPStatus current_status;       // Estado actual de sincronización
    unsigned long last_sync_time;   // Última vez que se sincronizó exitosamente
    unsigned long sync_check_interval; // Intervalo para verificar sincronización (ms)
    
    /**
     * @brief Constructor privado para patrón singleton
     */
    TimeManager();

    /**
     * @brief Evalúa el estado actual del tiempo y actualiza variables públicas
     */
    void updateTimeStatus();

    /**
     * @brief Convierte código de estado NTP a string descriptivo
     * @param status Código de estado NTP
     * @return String descriptivo del estado
     */
    String statusToString(NTPStatus status);

    /**
     * @brief Verifica si el tiempo actual es razonable (no está en 1970 o futuro lejano)
     * @return true si el tiempo parece válido
     */
    bool isTimeReasonable();

public:
    // Variables públicas para acceso directo
    String tm;              // Timestamp actual en formato ISO8601
    String ntp_status;      // Estado textual del NTP ("success", "fail_local_time", etc.)
    bool time_valid;        // Indica si el tiempo es confiable para operaciones críticas

    /**
     * @brief Obtiene la instancia única del TimeManager (patrón singleton)
     * @return Referencia a la instancia única
     */
    static TimeManager& getInstance();

    /**
     * @brief Inicializa ezTime con timezone de Ciudad de México
     * @param timeout_seconds Tiempo máximo de espera para sincronización NTP
     * @return true si la inicialización fue exitosa
     */
    bool init(uint8_t timeout_seconds = 60);

    /**
     * @brief Obtiene timestamp actual en formato ISO8601
     * @return String con formato "2025-08-29T00:30:45-06:00" o estado de error
     */
    String getTimeISO8601();

    /**
     * @brief Obtiene timestamp actual como const char*
     * @return Puntero a string con timestamp ISO8601
     */
    const char* getTimeISO8601_cstr();

    /**
     * @brief Obtiene hora local en formato legible
     * @param format Formato de tiempo (por defecto "H:i:s")
     * @return String con hora formateada
     */
    String getLocalTime(const String& format = "H:i:s");

    /**
     * @brief Obtiene fecha actual en formato ISO
     * @return String con formato "2025-08-29"
     */
    String getDate();

    /**
     * @brief Verifica si el TimeManager está inicializado
     * @return true si está inicializado
     */
    bool isInitialized() const;

    /**
     * @brief Verifica si el tiempo es confiable para logging crítico
     * @return true si el tiempo es confiable
     */
    bool isTimeReliable() const;

    /**
     * @brief Fuerza una nueva sincronización con servidor NTP
     * @return true si la sincronización fue exitosa
     */
    bool forceSync();

    /**
     * @brief Obtiene el código de estado actual
     * @return Código de estado NTP actual
     */
    NTPStatus getStatusCode() const;

    /**
     * @brief Actualiza estado y retorna información para debugging
     * @return String con información detallada del estado
     */
    String getDebugInfo();
};

#endif // TIME_MANAGER_H