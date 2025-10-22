#ifndef NEOPIXEL_AV_H
#define NEOPIXEL_AV_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Enumeración de patrones
typedef enum
{
    PATRON_APAGADO,
    PATRON_FIJO,
    PATRON_RAPIDO_1,             // 3 pulsos de 0.1s ON/OFF en 0.6s
    PATRON_RAPIDO_2,             // 2 pulsos dobles en 0.6s
    PATRON_CARGA_SISTEMA,        // Efecto de carga del sistema
    PATRON_SERVIDOR_CONECTADO,   // LED fijo 1s ON, 2s OFF (ciclo 3s) - Azul
    PATRON_SERVIDOR_DESCONECTADO // LED fijo 1s ON, 2s OFF (ciclo 3s) - Violeta
} PatronType_t;

// Enumeración de colores predefinidos
typedef enum
{
    COLOR_AZUL = 0x0000FF,       // Azul
    COLOR_VERDE = 0x00FF00,      // Verde
    COLOR_AMARILLO = 0xFFFF00,   // Amarillo
    COLOR_TURQUESA = 0x40E0D0,   // Turquesa
    COLOR_NARANJA = 0xFFA500,    // Naranja
    COLOR_ROJO = 0xFF0000,       // Rojo
    COLOR_VIOLETA = 0x800080,    // Violeta
    COLOR_AZUL_CLARO = 0x00BFFF, // Azul claro
    COLOR_APAGADO = 0x000000     // Negro/Apagado
} ColorType_t;

// Estados internos para patrones
typedef enum
{
    ESTADO_INACTIVO,
    ESTADO_ENCENDIDO,
    ESTADO_APAGADO,
    ESTADO_FINALIZADO
} EstadoPatron_t;

class NeoPixelController
{
protected:
    // Variables accesibles por métodos implementados en archivos separados
    Adafruit_NeoPixel pixels;
    uint8_t pin;
    uint16_t numPixels;

    // Estado interno
    bool configurado;
    PatronType_t patronActual;
    uint32_t colorActual;
    unsigned long tiempoInicio;
    EstadoPatron_t estadoPatron;
    uint8_t pulsoActual;
    bool stopAbsoluto;
    bool busy;
    bool cargaSistemaDetener;

    // Sistema de prioridades para patrones de servidor
    PatronType_t patronFondo;    // Patrón de fondo (servidor)
    uint32_t colorFondo;         // Color del patrón de fondo
    bool servidorHabilitado;     // Flag si hay patrón de servidor activo
    PatronType_t patronAnterior; // Patrón anterior antes de interrupción
    uint32_t colorAnterior;      // Color anterior antes de interrupción
    bool resetearTimerServidor;  // Flag para resetear timer del servidor después de interrupción

    // Sistema de error MP3 (nuevo sistema de turnos)
    bool errorMP3Activo;            // Flag si hay error de MP3 activo
    uint8_t contadorCiclosServidor; // Contador para alternar entre servidor y error MP3
    unsigned long timerErrorMP3;    // Timer para el nuevo sistema de error MP3
    bool pausarServidorParaMP3;     // Flag para pausar servidor durante ejecución de blinkMP3

private:
    // ============= MÉTODOS DE PATRONES DE SISTEMA =============
    void ejecutarPatronCargaSistema(void);
    void ejecutarPatronConfiguracion(void);

    // ============= MÉTODOS DE PATRONES DE COMUNICACIÓN =============
    void ejecutarPatronRapido1(void);
    void ejecutarPatronRapido2(void);

    // ============= MÉTODOS DE PATRONES DE SERVIDOR =============
    void ejecutarPatronServidorConectado(void);
    void ejecutarPatronServidorDesconectado(void);

    // ============= MÉTODOS DE PATRONES DE BATERÍA =============
    void ejecutarPatronBateriaNormal(void);
    void ejecutarPatronBateriaLow(void);
    void ejecutarPatronBateriaCritica(void);

    // Métodos internos
    void resetPattern(void);
    void restaurarPatronFondo(void); // Restaura patrón de servidor después de interrupción

    // Método interno de patrones (usado por API semántica)
    void setPatron(PatronType_t patron, uint32_t color);

public:
    // Constructor
    NeoPixelController(uint8_t pin, uint16_t numPixels);

    // Inicialización
    void init(void);

    // Control básico
    void apagar(void);
    void setColor(uint32_t color);
    void setAllLEDs(uint32_t color);
    void setPixelColor(uint8_t index, uint32_t color);

    // Sistema de patrones
    void loop(void);
    void pararTodo(void);

    // Comunicación RF/LoRa/Server
    void blinkServerExterno(void); // Parpadeo azul para servidor externo
    void blinkRF433MHz(void);      // Parpadeo amarillo para RF 433MHz
    void blinkLoRa(void);          // Parpadeo verde para LoRa

    // Errores del sistema
    void blinkMP3(void);           // Parpadeo turquesa para errores de periférico
    void activarErrorMP3(void);    // Activa el ciclo continuo de error MP3
    void desactivarErrorMP3(void); // Desactiva el error MP3

    // Niveles de batería
    void blinkBatteryNormal(void);   // Nivel normal de batería
    void blinkBatteryLow(void);      // Batería baja (naranja)
    void blinkBatteryCritical(void); // Batería crítica (rojo)

    // Carga del sistema
    void iniciarCargaSistema(void);
    void detenerCargaSistema(void);

    // Control de estado del servidor
    void setServidorConectado(void);    // Activa patrón infinito azul
    void setServidorDesconectado(void); // Activa patrón infinito violeta
    void deshabilitarServidor(void);    // Detiene cualquier patrón de servidor

    // Estados
    bool isConfigured(void) const;
    PatronType_t getCurrentPattern(void) const;
    bool isBusy(void) const;
    bool isMP3Active(void) const; // Verificar si MP3 está activo

    // Utilidad de colores
    static uint32_t getColor(ColorType_t color) { return static_cast<uint32_t>(color); }
};

#endif
