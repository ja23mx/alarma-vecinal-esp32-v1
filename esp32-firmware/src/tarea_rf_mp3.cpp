#include <Arduino.h>

#include "ConfigSistema.h"
#include "DataManager.h"
#include "LOGSistema.h"
#include "VariablesGlobales.h"
#include "gestor_procesos.h"
#include "procesos_cmd.h"
#include "rf_esp.h"
#include "voz_esp.h"

TaskHandle_t tareaProcesosHandle = NULL; // Handle para la tarea de procesos

// Función para crear la tarea
void crearTareaProcesos(void)
{
    LOG("\r\n\r\nCreando tarea de procesos...");

    // Crear la tarea
    xTaskCreate(
        tareaProcesos,       // Función de la tarea
        "TareaProcesos",     // Nombre de la tarea
        2048,                // Tamaño del stack en palabras
        NULL,                // Parámetros de entrada
        1,                   // Prioridad de la tarea
        &tareaProcesosHandle // Guardar el handle de la tarea
    );

    delay(10);
}

void cambiar_estado_procesos_alarma(uint8_t estado)
{
    // Cambiar el estado de la alarma
    if (estado == 0)
    {
        vTaskSuspend(tareaProcesosHandle); // pausa la tarea de alarma
    }
    else
    {
        vTaskSuspend(tareaProcesosHandle); // reanuda la tarea de alarma
    }
}

// Función que ejecutará la tarea
void tareaProcesos(void *pvParameters)
{

    LOG("\r\n\r\nTarea de procesos iniciada...");

    delay(3000);   // Espera que DFPlayer esté listo
    mp3_init();    // Inicializa el módulo MP3
    rf_esp_init(); // Inicializa el receptor RF

    proc_cmd_play_pista_msg(CONST_MP3_SYST_WAKE_UP); // Reproduce la pista de audio de despertar

    while (true)
    {
        lectura_pto(); // Lee el estado del módulo MP3
        mp3_loop();    // Verifica si hay eventos en el módulo MP3
        rf_loop();     // Verifica si hay una señal RF

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void play_1pista_alarma(uint16_t pista)
{

    if (!mp3State.Configurado) // Si el MP3 no está configurado, no se reproduce nada
    {
        LOG("\r\n\r\nMP3 NO CONFIGURADO");
        return;
    }
    
    if (pista < 3) // Si la pista es menor a 3, no se reproduce nada
    {
        return;
    }

    estadoAlarma.audio1Pista = pista;         // Asigna la pista a reproducir
    estadoAlarma.origenEmg = AL_PLAY_1_PISTA; // Origen de la emergencia, reproducción de una pista de audio
    estadoAlarma.alarma_on = 1;
    delay(10);
}