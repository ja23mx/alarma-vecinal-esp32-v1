#ifndef CONFIG_SISTEMA_H
#define CONFIG_SISTEMA_H

#define PROG_LOCAL          0   // 1: Programación local, 0: Programación ACCESS POINT

// Definición de pines
#define BTN_PROG            0   // Pin del botón de programación
#define LED_STATUS          2 
#define LED_ST_ON           1
#define LED_ST_OFF          0
#define RF_RX               4   // Pin de recepción de señales RF
#define AMPLIFICADOR       12   // Pin de control del amplificador 
#define AMP_ON              1   // Estado del amplificador encendido
#define AMP_OFF             0   // Estado del amplificador apagado
#define SALIDA_1           27   // Pin de salida 1   

// MP3 //
#define PIN_MP3_UART_RX    12   // Pin de recepción del UART1
#define PIN_MP3_UART_TX    14   // Pin de transmisión del UART1
#define PIN_MP3_BUSY       26   // Pin de busy del MP3

#define PIN_ENT_DIG_1      35   // Pin de entrada digital 1
#define LED_MP3            32   // Pin del LED del MP3
#define PIN_RELE           15   // Pin del relé
#define PIN_NEOPIXEL       19   // Pin de Neopixel

// LoRa RFM95
#define PIN_LORA_MISO      19   // Pin MISO del LoRa
#define PIN_LORA_MOSI      23   // Pin MOSI del LoRa
#define PIN_LORA_SCK       18   // Pin SCK del LoRa
#define PIN_LORA_NSS        5   // Pin CS del LoRa
#define PIN_LORA_RST       13   // Pin de reset del LoRa
#define PIN_LORA_DIO0      26   // Pin DIO0 del LoRa

/* 
PINES TARJETA DDI 
SALIDA_1        33
AMPLIFICADOR    32 

PINES TARJETA MENIAM
SALIDA_1        33
AMPLIFICADOR    12 
PIN_NEOPIXEL    19

PIN_MP3_UART_RX    16
PIN_MP3_UART_TX    17  

PINES TARJETA NEGRA
AMPLIFICADOR       12 **
#define SALIDA_1   27
PIN_MP3_UART_RX    12
PIN_MP3_UART_TX    14  
PIN_MP3_BUSY       26
PIN_NEOPIXEL       19
LED_MP3            32

PINES TARJETA MN 2.0
PIN_MP3_BUSY       33
AMPLIFICADOR       12
SALIDA_1           25
*/

// Configuración de paramatros BLE
#define PREFIJO_BLE               "AV_"      // Prefijo de red BLE
#define TM_ESPERA_BLE           120000       // Tiempo de espera para BLE


#define CONT_ESP_OFF_ALARMA     5000        // Tiempo de espera para apagar la alarma
#define CONT_ESP_ON_ALARMA      3000        // Tiempo de espera para encender la alarma


#define AL_CTRL_TP_AV                   1
#define AL_CTRL_TP_GUARDIAN             3
#define AL_CTRL_TP_INTEGRADOR           4  
#define AL_EMG_MQTT                     2   
#define AL_PLAY_1_PISTA                 5

// Mp3
#define CONST_MP3_REPRODUCIENDO     false 
#define CONST_MP3_ESP_REP_MP3        2500
#define CONST_MP3_LIM_T_PISTA       60000

#define CONST_NEOPIXEL_LEDS               16

// MP3 AUDIOS SISTEMA
#define CONST_MP3_SYST_ERROR_AUDIO_404          2
#define CONST_MP3_SYST_PRUEBA                  10
#define CONST_MP3_SYST_INIT_PROG               11
#define CONST_MP3_SYST_FIN_PROG                12
#define CONST_MP3_SYST_RST                     13
#define CONST_MP3_SYST_WAKE_UP                 14
#define CONST_MP3_SYST_WIFI_DESC               15
#define CONST_MP3_SYST_TAMPER_EN_0             16
#define CONST_MP3_SYST_TAMPER_EN_1             17
#define CONST_MP3_SYST_BATTERY_LOW             18
#define CONST_MP3_SYST_BATTERY_BACKUP_INIT     19
#define CONST_MP3_MC_SD_INSERTED               20


// COSTANTES DE CONFIGURACION DE CONTROL

#define CTRL_LIMITE_CTRL           200

#define CTRL_CNF_AV                 1
#define CTRL_CNF_ALERTA_SON_MSG     2
#define CTRL_CNF_INTEGRADOR         3

#define CTRL_TP_DP_LIMITE            3
#define CTRL_TP_DSP_RF_1          "Alarma Vecinal"
#define CTRL_TP_DSP_RF_2          "Alerta Sonora y Mensaje"
#define CTRL_TP_DSP_RF_3          "Integrador"

#define CTRL_TP_BOTON_DESHABILITADO 0
#define CTRL_TP_BOTON_SILENCIOSO    1
#define CTRL_TP_BOTON_AV            2
#define CTRL_TP_BOTON_UNA_PISTA     3
#define CTRL_TP_BOTON_UNA_PISTA_AS  4
#define CTRL_TP_BOTON_PISTA_MSG     5
#define CTRL_TP_BOTON_MSG           6   
#define CTRL_TP_BOTON_INIT_PROG     7
#define CTRL_TP_BOTON_TAMPER_ON     8
#define CTRL_TP_BOTON_RST           9

///// MQTT  ///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#define TOPIC_SUS_1                 "NODO/"                     // Topico de suscripción MQTT
#define TOPIC_SUS_2                 "/CMD"
#define TOPIC_SUS_3                 "/ACK"


#define TOPIC_PUB                   "CONTROLADOR/HB"            // Topico de publicación MQTT de heartbeat
#define TOPIC_STATUS                "CONTROLADOR/STATUS"        // Topico de publicación MQTT de heartbeat

#define MQTT_CNF_TM_HB_SG                  60                       // Tiempo de espera en segungos para el heartbeat MQTT
#define MQTT_CNF_TM_REV_LOOP_MS            500                      // Tiempo de espera en milisegundos para el bucle MQTT
#define MQTT_CNF_TM_INT_RECONEXION        10000                     // Tiempo de espera en milisegundos para la reconexión MQTT           
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


// SALIDAS ///////////////////////////////////////////////////////////////////////////////////////

#define GST_SALIDA_PWM_FREQUENCY          5000
#define GST_SALIDA_PWM_RESOLUTION           8
#define GST_SALIDA_PWM_MIN_DUTY_CYCLE           10
#define GST_SALIDA_FADE_OPCION                  10
#define GST_SALIDA_TONO_OPCION                  10
#define GST_SALIDA_MAX_PISTA_MP3             10000
#define GST_SALIDA_SD_MAX_TM_1_SG            43200  // maximo tiempo de espera en segundos para la salida 1

#define GST_SALIDA_NUM_PRF              4   // Numero de salidas de la alarma   
//////////////////////////////////////////////////////////////////////////////////////////////////
#endif // CONFIG_SISTEMA_H
