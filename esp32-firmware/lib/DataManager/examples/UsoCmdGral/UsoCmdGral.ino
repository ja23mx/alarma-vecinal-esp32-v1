#include <Arduino.h>
#include <DataManager.h>

void setup() {
  Serial.begin(115200);
  while (!Serial); // Espera a que se abra el puerto serie (opcional)

  // Inicializa la librería DataManager
  Data.begin();
  
  Serial.println("UsoCmdGral: Ingrese un comando por Serial");
  Serial.println("Ejemplos:");
  Serial.println("  setID:ALMX001");
  Serial.println("  getID");
  Serial.println("  setConfig:0,1");
  Serial.println("  resetConfig");
  Serial.println("  getConfigGral");
  Serial.println("  ConfigJson:{\"id\":\"ALMX002\",\"estados\":[1,1,0]}");
  Serial.println("  getConfigJson");
  Serial.println("  cargarConfiguracion");
  Serial.println("  habilitarRF");
  Serial.println("  deshabilitarRF");
  Serial.println("  bloquearRF");
  Serial.println("  obtenerEstadoRF");
}

void loop() {
  if (Serial.available() > 0) {
    // Lee la línea completa hasta el salto de línea
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      // Procesa el comando recibido
      CmdResponse response = Data.CmdGral(input);
      Serial.print("Status: ");
      Serial.println(response.status);
      Serial.print("Response: ");
      Serial.println(response.response);
    }
  }
  delay(10);
}
