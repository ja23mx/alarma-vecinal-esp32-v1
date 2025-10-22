#include <Arduino.h>
#include "DFRobotDFPlayerMini.h" // Incluye la librería para el DFPlayer
// #include "MP3Controller.h"        // Nueva clase MP3Controller (DESHABILITADA TEMPORALMENTE)

#include <ConfigSistema.h>
#include "VariablesGlobales.h"
#include <LOGSistema.h>
#include "voz_esp.h"
#include "rf_esp.h"
#include "GestorCmd.h"
#include "tarea_neopixel.h"


// Define para habilitar/deshabilitar la gestión de errores MP3
#define ENABLE_MP3_ERROR_MANAGEMENT 1

DFRobotDFPlayerMini dfPlayer; // Instancia del DFPlayer
HardwareSerial SerialMP3(1);  // define a Serial for UART1

// Instancia del MP3Controller (nueva clase)
//MP3Controller mp3Controller(dfPlayer, SerialMP3, PIN_MP3_UART_RX, PIN_MP3_UART_TX, PIN_MP3_BUSY, AMPLIFICADOR);

DATOS_INSTRUCCION cmdDataVoz;    // Estructura para almacenar el comando de salida
std::vector<long> numerosArrVoz; // Vector para almacenar los números separados

bool async_pista_1_ciclo_flag = false;
uint16_t async_pista_1_ciclo_pista = false;

bool error_mp3_no_encontrado;
uint16_t pista_reproducida;

bool lista_pistas_arr_flag = false;     // bandera para indicar si se reproduce una lista de pistas
uint16_t lista_pistas_arr[MAX_BOTONES]; // Lista de pistas a reproducir

bool async_init_mp3_cmd_flag = false; // bandera para indicar si se ha inicializado el comando MP3 via cmd
bool async_stop_mp3_cmd_flag = false; // bandera para indicar si se ha detenido el comando MP3 via cmd

bool async_play_1_pista_flag_bucle = false; // bandera para indicar si se reproduce una pista en bucle
uint16_t async_play_1_pista_num = 0;        // número de pista a reproducir

bool async_reproduccion_timer_flag = false;        // bandera para indicar si se reproduce una pista en bucle
unsigned long async_reproduccion_timer_limite = 0; // número de pista a reproducir
unsigned long async_reproduccion_timer_inicio = 0;

bool async_pista_prueba_pendiente_flag = false; // bandera para ejecutar prueba de pista automática

void async_config_mp3_cmd(void)
{
    uint8_t cnt_pistas;
    uint16_t lista_pistas[10] = {0}; // Lista de pistas a reproducir

    if (!cmdAudioPendiente) // Si hay comandos de salida
        return;

    cmdAudioPendiente = false; // Reiniciar el estado del comando de audio
    String comando = cmdAudio; // Guardar el comando en la variable local
    cmdAudio = "";             // Reiniciar el comando de audio

    Serial.println("\r\n\r\nconfig_mp3_cmd: " + comando);

    if (GestorCmd.separarComando(comando, cmdDataVoz))
    {

        LOGF("\r\nComando: %s", cmdDataVoz.cmd.c_str());
        LOGF("\r\nNum Prf: %s", cmdDataVoz.numPeriferico.c_str());
        LOGF("\r\nParametros: %s", cmdDataVoz.parametros.c_str());

        if (cmdDataVoz.cmd == "SA") // Si el comando es "SD"
        {
            uint8_t numSalida = cmdDataVoz.numPeriferico.toInt(); // Convertir el número de salida a entero
            // salida fuera de rango
            if (numSalida == 1)
            {

                int configuracion;

                GestorCmd.separarArrNumeros(cmdDataVoz.parametros, ':', numerosArrVoz); // Separar los parámetros
                configuracion = numerosArrVoz[0];                                       // Obtener la configuración

                /* for (size_t i = 0; i < numerosArrVoz.size(); i++)
                {
                    LOGF("\r\n\r\nnumerosArrVoz[%d]: %d", i, numerosArrVoz[i]);
                } */

                switch (configuracion)
                {
                // APAGADO 0:0:0
                case 0:
                    // todo en 0 y debe estar reproduciendo
                    if (numerosArrVoz[1] == 0 && numerosArrVoz[2] == 0 && async_init_mp3_cmd_flag == true)
                    {
                        LOG("\r\n\r\n\r\ncmdAudioBusy: false\r\n\r\n");
                        cmdAudioBusy = false;           // Reiniciar el estado del comando de audio
                        async_stop_mp3_cmd_flag = true; // bandera para indicar si se ha detenido el comando MP3 via cmd
                    }
                    break;

                // reproduccion de una pista una vez
                case 1:
                    LOG("\r\n\r\n\r\ncmdAudioBusy: true\r\n\r\n");
                    cmdAudioBusy = true; // Reiniciar el estado del comando de audio
                    async_init_mp3_cmd_flag = true;
                    async_cnf_mp3_play_una_pista(numerosArrVoz[1]);
                    break;

                // reproduccion de una pista en bucle ******* REVISION. FALLA ********
                case 2:
                    async_init_mp3_cmd_flag = true;
                    async_play_1_pista_flag_bucle = true; // bandera para indicar si se reproduce una pista en bucle
                    async_play_1_pista_num = numerosArrVoz[1];
                    break;

                case 5:
                    cnt_pistas = numerosArrVoz[1]; // Contador de pistas

                    async_init_mp3_cmd_flag = true; // bandera para indicar si se ha inicializado el comando MP3 via cmd

                    for (size_t i = 0; i < cnt_pistas; i++)
                        lista_pistas[i] = numerosArrVoz[i + 2]; // Guardar la pista en la lista

                    async_cnf_mp3_play_lista(lista_pistas, cnt_pistas); // Enviar la lista de pistas a reproducir
                    break;

                // reproduccion de lista temporizada
                case 6:
                    LOG("\r\n\r\n\r\ncmdAudioBusy: true\r\n\r\n");
                    cmdAudioBusy = true; // Reiniciar el estado del comando de audio

                    cnt_pistas = numerosArrVoz[1]; // Contador de pistas

                    for (size_t i = 0; i < cnt_pistas; i++)
                        lista_pistas[i] = numerosArrVoz[i + 3]; // Guardar la pista en la lista

                    async_init_mp3_cmd_flag = true; // bandera para indicar si se ha inicializado el comando MP3 via cmd

                    async_reproduccion_timer_flag = true;                      // bandera para indicar si se reproduce una pista en bucle
                    async_reproduccion_timer_limite = numerosArrVoz[2] * 1000; // tiempo de reproduccion
                    async_reproduccion_timer_inicio = millis();                // tiempo de inicio de la reproduccion

                    async_cnf_mp3_play_lista(lista_pistas, cnt_pistas); // Enviar la lista de pistas a reproducir
                    break;
                }
            }
        }
    }
}

void mp3_init(void)
{

    // LOG("\r\n\r\nmp3_init");
    SerialMP3.begin(9600, SERIAL_8N1, PIN_MP3_UART_RX, PIN_MP3_UART_TX);
    delay(50);
    
    // Inicializar nuevas variables
    mp3State.pistaActual = 0;
    mp3State.amplificadorActivo = false;
    
    mp3_init_comandos();

    mp3_pista_prueba_init(CONST_MP3_SYST_ERROR_AUDIO_404, false);
    delay(100);

    mp3State.Configurado = true; // Indicar que el MP3 está configurado

#if ENABLE_MP3_ERROR_MANAGEMENT
    // Si todo se inicializó correctamente, desactivar error MP3
    if (mp3State.conexion && mp3State.pistaInit)
    {
        async_error_mp3_desactivar();
        async_led_mp3_on(); // MP3 inicializado correctamente - LED ON
    }
    else
    {
        async_error_mp3_activar();
        async_led_mp3_off(); // Error en inicialización - LED OFF
    }
#endif

    LOG("\r\n\r\nRESUMEN STATE MP3");
    LOGF("\r\nConexion: %d", mp3State.conexion);
    LOGF("\r\nPista Init: %d", mp3State.pistaInit);
    LOGF("\r\nMicroSD: %d", mp3State.microSD);
    LOGF("\r\nPista Actual: %d", mp3State.pistaActual);
    LOGF("\r\nAmplificador: %d", mp3State.amplificadorActivo);
    LOG("\r\n\r\n");
}

void mp3_init_comandos(void)
{

    // LOG("\r\n\r\nmp3_init_comandos");

    for (uint8_t i = 0; i < 2; i++) // dos intentos de comunicacion
    {
        if (dfPlayer.begin(SerialMP3))
        {
            mp3State.conexion = true;
            // LOG("\r\nComunicacion DFPlayer: ok");
#if ENABLE_MP3_ERROR_MANAGEMENT
            async_error_mp3_desactivar(); // Conexión exitosa, desactivar error
#endif
        }
        else
        {
            mp3State.conexion = false;
            // LOG("\r\nComunicacion DFPlayer: error");
#if ENABLE_MP3_ERROR_MANAGEMENT
            async_error_mp3_activar(); // Error de conexión, activar error
#endif
        }
        if (mp3State.conexion)
            break;
        delay(1000);
    }

    LOG("\r\n\r\n mp3State.conexion: " + String(mp3State.conexion));

    delay(50);
    dfPlayer.volume(30);
    delay(50);
}

void mp3_play_pst_cmd(uint16_t pista)
{

    LOG("\r\n\r\nmp3_play_pst_cmd: " + String(pista));
    dfPlayer.playMp3Folder(pista);
    delay(100);
}

void mp3_stop_cmd(void)
{

    // LOG("\r\n\r\nmp3_stop_cmd");
    dfPlayer.stop();
    delay(100);
}

void mp3_loop(void)
{

    async_ejec_mp3_play_una_pista();

    async_ejec_mp3_play_lista();

    loop_1_pista_cmd();

    async_ejec_pista_prueba_pendiente();
}

void rev_reproduccion_temporizada(void)
{
    if (!async_reproduccion_timer_flag)
        return;

    if (millis() - async_reproduccion_timer_inicio >= async_reproduccion_timer_limite)
    {

        // alarma fue activada por un comando
        if (estadoAlarma.onCmd)
        {
            estadoAlarma.onCmd = false; // Reiniciar el estado del comando de alarma
            LOG("\r\n\r\nReproduccion temporizada finalizada por comando de alarma.");
        }

        async_reproduccion_timer_flag = false; // bandera para indicar si se reproduce una pista en bucle
        async_play_1_pista_flag_bucle = false; // bandera para indicar si se reproduce una pista en bucle
        async_init_mp3_cmd_flag = false;       // bandera para indicar si se ha inicializado el comando MP3 via cmd
        async_stop_mp3_cmd_flag = true;        // bandera para indicar si se ha detenido el comando MP3 via cmd
    }
}

void loop_1_pista_cmd(void)
{
    if (!async_play_1_pista_flag_bucle) //
        return;                         // Si no se reproduce una pista en bucle, salir

    Serial.println("\r\n\r\nloop_1_pista_cmd");

    mp3_amplificador_on(); // encender amplificador

    while (async_play_1_pista_flag_bucle)
    {
        if (!mp3_reproducion_audio(async_play_1_pista_num, 1)) // si no se reproduce la pista, salir
        {
            async_play_1_pista_flag_bucle = false;
            async_init_mp3_cmd_flag = false;
            break;
        }
        delay(500); // espera para que empiece a reproducir
    }

    mp3_amplificador_off();
}

void async_ejec_pista_prueba_pendiente(void)
{
    if (!async_pista_prueba_pendiente_flag) // Si no hay prueba pendiente
        return;

    async_pista_prueba_pendiente_flag = false; // Resetear la bandera

    // Solo ejecutar si no hay reproducción activa
    if (!cmdAudioBusy && !async_play_1_pista_flag_bucle)
    {
        LOG("\r\n\r\nEjecutando prueba de pista automática por evento de tarjeta");
        mp3_pista_prueba_init(CONST_MP3_MC_SD_INSERTED, true);
    }
}

void mp3_pista_prueba_init(uint16_t pista, bool habilitar_amplificador) // comprobar si la pista especificada se reproduce
{

    // LOG("\r\n\r\nmp3_pista_init");

    mp3State.pistaInit = false;
    mp3State.pistaReprodOk = false;
    mp3State.microSD = false;
    mp3State.pistaActual = 0;

    // Manejo del amplificador basado en loop_1_pista_cmd
    if (habilitar_amplificador) {
        mp3_amplificador_on(); // encender amplificador
    }

    if (mp3_reproducion_audio(pista, 0)) // intento de reproduccion
    {
        LOG("\r\n\r\nPista de prueba OK");
        mp3State.pistaInit = true;
        mp3State.pistaReprodOk = true;
        mp3State.pistaActual = pista;
#if ENABLE_MP3_ERROR_MANAGEMENT
        async_error_mp3_desactivar(); // Pista de prueba exitosa, desactivar error
#endif
        async_led_mp3_on(); // Pista de prueba exitosa - LED ON
        
        // Apagar amplificador si fue encendido
        if (habilitar_amplificador) {
            mp3_amplificador_off();
        }
        return;
    }
#if ENABLE_MP3_ERROR_MANAGEMENT
    else
    {
        async_error_mp3_activar(); // Error en pista de prueba, activar error
        async_led_mp3_off();       // Error en pista de prueba - LED OFF
    }
#endif
    
    // Apagar amplificador si fue encendido (caso de error)
    if (habilitar_amplificador) {
        mp3_amplificador_off();
    }
    
    delay(1000);

    return;
}

bool mp3_reproducion_audio(uint16_t pista, bool stop_en)
{
    bool rsp = false;
    uint16_t pista_aux = pista;

    mp3State.pistaReprodOk = false;        // Reset estado de reproducción
    mp3State.initReproduccion = false;     // Indica que se ha iniciado la reproducción
    mp3State.pistaActual = pista_aux;      // Guardar pista actual

    if (mp3State.conexion)
    {
        rsp = mp3_pista_play_comunic_1(pista_aux, stop_en); // intento de reproduccion
        mp3State.microSD = rsp;                             // Actualizar estado de microSD
        mp3State.pistaReprodOk = rsp;                       // Actualizar estado de reproducción
        mp3_stop_cmd();                                     // detiene la reproduccion de la pista
        mp3State.initReproduccion = false;                  // puesta a cero de bandera de reproduccion
        if (!rsp) mp3State.pistaActual = 0;                // Reset si falló
        return rsp;
    }
    else
    {
        bool rsp = mp3_pista_play_comunic_0(pista_aux, stop_en); // intento de reproduccion
        mp3State.microSD = rsp;                                  // Actualizar estado de microSD
        mp3State.pistaReprodOk = rsp;                            // Actualizar estado de reproducción
        mp3_stop_cmd();                                          // detiene la reproduccion de la pista
        mp3State.initReproduccion = false;                       // puesta a cero de bandera de reproduccion
        if (!rsp) mp3State.pistaActual = 0;                     // Reset si falló
        return rsp;
    }
}

void lectura_pto(void)
{
    if (dfPlayer.available())
    {
        mp3_serial_info(dfPlayer.readType(), dfPlayer.read()); // Imprime el detalle del evento
    }
}

bool mp3_pista_play_comunic_1(uint16_t pista, bool stop_en)
{
    unsigned long tiempo_busy = 0;         // tiempo de espera para que empiece a reproducir
    bool empezo_reproduccion_busy = false; //

    pista_reproducida = 0;           // pista reproducida
    error_mp3_no_encontrado = false; // error pista no encontrada

    mp3_play_pst_cmd(pista);         // envío serial de la pista
    async_led_mp3_reproduciendo();   // Iniciar parpadeo del LED MP3
    delay(100);                      // espera para que empiece a reproducir
    unsigned long tiempo = millis(); //

    while (1)
    {
        lectura_pto(); // Lee el estado del módulo MP3

        // respuesta de error de pista no encontrada, y se reprodujo pista 1 eso significa que
        if (error_mp3_no_encontrado == true && pista_reproducida == 1)
        {
            LOG("\r\n\r\nerror_mp3_no_encontrado == true. \r\npista no encontrada");
            mp3State.microSD = true;  // MicroSD presente pero archivo no encontrado
            async_led_mp3_stop_reproduciendo(0); // Archivo no encontrado - LED OFF
            return false;
        }

        // empezo_reproduccion esta en cero        comenzó a reproducir
        if (empezo_reproduccion_busy == false && digitalRead(PIN_MP3_BUSY) == CONST_MP3_REPRODUCIENDO)
        {
            // LOG("\r\n\r\nEMPEZO REPRODUCCION");
            empezo_reproduccion_busy = true;  // deja de esperar un tiempo para que empiece a reproducir
            mp3State.initReproduccion = true; // indica que se ha iniciado la reproduccion
            tiempo_busy = millis();           // tiempo de espera para que empiece a reproducir
        }

        // aun no ha comenzado a reproducir y ya paso el tiempo de espera
        if (empezo_reproduccion_busy == false && millis() - tiempo > CONST_MP3_ESP_REP_MP3)
            break;
        // ya comenzo a reproducir
        else if (empezo_reproduccion_busy == true)
        {
            // espera de 5 segundos para que el mp3 termine de reproducir la pista
            // si no se encuentra la pista, se espera 5 segundos para que el mp3 termine de reproducir la pista
            if ((unsigned long)millis() - tiempo_busy >= 5000)
            {
                // no hay respuesta via comunicacion, pin de busy en bajo
                if (error_mp3_no_encontrado == false && digitalRead(PIN_MP3_BUSY) == !CONST_MP3_REPRODUCIENDO)
                {
                    // LOG("\r\n\r\nfin reproduccion por BUSY");
                    mp3State.microSD = true;  // MicroSD funcionando correctamente
#if ENABLE_MP3_ERROR_MANAGEMENT
                    async_error_mp3_desactivar(); // Reproducción exitosa, desactivar error
#endif
                    async_led_mp3_stop_reproduciendo(1); // Reproducción exitosa - LED fijo ON
                    return true;
                }
            }
            // ya paso el tiempo limite de pista
            if (millis() - tiempo > CONST_MP3_LIM_T_PISTA)
                break;
        }

        if (stop_en == true && async_init_mp3_cmd_flag == false) //
        {
            async_led_mp3_stop_reproduciendo(-1); // Interrupción - mantener estado anterior
            return false;                         // si la alarma se apaga, salir del bucle
        }

        rev_reproduccion_temporizada();      // funcion para reproducir una lista de pistas de audio
        if (async_stop_mp3_cmd_flag == true) // si se recibe el comando de stop
        {
            async_stop_mp3_cmd_flag = false; // reiniciar la bandera
            LOG("\r\n\r\nALARMA DESACTIVADA POR COMANDO EN mp3_reproducion_audio\r\n\r\n");
            async_led_mp3_stop_reproduciendo(-1); // Stop por comando - mantener estado anterior
            return false;
        }

        rf_esp_nv_dato(); // verificar si hay un nuevo dato RF

        delay(1);
    }
    mp3State.conexion = false; // no hay conexion
#if ENABLE_MP3_ERROR_MANAGEMENT
    async_error_mp3_activar(); // Error en reproducción, activar error
#endif
    async_led_mp3_stop_reproduciendo(0); // Error de reproducción - LED OFF
    return false;
}

bool mp3_pista_play_comunic_0(uint16_t pista, bool stop_en)
{

    LOGF("\r\n\r\nmp3_pista_play_comunic_0. %d", pista);
    mp3_play_pst_cmd(pista);       // envío serial de la pista
    async_led_mp3_reproduciendo(); // Iniciar parpadeo del LED MP3
    delay(100);                    // espera para que empiece a reproducir

    unsigned long ti_pista = 0;
    unsigned long ti_cmd = millis(); // inicio de espera para empezar a reproducir

    bool pista_ok = 0;

    while (!pista_ok)
    {
        // empezo a reproducir
        if (digitalRead(PIN_MP3_BUSY) == CONST_MP3_REPRODUCIENDO)
            pista_ok = 1;
        // limite de espera para que empiece a reproducir
        if ((unsigned long)millis() - ti_cmd >= CONST_MP3_ESP_REP_MP3)
        {
            async_led_mp3_stop_reproduciendo(0); // Timeout - LED OFF
            return false;
        }
        delay(50);
    }

    if (pista_ok)
    {
        ti_pista = millis();
        // LOG("\r\nINIT REPRODUCCION");

        while (digitalRead(PIN_MP3_BUSY) == CONST_MP3_REPRODUCIENDO) // sigue reproduciendo
        {
            // límite de tiempo de pista
            if (millis() - ti_pista >= CONST_MP3_LIM_T_PISTA)
            {
                // LOG("\r\n\r\nfin reproduccion");
                return true;
            }

            rev_reproduccion_temporizada(); // funcion para reproducir una lista de pistas de audio
            delay(50);
        }

        rf_esp_nv_dato(); // verificar si hay un nuevo dato RF

        // LOG("\r\n\r\nfin reproduccion");
#if ENABLE_MP3_ERROR_MANAGEMENT
        async_error_mp3_desactivar(); // Reproducción exitosa, desactivar error
#endif
        async_led_mp3_stop_reproduciendo(1); // Reproducción exitosa - LED ON
        return true;
    }

    async_led_mp3_stop_reproduciendo(0); // Error general - LED OFF
    return false;
}

void async_ejec_mp3_play_una_pista(void)
{

    if (!async_pista_1_ciclo_flag) // si no hay una pista en reproduccion
        return;

    async_pista_1_ciclo_flag = false; // bandera para indicar que se reproduce una pista de audio

    mp3_amplificador_on(); // encender amplificador

    bool resultado = mp3_reproducion_audio(async_pista_1_ciclo_pista, 1); //
    mp3_amplificador_off(); // apagar amplificador

#if ENABLE_MP3_ERROR_MANAGEMENT
    if (resultado)
    {
        async_error_mp3_desactivar(); // Reproducción exitosa, desactivar error
    }
    else
    {
        async_error_mp3_activar(); // Error en reproducción, activar error
    }
#endif

    LOG("\r\n\r\n\r\ncmdAudioBusy: false\r\n\r\n");
    cmdAudioBusy = false; // Reiniciar el estado del comando de audio
    delay(100);
}

void async_cnf_mp3_play_una_pista(uint16_t pista)
{
    if (pista < 3) // si la pista es menor a 3, no se reproduce nada
    {
        async_pista_1_ciclo_flag = false; // bandera para indicar que no se reproduce una pista de audio
        return;
    }

    async_pista_1_ciclo_pista = pista;
    async_pista_1_ciclo_flag = true;
    delay(100);
}

void async_ejec_mp3_play_lista(void) // funcion para reproducir una lista de pistas de audio
{

    if (!lista_pistas_arr_flag) // si no hay pistas para reproduccion
        return;

    lista_pistas_arr_flag = false; // bandera para indicar que se reproduce una lista de pistas

    uint8_t num_pistas = 0; // numero de pistas a reproducir
    for (size_t i = 0; i < MAX_BOTONES; i++)
    {
        if (lista_pistas_arr[i] != 0) // si hay pistas para reproducir
            num_pistas++;
    }

    mp3_amplificador_on(); // encender amplificador

    bool fin_reproduccion = false; // bandera para indicar si se ha terminado la reproduccion

    while (1)
    {

        rf_nuevo_dato(1); // verificar si hay un nuevo dato RF

        for (size_t i = 0; i < num_pistas; i++)
        {
            if (!mp3_reproducion_audio(lista_pistas_arr[i], 1))
            {
                fin_reproduccion = true; // se ha terminado la reproduccion
                break;
            }
            if (async_init_mp3_cmd_flag == false)
                break; // si la alarma se apaga, salir del bucle
        }
        if (async_init_mp3_cmd_flag == false)
            break; // si la alarma se apaga, salir del bucle
        if (fin_reproduccion)
            break;
    }

    mp3_amplificador_off(); // apagar amplificador

    LOG("\r\n\r\n\r\ncmdAudioBusy: false\r\n\r\n");
    cmdAudioBusy = false; // Reiniciar el estado del comando de audio
    delay(100);
}

void async_cnf_mp3_play_lista(const uint16_t lista[], size_t size) // funcion para configurar una lista de pistas de audio
{

    for (size_t i = 0; i < MAX_BOTONES; i++) //
        lista_pistas_arr[i] = 0;             // limpiar lista de pistas

    LOG("\r\n\r\nasync_cnf_mp3_play_lista:\r\n");
    for (size_t i = 0; i < size; i++)
    {
        lista_pistas_arr[i] = lista[i];
        LOGF("\r\nPista %d: %d", i + 1, lista_pistas_arr[i]);
    }

    lista_pistas_arr_flag = true; // bandera para indicar que se reproduce una lista de pistas
    delay(100);
}

void mp3_serial_info(uint8_t type, int value)
{
    switch (type)
    {
    case TimeOut:
        // LOG("\r\n\r\nTime Out!");
        break;
    case WrongStack:
        LOG("\r\n\r\nStack Wrong!");
        break;
    case DFPlayerCardInserted:
        LOG("\r\n\r\nCard Inserted!");
#if ENABLE_MP3_ERROR_MANAGEMENT
        async_error_mp3_desactivar(); // Desactivar error MP3 temporalmente
#endif
        // Programar prueba de pista automática al insertar tarjeta
        async_pista_prueba_pendiente_flag = true;
        break;
    case DFPlayerCardRemoved:
        LOG("\r\n\r\nCard Removed!");
#if ENABLE_MP3_ERROR_MANAGEMENT
        async_error_mp3_activar(); // Activar error MP3
#endif
        async_led_mp3_off(); // Tarjeta removida - LED OFF
        break;
    case DFPlayerCardOnline:
        LOG("\r\n\r\nCard Online!");
#if ENABLE_MP3_ERROR_MANAGEMENT
        async_error_mp3_desactivar(); // Desactivar error MP3 temporalmente
#endif
        // Programar prueba de pista automática cuando la tarjeta está online
        async_pista_prueba_pendiente_flag = true;
        break;
    case DFPlayerPlayFinished:
        pista_reproducida = value;
        // LOGF("\r\n\r\nPISTA REPRODUCIDA: %d", pista_reproducida);

        // Solo cambiar LED si realmente estaba reproduciendo
        if (mp3State.initReproduccion)
        {
            async_led_mp3_stop_reproduciendo(1); // Reproducción terminó exitosamente - LED ON
        }
        break;
    case DFPlayerError:
        LOG("\r\n\r\nDFPlayerError:");
        switch (value)
        {
        case Busy:
            LOG("\r\n\r\nCard not found");
            break;
        case Sleeping:
            LOG("\r\n\r\nSleeping");
            break;
        case SerialWrongStack:
            LOG("\r\n\r\nGet Wrong Stack");
            break;
        case CheckSumNotMatch:
            LOG("\r\n\r\nCheck Sum Not Match");
            break;
        case FileIndexOut:
            LOG("\r\n\r\nFile Index Out of Bound");
            break;
        case FileMismatch:
            error_mp3_no_encontrado = true; // pistas no encontrada
            LOG("\r\n\r\nCannot Find File");
            break;
        case Advertise:
            LOG("\r\n\r\nIn Advertise");
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void mp3_amplificador_on(void)
{
    digitalWrite(AMPLIFICADOR, AMP_ON);
    mp3State.amplificadorActivo = true;
    delay(100);
}

void mp3_amplificador_off(void)
{
    digitalWrite(AMPLIFICADOR, AMP_OFF);
    mp3State.amplificadorActivo = false;
}
