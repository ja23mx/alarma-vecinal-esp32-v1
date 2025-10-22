#ifndef ESTRUCTURAS_GLOBALES_H
#define ESTRUCTURAS_GLOBALES_H

#include <Arduino.h> // Para String y otras funciones de Arduino
#include <stdint.h>  // Para uint16_t y otros tipos de enteros de tamaño fijo
#include <string>    // Para std::string
#include <vector>    // Para std::vector

/*
 * Estructura que contiene el estado del reproductor de MP3
 */
struct STATE_MP3
{
    /**
     *    @brief Estado de la comunicación con el DFPlayer
     *    0: No hay comunicación
     *    1: Comunicación establecidada
     */
    bool conexion;
    
    /**
     *    @brief Estado de la inicialización de la pista
     *    0: No se ha inicializado
     *    1: Inicializado
     */
    bool pistaInit;

    /**
     *    @brief Estado de reproducción de la pista
     *    false: No ha terminado reproducción o error
     *    true: Pista terminada exitosamente
     */
    bool pistaReprodOk;

    /*
     *    @brief Estado de la reproducción
     *    0: No se ha iniciado la reproducción
     *    1: Reproducción iniciada
     */
    bool initReproduccion;

    /**
     *    @brief Estado de la MicroSD
     *    false: No hay MicroSD
     *    true: MicroSD detectada
     */
    bool microSD;

    /*
     *    @brief MP3 configurado
     *    0: No se ha configurado el MP3
     *   1: MP3 configurado
     */
    bool Configurado;

    /**
     *    @brief Pista actual que se está reproduciendo
     *    0: Ninguna pista
     *    >0: Número de pista actual
     */
    uint16_t pistaActual;

    /**
     *    @brief Estado del amplificador
     *    false: Amplificador apagado
     *    true: Amplificador encendido
     */
    bool amplificadorActivo;
};

/**
    @brief Estructura que contiene la respuesta de la comparación de una señal RF
*/
struct RF_COMP_AV
{
    /**
     *    @brief Configuracion del control encontrado
     *    1: Dispositivo de Alarma Vecinal
     *    2: Dispositivo de Alerta Sonora & Mensaje
     *    3: Dispositivo de Integrador
     */
    uint8_t configuracion;

    /**
     *    @brief Status del control encontrado
     *    0: Control Deshabilitado
     *    1: Control Habilitado
     */
    uint8_t status;
    /**
     *    @brief Número del control encontrado
     *    Inicia en 1
     */
    uint16_t control;
    /**
     *    @brief Número del botón encontrado
     *    Inicia en 1
     */
    char btnStr[10];

    /**
     *    @brief Indice del botón encontrado
     *    Inicia en 0
     */
    uint8_t btnIndice;
    /**
     *    @brief Indica si se encontró una coincidencia
     *    0: No se encontró coincidencia
     *    1: Se encontró coincidencia
     */
    bool valido;
};

#define MAX_BOTONES 10
struct CTRL_MODELO_AV
{
    /**
     *    @brief Modelo de control
     *    Nombre del modelo de control, por ejemplo: "EVAV4TM1", es unico para cada control
     */
    char modelo[30];

    char nombre[30]; // Nombre del modelo, por ejemplo: "CTRL BLANCO 4T AV M1", es unico para cada control

    /**
     *    @brief Modelo de control
     *    Nombre del codificador, por ejemplo: "EV1527", diferentes modelos pueden tener el mismo nombre pero diferente orden de botones,
     *    entre otras configuraciones
     *    @note Este campo es opcional y puede ser utilizado para identificar el tipo de codificador utilizado en el control.
     */
    char codificador[30];

    /**
     *    @brief Configuracion del modelo de dispositivo RF
     *    1: Dispositivo de Alarma Vecinal
     *    2: Dispositivo de Alerta Sonora & Mensaje
     *    3: Dispositivo de Integrador
     */
    uint8_t configuracion;

    /**
     *    @brief ID de configuracion de control
     *    Identificador unico que inicia en "A" y se incrementa en 1 por cada control, lo asigna el sistema
     */
    char id_cnf_ctrl;

    /**
     *    @brief Bits de limpieza
     *    ejemplo: si bits_limpieza = 5, se limpian los 5 bits menos significativos
     *    1111 1111 1111 1111 1111 1111 1111 1111 -> 4294967295   0xFFFFFFFF
     *    0000 0000 0000 0000 0000 0000 0000 0111 -> 7            0x00000007
     *    1111 1111 1111 1111 1111 1111 1111 1000 -> 4294967288   0xFFFFFFF8
     */
    uint8_t bits_limpieza;
    uint8_t numero_botones;
    uint8_t boton_desactivacion;

    /**
     *    @brief Valores que se suma al valor guardado en la base de datos, por ejemplo:
     *    si el valor guardado en la base de datos es 1000 y el valor de suma[0] es 8, el valor a comparar es 1008
     *    se suman cada uno de los valores de suma_botones a la señal base para saber que boton se presiono, si
     *    tenemos los valores 8, 4, 2 y 1; el resultado es 1000 + 8 = 1008, 1000 + 4 = 1004, 1000 + 2 = 1002 y 1000 + 1 = 1001
     */
    uint8_t suma_botones[MAX_BOTONES];
    char *valor_botones[MAX_BOTONES]; // numero de boton

    /**
     * @brief Tipos de botones disponibles en el sistema.
     * @note Los botones tienen diferentes configuraciones según el tipo de dispositivo:
     *       - 0: Deshabilitado
     *       - 1: Silencioso (AV)
     *       - 2: Alarma vecinal (AV)
     *       - 3: Una pista (AV)
     *       - 4: Una pista (Alerta Son/Integ)
     *       - 5: Pista & Mensaje (Alerta Son/Integ)
     *       - 6: Solo Mensaje (Alerta Son/Integ)
     *       - 7: Init Programación (Integrador)
     *       - 8: Tamper On/Off (Integrador)
     *       - 9: Reset (Integrador)
     */
    uint8_t tipo_botones[MAX_BOTONES];

    // ***************** ALARMA VECINAL ********************
    /// valores de botones AV de una pista /////////////////
    uint16_t av_una_pista_lista[MAX_BOTONES]; // lista de pistas a reproducir
    /////////////////////////////////////////////////////

    // valores de botones de alarma vecinal  //////////////
    uint8_t av_tamano_ciclo_btn_av;             // numero de pistas a reproducir
    int16_t av_lista_ciclo_btn_av[MAX_BOTONES]; // lista de pistas a reproducir
    uint16_t av_pst_us_init;                    // init indice de pista de usuario

    /**
        @brief Lista de pistas a reproducir en funcion de boton
        @note
        0: pista de usuario + control
        -1: pista de alarma vecinal
        > 0: pista de alarma vecinal
    */
    uint16_t av_lista_pst_variable[MAX_BOTONES];
    ///////////////////////////////////////////////////////
    // ****************************************************

    // ***************** Alerta Sonora ********************

    /**
     *   @brief Lista de pistas a reproducir en funcion de boton
     *   @details
     *   Debe de llenarse con el total de botones disponibles, en caso de no utilizarse
     *   se llenara con 0, por ejemplo, si el control tiene 4 botones: 1) 4 (Una pista), 2) 4 (Una Pista),
     *   3) 5 (Pista y Mensaje), 4) 6 (Solo Mensaje). La matriz debe tener x, x, x, 0
     */
    uint16_t as_lista_pst_gral[MAX_BOTONES];

    // ****************************************************
    // ****************************************************

    // ***************** INTEGRADOR ********************
    uint16_t int_una_pista_lista[MAX_BOTONES]; // lista de pistas a reproducir
    uint8_t int_ini_prog_pista;                // pista de inicializacion de programacion
    uint8_t int_fin_prog_pista;                // pista de inicializacion de programacion
    uint8_t int_tamper_off_pista;              // pista de tamper off
    uint8_t int_tamper_on_pista;               // pista de tamper on
    uint8_t int_reset_pista;                   // pista de reset
    // ****************************************************
    // ****************************************************
};

struct ESTADO_ALARMA
{
    /**
     *    @brief Estado de la alarma
     *    0: Alarma desactivada
     *    1: Alarma activada
     */
    bool alarma_on = false;

    /*
     *    @brief Indica si la alarma fue activada por un comando
     *    0: Alarma no activada por un comando
     *    1: Alarma activada por un comando
     */
    bool onCmd = false;

    /**
     *    @brief Variable global para indicar si la alarma está disponible para activacion o desactivacion
     *    0: Alarma no disponible
     *    1: Alarma disponible
     */
    bool alarma_disponible = true;

    /**
     *    @brief Indica si la alarma está reproduciendo una pista de audio, es diferente a que
     *           haya sido activada por un control RF o por la app movil
     *           Esta variable se utiliza para evitar que se confunda la activación de la alarma
     *           con la reproducción de una pista de audio. Se utiliza para evitar que se confunda
     *           la activación de la alarma con la reproducción de una pista de audio.
     *   0: No está reproduciendo
     *   1: Está reproduciendo
     */
    bool alarma_audio_play = false;

    /**
     *    @brief Reproducción de una pista de audio
     */
    uint16_t audio1Pista;

    /**
     *    @brief Tipo de emergencia
     *    1: Control RF alarma vecinal
     *    2: App movil
     *    3: reproducción de una pista de audio
     */
    uint8_t origenEmg = 0;

    String appEmgMqtt; // Mensaje de emergencia recibido por MQTT

    /**
     *    @brief Número de emergencia, puede ser de un control RF o de la app movil, entre otros
     *    Control RF alarma vecinal
     *    1: Emergencia 1
     *        . . .
     *    n: Emergencia n
     *    App movil
     *    1: Emergencia 1
     *        . . .
     *    n: Emergencia n
     */
    uint8_t numEmergencia = 0;

    uint8_t numPistas;                         // Número de pistas de emergencia
    uint16_t listaPistas[5] = {0, 0, 0, 0, 0}; // Lista de pistas de emergencia

    uint16_t ctrlNum = 0; // Número de control que activo la alarma
    uint8_t ctrlBtn = 0;  // Botón que activo la alarma
};

struct ESCANEO_STATUS
{
    uint8_t redesTotales;           // Número total de redes detectadas
    std::vector<String> ssid;       // Lista de nombres de redes detectadas
    std::vector<int> rssi;          // Intensidad de señal en dBm para cada red
    std::vector<uint8_t> rssiNivel; // Nivel de señal categorizado (1 = fuerte, 2 = media, 3 = débil)
    std::vector<uint8_t> seguridad; // Tipo de seguridad de la red (0 = abierta, 1 = WEP, 2 = WPA, etc.)
    std::vector<uint8_t> canal;     // Canal en el que opera cada red detectada
    uint8_t canalAP;                // Canal menos saturado recomendado para crear la red
};

struct CMD_DATA
{
    String modelo;
    String numeroSerie;
    String comando;
};

struct DATOS_INSTRUCCION
{
    String cmd;
    String numPeriferico;
    String parametros;
};

// Enum para identificar el origen del comando
enum class CmdOrigen
{
    SRV_EXT,     // respuesta en formato JSON. Para comandos MQTT
    SRV_LOC_CMD, // respuesta en formato JSON. Consola web
    CONSOLA,     // respuesta en formato JSON. Para imprimir en consola
    INTERNAL,    // Para acciones generadas internamente.
    D2D          // respuesta en formato compacto/binario, no JSON (ej. LoRa)
};

struct SettingsResponse
{
    uint8_t error; // 0 = error, 1 = ok, 2 = advertencia
    String cmd;    // Comando que se ejecutó
};
#endif // ESTRUCTURAS_GLOBALES_H
