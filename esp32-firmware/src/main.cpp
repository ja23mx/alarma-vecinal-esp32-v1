#include <Arduino.h>
#include "esp_task_wdt.h"
#include "esp_ota_ops.h"
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

// Nueva bandera para controlar el LED desde manejarBotonProg
bool led_control_boton_activo = false;

#include <voz_esp.h>

void setup()
{

  Serial.begin(115200);
  LOG("\r\n\r\n\r\n\r\n\r\n\r\n" + String(SISTEMA_FIRMWARE));
  LOG("\r\n" + String(SISTEMA_VERSION) + "\r\n" + String(SISTEMA_ETAPA) + "\r\nTRJ VRS: " + String(TARJETA_VERSION));
  LOG("\r\nFECHA COMPILACION: " + String(SISTEMA_DATE));
  ENTORNO == 1 ? LOG("\r\nENTORNO DE DESARROLLO\r\n\r\n") : LOG("\r\nENTORNO DE PRODUCCION\r\n\r\n");

  delay(100); // Esperar 1 segundo para estabilizar el puerto serie
  // Configurar el tiempo de espera del Watchdog a 10 segundos

  // RAMStatus(1); // Imprimir info de la flash y cantidad de RAM usada

  pinMode(PIN_MP3_BUSY, INPUT);
  pinMode(BTN_PROG, INPUT_PULLUP);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(AMPLIFICADOR, OUTPUT);
  pinMode(SALIDA_1, OUTPUT);
  pinMode(PIN_CNF_RED_ADC, INPUT);

#if defined(SALIDA_2)
  pinMode(SALIDA_2, OUTPUT);
  digitalWrite(SALIDA_2, LOW);
#endif

  pinMode(LED_MP3, OUTPUT);

  digitalWrite(LED_STATUS, LED_ST_OFF);
  digitalWrite(AMPLIFICADOR, LOW);
  digitalWrite(SALIDA_1, LOW);
  digitalWrite(LED_MP3, LOW);

  mp3State.Configurado = false; // Inicializar el estado del MP3 como no configurado
  cmdAudioPendiente = false;    // Inicializar el estado del comando de audio como no pendiente
  cmdSalidasPendiente = false;  // Inicializar el estado de los comandos de salida como no pendientes
  cmdAudioBusy = false;         // Inicializar el estado del comando de audio como no ocupado
  cmdSalidasBusy = false;       // Inicializar el estado de los comandos de salida

  Data.begin(); // Inicializa la librería DataManager, configuraciones, redes wifi, controles RF, etc.

  crearTareaNeoPixel();      // Crear la tarea de NeoPixel
  crearTareaProcesos();      // Crear la tarea de procesos
  crearTareaProcesosCmd();   // Crear la tarea de procesos de comandos
  crearTareaGestionSalida(); // Crear la tarea de gestión de salidas
  get_config_red();          // Leer la configuración de red en pic ADC (WiFi o Ethernet)

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

  manejarBotonProg(); // Manejar el botón de programación

  if (Serial.available())
  {
    String cmd = Serial.readString();
    LOG("\r\nComando recibido: " + cmd);

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
  // Si el control del LED está siendo manejado por manejarBotonProg, no hacer nada
  if (led_control_boton_activo)
  {
    return;
  }

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
  static bool funcionLlamada20s = false;
  static bool parpadeoIniciado = false;
  static unsigned long tiempoParpadeo = 0;
  static bool estadoLedParpadeo = false;
  static bool ventanaDecisionActiva = false;

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
    // Si se suelta el botón durante la ventana de decisión (5s-10s), iniciar programación web
    if (ventanaDecisionActiva && botonPresionado)
    {
      LOG("\r\n\r\nIniciando programación web...");
      origen_fin_prog = serverWeb.begin(1); // Inicia programación via boton de programación
      gestion_fin_ap(origen_fin_prog);      // Manejar el fin del modo AP
    }

    // Reiniciar todo si se suelta el botón
    if (botonPresionado)
    {
      botonPresionado = false;
      funcionLlamada5s = false;
      esperandoConfirmacion = false;
      funcionLlamada20s = false;
      parpadeoIniciado = false;
      ventanaDecisionActiva = false;
      led_control_boton_activo = false; // Devolver control del LED a blink_led_mqtt
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
    funcionLlamada20s = false;
    parpadeoIniciado = false;
    ventanaDecisionActiva = false;
    tiempoInicioPresion = millis();
  }

  // Si el botón está presionado
  if (botonPresionado && estadoBoton == LOW)
  {
    unsigned long tiempoPresion = millis() - tiempoInicioPresion;

    // Activar ventana de decisión a los 5 segundos
    if (tiempoPresion >= 5000 && !funcionLlamada5s)
    {
      funcionLlamada5s = true;
      ventanaDecisionActiva = true;
      led_control_boton_activo = true; // Tomar control del LED
      LOG("\r\n\r\nVentana de decisión: suelte para programación web, mantenga para reset de datos...");
    }

    // Cancelar ventana de decisión a los 10 segundos si sigue presionado
    if (tiempoPresion >= 10000 && ventanaDecisionActiva)
    {
      ventanaDecisionActiva = false;
      LOG("Ventana de decisión cerrada - continuando hacia reset de datos...");
    }

    // Parpadeo desde los 5 segundos: indicación visual para soltar y programar
    if (tiempoPresion >= 5000 && !parpadeoIniciado)
    {
      parpadeoIniciado = true;
      tiempoParpadeo = millis();
    }

    // Controlar parpadeo del LED (300ms de período)
    if (parpadeoIniciado && !funcionLlamada20s)
    {
      if (millis() - tiempoParpadeo >= 150)
      { // 150ms ON, 150ms OFF = 300ms período
        estadoLedParpadeo = !estadoLedParpadeo;
        digitalWrite(LED_STATUS, estadoLedParpadeo);
        tiempoParpadeo = millis();
      }
    }

    // Función a los 25 segundos (solo si no está en ventana de decisión)
    if (tiempoPresion >= 25000 && !funcionLlamada20s && !ventanaDecisionActiva)
    {
      funcionLlamada20s = true;
      digitalWrite(LED_STATUS, LOW); // Apagar LED después del parpadeo
      LOG("\r\n\r\nReiniciando datos...");
      Data.rstData(); // Ejecutar reset de datos
      ESP.restart();  // Reiniciar el ESP para aplicar los cambios
    }
  }
}

int leerSwitchConAntirrebote(int pinADC, int numLecturas, int retardoMs)
{
  long suma = 0;
  int lecturaActual;
  int tolerancia = 100;

  // Realiza 'numLecturas' lecturas para filtrar ruido y rebotes
  for (int i = 0; i < numLecturas; i++)
  {
    lecturaActual = analogRead(pinADC);
    suma += lecturaActual;
    delay(retardoMs); // Pequeña pausa entre lecturas
  }

  // Calcula el promedio
  int promedio = suma / numLecturas;

  // Serial.print("\r\n\r\npromedio: " + String(promedio));

  // Verifica en qué rango está la lectura con tolerancia
  if (abs(promedio - 4095) <= tolerancia)
  {
    return 0;
  }
  else if (abs(promedio - 1984) <= tolerancia)
  {
    return 1;
  }
  else if (abs(promedio - 968) <= tolerancia)
  {
    return 2;
  }

  // Si no está en ningún rango válido, retorna -1 (error o estado inválido)
  return -1;
}

void get_config_red(void)
{
  config_red = leerSwitchConAntirrebote(PIN_CNF_RED_ADC, 15, 50); // lectura de configuracion de red

  if (config_red == 0 || config_red == -1) // por defecto es 1 (ethernet) en caso de que no sea lectura correcta
    config_red = 1;

  Serial.print("\r\n\r\nCONFIG RED= " + String(config_red));
}