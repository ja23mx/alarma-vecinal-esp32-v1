#include "NeoPixelAV.h"

// ============================================================================
// MÉTODOS DE PATRONES DE SISTEMA
// ============================================================================

void NeoPixelController::ejecutarPatronConfiguracion(void)
{
    // Patrón de configuración
    // Según tabla: LED fijo azul claro
    // Patrón: Todos los LEDs 1-16 azul claro fijo
    
    // Implementación: LEDs azul claro fijo
    setAllLEDs(COLOR_AZUL_CLARO);
    busy = false; // No es un patrón que bloquee
    estadoPatron = ESTADO_INACTIVO; // Patrón fijo, no necesita loop
}
