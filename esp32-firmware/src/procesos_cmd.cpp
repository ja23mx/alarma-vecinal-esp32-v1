#include <Arduino.h>

#include "ConfigSistema.h"
#include "LOGSistema.h"
#include "procesos_cmd.h"
#include "rf_esp.h"
#include "voz_esp.h"
#include "VariablesGlobales.h"
#include "DataManager.h"
#include "voz_esp.h"
#include "ErroresCMD.h"
#include "gestor_salidas.h"
#include "GestorCmd.h"

bool pendienteAsyncEvCtrl = false; // Variable para verificar si hay un evento de control pendiente
uint8_t varAsyncEvTipoCtrl;        // Variable para almacenar el tipo de control pendiente
bool pendienteSyncEvMqtt = false;  // Variable para verificar si hay un evento MQTT pendiente

bool pendienteAsyncPistaLedAtm = false; // Variable para verificar si hay una pista LED ATM pendiente
uint16_t varAsyncPistaLedAtm = 0;       // Variable para almacenar la pista LED ATM pendiente
uint16_t varAsyncPistaLedAtmConfig = 0; // Variable para almacenar la configuración de la pista LED ATM
uint16_t varAsyncPistaLedAtmCarga = 0;  // Variable para almacenar la carga de la pista LED ATM

TaskHandle_t tareaProcesosCmdHandle = NULL; // Handle para la tarea de procesos

// Función para crear la tarea de alarma
void crearTareaProcesosCmd(void)
{

    LOG("\r\n\r\nCreando tarea de alarma...");

    // Crear la tarea
    xTaskCreate(
        tareaProcesosCmd,       // Función de la tarea
        "TareaAlarma",          // Nombre de la tarea
        2048,                   // Tamaño del stack en palabras
        NULL,                   // Parámetros de entrada
        2,                      // Prioridad de la tarea
        &tareaProcesosCmdHandle // Guardar el handle de la tarea
    );

    delay(10);
}

void cambiar_estado_tarea_proc_cmd(uint8_t estado)
{
    // Cambiar el estado de la alarma
    if (estado == 0)
    {
        vTaskSuspend(tareaProcesosCmdHandle); // pausa la tarea de alarma
    }
    else
    {
        vTaskSuspend(tareaProcesosCmdHandle); // reanuda la tarea de alarma
    }
}

// Función que ejecutará la tarea
void tareaProcesosCmd(void *pvParameters)
{
    uint8_t estado_alarma_local = 0;

    LOG("\r\n\r\nTarea de procesos Cmd iniciada...");

    while (true)
    {

        rev_async_evento_ctrl_av(); // Revisar si hay un evento de control pendiente
        rev_sync_evento_mqtt();     // Revisar si hay un evento MQTT pendiente
        rev_async_pista_led_atm();  // Revisar si hay una pista LED ATM pendiente

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void call_async_eventos(void)
{
    async_config_mp3_cmd(); // Llamar a la función para gestionar el MP3
    delay(100);             // Esperar un poco para evitar problemas de sincronización de variables
    async_gestion_salida(); // Llamar a la función para gestionar las salidas
}

void rev_async_evento_ctrl_av(void)
{

    if (!pendienteAsyncEvCtrl) // Si no hay pendiente de evento de control, salir
        return;                // Salir de la función

    pendienteAsyncEvCtrl = false; // Reiniciar la variable de pendiente de evento de control

    LOG("\r\n\r\nrev_async_evento_ctrl_av.");
    LOG("\r\nTipo de control: " + String(varAsyncEvTipoCtrl));
    GestorCmd.CtrlAv(varAsyncEvTipoCtrl); // Procesar el evento de control AV, obtiene los CODI (comandos digitales) de salida y audio
    call_async_eventos();                 // Llamar a la función para gestionar el MP3 y las salidas
}

void async_evento_ctrl_av(uint8_t tipoCtrl)
{
    if (tipoCtrl == 0) // Si el tipo de control es 0, no hacer nada
        return;        // Salir de la función

    switch (tipoCtrl)
    {
    case AL_CTRL_TP_AV:                // Control AV
        varAsyncEvTipoCtrl = tipoCtrl; // Almacenar el tipo de control pendiente
        pendienteAsyncEvCtrl = true;   // Marcar como pendiente el evento de control
        break;
    case AL_CTRL_TP_GUARDIAN:          // Control Guardian
        varAsyncEvTipoCtrl = tipoCtrl; // Almacenar el tipo de control pendiente
        pendienteAsyncEvCtrl = true;   // Marcar como pendiente el evento de control
        break;
    case AL_CTRL_TP_INTEGRADOR:        // Control Integrador
        varAsyncEvTipoCtrl = tipoCtrl; // Almacenar el tipo de control pendiente
        pendienteAsyncEvCtrl = true;   // Marcar como pendiente el evento de control
        break;
    }

    delay(100); // Esperar un poco para evitar problemas de sincronización
}

void rev_sync_evento_mqtt(void)
{
    if (!pendienteSyncEvMqtt) // Si no hay pendiente de evento MQTT, salir
        return;               // Salir de la función

    pendienteSyncEvMqtt = false; // Reiniciar la variable de pendiente de evento MQTT

    LOG("\r\n\r\nrev_sync_evento_mqtt."); //
    call_async_eventos();                 // Llamar a la función para gestionar el MP3 y las salidas
}

void sync_evento_mqtt(String payload)
{

    GestorCmd.process(payload, CmdOrigen::SRV_EXT); // Procesar el payload recibido
    pendienteSyncEvMqtt = true;                     // Marcar como pendiente el evento MQTT
    delay(100);                                     // Esperar un poco para evitar problemas de sincronización
}

void proc_cmd_play_pista_msg(uint8_t pista)
{

    if (pista == 0 || pista > 1000) // Si la pista es 0 o mayor a 1000, no hacer nada
        return;                     // Salir de la función

    // configuración = 2 (PWM) y carga = 70
    async_play_pista_led_atm(pista, 2, 70); // Procesar la pista LED ATM pendiente
}

/*
    @brief: Método para activar una pista de audio y salida led dependiente de la reproduccion
    @param pista: Número de la pista de audio a reproducir.
    @param config: Configuración del LED (1: ON-OFF, 2: PWM, 3: FADE).
    @param cargaPeriferico: Valor de carga al periférico (depende de la configuración).
*/
void async_play_pista_led_atm(uint8_t pista, uint8_t config, uint8_t cargaPeriferico)
{
    if (cargaPeriferico == 0)                   // Si no se debe cargar el periférico, no hacer nada
        return;                                 // Salir de la función
    if (config == 0)                            // Si la configuración es 0, no hacer nada
        return;                                 // Salir de la función
    if (pista == 0 || pista > 1000)             // Si la pista es 0 o mayor a 1000, no hacer nada
        return;                                 // Salir de la función
                                                //
    varAsyncPistaLedAtm = pista;                // Almacenar la pista LED ATM pendiente
    varAsyncPistaLedAtmConfig = config;         // Almacenar la configuración de la pista LED ATM
    varAsyncPistaLedAtmCarga = cargaPeriferico; // Almacenar la carga de la pista LED ATM
    pendienteAsyncPistaLedAtm = true;           // Marcar como pendiente el evento de pista LED ATM
    delay(100);                                 // Esperar un poco para evitar problemas de sincronización
}

void rev_async_pista_led_atm(void)
{
    if (!pendienteAsyncPistaLedAtm) // Si no hay pendiente de pista LED ATM, salir
        return;                     // Salir de la función

    pendienteAsyncPistaLedAtm = false; // Reiniciar la variable de pendiente de pista LED ATM

    LOG("\r\n\r\nrev_async_pista_led_atm.");
    LOG("\r\nPista LED ATM: " + String(varAsyncPistaLedAtm));
    LOG("\r\nConfiguración Pista LED ATM: " + String(varAsyncPistaLedAtmConfig));
    LOG("\r\nCarga Pista LED ATM: " + String(varAsyncPistaLedAtmCarga));
    GestorCmd.playPistaLedAtm(varAsyncPistaLedAtm, varAsyncPistaLedAtmConfig, varAsyncPistaLedAtmCarga); // Procesar la pista LED ATM pendiente
    call_async_eventos();                                                                                // Llamar a la función para gestionar el MP3 y las salidas
}



/*
    @brief: Método para procesar la pista y encendido del LED de forma sincronizada. No puede ser llamado desde la tarea de RF y Voz
    @param pista: Número de la pista de audio a reproducir.
    @param config: Configuración del LED (1: ON-OFF, 2: PWM, 3: FADE).
    @param cargaPeriferico: Valor de carga al periférico (depende de la configuración).
*/
void sync_proc_cmd_play_pista_led_atm(uint8_t pista, uint8_t config, uint8_t cargaPeriferico)
{
    if (cargaPeriferico == 0)                                 // Si no se debe cargar el periférico, no hacer nada
        return;                                               // Salir de la función
    if (config == 0)                                          // Si la configuración es 0, no hacer nada
        return;                                               // Salir de la función
    if (pista == 0 || pista > 1000)                           // Si la pista es 0 o mayor a 1000, no hacer nada
        return;                                               // Salir de la función
                                                              //
    async_play_pista_led_atm(pista, config, cargaPeriferico); // Procesar la pista LED ATM pendiente
    delay(500);                                               // Esperar un poco para evitar problemas de sincronización

    LOG("\r\n\r\nsync_proc_cmd_play_pista_led_atm INIT");

    uint32_t timerInitRst = millis(); // almacenar el tiempo actual

    while ( millis() - timerInitRst < 50000) // limite de tiempo de espera de 50 segundos
    {
        if (!cmdAudioBusy)
        {
            LOG("\r\n\r\nsync_proc_cmd_play_pista_led_atm cmdAudioBusy: false ");
            break;
        }
        delay(50); // Esperar 50ms para evitar problemas de sincronización
    }
}