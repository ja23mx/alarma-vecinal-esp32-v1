#include <Arduino.h>
#include "esp_task_wdt.h"

#include "WiFiTool.h"
#include "ServerWeb.h"
#include "DataManager.h"
#include "BLEManager.h"

#include "CnfTarjeta.h"
#include "ConfigSistema.h"
#include "VariablesGlobales.h"
#include "LogSistema.h"
#include "MonitorRAM.h"
#include "main.h"
#include "gestor_procesos.h"
#include "procesos_cmd.h"
#include "wifi_mqtt_esp.h"
#include "gestor_salidas.h"
#include "voz_esp.h"
#include "tarea_neopixel.h"

// Instancia de la clase WiFiTool
WiFiTool WiFiTools;
// crear la instancia de BLEManager, inyectando la referencia a WiFiTool.
BLEManager BLE(WiFiTools);
// Crear la instancia de ServerWeb, inyectando la referencia a WiFiTool.
ServerWeb serverWeb(WiFiTools);

bool en_loop_mqtt = true;      // Variable para autorizar el bucle MQTT
bool conexion_mqtt = false;    // Variable para verificar la conexión MQTT
bool conexion_wifi = false;    // Variable para verificar la conexión WiFi
unsigned long tiempo_wifi = 0; // Variable para almacenar el tiempo de la alarma
unsigned long tiempo_mqtt = 0; // Variable para almacenar el tiempo de la conexión MQTT

#include <voz_esp.h>

void setup()
{

  Serial.begin(115200);
  LOG("\r\n\r\n\r\n\r\n\r\n\r\n\r\n" + String(SISTEMA_FIRMWARE) + " " + String(SISTEMA_VERSION) + " " + String(SISTEMA_BUILD) + "\r\n\r\n");
  LOG("Compilacion: " + String(SISTEMA_DATE) + "\r\n\r\n");
  LOG("Modelo: " + String(BOARD_MODELO) + "\r\n\r\n");

  delay(100); // Esperar 1 segundo para estabilizar el puerto serie
  // Configurar el tiempo de espera del Watchdog a 10 segundos

  // RAMStatus(1); // Imprimir info de la flash y cantidad de RAM usada

  pinMode(PIN_MP3_BUSY, INPUT);
  pinMode(BTN_PROG, INPUT_PULLUP);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(AMPLIFICADOR, OUTPUT);
  pinMode(SALIDA_1, OUTPUT);

  pinMode(PIN_ENT_DIG_1, INPUT_PULLUP);
  pinMode(LED_MP3, OUTPUT);
  pinMode(PIN_RELE, OUTPUT);

  digitalWrite(LED_STATUS, LED_ST_OFF);
  digitalWrite(AMPLIFICADOR, AMP_OFF);
  digitalWrite(SALIDA_1, LOW);
  digitalWrite(PIN_RELE, LOW);
  digitalWrite(LED_MP3, LOW);

  mp3State.Configurado = false; // Inicializar el estado del MP3 como no configurado
  cmdAudioPendiente = false;    // Inicializar el estado del comando de audio como no pendiente
  cmdSalidasPendiente = false;  // Inicializar el estado de los comandos de salida como no pendientes
  cmdAudioBusy = false;         // Inicializar el estado del comando de audio como no ocupado
  cmdSalidasBusy = false;       // Inicializar el estado de los comandos de salida

  Data.begin(); // Inicializa la librería DataManager, configuraciones, redes wifi, controles RF, etc.

  crearTareaNeoPixel();      // Crear la tarea de NeoPixel
  crearTareaProcesos();      // Crear la tarea de procesos
  crearTareaProcesosCmd();   //
  crearTareaGestionSalida(); // Crear la tarea de gestión de salidas

  if (PROG_LOCAL)
    //  Inicializa el servidor web y la red wifi, si no se conecta
    //  a la red wifi, inicia el AP
    serverWeb.begin(0);

  if (Data.perifericos[1])                     // Si el periférico WiFi está habilitado
  {                                            //
    conexion_wifi = gestionar_conexion_wifi(); // Inicia la conexión WiFi
    tiempo_wifi = millis();                    // Almacenar el tiempo de la conexión WiFi
                                               //
    async_stop_carga_sistema_and_wait(2000);   // Detener el efecto de carga del NeoPixel y esperar a que se libere
    if (conexion_wifi)                         //
      async_servidor_conectado();              // Marcar que el servidor está conectado
    else                                       //
      async_servidor_desconectado();           // Marcar que el servidor está desconectado
  }
  else
  {
    async_stop_carga_sistema();    // Detener el efecto de carga del NeoPixel
    delay(200);                    // Dar tiempo para procesar
    async_servidor_deshabilitar(); // Deshabilitar el sistema de servidor
    LOG("\r\nPeriférico WiFi deshabilitado.");
  }
}

void loop()
{

  if (Data.perifericos[1]) // Si el periférico WiFi está habilitado
  {
    blink_led_mqtt(1); // Llamar a la función de parpadeo del LED MQTT

    if (conexion_wifi) // Si la conexión WiFi es exitosa
    {
      if (en_loop_mqtt)
      {
        conexion_mqtt = mqtt_loop(); // Llamar a la función de bucle MQTT

        if (conexion_mqtt)
          blink_led_mqtt(0); // Llamar a la función de parpadeo del LED MQTT

        en_loop_mqtt = false;   // Desactivar el bucle MQTT
        tiempo_mqtt = millis(); // Almacenar el tiempo actual
      }

      // Determinar el intervalo según el estado de conexión MQTT
      unsigned long intervalo = conexion_mqtt ? MQTT_CNF_TM_REV_LOOP_MS : MQTT_CNF_TM_INT_RECONEXION;

      // Reactivar el bucle MQTT si ha pasado el intervalo
      if (millis() - tiempo_mqtt > intervalo)
      {
        en_loop_mqtt = true;
      }
    }
    else
    {
      if (millis() - tiempo_wifi > 10000)          // Si no se conecta a la red wifi en 10 segundos
      {                                            //
        conexion_wifi = gestionar_conexion_wifi(); // Inicia la conexión WiFi
        tiempo_wifi = millis();                    // Almacenar el tiempo de la conexión WiFi
      }
    }
  }

  /* if (digitalRead(BTN_PROG) == LOW)
  {
    delay(250);

    if (digitalRead(BTN_PROG) == LOW)
    {
      init_prog_ble = true;
      // cambiar_estado_tarea_alarma(0);    // Pausar la tarea de alarma
      // cambiar_estado_procesos_alarma(0); // Pausar la tarea de procesos
      LOG("\r\nINIT PROGRAMACION..."); // init_red_ble();                  // Inicializar la red BLE
      // Inicializa el servidor web y la red wifi, si no se conecta a la red wifi, inicia el AP
      serverWeb.begin();
    }
  } */

  manejarBotonProg(); // Manejar el botón de programación

  if (Serial.available())
  {
    String cmd = Serial.readString();
    LOG("\r\nComando recibido: " + cmd);

    /* if (cmd == "a")
      async_led_mp3_on(); // Encender el LED del MP3
    else if (cmd == "s")
      async_led_mp3_off(); // Apagar el LED del MP3 */

    if (cmd == "rst")
    {
      LOG("\r\n\r\nrestableciendo datos...");
      Data.rstData();
    }
    else
    {
      Data.CmdSerial(cmd);
    }
  }
  delay(1);
}

void blink_led_mqtt(bool revision)
{
  static bool en_blink = true;               // Variable para almacenar el estado del LED
  static unsigned long tiempo_en_fncion = 0; // Variable para almacenar el tiempo de la función
  static bool led_status = false;            // Variable para almacenar el estado del LED
  static unsigned long tiempo_blink = 0;     // Variable para almacenar el tiempo de parpadeo

  // inicio timer y revision de tiempo de la funcion
  // Si el tiempo de la función no ha pasado para volver a parpadear
  if (en_blink == false && millis() - tiempo_en_fncion < 1000) //
  {                                                            //
    return;                                                    // Salir de la función
  }
  // Si el tiempo de la función ha pasado para volver a parpadear
  else if (en_blink == false && millis() - tiempo_en_fncion > 1000) //
  {                                                                 //
    en_blink = true;                                                // Reiniciar el estado del LED
  }

  // inicio timer y revision de tiempo de parpadeo OFF
  if (led_status == false && revision == false)
  {
    tiempo_blink = millis();             // Almacenar el tiempo de parpadeo
    led_status = true;                   // Cambiar el estado del LED a encendido
    digitalWrite(LED_STATUS, LED_ST_ON); // Encender el LED de estado
  }
  // inicio timer y revision de tiempo de parpadeo ON
  else if (led_status == true && revision == true)
  {
    if (millis() - tiempo_blink > 350)      // Si ha pasado x segundos
    {                                       //
      en_blink = false;                     // Reiniciar el estado de la función
      led_status = false;                   // Cambiar el estado del LED a apagado
      digitalWrite(LED_STATUS, LED_ST_OFF); // Apagar el LED de estado
      tiempo_en_fncion = millis();          // Almacenar el tiempo de la función a partir de ahora
    }
  }
}

void gestion_fin_ap(uint8_t origenConfig)
{
  if (origenConfig == 1)
  {
    LOG("\r\n\r\nFIN DE PROGRAMACION POR CONTROL INTEGRADOR\r\n\r\n");
    LOG("pista: " + String(modeloCtrlAVRx.int_fin_prog_pista) + "\r\n");
  }
  else if (origenConfig == 0)
  {
    LOG("\r\n\r\nFIN DE PROGRAMACION POR BOTON\r\n\r\n");
    modeloCtrlAVRx.int_fin_prog_pista = CONST_MP3_SYST_FIN_PROG;
  }

  sync_proc_cmd_play_pista_led_atm(modeloCtrlAVRx.int_fin_prog_pista, 3, 10);

  LOG("\r\n\r\n\r\n\r\nRESET POR FIN PROG\r\n\r\n");
  ESP.restart(); // Reiniciar el ESP para aplicar los cambios
}

void manejarBotonProg()
{
  static unsigned long tiempoInicioPresion = 0;
  static bool botonPresionado = false;
  static bool funcionLlamada5s = false;
  static bool esperandoConfirmacion = false;
  static unsigned long tiempoEsperaConfirmacion = 0;
  static bool funcionLlamada13s = false;
  static bool parpadeoIniciado = false;
  static unsigned long tiempoParpadeo = 0;
  static bool estadoLedParpadeo = false;

  uint8_t origen_fin_prog;

  if (init_prog_rf)
  {
    LOG("\r\n\r\nLOOP Iniciando programación por RF...");
    origen_fin_prog = serverWeb.begin(1); // Inicia programación via control de integrador
    gestion_fin_ap(origen_fin_prog);      // Manejar el fin del modo AP
  }

  bool estadoBoton = digitalRead(BTN_PROG);

  if (estadoBoton == HIGH) // boton no presionado
  {
    // Reiniciar todo si se suelta el botón
    if (botonPresionado)
    {
      botonPresionado = false;
      funcionLlamada5s = false;
      esperandoConfirmacion = false;
      funcionLlamada13s = false;
      parpadeoIniciado = false;
      digitalWrite(LED_STATUS, LOW);
      LOG("Botón soltado - Timer reiniciado");
    }
    return;
  }

  // Detectar cuando se presiona el botón (LOW)
  if (estadoBoton == LOW && !botonPresionado)
  {
    botonPresionado = true;
    funcionLlamada5s = false;
    esperandoConfirmacion = false;
    funcionLlamada13s = false;
    parpadeoIniciado = false;
    tiempoInicioPresion = millis();
  }

  // Si el botón está presionado
  if (botonPresionado && estadoBoton == LOW)
  {
    unsigned long tiempoPresion = millis() - tiempoInicioPresion;

    // Función a los 5 segundos
    if (tiempoPresion >= 5000 && !funcionLlamada5s)
    {
      funcionLlamada5s = true;
      esperandoConfirmacion = true;
      tiempoEsperaConfirmacion = millis();
      LOG("\r\n\r\nIniciando programación..."); // Mensaje de inicio de programación
      origen_fin_prog = serverWeb.begin(1);     // Inicia programación via boton de programación
      gestion_fin_ap(origen_fin_prog);          // Manejar el fin del modo AP
    }

    // Esperar confirmación después de los 5 segundos (500ms de espera)
    if (esperandoConfirmacion)
    {
      if (millis() - tiempoEsperaConfirmacion >= 500)
      {
        esperandoConfirmacion = false;
        LOG("Confirmación: continuando hacia reset de datos...");
      }
    }

    // Parpadeo rápido antes de los 13 segundos (a partir de los 11 segundos)
    if (tiempoPresion >= 11000 && !parpadeoIniciado && !esperandoConfirmacion)
    {
      parpadeoIniciado = true;
      tiempoParpadeo = millis();
    }

    // Controlar parpadeo del LED (300ms de período)
    if (parpadeoIniciado && !funcionLlamada13s)
    {
      if (millis() - tiempoParpadeo >= 150)
      { // 150ms ON, 150ms OFF = 300ms período
        estadoLedParpadeo = !estadoLedParpadeo;
        digitalWrite(LED_STATUS, estadoLedParpadeo);
        tiempoParpadeo = millis();
      }
    }

    // Función a los 13 segundos (solo si no está esperando confirmación)
    if (tiempoPresion >= 13000 && !funcionLlamada13s && !esperandoConfirmacion)
    {
      funcionLlamada13s = true;
      digitalWrite(LED_STATUS, LOW); // Apagar LED después del parpadeo
      LOG("\r\n\r\nReiniciando datos...");
      Data.rstData(); // Ejecutar reset de datos
    }
  }
}