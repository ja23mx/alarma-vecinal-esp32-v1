#ifndef CONFIG_SISTEMA_H
#define CONFIG_SISTEMA_H

#define PROG_LOCAL 0 // 1: Programación local, 0: Programación ACCESS POINT

#define ENTORNO 1 // 1 = DEV, 2 = PROD

// ==============================================================================================
// SELECCIÓN DE VERSIÓN DE TARJETA - DESCOMENTA SOLO UNA OPCIÓN
// ==============================================================================================
// #define TARJETA_SLT_V_1_0 //
#define TARJETA_SLT_V_1_1 //

// ==============================================================================================
// PINES COMUNES A TODAS LAS TARJETAS
// ==============================================================================================
#define BTN_PROG 0 // Pin del botón de programación

// ==============================================================================================
// Configuración de pines específicos para cada versión de tarjeta - DESCOMENTA SOLO LA OPCIÓN CORRESPONDIENTE
// ==============================================================================================

#ifdef TARJETA_SLT_V_1_0
// TARJETA SLT V1.0
#define SALIDA_1 33        // Pin de salida 1
#define AMPLIFICADOR 12    // Pin de control del amplificador
#define PIN_MP3_UART_RX 16 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 17 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 35    // Pin de busy del MP3
#define LED_MP3 22         // Pin del LED del MP3
#define PIN_NEOPIXEL 14    // Pin de Neopixel
#define PIN_CNF_RED_ADC 39 // Pin de configuración de red
#define LED_STATUS 2       // Pin del LED de estado
#define LED_ST_ON 1        // Estado del LED encendido
#define LED_ST_OFF 0       // Estado del LED apagado
#define RF_RX 4            // Pin de recepción de señales RF
// --- Ethernet W5500 SPI ---
#define W5500_CS_PIN 5       // Chip Select W5500
#define W5500_RST_PIN 32     // Reset W5500 (GPIO libre en TARJETA_SLT_V1)
#define W5500_ENTROPY_PIN 36 // Pin ADC flotante para entropía SSLClient
#define TARJETA_VERSION "TARJETA_SLT_V_1_0"
#endif

#ifdef TARJETA_SLT_V_1_1
// TARJETA SLT V1.1
#define RF_RX 4              // Pin de recepción de señales RF
#define W5500_CS_PIN 5       // Chip Select W5500
#define AMPLIFICADOR 12      // Pin de control del amplificador
#define PIN_NEOPIXEL 14      // Pin de Neopixel
#define LED_MP3 19           // Pin del LED del MP3
#define PIN_MP3_BUSY 25      // Pin de busy del MP3
#define LED_STATUS 26        // Pin del LED de estado
#define PIN_MP3_UART_TX 27   // Pin de transmisión del UART1
#define SALIDA_1 28          // Pin de salida 1
#define PIN_MP3_UART_RX 32   // Pin de recepción del UART1
#define W5500_RST_PIN 33     // Reset W5500 (GPIO libre en TARJETA_SLT_V1)
#define W5500_ENTROPY_PIN 36 // Pin ADC flotante para entropía SSLClient
#define PIN_CNF_RED_ADC 39   // Pin de configuración de red

#define LED_ST_ON 1  // Estado del LED encendido
#define LED_ST_OFF 0 // Estado del LED apagado
#define TARJETA_VERSION "TARJETA_SLT_V_1_1"
#endif

// ==============================================================================================
// VALIDACIÓN DE CONFLICTOS DE PINES
// ==============================================================================================

// Configuración de parámetros BLE
#define PREFIJO_BLE "AV_"    // Prefijo de red BLE
#define TM_ESPERA_BLE 120000 // Tiempo de espera para BLE

#define CONT_ESP_OFF_ALARMA 5000 // Tiempo de espera para apagar la alarma
#define CONT_ESP_ON_ALARMA 3000  // Tiempo de espera para encender la alarma

#define AL_CTRL_TP_AV 1
#define AL_CTRL_TP_GUARDIAN 3
#define AL_CTRL_TP_INTEGRADOR 4
#define AL_EMG_MQTT 2
#define AL_PLAY_1_PISTA 5

// Mp3
#define VOZ_ENABLE
#define CONST_MP3_REPRODUCIENDO false
#define CONST_MP3_ESP_REP_MP3 2500
#define CONST_MP3_LIM_T_PISTA 60000

#define CONST_NEOPIXEL_LEDS 16

// MP3 AUDIOS SISTEMA
#define CONST_MP3_SYST_ERROR_AUDIO_404 2
#define CONST_MP3_SYST_PRUEBA 10
#define CONST_MP3_SYST_INIT_PROG 11
#define CONST_MP3_SYST_FIN_PROG 12
#define CONST_MP3_SYST_RST 13
#define CONST_MP3_SYST_WAKE_UP 14
#define CONST_MP3_SYST_WIFI_DESC 15
#define CONST_MP3_SYST_TAMPER_EN_0 16
#define CONST_MP3_SYST_TAMPER_EN_1 17
#define CONST_MP3_SYST_BATTERY_LOW 18
#define CONST_MP3_SYST_BATTERY_BACKUP_INIT 19
#define CONST_MP3_MC_SD_INSERTED 20

// CONSTANTES DE CONFIGURACION DE CONTROL

#define CTRL_LIMITE_CTRL 200

#define CTRL_CNF_AV 1
#define CTRL_CNF_ALERTA_SON_MSG 2
#define CTRL_CNF_INTEGRADOR 3

#define CTRL_TP_DP_LIMITE 3
#define CTRL_TP_DSP_RF_1 "Alarma Vecinal"
#define CTRL_TP_DSP_RF_2 "Alerta Sonora y Mensaje"
#define CTRL_TP_DSP_RF_3 "Integrador"

#define CTRL_TP_BOTON_DESHABILITADO 0
#define CTRL_TP_BOTON_SILENCIOSO 1
#define CTRL_TP_BOTON_AV 2
#define CTRL_TP_BOTON_UNA_PISTA 3
#define CTRL_TP_BOTON_UNA_PISTA_AS 4
#define CTRL_TP_BOTON_PISTA_MSG 5
#define CTRL_TP_BOTON_MSG 6
#define CTRL_TP_BOTON_INIT_PROG 7
#define CTRL_TP_BOTON_TAMPER_ON 8
#define CTRL_TP_BOTON_RST 9

///// MQTT  ///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// PREFIJO 1 - DEV, 2 - PROD

#if defined(ENTORNO) && ENTORNO == 1

// TOPICS DE SUSCRIPCION
#define TOPIC_SUS_1 "AV/DEV/NODO/" // Topico de suscripción MQTT
#define TOPIC_SUS_2 "/CMD/"
#define TOPIC_SUS_3 "/ACK/"

// TOPICS DE PUBLICACION
#define TOPIC_PUB "AV/DEV/CONTROLADOR/HB/"        // Topico de publicación MQTT de heartbeat
#define TOPIC_STATUS "AV/DEV/CONTROLADOR/STATUS/" // Topico de publicación MQTT de heartbeat

#elif defined(ENTORNO) && ENTORNO == 2

// TOPICS DE SUSCRIPCION
#define TOPIC_SUS_1 "AV/PROD/NODO/"
#define TOPIC_SUS_2 "/CMD/"
#define TOPIC_SUS_3 "/ACK/"

// TOPICS DE PUBLICACION
#define TOPIC_PUB "AV/PROD/CONTROLADOR/HB/"
#define TOPIC_STATUS "AV/PROD/CONTROLADOR/STATUS/"
#endif

#define MQTT_CNF_TM_HB_SG 60             // Tiempo de espera en segungos para el heartbeat MQTT
#define MQTT_CNF_TM_REV_LOOP_MS 500      // Tiempo de espera en milisegundos para el bucle MQTT
#define MQTT_CNF_TM_INT_RECONEXION 30000 // Tiempo de espera en milisegundos para la reconexión MQTT
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// SALIDAS ///////////////////////////////////////////////////////////////////////////////////////

#define GST_SALIDA_PWM_FREQUENCY 5000
#define GST_SALIDA_PWM_RESOLUTION 8
#define GST_SALIDA_PWM_MIN_DUTY_CYCLE 10
#define GST_SALIDA_FADE_OPCION 10
#define GST_SALIDA_TONO_OPCION 10
#define GST_SALIDA_MAX_PISTA_MP3 10000
#define GST_SALIDA_SD_MAX_TM_1_SG 43200 // maximo tiempo de espera en segundos para la salida 1

#define GST_SALIDA_NUM_PRF 4 // Numero de salidas de la alarma
//////////////////////////////////////////////////////////////////////////////////////////////////
#endif // CONFIG_SISTEMA_H
