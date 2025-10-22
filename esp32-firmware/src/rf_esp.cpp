#include <RCSwitch.h>

#include "ConfigSistema.h"
#include "VariablesGlobales.h"
#include "rf_esp.h"

#include "DataManager.h"
#include "LOGSistema.h"
#include "procesos_cmd.h"
#include "voz_esp.h"
#include "wifi_mqtt_esp.h"
#include "tarea_neopixel.h"

RCSwitch rf_433 = RCSwitch();

unsigned long senalRF = 0;

void rf_esp_init()
{
    rf_433.enableReceive(RF_RX); // Inicializar el receptor RF
}

bool rf_nuevo_dato(bool blink) // Función que verifica si hay un nuevo dato RF
{
    if (!rf_433.available()) // no hay datos disponibles
        return false;

    senalRF = rf_433.getReceivedValue();
    rf_433.resetAvailable();

    if (senalRF == 0)
    {
        LOG("\r\n\r\nCODIGO DESCONOCIDO");
        return false;
    }

    LOG("\r\n\r\nRF RX: ");
    LOG(senalRF);
    if (blink)
        async_led_rf_rx(); // Llamar a la función asíncrona para encender el LED de RF
    return true;
}

void rf_loop(void)
{
    rf_esp_nv_dato();          // Verifica si hay una señal RF, alarma esta en modo TRABAJO
    rf_esp_prog_nv_ver_ctrl(); // Verifica si hay una señal RF, alarma esta en modo PROGRAMACION
}

bool rf_esp_nv_dato(void)
{
    if (init_prog_ble) // si se está programando, no se procesa la señal RF
        return false;

    if (Data.deteccionDispRF) // si se está configurando un nuevo control, no se procesa la señal RF
        return false;

    if (!rf_nuevo_dato(1))
        return false;

    if (Data.controlesNVSok == false) // controles no cargados
    {
        LOG("\r\n\r\nCONTROLES NO CARGADOS");
        return false;
    }

    bool rsp = Data.busqDspRF(senalRF);

    if (!rsp) // no se encontró coincidencia
    {
        LOG("\r\n\r\nCONTROL INVALIDO");
        return false;
    }

    LOGF("\r\ncontrol: %d", estadoCompRFAv.control);
    LOGF("\r\nboton: %d", estadoCompRFAv.btnIndice); /* 3434 */
    LOGF("\r\nconfiguracion: %d", estadoCompRFAv.configuracion);

    switch (estadoCompRFAv.configuracion) // Obtener la configuración del control
    {
    // CONTROL ALARMA VECINAL
    case 1:                                  // alerta vecinal
        async_mqtt_msg_ctrl_alarma();        // Enviar mensaje MQTT de control de alarma
        async_evento_ctrl_av(AL_CTRL_TP_AV); // Llamar a la función para procesar el evento de control
        return true;                         // si se encendio o apago la alarma, salir de la funcion
        break;
    // CONTROL DE GUARDIAN DE ALARMA
    case 2:                                        // alerta sonora y mensaje
        async_evento_ctrl_av(AL_CTRL_TP_GUARDIAN); // Llamar a la función para procesar el evento de control
        return true;                               // si se encendio o apago la alarma, salir de la funcion
        break;
    case 3:                       // control de integrador
        rf_esp_integrador_ctrl(); //
        break;
    default:
        LOGF("\r\n\r\nCONTROL INVALIDO, configuracion: %d", estadoCompRFAv.configuracion);
        LOG("\r\n\r\nCONTROL INVALIDO, configuracion no reconocida");
        break;
    }

    /* LOG("\r\n\r\nCONTROL VALIDO. estado de la alarma: ");
    LOG(estadoAlarma.alarma_on);
    LOGF("\r\nstatus: %d", estadoCompRFAv.status);
    LOGF("\r\ncontrol: %d", estadoCompRFAv.control);
    LOGF("\r\nboton: %d", estadoCompRFAv.boton);
    Data.infoCtrlAVLog(Data.controlRecepionado);*/

    return false;
}

void rf_esp_integrador_ctrl(void)
{
    if (cmdAudioBusy || cmdSalidasBusy) //
        return;                         // si hay un comando de audio o salidas en curso, no se procesa la señal RF

    uint8_t tipo_boton = modeloCtrlAVRx.tipo_botones[estadoCompRFAv.btnIndice];
    LOGF("\r\n\r\nALARMA ACTIVADA POR RF INTEGRADOR, BOTON: %d", tipo_boton);

    switch (tipo_boton)
    {
    case CTRL_TP_BOTON_UNA_PISTA_AS:
        LOG("\r\n\r\nALARMA ACTIVADA POR RF INTEGRADOR, BOTON UNA PISTA");
        async_evento_ctrl_av(AL_CTRL_TP_INTEGRADOR); // Llamar a la función para procesar el evento de control
        break;

    case CTRL_TP_BOTON_RST:
        LOG("\r\n\r\nALARMA ACTIVADA POR RF INTEGRADOR, BOTON RESET");
        async_evento_ctrl_av(AL_CTRL_TP_INTEGRADOR); // Llamar a la función para procesar el evento de control
        revTimerRst();                               // Reiniciar el temporizador de reinicio
        break;

    case CTRL_TP_BOTON_INIT_PROG:
        LOG("\r\n\r\nALARMA ACTIVADA POR RF INTEGRADOR, BOTON INIT PROG");
        async_evento_ctrl_av(AL_CTRL_TP_INTEGRADOR); // Llamar a la función para procesar el evento de control
        init_prog_rf = true;                         // Variable para indicar inicio de la programacion via RF
        break;
    }
}

void revTimerRst(void)
{
    mp3_loop(); // reproduccion mp3

    LOG("\r\n\r\n\r\nEsperando audio de reinicio...\r\n\r\n\r\n");
    ESP.restart(); // Reiniciar el ESP32
}

void revCtrlGuardadoLoopProg(void)
{
    bool rsp = Data.busqDspRF(senalRF);

    if (!rsp) // no se encontró coincidencia
        return;

    if (estadoCompRFAv.configuracion != 3) // si no es un control de integrador
        return;

    uint8_t tipo_boton = modeloCtrlAVRx.tipo_botones[estadoCompRFAv.btnIndice];

    if (tipo_boton != CTRL_TP_BOTON_INIT_PROG)
        return;

    async_led_rf_rx(); // Llamar a la función asíncrona para encender el LED de RF

    init_prog_rf = false; // Reiniciar la variable de inicio de programación RF

    // LOG("\r\n\r\n\r\nCONTROL DE INTEGRADOR fin prog\r\n\r\n");
}

void rf_esp_prog_nv_ver_ctrl(void)
{

    if (!init_prog_ble) // si no se está programando, no se procesa la señal RF
        return;

    while (init_prog_ble) // espera mientras llega la instruccion de grabar nuevo control o verificar si existe control
    {

        mp3_loop(); // reproduccion mp3

        // verificar si hay una nueva señal RF, blink OFF
        bool lectura = rf_nuevo_dato(0);

        // detectar si es un control de integrador guardado
        // si no se está configurando un nuevo control o verificando
        if (lectura == true && Data.deteccionDispRF == false)
        {
            revCtrlGuardadoLoopProg(); // verificar si es un control de integrador guardado
        }

        // si se está configurando un nuevo control o verificando
        if (Data.deteccionDispRF)
            break;
        delay(10); // delay para evitar el uso excesivo de CPU
    }

    // si no se está configurando un nuevo control o verificando no se procesa la señal RF
    if (Data.deteccionDispRF == false)
        return;

    bool nueva_senal = false;         // variable para almacenar la nueva señal RF recibida
    unsigned long buffer_senalRF = 0; // variable para almacenar la señal RF recibida
    unsigned long timer = 0;          // variable para almacenar el tiempo actual

    LOG("\r\n\r\nINIT DETECCION DE RF\r\n\r\n");

    while (Data.deteccionDispRF > 0)
    {
        if (rf_nuevo_dato(1))
        {
            // si llego una nueva señal RF y no es la misma que la anterior
            // se almacena la señal RF recibida en buffer_senalRF
            if (buffer_senalRF != senalRF && nueva_senal == false)
            {

                timer = millis();                            // almacenar el tiempo actual
                nueva_senal = true;                          // se encontró una nueva señal RF
                buffer_senalRF = senalRF;                    // almacenar la señal RF recibida
                Data.ctrlRfLoopSignal = senalRF;             // almacenar la señal RF recibida
                if (Data.busqDspRF(senalRF))                 // se encontró coincidencia
                {                                            //
                    LOG("\r\n\r\nNUEVO CONTROL VERIFICADO"); //
                    LOG("\r\n\r\nCONTROL VALIDO\r\n\r\n");   //
                    Data.detecConfigComp = 2;                // Nueva señal RF recibida que existe en la memoria
                }
                else
                {
                    LOG("\r\n\r\nNUEVO SENAL NO GUARADADA\r\n\r\n");
                    Data.detecConfigComp = 1; // Nueva señal RF recibida que no existe en la memoria
                }

                if (Data.detecConfigComp == 2)
                {
                    // LOG("\r\n");
                    // Data.infoCtrlAVLog(modeloCtrlAVRx);
                    LOGF("\r\n\r\nstatus: %d", estadoCompRFAv.status);
                    LOGF("\r\ncontrol: %d", estadoCompRFAv.control);
                    LOGF("\r\nboton: %d", estadoCompRFAv.btnIndice);
                }

                uint16_t buffer_deteccion = Data.detecConfigComp; // almacenar el tipo de detección

                unsigned long timer2 = millis(); // almacenar el tiempo actual

                while (buffer_deteccion == Data.detecConfigComp && // si cambia estado es debido a que server lo modifico
                       Data.detecConfigComp == 255)                // 255 lo leyo el server
                {
                    if (rf_nuevo_dato(0))
                    {
                        LOG("\r\n\r\nCONTROL RECEPCIONADO WHILE\r\n\r\n");
                    }
                    if (millis() - timer2 > 30000) // si no se recibe una señal RF en x segundos
                    {
                        LOG("\r\n\r\nCONTROL NO RECEPCIONADO\r\n\r\n"); //
                        Data.detecConfigComp = 0;                       // limite de tiempo de accion
                        break;
                    }
                    delay(1);
                }
            }
        }

        while (nueva_senal)
        {
            if (rf_nuevo_dato(0)) // verificar si hay una nueva señal RF, blink OFF
            {
                LOG("\r\n\r\nCONTROL RECEPCIONADO nueva_senal\r\n\r\n");
            }

            /// si la señal RF recibida es la misma que la anterior
            // se verifica si el tiempo transcurrido es mayor a x segundos
            if (nueva_senal == true && millis() - timer > 500)
            {
                buffer_senalRF = 0;  // se reinicia el buffer de señal RF
                nueva_senal = false; // se reinicia la variable nueva_senal
            }
            delay(1); // delay para evitar el uso excesivo de CPU
        }
    }

    LOG("\r\n\r\nFIN CONFIGURACION DE CONTROL\r\n\r\n");
}
