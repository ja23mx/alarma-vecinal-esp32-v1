#include "NeoPixelAV.h"

// ============================================================================
// MÉTODOS DE PATRONES DE COMUNICACIÓN
// ============================================================================

void NeoPixelController::ejecutarPatronRapido1(void)
{
    // Patrón: 3 pulsos de 0.1s ON/OFF en 0.6s total
    // Tiempo: |0--0.1--0.2--0.3--0.4--0.5--0.6|
    // LED:    |■_|■_|■_|
    // Usado para: RF 433MHz (amarillo) y LoRa (verde)

    static bool reset = true;
    static uint8_t contadorPulsos = 1;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static uint32_t colorPatron = 0; // Guardar el color del patrón

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true; // Forzar reset de variables estáticas
        busy = false; // Liberar bandera
        return;
    }

    // Activar bandera busy
    busy = true;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        contadorPulsos = 1;
        ultimoCambio = millis();
        estadoEnviado = false;
        colorPatron = colorActual; // Guardar el color original
    }

    // Verificar si es tiempo de cambiar (cada 100ms)
    if (millis() - ultimoCambio >= 100)
    {
        ultimoCambio = millis();
        contadorPulsos++;
        estadoEnviado = false; // Permitir nuevo envío
    }

    // Enviar estado solo una vez por pulso
    if (!estadoEnviado)
    {
        if (contadorPulsos % 2 == 1) // Pulsos impares: ON
            setAllLEDs(colorPatron); // Usar el color guardado
        else                         // Pulsos pares: OFF
            setAllLEDs(0);
        estadoEnviado = true;
    }

    // Finalizar después de exactamente 6 pulsos (600ms)
    if (contadorPulsos >= 6)
    {
        reset = true; // Preparar para próxima ejecución
        busy = false; // Liberar bandera
        
        // Si era un blinkMP3, reactivar servidor
        if (colorPatron == COLOR_TURQUESA && pausarServidorParaMP3) {
            pausarServidorParaMP3 = false; // REACTIVAR servidor
        }
        
        apagar();
        
        // Restaurar patrón de fondo si está habilitado el servidor
        if (servidorHabilitado)
        {
            restaurarPatronFondo();
        }
    }
}

void NeoPixelController::ejecutarPatronRapido2(void)
{
    // Patrón: 3 pulsos de 0.1s ON/OFF en 0.6s total (igual que PATRON_RAPIDO_1)
    // Tiempo: |0--0.1--0.2--0.3--0.4--0.5--0.6|
    // LED:    |■_|■_|■_|
    // Usado para: Error MP3 (turquesa)

    static bool reset = true;
    static uint8_t contadorPulsos = 1;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static uint32_t colorPatron = 0;

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        busy = false; // Liberar bandera
        return;
    }

    // Activar bandera busy
    busy = true;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        contadorPulsos = 1;
        ultimoCambio = millis();
        estadoEnviado = false;
        colorPatron = colorActual;
    }

    // Verificar si es tiempo de cambiar (cada 100ms - igual que PATRON_RAPIDO_1)
    if (millis() - ultimoCambio >= 100)
    {
        ultimoCambio = millis();
        contadorPulsos++;
        estadoEnviado = false;
    }

    // Enviar estado solo una vez por pulso
    if (!estadoEnviado)
    {
        if (contadorPulsos % 2 == 1) // Pulsos impares: ON
            setAllLEDs(colorPatron);
        else                         // Pulsos pares: OFF
            setAllLEDs(0);
        estadoEnviado = true;
    }

    // Finalizar después de exactamente 6 pulsos (600ms)
    if (contadorPulsos >= 6)
    {
        reset = true;
        busy = false; // Liberar bandera
        
        // Si era un blinkMP3, reactivar servidor
        if (colorPatron == COLOR_TURQUESA && pausarServidorParaMP3) {
            pausarServidorParaMP3 = false; // REACTIVAR servidor
        }
        
        apagar();
        
        // Restaurar patrón de fondo si está habilitado el servidor
        if (servidorHabilitado)
        {
            restaurarPatronFondo();
        }
    }
}


// ============================================================================
// API SEMÁNTICA "BLINK" - MÉTODOS PÚBLICOS
// ============================================================================

void NeoPixelController::blinkServerExterno(void)
{
    // Parpadeo azul para comunicación con servidor externo
    // Internamente usa PATRON_RAPIDO_1 con color azul
    setPatron(PATRON_RAPIDO_1, COLOR_AZUL);
}

void NeoPixelController::blinkRF433MHz(void)
{
    // Parpadeo amarillo para comunicación RF 433MHz
    // Internamente usa PATRON_RAPIDO_1 con color amarillo
    setPatron(PATRON_RAPIDO_1, COLOR_AMARILLO);
}

void NeoPixelController::blinkLoRa(void)
{
    // Parpadeo verde para comunicación LoRa
    // Internamente usa PATRON_RAPIDO_1 con color verde
    setPatron(PATRON_RAPIDO_1, COLOR_VERDE);
}

void NeoPixelController::blinkMP3(void)
{
    // Parpadeo turquesa para error de MP3 (600ms)
    setPatron(PATRON_RAPIDO_2, COLOR_TURQUESA);
}

void NeoPixelController::activarErrorMP3(void)
{
    // Activa el sistema de error MP3 con nuevo sistema de turnos
    errorMP3Activo = true;
    // NO resetear contador - mantener el estado actual para que funcione inmediatamente
}

void NeoPixelController::desactivarErrorMP3(void)
{
    // Desactiva el sistema de error MP3
    errorMP3Activo = false;
    contadorCiclosServidor = 0; // Reiniciar contador al desactivar
}
