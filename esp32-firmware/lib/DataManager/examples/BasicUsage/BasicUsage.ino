#include <Arduino.h>
#include <DataManager.h>

void setup() {
  Serial.begin(115200);
  while (!Serial); // Espera a que se abra el puerto serie (opcional)

  // Inicializa la librería DataManager
  Data.begin();

  // Restablece la configuración a los valores por defecto
  Data.resetConfig();
  Serial.println("Configuración reseteada.");
  
  // Muestra la configuración inicial en formato JSON
  Serial.println("Configuración inicial:");
  Serial.println(Data.getConfigJson());

  // Actualiza la configuración
  Data.setID("ALMX123");
  Data.setConfig(0, 1); // RF habilitado
  Data.setConfig(1, 2); // WIFI bloqueado (por ejemplo)
  Data.setConfig(2, 0); // PERIFONEO deshabilitado
  
  // Muestra la configuración actualizada en JSON
  Serial.println("Configuración actualizada:");
  Serial.println(Data.getConfigJson());

  // Muestra valores individuales
  Serial.print("ID actual: ");
  Serial.println(Data.getID());
  
  Serial.print("Estado de RF: ");
  Serial.println(Data.obtenerEstadoRF());
}

void loop() {
  // No se requiere acción continua en este ejemplo
  delay(5000);
}
