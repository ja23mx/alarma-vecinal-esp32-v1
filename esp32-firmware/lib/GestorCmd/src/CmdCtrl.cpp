#include "GestorCmd.h"

#include "ConfigSistema.h"
#include "logSistema.h"
#include "VariablesGlobales.h"

void GestorCMD::CtrlAv(uint8_t tipoCtrl)
{
    bool btnDesactivacion = (estadoCompRFAv.btnIndice + 1 == modeloCtrlAVRx.boton_desactivacion);
    String modelo = String(modeloCtrlAVRx.modelo);
    int btn = estadoCompRFAv.btnIndice;

    // Verificar desactivación primero (común para ambos tipos)
    if ((cmdAudioBusy || cmdSalidasBusy) && btnDesactivacion && (tipoCtrl == AL_CTRL_TP_AV || tipoCtrl == AL_CTRL_TP_GUARDIAN))
    {
        LOG("\r\n\r\nALARMA DESACTIVADA POR CONTROL AV");
        desactivacionAud1Sal1();
        return;
    }

    // Si periféricos están ocupados o es botón de desactivación, salir
    if (cmdAudioBusy || cmdSalidasBusy || btnDesactivacion && (tipoCtrl == AL_CTRL_TP_AV || tipoCtrl == AL_CTRL_TP_GUARDIAN))
        return;

    // Activar alarma (común para ambos tipos)
    estadoAlarma.onCmd = true;

    /*
        Los botones tienen diferentes configuraciones según el tipo de dispositivo: - 0: Deshabilitado -
        1: Silencioso (AV) - 2: Alarma vecinal (AV) - 3: Una pista (AV) - 4: Una pista (Alerta Son/Integ) -
        5: Pista & Mensaje (Alerta Son/Integ) - 6: Solo Mensaje (Alerta Son/Integ) -
        7: Init Programación (Integrador) - 8: Tamper On/Off (Integrador) - 9: Reset (Integrador)
    */

    switch (tipoCtrl)
    {
    case AL_CTRL_TP_AV:
        procesarControlAV(modelo, btn);
        break;

    case AL_CTRL_TP_GUARDIAN:
        procesarControlGuardian(modelo, btn);
        break;

    case AL_CTRL_TP_INTEGRADOR:
        procesarControlIntegrador(modelo, btn);
        break;
    }
}

void GestorCMD::desactivacionAud1Sal1(void)
{
    String instruccionesTot[5];

    instruccionesTot[0] = "*SD*1*0:0:0##";
    instruccionesTot[1] = "*SA*1*0:0:0##";

    procesarInstrucciones(2, instruccionesTot);
}

void GestorCMD::procesarControlAV(const String &modelo, int btn)
{
    LOG("\r\n\r\nALARMA ACTIVADA POR CONTROL AV\r\n\r\n");
    logInformacionControl();

    String instruccionSalida, instruccionAudio;

    if (modelo == "CTAV1")
    {
        LOG("\r\n\r\nALARMA ACTIVADA POR CONTROL AV CTAV1");
        if (btn == 0 || btn == 1)
        {
            String emgValue = (btn == 0) ? "50" : "51";
            String ctrlValue = String(1000 + estadoCompRFAv.control);
            instruccionSalida = "*SD*1*3:1:-1##";
            instruccionAudio = "*SA*1*6:2:60:" + emgValue + ":" + ctrlValue + "##";
        }
        else if (btn == 2)
        {
            cmd1PistaLedAtm(52, 6); // Pista 52, Fade 6
        }
    }
    else if (modelo == "CTAV2")
    {
        LOG("\r\n\r\nALARMA ACTIVADA POR CONTROL AV CTAV2");
        if (btn == 0 || btn == 1)
        {
            String emgValue = (btn == 0) ? "60" : "61";
            instruccionSalida = "*SD*1*3:1:-1##";
            instruccionAudio = "*SA*1*6:2:60:" + emgValue + ":" + emgValue + "##";
        }
        else if (btn == 2)
        {
            cmd1PistaLedAtm(62, 6); // Pista 62, Fade 6
        }
    }

    ejecutarInstrucciones(instruccionSalida, instruccionAudio);
}

void GestorCMD::procesarControlGuardian(const String &modelo, int btn)
{
    LOG("\r\n\r\nALARMA ACTIVADA POR CONTROL DE GUARDIAN\r\n\r\n");
    logInformacionControl();

    String instruccionSalida, instruccionAudio;

    if (modelo == "CTGD1" && btn >= 0 && btn <= 2)
    {
        uint16_t num_pista = 70 + btn;                  // Pistas 70, 71, 72
        cmd1PistaLedAtm(num_pista, (btn < 2) ? 10 : 3); // Pista 70, 71, 72, Fade 10-Nivel de notificación
    }

    ejecutarInstrucciones(instruccionSalida, instruccionAudio);
}

void GestorCMD::procesarControlIntegrador(const String &modelo, int btn)
{
    LOG("\r\n\r\nALARMA ACTIVADA POR CONTROL INTEGRADOR\r\n\r\n");
    logInformacionControl();

    String instruccionSalida, instruccionAudio;

    if (modelo == "CTINT1")
    {
        switch (btn)
        {
        // prueba de sistema
        case 0:
            cmd1PistaLedAtm(80, 10); // Pista 80, Fade 10
            break;
        // inicio o fin de programación
        case 1:
            cmd1PistaLedAtm(init_prog_ble ? 81 : 82, 10); // Pista 81 o 82, Fade 10
            break;
        // reset del sistema
        case 3:
            // Si comandos están ocupados, desactivar y salir
            if (cmdAudioBusy && cmdSalidasBusy)
            {
                desactivacionAud1Sal1(); //
                return;                  // Salir si comandos están ocupados
            }
            // Si no están ocupados, reproducir pista de reset
            else
            {
                cmd1PistaLedAtm(85, 1); // Pista 85, Fade 1
            }
            break;
        }
    }

    ejecutarInstrucciones(instruccionSalida, instruccionAudio);
}

void GestorCMD::cmd1PistaLedAtm(uint16_t pista, uint16_t ledFade)
{
    String instruccionSalida = "*SD*1*3:" + String(ledFade) + ":-1##";
    String instruccionAudio = "*SA*01*1:" + String(pista) + ":0##";

    ejecutarInstrucciones(instruccionSalida, instruccionAudio);
}

void GestorCMD::logInformacionControl()
{
    LOGF("\r\nControl: %d", estadoCompRFAv.control);
    LOGF("\r\nBoton: %d", estadoCompRFAv.btnIndice + 1);
    LOGF("\r\nTipo de boton: %d", modeloCtrlAVRx.tipo_botones[estadoCompRFAv.btnIndice]);
}

void GestorCMD::ejecutarInstrucciones(const String &instruccionSalida, const String &instruccionAudio)
{
    if (instruccionSalida.isEmpty() || instruccionAudio.isEmpty())
        return;

    String instruccionesTot[5];
    instruccionesTot[0] = instruccionSalida;
    instruccionesTot[1] = instruccionAudio;

    LOGF("\r\nInstruccion de salida: %s", instruccionSalida.c_str());
    LOGF("\r\nInstruccion de audio: %s", instruccionAudio.c_str());

    procesarInstrucciones(2, instruccionesTot);
}

void GestorCMD::procesarInstrucciones(uint16_t numInstrucciones, String *instruccionesTot)
{
    // Validación de parámetros
    if (numInstrucciones <= 0 || instruccionesTot == nullptr)
        return;

    ComandoProcesadoInfo info; // Variable local

    cmdAudioPendiente = false;   // Limpiar los estados de comandos pendientes
    cmdSalidasPendiente = false; //
    cmd_totales.clear();         // Limpiar el vector de comandos totales
    cmdSalidas.clear();          // Limpiar el vector de comandos de salida
    cmdAudio = "";               // Limpiar el comando de audio

    // Procesar cada instrucción
    for (uint16_t i = 0; i < numInstrucciones; i++)
    {
        String comando = instruccionesTot[i];

        if (revisarCMD(comando, info))      // comando válido
            cmd_totales.push_back(comando); // Agregar el comando a la lista de comandos totales
    }

    // Imprimir el contenido del vector cmd_totales
    LOG("\r\nContenido de cmd_totales:");
    for (const auto &cmd : cmd_totales)
    {
        LOGF("\r\n  %s", cmd.c_str());
    }

    // Actualizar estados de comandos pendientes
    if (cmdSalidas.size() > 0)
    {
        cmdSalidasPendiente = true; // Indicar que hay comandos de salida pendientes
    }

    if (cmdAudio != "")
    {
        cmdAudioPendiente = true; // Indicar que hay un comando de audio pendiente
    }
}
