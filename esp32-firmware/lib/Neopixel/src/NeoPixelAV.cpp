#include "NeoPixelAV.h"
#include "LOGSistema.h"

// Constructor
NeoPixelController::NeoPixelController(uint8_t pin, uint16_t numPixels)
    : pixels(numPixels, pin, NEO_GRB + NEO_KHZ800), pin(pin), numPixels(numPixels)
{
    configurado = false;
    patronActual = PATRON_APAGADO;
    colorActual = 0;
    tiempoInicio = 0;
    estadoPatron = ESTADO_INACTIVO;
    pulsoActual = 0;
    stopAbsoluto = false;
    busy = false;
    cargaSistemaDetener = false;
    
    // Inicializar sistema de prioridades para servidor
    patronFondo = PATRON_APAGADO;
    colorFondo = 0;
    servidorHabilitado = false;
    patronAnterior = PATRON_APAGADO;
    colorAnterior = 0;
    resetearTimerServidor = false;
    
    // Inicializar sistema de error MP3 (nuevo sistema de turnos)
    errorMP3Activo = false;
    contadorCiclosServidor = 0;
    timerErrorMP3 = 0;
    pausarServidorParaMP3 = false;
}

void NeoPixelController::init(void)
{
    LOG("\r\n\r\nInicializando NeoPixel...");

    pixels.begin();
    pixels.clear();
    pixels.show();

    configurado = true;
    patronActual = PATRON_APAGADO;
    colorActual = 0;
    estadoPatron = ESTADO_INACTIVO;
}

void NeoPixelController::iniciarCargaSistema(void)
{
    if (!configurado)
        return;

    cargaSistemaDetener = false;
    setPatron(PATRON_CARGA_SISTEMA, COLOR_VERDE);
}

void NeoPixelController::detenerCargaSistema(void)
{
    if (patronActual == PATRON_CARGA_SISTEMA)
    {
        cargaSistemaDetener = true;
    }
}

void NeoPixelController::setServidorConectado(void)
{
    if (!configurado)
        return;

    // Guardar patrón actual si hay una interrupción en progreso
    if (busy && (patronActual == PATRON_RAPIDO_1 || patronActual == PATRON_RAPIDO_2))
    {
        patronAnterior = patronActual;
        colorAnterior = colorActual;
    }

    // Establecer patrón de fondo
    patronFondo = PATRON_SERVIDOR_CONECTADO;
    colorFondo = COLOR_AZUL;
    servidorHabilitado = true;

    // Solo activar directamente si no hay patrón de interrupción activo
    if (!busy)
    {
        setPatron(PATRON_SERVIDOR_CONECTADO, COLOR_AZUL);
    }
}

void NeoPixelController::setServidorDesconectado(void)
{
    if (!configurado)
        return;

    // Guardar patrón actual si hay una interrupción en progreso
    if (busy && (patronActual == PATRON_RAPIDO_1 || patronActual == PATRON_RAPIDO_2))
    {
        patronAnterior = patronActual;
        colorAnterior = colorActual;
    }

    // Establecer patrón de fondo
    patronFondo = PATRON_SERVIDOR_DESCONECTADO;
    colorFondo = COLOR_VIOLETA;
    servidorHabilitado = true;

    // Solo activar directamente si no hay patrón de interrupción activo
    if (!busy)
    {
        setPatron(PATRON_SERVIDOR_DESCONECTADO, COLOR_VIOLETA);
    }
}

void NeoPixelController::deshabilitarServidor(void)
{
    // Deshabilitar sistema de servidor
    servidorHabilitado = false;
    patronFondo = PATRON_APAGADO;
    colorFondo = 0;
    patronAnterior = PATRON_APAGADO;
    colorAnterior = 0;

    // Si se está ejecutando blinkMP3 cuando se deshabilita servidor, cancelarlo
    if (patronActual == PATRON_RAPIDO_1 && colorActual == COLOR_TURQUESA && errorMP3Activo) {
        stopAbsoluto = true;
        // Timer anterior removido - nuevo sistema en Paso 2
    }

    // Si actualmente se está ejecutando un patrón de servidor, apagarlo
    if (patronActual == PATRON_SERVIDOR_CONECTADO || patronActual == PATRON_SERVIDOR_DESCONECTADO)
    {
        apagar();
    }
}

void NeoPixelController::setPatron(PatronType_t patron, uint32_t color)
{
    if (!configurado)
        return;

    // Si se está ejecutando un blinkMP3 y llega RF/LoRa/Server, interrumpir inmediatamente
    if (patronActual == PATRON_RAPIDO_2 && colorActual == COLOR_TURQUESA && errorMP3Activo) {
        if (patron == PATRON_RAPIDO_1 && (color == COLOR_AMARILLO || color == COLOR_VERDE || color == COLOR_AZUL)) {
            // Cancelar blinkMP3 y reiniciar timer
            stopAbsoluto = true;
            pausarServidorParaMP3 = false; // REACTIVAR servidor al interrumpir MP3
            // Timer anterior removido - nuevo sistema en Paso 2
        }
    }

    // Verificar si el mismo patrón ya está ejecutándose
    bool mismoPatronEjecutandose = (patronActual == patron && estadoPatron != ESTADO_INACTIVO);

    patronActual = patron;
    colorActual = color;

    // Solo reiniciar tiempo si no es el mismo patrón
    if (!mismoPatronEjecutandose)
    {
        tiempoInicio = millis();
        estadoPatron = ESTADO_ENCENDIDO;
        pulsoActual = 0;
    }

    switch (patron)
    {
    case PATRON_APAGADO:
        apagar();
        break;
    case PATRON_FIJO:
        setAllLEDs(color);
        estadoPatron = ESTADO_INACTIVO;
        break;
    case PATRON_RAPIDO_1:
    case PATRON_RAPIDO_2:
        // Patrones de interrupción: activar busy
        busy = true;
        break;
    case PATRON_CARGA_SISTEMA:
        // Patrón especial: activar busy
        busy = true;
        break;
    case PATRON_SERVIDOR_CONECTADO:
    case PATRON_SERVIDOR_DESCONECTADO:
        // Patrones de fondo: no activar busy, permitir interrupciones
        // MANTENER estadoPatron = ESTADO_ENCENDIDO para que se ejecute
        busy = false;
        break;
    }
}

void NeoPixelController::loop(void)
{
    // Verificar stop absoluto primero
    if (stopAbsoluto)
    {
        stopAbsoluto = false; // Reset flag
        return;
    }

    if (!configurado)
        return;

    // [PASO 1 - LIMPIEZA] Lógica anterior de error MP3 temporalmente comentada
    // TODO: Implementar nuevo sistema de turnos en Paso 3
    /*
    if (!servidorHabilitado && estadoPatron == ESTADO_INACTIVO && puedeEjecutarErrorMP3())
    {
        Serial.println("[LOOP] Ejecutando blinkMP3 - servidor deshabilitado");
        blinkMP3();
        return;
    }
    */

    if (estadoPatron == ESTADO_INACTIVO)
        return;

    switch (patronActual)
    {
    case PATRON_RAPIDO_1:
        ejecutarPatronRapido1();
        break;
    case PATRON_RAPIDO_2:
        ejecutarPatronRapido2();
        break;
    case PATRON_CARGA_SISTEMA:
        ejecutarPatronCargaSistema();
        break;
    case PATRON_SERVIDOR_CONECTADO:
        ejecutarPatronServidorConectado();
        break;
    case PATRON_SERVIDOR_DESCONECTADO:
        ejecutarPatronServidorDesconectado();
        break;
    default:
        break;
    }
}

bool NeoPixelController::isConfigured(void) const
{
    return configurado;
}

PatronType_t NeoPixelController::getCurrentPattern(void) const
{
    return patronActual;
}

bool NeoPixelController::isBusy(void) const
{
    return busy;
}

bool NeoPixelController::isMP3Active(void) const
{
    return (patronActual == PATRON_RAPIDO_2 && colorActual == COLOR_TURQUESA && errorMP3Activo);
}

// ============================================================================
// MÉTODOS DE PATRONES DE SISTEMA
// ============================================================================

void NeoPixelController::ejecutarPatronCargaSistema(void)
{
    // Patrón: Efecto de carga con LEDs 2-16, LED 1 siempre verde
    // Efecto: [■][■][______________] -> [■][■■][_____________] -> [■][■■■][____________] -> ... -> [■][_____________■■■] -> [■][______________■■] -> [■][_______________■] -> pausa
    // Timing: 400ms por paso, ciclo normal; al detener, secuencia final de parpadeo rápido y encendido final

    static bool reset = true;
    static uint8_t paso = 0;
    static unsigned long ultimoCambio = 0;
    static bool estadoEnviado = false;
    static bool secuenciaFinal = false;
    static uint8_t cicloFinal = 0;

    // Verificar stop absoluto al inicio
    if (stopAbsoluto)
    {
        reset = true;
        secuenciaFinal = false;
        cicloFinal = 0;
        busy = false;
        return;
    }

    // Activar bandera busy
    busy = true;

    // Primera vez o reinicio
    if (reset)
    {
        reset = false;
        paso = 0;
        ultimoCambio = millis();
        estadoEnviado = false;
        secuenciaFinal = false;
        cicloFinal = 0;

        // LED 1 siempre verde durante toda la secuencia
        setPixelColor(0, COLOR_VERDE);
    }

    // Verificar si se debe detener y pasar a secuencia final
    if (cargaSistemaDetener && !secuenciaFinal)
    {
        secuenciaFinal = true;
        cicloFinal = 0;
        ultimoCambio = millis();
        estadoEnviado = false;
    }

    // Ejecutar secuencia final
    if (secuenciaFinal)
    {
        if (cicloFinal < 4)
        {
            // Timing para secuencia final (300ms por estado) - solo para ciclos 0-3
            if (millis() - ultimoCambio >= 300)
            {
                ultimoCambio = millis();
                estadoEnviado = false;
            }

            if (!estadoEnviado)
            {
                if (cicloFinal % 2 == 0)
                { // Estados pares: ON
                    for (int i = 1; i < 16; i++)
                    { // LEDs 2-16 (índices 1-15)
                        setPixelColor(i, COLOR_VERDE);
                    }
                }
                else
                { // Estados impares: OFF
                    for (int i = 1; i < 16; i++)
                    { // LEDs 2-16 (índices 1-15)
                        setPixelColor(i, 0);
                    }
                }
                cicloFinal++;
                estadoEnviado = true;
            }
        }
        else if (cicloFinal == 4)
        { // Estado final: ON por 1000ms
            for (int i = 1; i < 16; i++)
            { // LEDs 2-16 (índices 1-15)
                setPixelColor(i, COLOR_VERDE);
            }
            cicloFinal++;
            ultimoCambio = millis(); // Reset timer para 1000ms - SOLO AQUÍ
        }
        else if (cicloFinal == 5)
        {
            // Verificar timeout de 1000ms - NO resetear ultimoCambio aquí
            if (millis() - ultimoCambio >= 1000)
            {                  // Apagar todo después de 1000ms
                setAllLEDs(0); // Apagar todos los LEDs (1-16)
                reset = true;
                secuenciaFinal = false;
                cicloFinal = 0;
                busy = false; // Liberar bandera busy
                cargaSistemaDetener = false;
                patronActual = PATRON_APAGADO;
                estadoPatron = ESTADO_INACTIVO;
                return;
            }
        }
        return;
    }

    // Efecto de carga normal (repetir ciclo cada ~7.2 segundos)
    if (millis() - ultimoCambio >= 400) // 400ms por paso
    {
        ultimoCambio = millis();
        paso++;
        if (paso > 17)
            paso = 0; // Reiniciar ciclo (pasos 0-16, paso 17 es pausa)
        estadoEnviado = false;
    }

    // Enviar estado solo una vez por paso
    if (!estadoEnviado)
    {
        // Apagar LEDs 2-16 primero (índices 1-15)
        for (int i = 1; i < 16; i++)
        {
            setPixelColor(i, 0);
        }

        // Patrón progresivo: 1→2→3→3→3...→3→2→1→pausa
        if (paso < 17)
        {
            if (paso == 0)
            { // Paso 0: LED 2 ON (1 LED)
                setPixelColor(1, COLOR_VERDE);
            }
            else if (paso == 1)
            { // Paso 1: LEDs 2,3 ON (2 LEDs)
                setPixelColor(1, COLOR_VERDE);
                setPixelColor(2, COLOR_VERDE);
            }
            else if (paso >= 2 && paso <= 14)
            { // Pasos 2-14: 3 LEDs consecutivos desplazándose
                for (int j = 0; j < 3; j++)
                {
                    setPixelColor(paso + j, COLOR_VERDE); // LEDs desde índice paso hasta paso+2
                }
            }
            else if (paso == 15)
            { // Paso 15: LEDs 15,16 ON (2 LEDs)
                setPixelColor(14, COLOR_VERDE);
                setPixelColor(15, COLOR_VERDE);
            }
            else if (paso == 16)
            { // Paso 16: LED 16 ON (1 LED)
                setPixelColor(15, COLOR_VERDE);
            }
        }
        // paso == 17 es pausa (todos LEDs 2-16 OFF, solo LED 1 verde)

        estadoEnviado = true;
    }
}

void NeoPixelController::pararTodo(void)
{
    stopAbsoluto = true;
    busy = false;   // Liberar bandera inmediatamente
    apagar();       // Apagar LEDs inmediatamente
    resetPattern(); // Resetear estado
}
