#include "NeoPixelAV.h"

// ============================================================================
// MÉTODOS DE CONTROL BÁSICO
// ============================================================================

void NeoPixelController::setAllLEDs(uint32_t color)
{
    if (!configurado)
        return;

    for (int i = 0; i < numPixels; i++)
    {
        pixels.setPixelColor(i, color);
    }
    pixels.show();
    colorActual = color;
}

void NeoPixelController::setPixelColor(uint8_t index, uint32_t color)
{
    if (!configurado || index >= numPixels)
        return;

    pixels.setPixelColor(index, color);
    pixels.show();
}

void NeoPixelController::apagar(void)
{
    setAllLEDs(0);
    patronActual = PATRON_APAGADO;
    estadoPatron = ESTADO_INACTIVO;
    busy = false;
}

void NeoPixelController::setColor(uint32_t color)
{
    setAllLEDs(color);
    patronActual = PATRON_FIJO;
    estadoPatron = ESTADO_INACTIVO;
    busy = false;
}

// ============================================================================
// MÉTODOS AUXILIARES
// ============================================================================

void NeoPixelController::resetPattern(void)
{
    patronActual = PATRON_APAGADO;
    estadoPatron = ESTADO_INACTIVO;
    pulsoActual = 0;
}

void NeoPixelController::restaurarPatronFondo(void)
{
    // Activar flag para resetear timer del servidor y comenzar desde estado inicial
    resetearTimerServidor = true;

    // Limpiar patrón anterior
    patronAnterior = PATRON_APAGADO;
    colorAnterior = 0;

    // Solo restaurar el patrón de servidor si está habilitado
    if (servidorHabilitado && patronFondo != PATRON_APAGADO)
    {
        setPatron(patronFondo, colorFondo);
    }
}
