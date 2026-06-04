#include <Arduino.h>
#include <ArduinoJson.h>

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

// Mutex para proteger variables compartidas
SemaphoreHandle_t mutexEventosCtrl = NULL;
SemaphoreHandle_t mutexEventosMqtt = NULL;
SemaphoreHandle_t mutexPistaLed = NULL;
SemaphoreHandle_t mutexCtrlAlarma = NULL;

bool pendienteAsyncEvCtrl = false;
uint8_t varAsyncEvTipoCtrl;
bool pendienteSyncEvMqtt = false;

bool pendienteAsyncPistaLedAtm = false;
uint16_t varAsyncPistaLedAtm = 0;
uint16_t varAsyncPistaLedAtmConfig = 0;
uint16_t varAsyncPistaLedAtmCarga = 0;

static uint8_t varAsyncEvEstadoAlarma = 0;
static uint8_t g_estadoAlarma = 0;
static uint32_t g_tsActivacion = 0;

TaskHandle_t tareaProcesosCmdHandle = NULL;

// Función para crear la tarea de alarma
void crearTareaProcesosCmd(void)
{

    LOG("\r\n\r\nCreando tarea de alarma...");

    // Crear mutex antes de la tarea
    mutexEventosCtrl = xSemaphoreCreateMutex();
    mutexEventosMqtt = xSemaphoreCreateMutex();
    mutexPistaLed = xSemaphoreCreateMutex();
    mutexCtrlAlarma = xSemaphoreCreateMutex();

    if (mutexEventosCtrl == NULL || mutexEventosMqtt == NULL || mutexPistaLed == NULL || mutexCtrlAlarma == NULL)
    {
        LOG("\r\n ERROR: No se pudieron crear los mutex");
        return;
    }

    // Crear la tarea
    xTaskCreate(
        tareaProcesosCmd,       // Función de la tarea
        "TareaAlarma",          // Nombre de la tarea
        8192,                   // Aumentado a 8192 para mayor seguridad
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

uint8_t get_estado_alarma(void)
{
    uint8_t estado = 0;
    if (mutexCtrlAlarma != NULL && xSemaphoreTake(mutexCtrlAlarma, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        estado = g_estadoAlarma;
        xSemaphoreGive(mutexCtrlAlarma);
    }
    return estado;
}

void reset_estado_alarma(void)
{
    if (mutexCtrlAlarma != NULL && xSemaphoreTake(mutexCtrlAlarma, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        g_estadoAlarma = 0;
        xSemaphoreGive(mutexCtrlAlarma);
    }
}

void call_async_eventos(void)
{
    async_config_mp3_cmd(); // Llamar a la función para gestionar el MP3
    delay(100);             // Esperar un poco para evitar problemas de sincronización de variables
    async_gestion_salida(); // Llamar a la función para gestionar las salidas
}

void rev_async_evento_ctrl_av(void) // 3434
{
    bool hayEvento = false;
    uint8_t tipoCtrl = 0;
    uint8_t estadoAlarma = 0;

    if (xSemaphoreTake(mutexEventosCtrl, portMAX_DELAY) == pdTRUE)
    {
        if (pendienteAsyncEvCtrl)
        {
            hayEvento = true;
            tipoCtrl = varAsyncEvTipoCtrl;
            estadoAlarma = varAsyncEvEstadoAlarma;
            pendienteAsyncEvCtrl = false;
        }
        xSemaphoreGive(mutexEventosCtrl);
    }

    if (!hayEvento)
        return;

    bool filtrado = false;
    if (xSemaphoreTake(mutexCtrlAlarma, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (estadoAlarma == 1)
        {
            if (g_estadoAlarma == 1)
                filtrado = true;
            else
            {
                g_estadoAlarma = 1;
                g_tsActivacion = millis();
            }
        }
        else
        {
            if (g_estadoAlarma == 0)
                filtrado = true;
            else
                g_estadoAlarma = 0;
        }
        xSemaphoreGive(mutexCtrlAlarma);
    }
    LOG("\r\n rev_async_ev. estadoAlarma:" + String(estadoAlarma) + " g_est:" + String(g_estadoAlarma) + " filtrado:" + String(filtrado));
    if (filtrado)
        return;

    LOG("\r\n\r\nrev_async_evento_ctrl_av.");
    LOG("\r\nTipo de control: " + String(tipoCtrl));
    LOG("\r\nEstado alarma: " + String(estadoAlarma));

    GestorCmd.CtrlAv(tipoCtrl);
    call_async_eventos();
}

void async_evento_ctrl_av(uint8_t tipoCtrl, uint8_t estadoAlarma) // 3434
{
    if (tipoCtrl == 0)
        return;

    if (mutexEventosCtrl == NULL)
        return;

    // Proteger acceso con mutex
    if (xSemaphoreTake(mutexEventosCtrl, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        switch (tipoCtrl)
        {
        case AL_CTRL_TP_AV:
        case AL_CTRL_TP_GUARDIAN:
        case AL_CTRL_TP_INTEGRADOR:
            varAsyncEvTipoCtrl = tipoCtrl;
            varAsyncEvEstadoAlarma = estadoAlarma;
            pendienteAsyncEvCtrl = true;
            break;
        }
        xSemaphoreGive(mutexEventosCtrl);
    }

    delay(10); // Reducido de 100 a 10
}

void rev_sync_evento_mqtt(void)
{
    bool hayEvento = false;

    if (xSemaphoreTake(mutexEventosMqtt, portMAX_DELAY) == pdTRUE)
    {
        if (pendienteSyncEvMqtt)
        {
            hayEvento = true;
            pendienteSyncEvMqtt = false;
        }
        xSemaphoreGive(mutexEventosMqtt);
    }

    if (!hayEvento)
        return;

    LOG("\r\n\r\nrev_sync_evento_mqtt.");
    call_async_eventos();
}

/**
 * @brief Procesa payload MQTT entrante: dedup de estado, despacho a GestorCmd o simulación de control RF.
 *
 * Si el campo `tipo` es `"cmd-ctrl"`, carga `estadoCompRFAv.btnIndice` desde el campo `btn`
 * del payload y llama `async_evento_ctrl_av(AL_CTRL_TP_AV, ...)` reproduciendo el mismo
 * flujo que un control RF físico. En caso contrario, ejecuta el flujo clásico vía
 * `GestorCmd.process()`.
 *
 * @param payload JSON recibido desde el broker MQTT.
 *
 * @note Llamado desde la tarea MQTT. No usar delays largos dentro de este contexto.
 * @warning `estadoCompRFAv` se escribe sin mutex — misma convención que rf_esp.cpp.
 */
void sync_evento_mqtt(String payload) // 3434
{
    if (mutexEventosMqtt == NULL)
        return;

    StaticJsonDocument<512> doc;
    if (!deserializeJson(doc, payload))
    {
        int8_t estadoAlarma = doc["estado-alarma"] | -1;
        if (estadoAlarma == -1)
        {
            int8_t estadoDisp = doc["estado-dispositivo"] | -1;
            if (estadoDisp != -1)
                estadoAlarma = estadoDisp;
        }
        const char *tipoCmd = doc["tipo"] | "";
        if (strcmp(tipoCmd, "cmd-ctrl") == 0)
        {
            int8_t btnIdx = doc["btn"] | 0;
            int8_t ctrlIdx = doc["ctrl"] | 0;
            const char *modeloStr = doc["modelo"] | "";
            estadoCompRFAv.btnIndice = (uint8_t)btnIdx;
            estadoCompRFAv.control = (uint8_t)ctrlIdx;
            if (strlen(modeloStr) > 0)
            {
                for (const auto &ctrl : Data.CtrlModelos)
                {
                    if (strcmp(ctrl.modelo, modeloStr) == 0)
                    {
                        modeloCtrlAVRx = ctrl;
                        break;
                    }
                }
            }
            uint8_t activarPerifericos = (estadoCompRFAv.btnIndice + 1 != modeloCtrlAVRx.boton_desactivacion) ? 1 : 0;
            // LOGF("\r\n[cmd-ctrl] btnIndice:%d boton_desactivacion:%d activarPerifericos:%d modelo:%s", estadoCompRFAv.btnIndice, modeloCtrlAVRx.boton_desactivacion, activarPerifericos, modeloCtrlAVRx.modelo);
            async_evento_ctrl_av(AL_CTRL_TP_AV, activarPerifericos);
            return;
        }

        if (estadoAlarma != -1 && mutexCtrlAlarma != NULL)
        {
            bool filtrado = false;
            if (xSemaphoreTake(mutexCtrlAlarma, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                if (estadoAlarma == 1)
                {
                    if (g_estadoAlarma == 1)
                        filtrado = true;
                    else
                    {
                        g_estadoAlarma = 1;
                        g_tsActivacion = millis();
                    }
                }
                else
                {
                    if (g_estadoAlarma == 0)
                        filtrado = true;
                    else
                        g_estadoAlarma = 0;
                }
                xSemaphoreGive(mutexCtrlAlarma);
            }
            LOG("\r\nsync_evento_mqtt. estadoAlarma:" + String(estadoAlarma) + " g:" + String(g_estadoAlarma) + " filtrado:" + String(filtrado));
            if (filtrado)
                return;
        }
    }

    GestorCmd.process(payload, CmdOrigen::SRV_EXT);

    if (xSemaphoreTake(mutexEventosMqtt, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        pendienteSyncEvMqtt = true;
        xSemaphoreGive(mutexEventosMqtt);
    }

    delay(10); // Reducido de 100 a 10
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
    if (cargaPeriferico == 0 || config == 0 || pista == 0 || pista > 1000)
        return;

    if (mutexPistaLed == NULL)
        return;

    if (xSemaphoreTake(mutexPistaLed, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        varAsyncPistaLedAtm = pista;
        varAsyncPistaLedAtmConfig = config;
        varAsyncPistaLedAtmCarga = cargaPeriferico;
        pendienteAsyncPistaLedAtm = true;
        xSemaphoreGive(mutexPistaLed);
    }

    delay(10); // Reducido de 100 a 10
}

void rev_async_pista_led_atm(void)
{
    bool hayEvento = false;
    uint16_t pista = 0;
    uint16_t config = 0;
    uint16_t carga = 0;

    if (xSemaphoreTake(mutexPistaLed, portMAX_DELAY) == pdTRUE)
    {
        if (pendienteAsyncPistaLedAtm)
        {
            hayEvento = true;
            pista = varAsyncPistaLedAtm;
            config = varAsyncPistaLedAtmConfig;
            carga = varAsyncPistaLedAtmCarga;
            pendienteAsyncPistaLedAtm = false;
        }
        xSemaphoreGive(mutexPistaLed);
    }

    if (!hayEvento)
        return;

    LOG("\r\n\r\nrev_async_pista_led_atm.");
    LOG("\r\nPista LED ATM: " + String(pista));
    LOG("\r\nConfiguración Pista LED ATM: " + String(config));
    LOG("\r\nCarga Pista LED ATM: " + String(carga));

    GestorCmd.playPistaLedAtm(pista, config, carga);
    call_async_eventos();
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

    while (millis() - timerInitRst < 50000) // limite de tiempo de espera de 50 segundos
    {
        if (!cmdAudioBusy)
        {
            LOG("\r\n\r\nsync_proc_cmd_play_pista_led_atm cmdAudioBusy: false ");
            break;
        }
        delay(50); // Esperar 50ms para evitar problemas de sincronización
    }
}