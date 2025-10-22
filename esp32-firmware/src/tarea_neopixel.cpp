#include <Arduino.h>

#include "ConfigSistema.h"
#include "DataManager.h"
#include "LOGSistema.h"
#include "VariablesGlobales.h"
#include "NeoPixelAV.h"
#include "tarea_neopixel.h"

TaskHandle_t tareaNeoPixelHandle = NULL; // Handle para la tarea de NeoPixel

// Instancia global de la clase NeoPixel
NeoPixelController neopixel(PIN_NEOPIXEL, CONST_NEOPIXEL_LEDS);

bool pendiente_led_server_data = false;    // Bandera para indicar si hay datos del servidor pendientes
bool pendiente_led_rf_rx = false;          // Bandera para indicar si hay un LED RF pendiente de recibir
bool pendiente_stop_carga_sistema = false; // Bandera para indicar si hay un stop de carga de sistema pendiente

// Banderas para control asíncrono del servidor
bool pendiente_servidor_conectado = false;    // Bandera para indicar servidor conectado
bool pendiente_servidor_desconectado = false; // Bandera para indicar servidor desconectado
bool pendiente_servidor_deshabilitar = false; // Bandera para deshabilitar sistema de servidor

// Banderas para control asíncrono del error MP3
bool pendiente_error_mp3_activar = false;    // Bandera para activar error MP3
bool pendiente_error_mp3_desactivar = false; // Bandera para desactivar error MP3

// ================ BANDERAS PARA CONTROL ASÍNCRONO DEL LED MP3 ================
bool pendiente_led_mp3_on = false;                    // Bandera para encender LED MP3 (reproductor reconocido)
bool pendiente_led_mp3_off = false;                   // Bandera para apagar LED MP3 (reproductor no reconocido)
bool pendiente_led_mp3_reproduciendo = false;         // Bandera para iniciar parpadeo de reproducción
bool pendiente_led_mp3_stop_reproduciendo = false;    // Bandera para detener parpadeo y quedar fijo ON
int8_t pendiente_led_mp3_stop_estado = 1;             // Estado final después del stop (-1:anterior, 0:OFF, 1:ON)

// Variables de estado interno del LED MP3
bool led_mp3_estado_reproduciendo = false;            // Flag si está en modo reproduciendo
unsigned long led_mp3_timer = 0;                      // Timer para control de parpadeo
bool led_mp3_estado_parpadeo = false;                 // Estado actual del parpadeo (true=ON, false=OFF)
bool led_mp3_estado_anterior = false;                 // Estado anterior antes de reproducir

// Función para crear la tarea
void crearTareaNeoPixel(void)
{
    LOG("\r\n\r\nCreando tarea de NeoPixel...");

    // Crear la tarea
    xTaskCreate(
        tareaNeoPixel,       // Función de la tarea
        "TareaNeoPixel",     // Nombre de la tarea
        2048,                // Tamaño del stack en palabras
        NULL,                // Parámetros de entrada
        4,                   // Prioridad de la tarea
        &tareaNeoPixelHandle // Guardar el handle de la tarea
    );

    delay(10);
}

void cambiar_estado_neopixel(uint8_t estado)
{
    // Cambiar el estado del NeoPixel
    if (estado == 0)
    {
        vTaskSuspend(tareaNeoPixelHandle); // pausa la tarea de NeoPixel
    }
    else
    {
        vTaskResume(tareaNeoPixelHandle); // reanuda la tarea de NeoPixel
    }
}

// Función que ejecutará la tarea
void tareaNeoPixel(void *pvParameters)
{
    LOG("\r\n\r\nTarea de NeoPixel iniciada...");

    delay(1000);     // Espera inicial
    neopixel.init(); // Inicializa el NeoPixel

    // Inicializar LED MP3
    pinMode(LED_MP3, OUTPUT);      // Configurar pin como salida
    digitalWrite(LED_MP3, LOW);    // Empezar apagado

    // Ejemplo de uso de patrones semánticos
    /* LOG("Ejecutando parpadeo RF 433MHz...");
    neopixel.blinkRF433MHz();
    LOG("Ejecutando parpadeo de error...");
    neopixel.blinkError();
    delay(1000); */

    neopixel.iniciarCargaSistema(); // Iniciar el efecto de carga del NeoPixel
                                    //  en el bucle principal (loop arduino) se llama a async_stop_carga_sistema
                                    //  para detener el efecto de carga del NeoPixel

    while (true)
    {

        check_stop_carga_sistema(); // Verifica si hay un stop de carga de sistema pendiente
        check_servidor_status();    // Verifica si hay cambios de estado del servidor pendientes
        check_error_mp3();          // Verifica si hay cambios de estado del error MP3 pendientes
        check_led_rf_rx();          // Verifica si hay un LED RF pendiente
        check_led_server_data();    // Verifica si hay datos del servidor pendientes
        check_led_mp3_status();     // Verifica si hay cambios de estado del LED MP3 pendientes

        neopixel.loop(); // Ejecuta la lógica de patrones

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/*
    @brief: Función para detener el efecto de carga del NeoPixel.
    @note: Esta función se llama desde la tarea de NeoPixel para detener el efecto de carga.
*/
void async_stop_carga_sistema(void)
{
    if (pendiente_stop_carga_sistema) // Si ya hay un stop de carga de sistema pendiente, no hacer nada
        return;

    pendiente_stop_carga_sistema = true; // Marcar que hay un stop de carga de sistema pendiente
    delay(100);                          // Esperar un poco para evitar colisiones
}

void check_stop_carga_sistema(void)
{

    if (!pendiente_stop_carga_sistema) // Si no hay una carga de sistema pendiente, no hacer nada
        return;

    pendiente_stop_carga_sistema = false; // Marcar que ya no hay una carga de sistema pendiente

    neopixel.detenerCargaSistema(); // Detener el efecto de carga
}

void check_servidor_status(void)
{
    // Verificar deshabilitación del servidor (prioridad alta)
    if (pendiente_servidor_deshabilitar)
    {
        pendiente_servidor_deshabilitar = false; // Marcar que ya no hay un estado pendiente

        neopixel.deshabilitarServidor(); // Deshabilitar sistema de servidor
        return;                          // Salir para evitar procesar otros estados
    }

    // Verificar conexión del servidor
    if (pendiente_servidor_conectado)
    {
        pendiente_servidor_conectado = false; // Marcar que ya no hay un estado pendiente

        neopixel.setServidorConectado(); // Activar patrón de servidor conectado
        return;
    }

    // Verificar desconexión del servidor
    if (pendiente_servidor_desconectado)
    {
        pendiente_servidor_desconectado = false; // Marcar que ya no hay un estado pendiente

        neopixel.setServidorDesconectado(); // Activar patrón de servidor desconectado
        return;
    }
}

/*
    @brief: Función para indicar recepción RF en led neopixel.
    @note: Esta función se llama desde la tarea de recepción RF para indicar
    que se ha recibido una señal RF.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_led_rf_rx(void)
{
    if (pendiente_led_rf_rx) // Si ya hay un LED RF pendiente, no hacer nada
        return;

    pendiente_led_rf_rx = true; // Marcar que hay un LED RF pendiente
    delay(100);                 // Esperar un poco para evitar colisiones
}

void check_led_server_data(void)
{
    if (!pendiente_led_server_data) // Si no hay un LED Server Data pendiente, no hacer nada
        return;

    pendiente_led_server_data = false; // Marcar que ya no hay un LED Server Data pendiente

    if (neopixel.isBusy()) // Si el NeoPixel está ocupado, no hacer nada
        return;

    neopixel.blinkServerExterno(); // Llamar a la función para indicar recepción de datos del servidor
}

void async_led_server_data(void)
{
    if (pendiente_led_server_data) // Si ya hay un LED Server Data pendiente, no hacer nada
        return;

    pendiente_led_server_data = true; // Marcar que hay un LED Server Data pendiente
    delay(100);                       // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para indicar que el servidor MQTT se ha conectado.
    @note: Esta función se llama desde main.cpp cuando se establece conexión con el broker MQTT.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_servidor_conectado(void)
{
    if (pendiente_servidor_conectado) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_servidor_conectado = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                           // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para indicar que el servidor MQTT se ha desconectado.
    @note: Esta función se llama desde main.cpp cuando se pierde conexión con el broker MQTT.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_servidor_desconectado(void)
{
    if (pendiente_servidor_desconectado) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_servidor_desconectado = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                              // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para deshabilitar el sistema de indicación del servidor.
    @note: Esta función se llama desde funciones de configuración o main.cpp.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_servidor_deshabilitar(void)
{
    if (pendiente_servidor_deshabilitar) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_servidor_deshabilitar = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                              // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para activar el sistema de error MP3.
    @note: Esta función se llama desde otras tareas cuando se detecta un error de MP3.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_error_mp3_activar(void)
{
    if (pendiente_error_mp3_activar) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_error_mp3_activar = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                          // Esperar un poco para evitar colisiones

    //Serial.println("[ASYNC] Solicitando activación de error MP3");
}

/*
    @brief: Función para desactivar el sistema de error MP3.
    @note: Esta función se llama desde otras tareas cuando se resuelve el error de MP3.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_error_mp3_desactivar(void)
{
    if (pendiente_error_mp3_desactivar) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_error_mp3_desactivar = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                             // Esperar un poco para evitar colisiones

    //Serial.println("[ASYNC] Solicitando desactivación de error MP3");
}

void check_error_mp3(void)
{
    // Verificar activación del error MP3
    if (pendiente_error_mp3_activar)
    {
        pendiente_error_mp3_activar = false; // Marcar que ya no hay un estado pendiente

        neopixel.activarErrorMP3(); // Activar sistema de error MP3
        return;
    }    // Verificar desactivación del error MP3
    if (pendiente_error_mp3_desactivar)
    {
        pendiente_error_mp3_desactivar = false; // Marcar que ya no hay un estado pendiente

        neopixel.desactivarErrorMP3(); // Desactivar sistema de error MP3
        return;
    }
}

void check_led_rf_rx(void)
{

    if (!pendiente_led_rf_rx) // Si no hay un LED RF pendiente, no hacer nada
        return;

    pendiente_led_rf_rx = false; // Marcar que ya no hay un LED RF pendiente

    // CAMBIO: RF puede interrumpir MP3, solo verificar busy para otros patrones
    if (neopixel.isBusy() && !neopixel.isMP3Active()) {
        return; // Solo bloquear si no es MP3 ejecutándose
    }

    rf_led_rx();              // Llamar a la función para indicar recepción RF
    neopixel.blinkRF433MHz(); // Usar la nueva API semántica
}

void rf_led_rx(void)
{
    int i, led;

    if (digitalRead(LED_STATUS) == LED_ST_ON) // buffer de estado de LED
        led = LED_ST_ON;                      //
    else                                      //
        led = LED_ST_OFF;                     //

    for (i = 0; i < 3; i++)                  // bucle que indica recepción de datos
    {                                        //
        digitalWrite(LED_STATUS, LED_ST_ON); //
        delay(100);
        digitalWrite(LED_STATUS, LED_ST_OFF);
        delay(100);
    }

    if (led == 1)                            // escritura de buffer de estado de led
        digitalWrite(LED_STATUS, LED_ST_ON); //
    else
        digitalWrite(LED_STATUS, LED_ST_OFF);
}

bool is_neopixel_busy(void)
{
    return neopixel.isBusy();
}

void async_stop_carga_sistema_and_wait(unsigned long timeout_ms)
{
    // Detener el efecto de carga del NeoPixel
    async_stop_carga_sistema();

    // Esperar activamente a que el sistema NeoPixel se libere
    unsigned long tiempoEspera = millis();
    while (is_neopixel_busy() && (millis() - tiempoEspera < timeout_ms))
    {
        delay(10); // Esperar en incrementos de 10ms
    }
}

// ============================================================================
// SISTEMA DE CONTROL LED MP3
// ============================================================================

/*
    @brief: Función para encender el LED MP3 de forma fija.
    @note: Indica que el reproductor MP3 está reconocido correctamente.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_led_mp3_on(void)
{
    if (pendiente_led_mp3_on) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_led_mp3_on = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                   // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para apagar el LED MP3.
    @note: Indica que el reproductor MP3 no fue reconocido (sin MicroSD o formato incorrecto).
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_led_mp3_off(void)
{
    if (pendiente_led_mp3_off) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_led_mp3_off = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                    // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para iniciar el parpadeo del LED MP3 durante reproducción.
    @note: Parpadeo de 500ms ON - 500ms OFF continuo mientras reproduce.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_led_mp3_reproduciendo(void)
{
    if (pendiente_led_mp3_reproduciendo) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_led_mp3_reproduciendo = true; // Marcar que hay un cambio de estado pendiente
    delay(10);                              // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para detener el parpadeo del LED MP3 con control de estado final.
    @param estado_final: -1 = mantener estado anterior, 0 = apagar LED, 1 = encender LED
    @note: Indica que la reproducción ha terminado con el estado especificado.
    @note: Se usa una bandera para evitar colisiones con la tarea de NeoPixel.
*/
void async_led_mp3_stop_reproduciendo(int8_t estado_final)
{
    if (pendiente_led_mp3_stop_reproduciendo) // Si ya hay un estado pendiente, no hacer nada
        return;

    pendiente_led_mp3_stop_reproduciendo = true; // Marcar que hay un cambio de estado pendiente
    pendiente_led_mp3_stop_estado = estado_final; // Guardar el estado final deseado
    delay(10);                                   // Esperar un poco para evitar colisiones
}

/*
    @brief: Función para procesar los cambios de estado del LED MP3.
    @note: Se ejecuta en el loop principal de la tarea NeoPixel.
*/
void check_led_mp3_status(void)
{
    // Verificar encendido del LED MP3 (prioridad alta - estado base)
    if (pendiente_led_mp3_on)
    {
        pendiente_led_mp3_on = false; // Marcar que ya no hay un estado pendiente

        // Detener modo reproduciendo si estaba activo
        led_mp3_estado_reproduciendo = false;
        
        digitalWrite(LED_MP3, HIGH); // Encender LED fijo
        return; // Salir para evitar procesar otros estados
    }

    // Verificar apagado del LED MP3 (prioridad alta)
    if (pendiente_led_mp3_off)
    {
        pendiente_led_mp3_off = false; // Marcar que ya no hay un estado pendiente

        // Detener modo reproduciendo si estaba activo
        led_mp3_estado_reproduciendo = false;
        
        digitalWrite(LED_MP3, LOW); // Apagar LED
        return; // Salir para evitar procesar otros estados
    }

    // Verificar inicio de reproducción
    if (pendiente_led_mp3_reproduciendo)
    {
        pendiente_led_mp3_reproduciendo = false; // Marcar que ya no hay un estado pendiente

        // Capturar estado actual antes de empezar reproducción
        led_mp3_estado_anterior = digitalRead(LED_MP3);
        
        led_mp3_estado_reproduciendo = true;  // Activar modo reproduciendo
        led_mp3_timer = millis();             // Inicializar timer
        led_mp3_estado_parpadeo = true;       // Empezar con LED ON
        digitalWrite(LED_MP3, HIGH);          // Encender LED
        return;
    }

    // Verificar fin de reproducción
    if (pendiente_led_mp3_stop_reproduciendo)
    {
        pendiente_led_mp3_stop_reproduciendo = false; // Marcar que ya no hay un estado pendiente

        led_mp3_estado_reproduciendo = false; // Desactivar modo reproduciendo
        
        // Aplicar estado final según parámetro
        switch (pendiente_led_mp3_stop_estado)
        {
            case -1: // Mantener estado anterior
                digitalWrite(LED_MP3, led_mp3_estado_anterior ? HIGH : LOW);
                break;
            case 0:  // Apagar LED (reproducción falló)
                digitalWrite(LED_MP3, LOW);
                break;
            case 1:  // Encender LED (reproducción exitosa)
            default:
                digitalWrite(LED_MP3, HIGH);
                break;
        }
        return;
    }

    // Manejar parpadeo durante reproducción
    if (led_mp3_estado_reproduciendo)
    {
        // Verificar si han pasado 500ms para cambiar estado
        if (millis() - led_mp3_timer >= 500)
        {
            led_mp3_timer = millis();                      // Resetear timer
            led_mp3_estado_parpadeo = !led_mp3_estado_parpadeo; // Alternar estado
            
            // Aplicar nuevo estado al LED
            digitalWrite(LED_MP3, led_mp3_estado_parpadeo ? HIGH : LOW);
        }
    }
}
