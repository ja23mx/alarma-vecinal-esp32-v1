#include <Arduino.h>
#include "NeoPixelAV.h"

// Crear instancia del NeoPixel
NeoPixelController neopixel(19, 8); // Pin 19, 8 LEDs

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando ejemplo básico de NeoPixelAV...");
    
    // Inicializar el NeoPixel
    neopixel.init();
    
    // Secuencia de demostración
    demonstratePatterns();
}

void loop() {
    // Llamar loop continuamente para manejar patrones
    neopixel.loop();
    
    delay(10);
}

void demonstratePatterns() {
    Serial.println("=== Demostrando patrones NeoPixelAV ===");
    
    // 1. Color fijo azul
    Serial.println("1. Color fijo azul por 2 segundos");
    neopixel.setColor(NeoPixelController::colorAzul());
    delay(2000);
    
    // 2. Apagar
    Serial.println("2. Apagar por 1 segundo");
    neopixel.apagar();
    delay(1000);
    
    // 3. Patrón rápido 1 con amarillo
    Serial.println("3. Patrón rápido 1 (3 pulsos) con amarillo");
    neopixel.setPatron(PATRON_RAPIDO_1, NeoPixelController::colorAmarillo());
    delay(1000); // Esperar a que termine el patrón
    
    // 4. Pausa
    Serial.println("4. Pausa por 1 segundo");
    delay(1000);
    
    // 5. Patrón rápido 2 con verde
    Serial.println("5. Patrón rápido 2 (2 pulsos dobles) con verde");
    neopixel.setPatron(PATRON_RAPIDO_2, NeoPixelController::colorVerde());
    delay(1000); // Esperar a que termine el patrón
    
    // 6. Demostrar todos los colores
    Serial.println("6. Demostrando todos los colores disponibles");
    demonstrateColors();
    
    Serial.println("=== Demostración completada ===");
}

void demonstrateColors() {
    uint32_t colors[] = {
        NeoPixelController::colorAzul(),
        NeoPixelController::colorVerde(),
        NeoPixelController::colorAmarillo(),
        NeoPixelController::colorTurquesa(),
        NeoPixelController::colorNaranja(),
        NeoPixelController::colorRojo(),
        NeoPixelController::colorVioleta(),
        NeoPixelController::colorAzulClaro()
    };
    
    const char* colorNames[] = {
        "Azul",
        "Verde", 
        "Amarillo",
        "Turquesa",
        "Naranja",
        "Rojo",
        "Violeta",
        "Azul Claro"
    };
    
    for (int i = 0; i < 8; i++) {
        Serial.printf("   Color: %s\n", colorNames[i]);
        neopixel.setColor(colors[i]);
        delay(800);
    }
    
    neopixel.apagar();
}
