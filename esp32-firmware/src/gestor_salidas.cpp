#include <Arduino.h>
#include "driver/ledc.h"

#include "gestor_salidas.h"

#include "ConfigSistema.h"
#include "LogSistema.h"
#include "VariablesGlobales.h"
#include "GestorCmd.h"

#define MAX_DUTY_CYCLE 255
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_0 LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_8_BIT // Resolución de 8 bits (0-255)
#define LEDC_FREQUENCY (5000)          // Frecuencia en Hz

TaskHandle_t tareaGestionSalidaHandle = NULL; // Handle para la tarea de procesos

DATOS_INSTRUCCION cmdSalidaData; // Estructura para almacenar el comando de salida

uint8_t pines_sal_arr[GST_SALIDA_NUM_PRF] = {SALIDA_1, 0, 0, 0}; // Pines de las salidas de la alarma

// 0 = OFF, 1 = ON-OFF, 2 = PWM
uint8_t config_sal_arr[GST_SALIDA_NUM_PRF] = {0, 0, 0, 0}; // Configuración de las salidas de la alarma

unsigned long time_init_sal_arr[GST_SALIDA_NUM_PRF] = {0, 0, 0, 0}; // Tiempo de inicio de las salidas de la alarma
long time_limite_sal_arr[GST_SALIDA_NUM_PRF] = {0, 0, 0, 0};        // Tiempo limite de las salidas de la alarma

bool nueva_cnf = false; // Variable para verificar si hay una nueva configuración de la alarma

unsigned long timer_init_fade[4];   // Timers para el fade
unsigned long timer_limite_fade[4]; //
uint8_t nivel_fade_arr[4];          //
bool ciclo_fade[4];                 // Ciclo de fade 0-ascendente, 1-descendente

std::vector<String> infoComandosSalidas; // Vector para almacenar los comandos de salida
std::vector<long> numerosArray;          // Vector para almacenar los números separados

void crearTareaGestionSalida(void)
{

    LOG("\r\n\r\nCreando tarea de alarma...");

    // Crear la tarea
    xTaskCreate(
        tareaGestionSalida,       // Función de la tarea
        "TareaGestionSalida",     // Nombre de la tarea
        2048,                     // Tamaño del stack en palabras
        NULL,                     // Parámetros de entrada
        3,                        // Prioridad de la tarea
        &tareaGestionSalidaHandle // Guardar el handle de la tarea
    );

    delay(10);
}

void tareaGestionSalida(void *pvParameters)
{

    LOG("\r\n\r\nTarea de gestion salida iniciada...");

    setup_led_fade(); // Configurar el temporizador LEDC

    while (true)
    {

        cnfSalidas();                       // Llamar a la función de configuración de salidas
        revTimerSalida();                   // Llamar a la función de temporizador de salidas
        vTaskDelay(1 / portTICK_PERIOD_MS); // Esperar 10 ms antes de la siguiente iteración
    }
}

// Función para verificar el tiempo de apagado automático de la salida
void revTimerSalida(void)
{
    bool off_salida = false; // Variable para verificar si la salida debe apagarse

    for (uint8_t i = 0; i < GST_SALIDA_NUM_PRF; i++)
    {
        // Si la salida está configurada y el tiempo de espera ha pasado
        if ((config_sal_arr[i] == 1 || config_sal_arr[i] == 2))
        {
            // estado de salida depende de Salida de Audio 1
            if (time_limite_sal_arr[i] == -1 && cmdAudioBusy == false)
                off_salida = true;
            // estado de salida depende de base tiempo
            else if (time_limite_sal_arr[i] > 0 && millis() - time_init_sal_arr[i] >= time_limite_sal_arr[i])
                off_salida = true;  // Si el tiempo de espera ha pasado, apagar la salida
            else                    //
                off_salida = false; // Si no, no apagar la salida

            if (off_salida)
            {
                if (config_sal_arr[i] == 1)
                    digitalWrite(pines_sal_arr[i], LOW); // Apagar la salida
                else if (config_sal_arr[i] == 2)         // 2=PWM
                    ledcWrite(i, 0);                     // Apagar el canal PWM
                                                         //
                config_sal_arr[i] = 0;                   // Restablecer la configuración de la salida

                // LOG("\r\n\r\nSalida " + String(i + 1) + " apagada por tiempo de espera..."); // Mensaje de salida apagada por tiempo de espera

                cmdSalidasBusy = false; // Reiniciar el estado del comando de salida
            }
        }
        else if (config_sal_arr[i] == 3) // Si la salida está configurada como FADE
        {
            // estado de salida depende de Salida de Audio 1
            if (time_limite_sal_arr[i] == -1 && cmdAudioBusy == false)
                off_salida = true;
            // estado de salida depende de base tiempo
            else if (time_limite_sal_arr[i] > 0 && millis() - time_init_sal_arr[i] >= time_limite_sal_arr[i])
                off_salida = true; // Si el tiempo de espera ha pasado, apagar la salida
            else
                off_salida = false; // Si no, no apagar la salida

            if (off_salida)
            {
                apagar_salida_fade(i);  // Apagar la salida si el tiempo de espera ha pasado
                cmdSalidasBusy = false; // Reiniciar el estado del comando de salida
            }
            // revision de pin para fade
            else
                rev_pin_fade(i); // Llamar a la función de verificación de fade
        }
    }
}

void cnfSalidas(void)
{
    if (!nueva_cnf) //
        return;     // Salir de la función si no hay nuevo comando

    LOG("\r\n\r\ngestionarSalidas()");

    bool config_ok; // Variable para verificar la configuración

    for (const auto &comando : infoComandosSalidas) // Iterar sobre los comandos de salida
    {
        // LOG("\r\n\r\nComando: " + comando);
        if (GestorCmd.separarComando(comando, cmdSalidaData))
        {

            LOGF("\r\nComando: %s", cmdSalidaData.cmd.c_str());
            LOGF("\r\nNum Prf: %s", cmdSalidaData.numPeriferico.c_str());
            LOGF("\r\nParametros: %s", cmdSalidaData.parametros.c_str());

            if (cmdSalidaData.cmd == "SD") // Si el comando es "SD"
            {
                uint8_t numSalida = cmdSalidaData.numPeriferico.toInt(); // Convertir el número de salida a entero
                // salida fuera de rango
                if (numSalida > 0 || numSalida <= GST_SALIDA_NUM_PRF)
                {
                    int configuracion, carga; // configura las variables de configuración, carga y tiempo
                    long time;                // Inicializar el tiempo a 0

                    GestorCmd.separarArrNumeros(cmdSalidaData.parametros, ':', numerosArray); // Separar los parámetros
                    configuracion = numerosArray[0];                                          // Obtener la configuración
                    carga = numerosArray[1];                                                  // Obtener la carga
                    time = (numerosArray.size() > 2) ? numerosArray[2] : 0;                   // Obtener el tiempo, si no hay tiempo asignar 0

                    numSalida--; // Restar 1 al número de salida para usarlo como índice

                    // Verificar configuración y tiempo
                    if (configuracion >= 1 && configuracion <= 3)
                    {
                        // time -1 -> off de periferico depende Salida de Audio, cuando esta se apagague
                        // (cmdAudioBusy= false) se apagan las salidas
                        if (time == -1)
                        {
                            time_limite_sal_arr[numSalida] = -1;
                        }
                        // time 0 -> es tiempo maximo, time > 0 -> es tiempo en segundos
                        else if (time == 0 || (time >= 1 && time <= GST_SALIDA_SD_MAX_TM_1_SG))
                        {
                            config_ok = true;                                                                       // Marcar como configuración correcta
                            time_limite_sal_arr[numSalida] = (time == 0) ? GST_SALIDA_SD_MAX_TM_1_SG : time * 1000; // Asignar tiempo límite
                        }
                    }

                    /* LOGF("\r\n\r\nConfiguracion: %d", configuracion);
                    LOGF("\r\nCarga: %d", carga);
                    LOGF("\r\nTiempo: %d", time);
                    LOGF("\r\nSalida: %d", numSalida + 1); */

                    switch (configuracion)
                    {
                    ///////////////// OFF ///////////////////////////
                    case 0:
                        if (carga == 0 && time == 0)                         // apagado de salida
                        {                                                    //
                            if (config_sal_arr[numSalida] == 3)              // Si estaba configurado como FADE
                                apagar_salida_fade(numSalida);               // Apagar la salida FADE
                            if (config_sal_arr[numSalida] == 2)              // Si estaba configurado como PWM
                                ledcWrite(numSalida, 0);                     // Apagar el canal PWM
                            else if (config_sal_arr[numSalida] == 1)         // Si estaba configurado como ON-OFF
                            {                                                //
                                pinMode(pines_sal_arr[numSalida], OUTPUT);   // Configurar el pin como salida
                                digitalWrite(pines_sal_arr[numSalida], LOW); // Apagar la salida
                            }
                            config_sal_arr[numSalida] = 0; // Configuración de la salida

                            LOG("\r\n\r\nSalida OFF: " + String(numSalida + 1));

                            cmdSalidasBusy = false; // Reiniciar el estado del comando de salida
                        }
                        break;
                    ///////////////// ON  ///////////////////////////
                    case 1:
                        if (config_ok)
                        {
                            time_init_sal_arr[numSalida] = millis();      // Almacenar el tiempo de inicio de la salida
                            pinMode(pines_sal_arr[numSalida], OUTPUT);    // Configurar el pin como salida
                            digitalWrite(pines_sal_arr[numSalida], HIGH); //
                            config_sal_arr[numSalida] = 1;                // Configuración de la salida ON

                            LOG("\r\n\r\nSalida " + String(numSalida + 1) + " ON. TM: " + String(time_limite_sal_arr[numSalida]));

                            cmdSalidasBusy = true; // Reiniciar el estado del comando de salida
                        }
                        break;
                    ///////////////// PWM  ///////////////////////////
                    case 2:
                        if (config_ok == true && carga >= GST_SALIDA_PWM_MIN_DUTY_CYCLE && carga <= 100) // Si la carga es mayor o igual al minimo y menor o igual al maximo
                        {                                                                                //
                            uint16_t dutyCycle = map(carga, 1, 100, 1, 255);                             //
                            ledcSetup(numSalida, GST_SALIDA_PWM_FREQUENCY, GST_SALIDA_PWM_RESOLUTION);   // Configurar el canal PWM
                            ledcAttachPin(pines_sal_arr[numSalida], numSalida);                          // Adjuntar el pin al canal PWM
                            ledcWrite(numSalida, dutyCycle);                                             // Configurar el canal PWM
                            config_sal_arr[numSalida] = 2;                                               // Configuración de la salida PWM
                            time_init_sal_arr[numSalida] = millis();                                     // Almacenar el tiempo de inicio de la salida

                            LOG("\r\n\r\nSalida " + String(numSalida + 1) + " PWM. TM: " + String(time_limite_sal_arr[numSalida]) +
                                " Carga: " + String(carga) + " Duty: " + String(dutyCycle)); // Mensaje de salida PWM
                        }
                        break;
                    ///////////////// FADE  ///////////////////////////
                    case 3:
                        if (config_ok)
                        {
                            reconfigurar_pin_para_fade(numSalida);

                            LOG("\r\n\r\nSalida " + String(numSalida + 1) +
                                " Fade. TM: " + String(time_limite_sal_arr[numSalida]) +
                                " NIVEL: " + String(carga));

                            config_pin_fade(numSalida, carga, true); // Configurar el pin para fade ascendente
                            time_init_sal_arr[numSalida] = millis(); // Almacenar el tiempo de inicio de la salida

                            cmdSalidasBusy = true; // Reiniciar el estado del comando de salida
                        }
                        break;
                    }
                }
            }
        }
    }

    infoComandosSalidas.clear(); // Limpiar el vector de comandos de salida
    nueva_cnf = false;           // Marcar como no nueva configuración
}

void async_gestion_salida(void)
{
    if (!cmdSalidasPendiente) //
        return;               // Si no hay comandos de salida pendientes, salir de la función

    cmdSalidasPendiente = false;      // Marcar como no pendiente
    infoComandosSalidas = cmdSalidas; // Guardar los comandos en el vector de comandos de salida local
    nueva_cnf = true;                 // Marcar como nueva configuración
    delay(10);                        // Esperar 10 ms antes de continuar
}

void reconfigurar_pin_para_fade(uint8_t salida_idx)
{
    if (salida_idx >= GST_SALIDA_NUM_PRF || pines_sal_arr[salida_idx] == 0)
        return;

    // Detener cualquier PWM estándar activo
    ledcDetachPin(pines_sal_arr[salida_idx]);

    // Reconfigurar el canal LEDC para FADE
    ledc_channel_config_t ledc_channel = {
        .gpio_num = pines_sal_arr[salida_idx],
        .speed_mode = LEDC_MODE,
        .channel = (ledc_channel_t)salida_idx,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0};

    ledc_channel_config(&ledc_channel);
}

void setup_led_fade(void) // 3434
{
    // Configurar el temporizador LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};
    if (ledc_timer_config(&ledc_timer) != ESP_OK)
    {
        Serial.println("Error configurando el timer LEDC");
    }

    // Configurar los canales LEDC en un bucle
    for (int i = 0; i < 1; i++)
    {
        if (pines_sal_arr[i] != 0) // Solo configurar pines válidos (no 0)
        {
            pinMode(pines_sal_arr[i], OUTPUT); // Configurar el pin como salida

            ledc_channel_config_t ledc_channel = {
                .gpio_num = pines_sal_arr[i],
                .speed_mode = LEDC_MODE,
                .channel = (ledc_channel_t)i, // Asigna LEDC_CHANNEL_0, LEDC_CHANNEL_1, etc.
                .intr_type = LEDC_INTR_DISABLE,
                .timer_sel = LEDC_TIMER, // Todos los canales usan el mismo temporizador
                .duty = 0,
                .hpoint = 0};

            if (ledc_channel_config(&ledc_channel) != ESP_OK)
                Serial.printf("\r\n\r\nError configurando el canal LEDC %d para el pin %d\n", i, pines_sal_arr[i]);
            else
                Serial.printf("\r\n\r\nCanal LEDC %d configurado para el pin %d\n", i, pines_sal_arr[i]);
        }
    }

    // Instalar el servicio de fade (solo se hace una vez para todos los canales)
    if (ledc_fade_func_install(0) != ESP_OK) // 0 son flags de interrupción, por defecto
    {
        Serial.println("Error instalando el servicio de fade LEDC");
    }
}

void rev_pin_fade(uint8_t salida_idx)
{
    if (salida_idx >= GST_SALIDA_NUM_PRF || config_sal_arr[salida_idx] == 0)
    {
        // Serial.printf("\r\nError: salida_idx %d fuera de rango.", salida_idx);
        return;
    }

    if (millis() - timer_init_fade[salida_idx] >= timer_limite_fade[salida_idx])
    {
        // Cambiar el estado del ciclo de fade
        cambio_fade(salida_idx); // Cambiar el nivel de fade
    }
}

void cambio_fade(uint8_t salida_idx)
{

    if (salida_idx >= GST_SALIDA_NUM_PRF)
    {
        Serial.printf("\r\nError: salida_idx %d fuera de rango.", salida_idx);
        return;
    }

    ciclo_fade[salida_idx] = !ciclo_fade[salida_idx];

    // Serial.printf("\r\n\r\nCambiando ciclo de fade %d a %d", salida_idx, ciclo_fade[salida_idx]);

    config_pin_fade(salida_idx, nivel_fade_arr[salida_idx], ciclo_fade[salida_idx]); // Cambiar el nivel de fade
}

void config_pin_fade(uint8_t salida_idx, uint8_t nivel_fade, bool flaco_fade) // Renombrado 'salida' a 'salida_idx' para claridad
{

    if (salida_idx >= GST_SALIDA_NUM_PRF)
    {
        Serial.printf("\r\nError: salida_idx %d fuera de rango.", salida_idx);
        return;
    }

    unsigned long fadeDurationMs = 200 + ((nivel_fade - 1) * 100);

    // Serial.printf("\r\n\r\nConfigurando Fade %d: %d ms, %s", salida_idx, fadeDurationMs, (flaco_fade ? "ASCENDENTE" : "DESCENDENTE"));

    init_pin_fade(salida_idx, fadeDurationMs, flaco_fade); //

    config_sal_arr[salida_idx] = 3;                 // Configuración de la salida como FADE
    timer_init_fade[salida_idx] = millis();         // Guardar el tiempo de inicio del fade
    nivel_fade_arr[salida_idx] = nivel_fade;        // Guardar el nivel de fade
    timer_limite_fade[salida_idx] = fadeDurationMs; // Guardar el tiempo límite del fade
    ciclo_fade[salida_idx] = flaco_fade;            // bandera de estado de fade 0-descendente, 1-ascendente
}

bool init_pin_fade(uint8_t salida_idx, unsigned long fade_time, bool flanco_fade)
{
    if (salida_idx >= GST_SALIDA_NUM_PRF || pines_sal_arr[salida_idx] == 0)
    {
        Serial.printf("\r\nError en init_pin_fade: salida_idx %d inválido o pin no configurado.", salida_idx);
        return false;
    }

    ledc_channel_t channel = (ledc_channel_t)salida_idx;

    uint16_t target_duty_value;
    uint16_t start_duty_for_print;
    const char *fade_description;

    if (flanco_fade) // true significa Ascendente
    {
        // Configurar para fade Ascendente (ej: 0 a MAX_DUTY_CYCLE)
        // Serial.printf("\r\n\r\n\r\nConfig Fade ASCENDENTE pin %d (Canal %d)...", pines_sal_arr[salida_idx], channel);
        // Asegurar que el fade comienza desde 0
        ledc_set_duty(LEDC_MODE, channel, 20);
        ledc_update_duty(LEDC_MODE, channel);

        target_duty_value = MAX_DUTY_CYCLE;
        start_duty_for_print = 10;
        fade_description = "ASCENDENTE";
    }
    else // false significa Descendente
    {
        // Configurar para fade Descendente (ej: MAX_DUTY_CYCLE a 0)
        // Serial.printf("\r\n\r\n\r\nConfig Fade DESCENDENTE  pin %d (Canal %d)...", pines_sal_arr[salida_idx], channel);
        // Asegurar que el fade comienza desde MAX_DUTY_CYCLE
        ledc_set_duty(LEDC_MODE, channel, MAX_DUTY_CYCLE);
        ledc_update_duty(LEDC_MODE, channel);

        target_duty_value = 20; // Objetivo para fade descendente. Cambia a 1 si quieres "hasta 1".
        start_duty_for_print = MAX_DUTY_CYCLE;
        fade_description = "DESCENDENTE";
    }

    // Serial.printf("\r\nInfo Fade %s: Pin %d (Canal %d), Desde %d a %d durante %d ms",
    //               fade_description, pines_sal_arr[salida_idx], channel, start_duty_for_print, target_duty_value, fade_time);

    esp_err_t set_fade_err = ledc_set_fade_with_time(LEDC_MODE,
                                                     channel,
                                                     target_duty_value, // Usar el valor objetivo correcto
                                                     fade_time);
    if (set_fade_err == ESP_OK)
    {
        esp_err_t start_fade_err = ledc_fade_start(LEDC_MODE,
                                                   channel,
                                                   LEDC_FADE_NO_WAIT);
        if (start_fade_err == ESP_OK)
        {
            // Serial.printf("\r\nFade %s iniciado correctamente.", fade_description);
            return true;
        }
        else
        {
            // Serial.printf("\r\nFallo init Fade %s (start error 0x%x).", fade_description, start_fade_err);
        }
    }
    else
    {
        // Serial.printf("\r\nFallo config del Fade %s (set error 0x%x).", fade_description, set_fade_err);
    }
    return false;
}

void apagar_salida_fade(uint8_t salida_idx)
{
    if (salida_idx >= GST_SALIDA_NUM_PRF)
    {
        Serial.printf("\r\nError en apagar_salida_fade: salida_idx %d fuera de rango.", salida_idx);
        return;
    }

    config_sal_arr[salida_idx] = 0; // Configuración de la salida como OFF

    // Asumimos que salida_idx corresponde directamente al número de canal LEDC
    ledc_channel_t channel = (ledc_channel_t)salida_idx;

    // Establecer el ciclo de trabajo a 0
    esp_err_t duty_err = ledc_set_duty(LEDC_MODE, channel, 0);
    if (duty_err != ESP_OK)
    {
        Serial.printf("\r\nError al establecer duty a 0 para canal %d (error 0x%x)", channel, duty_err);
        return;
    }

    // Aplicar el nuevo ciclo de trabajo
    esp_err_t update_err = ledc_update_duty(LEDC_MODE, channel);
    if (update_err != ESP_OK)
    {
        Serial.printf("\r\nError al actualizar duty a 0 para canal %d (error 0x%x)", channel, update_err);
        return;
    }

    Serial.printf("\r\nSalida LEDC en canal %d (pin %d) apagada.", channel, pines_sal_arr[salida_idx]);
}
