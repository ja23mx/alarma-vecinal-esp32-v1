#include <Arduino.h>

#include "ConfigSistema.h"
#include "DataManager.h"
#include "LOGSistema.h"
#include "VariablesGlobales.h"
#include "gestor_procesos.h"
#include "procesos_cmd.h"
#include "rf_esp.h"
#include "voz_esp.h"

TaskHandle_t tareaProcesosHandle = NULL;

// ✅ Mutex para proteger estadoAlarma
SemaphoreHandle_t mutexEstadoAlarma = NULL;

// Función para crear la tarea
void crearTareaProcesos(void)
{
    LOG("\r\n\r\nCreando tarea de procesos...");

    // ✅ Crear mutex
    mutexEstadoAlarma = xSemaphoreCreateMutex();
    if (mutexEstadoAlarma == NULL)
    {
        LOG("\r\n ERROR: No se pudo crear mutex de estado alarma");
        return;
    }

    // ✅ Stack aumentado a 4096
    xTaskCreate(
        tareaProcesos,
        "TareaProcesos",
        4096, // Aumentado para mp3_init() + rf_esp_init()
        NULL,
        1,
        &tareaProcesosHandle);

    delay(10);
}

void cambiar_estado_procesos_alarma(uint8_t estado)
{
    if (tareaProcesosHandle == NULL)
        return;

    // ✅ Corregido el bug
    if (estado == 0)
    {
        vTaskSuspend(tareaProcesosHandle);
        LOG("\r\nTarea de procesos SUSPENDIDA");
    }
    else
    {
        vTaskResume(tareaProcesosHandle); // ✅ RESUME en lugar de SUSPEND
        LOG("\r\nTarea de procesos REANUDADA");
    }
}

// Función que ejecutará la tarea
void tareaProcesos(void *pvParameters)
{

    LOG("\r\n\r\nTarea de procesos iniciada...");

#ifdef VOZ_ENABLE
    delay(3000);
    mp3_init();
    proc_cmd_play_pista_msg(CONST_MP3_SYST_WAKE_UP); // Reproduce la pista de audio de despertar
#endif

    // ✅ Verificar inicialización
    rf_esp_init();

    while (true)
    {
#ifdef VOZ_ENABLE
        lectura_pto();
        mp3_loop();
#endif
        rf_loop();

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void play_1pista_alarma(uint16_t pista)
{
    if (!mp3State.Configurado)
    {
        LOG("\r\n\r\nMP3 NO CONFIGURADO");
        return;
    }

    if (pista < 3)
        return;

    // ✅ Proteger acceso con mutex
    if (mutexEstadoAlarma != NULL && xSemaphoreTake(mutexEstadoAlarma, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        estadoAlarma.audio1Pista = pista;
        estadoAlarma.origenEmg = AL_PLAY_1_PISTA;
        estadoAlarma.alarma_on = 1;
        xSemaphoreGive(mutexEstadoAlarma);
    }

    delay(10);
}