#ifndef CONFIG_SISTEMA_H
#define CONFIG_SISTEMA_H

#define PROG_LOCAL 0 // 1: Programación local, 0: Programación ACCESS POINT

// ==============================================================================================
// SELECCIÓN DE VERSIÓN DE TARJETA - DESCOMENTA SOLO UNA OPCIÓN
// ==============================================================================================
// #define TARJETA_DDI // Tarjeta DDI
// #define TARJETA_MENIAM   // Tarjeta MENIAM
// #define TARJETA_NEGRA // Tarjeta NEGRA (default)
// #define TARJETA_MN_2_0 // Tarjeta MN 2.0
#define TARJETA_SL_DG // Tarjeta Salidas Digitales

// ==============================================================================================
// PINES COMUNES A TODAS LAS TARJETAS
// ==============================================================================================
#define BTN_PROG 0       // Pin del botón de programación
#define LED_STATUS 2     // Pin del LED de estado
#define LED_ST_ON 1      // Estado del LED encendido
#define LED_ST_OFF 0     // Estado del LED apagado
#define RF_RX 4          // Pin de recepción de señales RF
#define AMP_ON 1         // Estado del amplificador encendido
#define AMP_OFF 0        // Estado del amplificador apagado
#define PIN_ENT_DIG_1 35 // Pin de entrada digital 1
#define PIN_RELE 15      // Pin del relé

// ==============================================================================================
// CONFIGURACIÓN ESPECÍFICA POR TARJETA
// ==============================================================================================

#ifdef TARJETA_DDI
// TARJETA DDI
#define SALIDA_1 33        // Pin de salida 1
#define AMPLIFICADOR 32    // Pin de control del amplificador
#define PIN_MP3_UART_RX 16 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 17 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 27    // Pin de busy del MP3
#define LED_MP3 35         // Pin del LED del MP3
#define PIN_NEOPIXEL 25    // Pin de Neopixel
#define TARJETA_VERSION "DDI"

#elif defined(TARJETA_MENIAM)
// TARJETA MENIAM
#define SALIDA_1 33        // Pin de salida 1
#define AMPLIFICADOR 12    // Pin de control del amplificador
#define PIN_MP3_UART_RX 16 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 17 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 26    // Pin de busy del MP3
#define LED_MP3 32         // Pin del LED del MP3
#define PIN_NEOPIXEL 25    // Pin de Neopixel
#define TARJETA_VERSION "MENIAM"

#elif defined(TARJETA_NEGRA)
// TARJETA NEGRA (configuración por defecto)
#define SALIDA_1 33        // Pin de salida 1
#define AMPLIFICADOR 27    // Pin de control del amplificador
#define PIN_MP3_UART_RX 12 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 14 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 26    // Pin de busy del MP3
#define LED_MP3 32         // Pin del LED del MP3
#define PIN_NEOPIXEL 19    // Pin de Neopixel
#define TARJETA_VERSION "NEGRA"

#elif defined(TARJETA_MN_2_0)
// TARJETA MN 2.0
#define SALIDA_1 25 // Pin de salida 1

#define PIN_NEOPIXEL 19 // Pin de Neopixel

#define VOZ_ENABLED        // Habilitar funcionalidad de voz
#define AMPLIFICADOR 12    // Pin de control del amplificador
#define PIN_MP3_UART_RX 16 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 17 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 33    // Pin de busy del MP3
#define LED_MP3 32         // Pin del LED del MP3

#define TARJETA_VERSION "MN_2_0"

// LoRa RFM95 (comunes a todas las tarjetas)
#define PIN_LORA_MISO 19 // Pin MISO del LoRa
#define PIN_LORA_MOSI 23 // Pin MOSI del LoRa
#define PIN_LORA_SCK 18  // Pin SCK del LoRa
#define PIN_LORA_NSS 5   // Pin CS del LoRa
#define PIN_LORA_RST 13  // Pin de reset del LoRa
#define PIN_LORA_DIO0 26 // Pin DIO0 del LoRa

#elif defined(TARJETA_SL_DG)
// TARJETA SL DG
#define SALIDA_1 27 // 16 // Pin de salida 1
#define SALIDA_2 32 // 17 // Pin de salida 2

#define PIN_NEOPIXEL 19 // Pin de Neopixel

#define AMPLIFICADOR 16    // 27    // Pin de control del amplificador
#define PIN_MP3_UART_RX 12 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 14 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 26    // Pin de busy del MP3
#define LED_MP3 17         // 32         // Pin del LED del MP3

#define TARJETA_VERSION "SL_DG"

#else
// Si no se define ninguna tarjeta, usar TARJETA_NEGRA por defecto
#warning "No se ha seleccionado ninguna versión de tarjeta. Usando TARJETA_NEGRA por defecto."
#define SALIDA_1 27        // Pin de salida 1
#define AMPLIFICADOR 12    // Pin de control del amplificador
#define PIN_MP3_UART_RX 12 // Pin de recepción del UART1
#define PIN_MP3_UART_TX 14 // Pin de transmisión del UART1
#define PIN_MP3_BUSY 26    // Pin de busy del MP3
#define LED_MP3 32         // Pin del LED del MP3
#define PIN_NEOPIXEL 19    // Pin de Neopixel
#define TARJETA_VERSION "NEGRA_DEFAULT"
#endif

// ==============================================================================================
// VALIDACIÓN DE CONFLICTOS DE PINES
// ==============================================================================================
#if (PIN_MP3_UART_RX == AMPLIFICADOR)
#error "CONFLICTO: PIN_MP3_UART_RX y AMPLIFICADOR usan el mismo pin"
#endif

/* #if (PIN_NEOPIXEL == PIN_LORA_MISO)
#error "CONFLICTO: PIN_NEOPIXEL y PIN_LORA_MISO usan el mismo pin"
#endif */

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

#define TOPIC_SUS_1 "NODO/" // Topico de suscripción MQTT
#define TOPIC_SUS_2 "/CMD"
#define TOPIC_SUS_3 "/ACK"

#define TOPIC_PUB "CONTROLADOR/HB"        // Topico de publicación MQTT de heartbeat
#define TOPIC_STATUS "CONTROLADOR/STATUS" // Topico de publicación MQTT de heartbeat

#define MQTT_CNF_TM_HB_SG 60             // Tiempo de espera en segungos para el heartbeat MQTT
#define MQTT_CNF_TM_REV_LOOP_MS 500      // Tiempo de espera en milisegundos para el bucle MQTT
#define MQTT_CNF_TM_INT_RECONEXION 10000 // Tiempo de espera en milisegundos para la reconexión MQTT
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
