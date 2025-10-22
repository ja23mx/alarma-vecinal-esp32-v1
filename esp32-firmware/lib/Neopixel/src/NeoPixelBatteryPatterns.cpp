#include "NeoPixelAV.h"

// ============================================================================
// MÉTODOS DE PATRONES DE BATERÍA
// ============================================================================

void NeoPixelController::ejecutarPatronBateriaNormal(void)
{
    // Patrón de batería normal
    // Según tabla: Respaldo > 30% - Naranja, LED fijo por 1s, apagado por 4s (ciclo 5s)
    // Patrón: 1s ON naranja, 4s OFF, ciclo infinito de 5s

    static bool reset = true;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static bool estadoLED = true; // true = ON, false = OFF

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        busy = false;
        return;
    }

    // Los patrones de batería son de fondo, no activan busy
    busy = false;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        ultimoCambio = millis();
        estadoEnviado = false;
        estadoLED = true; // Empezar con ON
    }

    // Verificar timing: 1s ON, 4s OFF (ciclo 5s)
    unsigned long tiempoTranscurrido = millis() - ultimoCambio;
    bool cambioEstado = false;

    if (estadoLED && tiempoTranscurrido >= 1000) // ON por 1000ms
    {
        estadoLED = false;
        cambioEstado = true;
    }
    else if (!estadoLED && tiempoTranscurrido >= 4000) // OFF por 4000ms
    {
        estadoLED = true;
        cambioEstado = true;
    }

    // Cambiar estado y resetear timer
    if (cambioEstado)
    {
        ultimoCambio = millis();
        estadoEnviado = false;
    }

    // Enviar estado solo una vez por cambio
    if (!estadoEnviado)
    {
        if (estadoLED)
        {
            setAllLEDs(COLOR_NARANJA); // LEDs 1-16 naranjas
        }
        else
        {
            setAllLEDs(0); // LEDs 1-16 apagados
        }
        estadoEnviado = true;
    }
}

void NeoPixelController::ejecutarPatronBateriaLow(void)
{
    // Patrón de batería baja
    // Según tabla: Respaldo ≤ 30% - Rojo, LED fijo por 1s, apagado por 2s (ciclo 3s)
    // Patrón: 1s ON rojo, 2s OFF, ciclo infinito de 3s

    static bool reset = true;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static bool estadoLED = true; // true = ON, false = OFF

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        busy = false;
        return;
    }

    // Los patrones de batería son de fondo, no activan busy
    busy = false;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        ultimoCambio = millis();
        estadoEnviado = false;
        estadoLED = true; // Empezar con ON
    }

    // Verificar timing: 1s ON, 2s OFF (ciclo 3s)
    unsigned long tiempoTranscurrido = millis() - ultimoCambio;
    bool cambioEstado = false;

    if (estadoLED && tiempoTranscurrido >= 1000) // ON por 1000ms
    {
        estadoLED = false;
        cambioEstado = true;
    }
    else if (!estadoLED && tiempoTranscurrido >= 2000) // OFF por 2000ms
    {
        estadoLED = true;
        cambioEstado = true;
    }

    // Cambiar estado y resetear timer
    if (cambioEstado)
    {
        ultimoCambio = millis();
        estadoEnviado = false;
    }

    // Enviar estado solo una vez por cambio
    if (!estadoEnviado)
    {
        if (estadoLED)
        {
            setAllLEDs(COLOR_ROJO); // LEDs 1-16 rojos
        }
        else
        {
            setAllLEDs(0); // LEDs 1-16 apagados
        }
        estadoEnviado = true;
    }
}

void NeoPixelController::ejecutarPatronBateriaCritica(void)
{
    // Patrón de batería crítica
    // Patrón personalizado para niveles críticos de batería
    // Sugerencia: Parpadeo rápido rojo continuo

    static bool reset = true;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static bool estadoLED = true; // true = ON, false = OFF

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        busy = false;
        return;
    }

    // Los patrones de batería crítica podrían activar busy para mayor prioridad
    busy = true; // Crítico = mayor prioridad

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        ultimoCambio = millis();
        estadoEnviado = false;
        estadoLED = true; // Empezar con ON
    }

    // Verificar timing: Parpadeo rápido 0.3s ON, 0.3s OFF
    unsigned long tiempoTranscurrido = millis() - ultimoCambio;
    bool cambioEstado = false;

    if (tiempoTranscurrido >= 300) // Cambiar cada 300ms
    {
        estadoLED = !estadoLED; // Alternar estado
        cambioEstado = true;
    }

    // Cambiar estado y resetear timer
    if (cambioEstado)
    {
        ultimoCambio = millis();
        estadoEnviado = false;
    }

    // Enviar estado solo una vez por cambio
    if (!estadoEnviado)
    {
        if (estadoLED)
        {
            setAllLEDs(COLOR_ROJO); // LEDs 1-16 rojos
        }
        else
        {
            setAllLEDs(0); // LEDs 1-16 apagados
        }
        estadoEnviado = true;
    }
}

// ============================================================================
// API SEMÁNTICA "BLINK" - MÉTODOS PÚBLICOS DE BATERÍA
// ============================================================================

void NeoPixelController::blinkBatteryNormal(void)
{
    // Nivel normal de batería (>30%)
    // Naranja, LED fijo por 1s, apagado por 4s (ciclo 5s)
    // Este es un patrón de fondo que no interfiere con otros
    patronFondo = PATRON_FIJO;
    colorFondo = COLOR_NARANJA;
    servidorHabilitado = true;
    ejecutarPatronBateriaNormal();
}

void NeoPixelController::blinkBatteryLow(void)
{
    // Batería baja (15-30%)
    // Naranja, LED fijo por 1s, apagado por 2s (ciclo 3s)
    // Este es un patrón de fondo que no interfiere con otros
    patronFondo = PATRON_FIJO;
    colorFondo = COLOR_NARANJA;
    servidorHabilitado = true;
    ejecutarPatronBateriaLow();
}

void NeoPixelController::blinkBatteryCritical(void)
{
    // Batería crítica (<15%)
    // Rojo, LED fijo por 1s, apagado por 1s (ciclo 2s)
    // Este es un patrón de fondo que no interfiere con otros
    patronFondo = PATRON_FIJO;
    colorFondo = COLOR_ROJO;
    servidorHabilitado = true;
    ejecutarPatronBateriaCritica();
}
