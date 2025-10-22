#include "NeoPixelAV.h"

// ============================================================================
// MÉTODOS DE PATRONES DE SERVIDOR (PASO 1: LIMPIEZA COMPLETADA)
// ============================================================================

void NeoPixelController::ejecutarPatronServidorConectado(void)
{
    // Patrón: 2s OFF, 0.5s ON azul, ciclo infinito de 2.5s
    // Timing: |0----2s----2.5s|
    // LED:    |_____■■■■■_____|

    static bool reset = true;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static bool estadoLED = false; // true = ON, false = OFF - EMPEZAR CON OFF

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        busy = false;
        return;
    }

    // Verificar si hay que resetear timer después de interrupción
    if (resetearTimerServidor)
    {
        reset = true; // Forzar reset para empezar desde estado inicial
        resetearTimerServidor = false; // Limpiar bandera
    }

    // Activar bandera busy solo durante patrones de interrupción
    // Los patrones de fondo no activan busy
    busy = false;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        ultimoCambio = millis();
        estadoEnviado = false;
        estadoLED = false; // Empezar con OFF (2s)
    }

    // ============ MANEJO DE ERROR MP3 - PRIORIDAD ALTA ============
    if (errorMP3Activo && timerErrorMP3 > 0) {
        unsigned long tiempoErrorMP3 = millis() - timerErrorMP3;
        
        // Secuencia: 1s espera + 600ms patrón = 1.6s total
        if (tiempoErrorMP3 >= 1000 && tiempoErrorMP3 < 1600) {
            // Ejecutar patrón de error MP3 durante 600ms
            blinkMP3();
            return; // SALIR - No ejecutar patrón servidor durante MP3
        } else if (tiempoErrorMP3 >= 1600) {
            // Finalizar ciclo de error MP3
            timerErrorMP3 = 0; // Reset timer
        }
        // Durante 0-1000ms (espera): continuar con patrón servidor normal
    }

    // ============ PATRÓN SERVIDOR NORMAL ============
    // Verificar timing: 2s OFF, 0.5s ON
    unsigned long tiempoTranscurrido = millis() - ultimoCambio;
    bool cambioEstado = false;

    if (!estadoLED && tiempoTranscurrido >= 2000) // OFF por 2000ms
    {
        estadoLED = true;
        cambioEstado = true;
    }
    else if (estadoLED && tiempoTranscurrido >= 500) // ON por 500ms
    {
        estadoLED = false;
        cambioEstado = true;
    }

    // Cambiar estado y resetear timer
    if (cambioEstado)
    {
        ultimoCambio = millis();
        estadoEnviado = false;
        
        // PASO 2: Contador de ciclos para sistema de turnos (SERVIDOR CONECTADO)
        // Un ciclo completo = OFF→ON→OFF (2.5s total)
        if (!estadoLED) { // Al finalizar el ciclo ON→OFF
            contadorCiclosServidor++;
            
            // Verificar si es momento de ejecutar error MP3
            if (errorMP3Activo && contadorCiclosServidor >= 2) {
                contadorCiclosServidor = 0; // Reset contador
                timerErrorMP3 = millis(); // Iniciar timer para espera + patrón
            }
        }
    }

    // Enviar estado solo una vez por cambio
    if (!estadoEnviado)
    {
        if (estadoLED)
        {
            setAllLEDs(COLOR_AZUL); // LEDs 1-16 azules
        }
        else
        {
            setAllLEDs(0); // LEDs 1-16 apagados
        }
        estadoEnviado = true;
    }
}

void NeoPixelController::ejecutarPatronServidorDesconectado(void)
{
    // PAUSA DURANTE MP3: Si está ejecutándose blinkMP3, suspender servidor
    if (pausarServidorParaMP3) {
        return; // No ejecutar patrón servidor durante MP3
    }

    // Patrón: 2s OFF, 0.5s ON violeta, ciclo infinito de 2.5s
    // Timing: |0----2s----2.5s|
    // LED:    |_____■■■■■_____|

    static bool reset = true;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static bool estadoLED = false; // true = ON, false = OFF - EMPEZAR CON OFF

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        busy = false;
        return;
    }

    // Verificar si hay que resetear timer después de interrupción
    if (resetearTimerServidor)
    {
        reset = true; // Forzar reset para empezar desde estado inicial
        resetearTimerServidor = false; // Limpiar bandera
    }

    // Activar bandera busy solo durante patrones de interrupción
    // Los patrones de fondo no activan busy
    busy = false;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        ultimoCambio = millis();
        estadoEnviado = false;
        estadoLED = false; // Empezar con OFF (2s)
    }

    // Verificar timing: 2s OFF, 0.5s ON
    unsigned long tiempoTranscurrido = millis() - ultimoCambio;
    bool cambioEstado = false;

    if (!estadoLED && tiempoTranscurrido >= 2000) // OFF por 2000ms
    {
        estadoLED = true;
        cambioEstado = true;
    }
    else if (estadoLED && tiempoTranscurrido >= 500) // ON por 500ms
    {
        estadoLED = false;
        cambioEstado = true;
    }

    // Cambiar estado y resetear timer
    if (cambioEstado)
    {
        ultimoCambio = millis();
        estadoEnviado = false;
        
        // PASO 2: Contador de ciclos para sistema de turnos (SERVIDOR DESCONECTADO)
        // Un ciclo completo = OFF→ON→OFF (2.5s total)
        if (!estadoLED) { // Al finalizar el ciclo ON→OFF
            contadorCiclosServidor++;
            
            // Verificar si es momento de ejecutar error MP3
            if (errorMP3Activo && contadorCiclosServidor >= 2) {
                contadorCiclosServidor = 0; // Reset contador
                timerErrorMP3 = millis(); // Iniciar timer para espera + patrón
                pausarServidorParaMP3 = true; // PAUSAR servidor desde el inicio
            }
        }
    }

    // Enviar estado solo una vez por cambio
    if (!estadoEnviado)
    {
        if (estadoLED)
            setAllLEDs(COLOR_VIOLETA); // LEDs 1-16 violetas
        else
        {
            // PASO 2: Verificar si es momento de mostrar error MP3
            if (errorMP3Activo && timerErrorMP3 > 0) {
                unsigned long tiempoErrorMP3 = millis() - timerErrorMP3;
                
                // Secuencia: 1s espera + 600ms patrón = 1.6s total
                if (tiempoErrorMP3 >= 1000 && tiempoErrorMP3 < 1600) {
                    // Ejecutar patrón de error MP3 durante 600ms
                    blinkMP3();
                    return; // No continuar con LEDs apagados
                } else if (tiempoErrorMP3 >= 1600) {
                    // Finalizar ciclo de error MP3
                    timerErrorMP3 = 0; // Reset timer
                    pausarServidorParaMP3 = false; // REACTIVAR servidor
                }
            }
            
            setAllLEDs(0); // LEDs 1-16 apagados (sistema anterior MP3 removido)
        }
        estadoEnviado = true;
    }
}
