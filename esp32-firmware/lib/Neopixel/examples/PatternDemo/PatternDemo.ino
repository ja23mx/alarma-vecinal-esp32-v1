#include <Arduino.h>
#include "NeoPixelAV.h"

// Crear instancia del NeoPixel
NeoPixelController neopixel(19, 8); // Pin 19, 8 LEDs

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando demostración de patrones NeoPixelAV...");
    
    // Inicializar el NeoPixel
    neopixel.init();
    
    Serial.println("Simulando estados del sistema AV NEXUS:");
    Serial.println("- RF 433MHz: Patrón rápido 1 amarillo");
    Serial.println("- LoRa: Patrón rápido 2 verde");
    Serial.println("- Error: Patrón rápido 2 turquesa");
    Serial.println("- Batería baja: Patrón rápido 1 naranja");
    Serial.println();
}

void loop() {
    // Ejecutar loop para manejar patrones
    neopixel.loop();
    
    // Simular diferentes estados del sistema
    simulateSystemStates();
    
    delay(10);
}

void simulateSystemStates() {
    static unsigned long lastChange = 0;
    static uint8_t currentState = 0;
    
    // Cambiar estado cada 3 segundos
    if (millis() - lastChange > 3000) {
        lastChange = millis();
        
        switch (currentState) {
            case 0:
                Serial.println("Estado: RF 433MHz activo");
                neopixel.setPatron(PATRON_RAPIDO_1, NeoPixelController::colorAmarillo());
                break;
                
            case 1:
                Serial.println("Estado: LoRa activo");
                neopixel.setPatron(PATRON_RAPIDO_2, NeoPixelController::colorVerde());
                break;
                
            case 2:
                Serial.println("Estado: Error de periférico");
                neopixel.setPatron(PATRON_RAPIDO_2, NeoPixelController::colorTurquesa());
                break;
                
            case 3:
                Serial.println("Estado: Batería baja");
                neopixel.setPatron(PATRON_RAPIDO_1, NeoPixelController::colorNaranja());
                break;
                
            case 4:
                Serial.println("Estado: Conexión servidor activa");
                neopixel.setColor(NeoPixelController::colorAzul());
                break;
                
            case 5:
                Serial.println("Estado: Sin conexión servidor");
                neopixel.setColor(NeoPixelController::colorVioleta());
                break;
                
            case 6:
                Serial.println("Estado: Modo configuración");
                neopixel.setColor(NeoPixelController::colorAzulClaro());
                break;
                
            case 7:
                Serial.println("Estado: Sistema apagado");
                neopixel.apagar();
                break;
        }
        
        currentState = (currentState + 1) % 8;
    }
}
