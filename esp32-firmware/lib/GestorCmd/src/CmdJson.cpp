#include "GestorCmd.h"

#include <vector>

#include "CnfTarjeta.h"
#include "DataManager.h"
#include "LogSistema.h"
#include "VariablesGlobales.h"
#include "DataManager.h"
#include "ErroresCMD.h"

void GestorCMD::datosJSON(void)
{
    String numeroSerie;

    cmd_totales.clear(); // Limpiar el vector de comandos totales

    // Suponiendo que lastPayload contiene el JSON recibido
    DeserializationError error = deserializeJson(docJson, lastPayload);

    if (error)
    {
        Serial.print(F("Error al parsear JSON: "));
        Serial.println(error.f_str());
        return;
    }

    String dispositivo = docJson["dsp"] | "";
    String tipo = docJson["tipo"] | "";

    if (dispositivo == "" || tipo == "")
    {
        Serial.println("Error: Datos incompletos en el JSON.");
        return;
    }

    Serial.println("Dispositivo: " + dispositivo);
    Serial.println("Numero de serie: " + String(Data.numeroSerie));
    Serial.println("Tipo: " + tipo);

    // Serial.println("Modelo: " + modelo);
    // Serial.println("Numero de serie: " + numeroSerie);

    if (dispositivo != String(Data.numeroSerie))
    {
        Serial.println("El comando no es para este dispositivo.");
        rspJson = false; // No enviar respuesta JSON si el comando no es para este dispositivo
        return;          // El comando no es para este dispositivo
    }

    if (tipo == "cmd")
    {
        processCmdJson(); // Procesar comandos JSON
    }
    else if (tipo == "hd-res")
    {
        processHandShakeRspJson(); // Procesar comandos de Handshake Response del servidor, contiene las emergencias
                                   // y otros parametros de configuración
    }
    else if (tipo == "emg-codi")
    {
        processEmgCoDi(); // Procesar comandos de emergencia almacenados
    }
}

void GestorCMD::processEmgCoDi(void)
{
    LOG("\r\n\r\nprocessEmgCoDi - Procesando comandos de emergencia.");

    // Verificar que existe el campo emg en el JSON
    if (!docJson.containsKey("emg"))
    {
        LOG("\r\nError: No se encontró campo 'emg' en el JSON.");
        generateAckResponse("ack-emg-codi", ERROR_FORMATO_INVALIDO);
        rspJson = true;
        return;
    }

    // Obtener el índice de emergencia (1-15)
    int emergenciaIndex = docJson["emg"].as<int>();
    LOG("\r\nÍndice de emergencia recibido: " + String(emergenciaIndex));

    // Caso especial: emergencia 0 = apagado
    if (emergenciaIndex == 0)
    {
        LOG("\r\nEmergencia 0 detectada - Ejecutando comando de apagado.");
        // Limpiar comandos previos
        cmdSalidas.clear();
        // Asignar comandos de apagado
        cmdAudio = "*SA*01*0:0:0##";
        cmdSalidas.push_back("*SD*01*0:0:0##");

        cmdAudioPendiente = true;
        cmdSalidasPendiente = true;
        delay(100);

        generateAckResponse("ack-emg-codi", ERROR_NINGUNO);
        rspJson = true;

        LOG("\r\nComando de apagado completado. Audio: " + cmdAudio +
            ", Salidas: " + String(cmdSalidas.size()));
        return;
    }

    // Convertir a índice de vector (0-14)
    int vectorIndex = emergenciaIndex - 1;

    // Verificar que el índice es válido
    if (vectorIndex < 0 || vectorIndex >= emgCoDi.size())
    {
        LOG("\r\nError: Índice de emergencia fuera de rango. Índice: " + String(vectorIndex) +
            ", Tamaño vector: " + String(emgCoDi.size()));
        generateAckResponse("ack-emg-codi", ERROR_PARAMETROS_INVALIDOS);
        rspJson = true;
        return;
    }

    // Obtener la cadena de comandos concatenados
    String comandosConcatenados = emgCoDi[vectorIndex];
    LOG("\r\nComandos concatenados: " + comandosConcatenados);

    if (comandosConcatenados.length() == 0)
    {
        LOG("\r\nError: No hay comandos en la posición especificada.");
        generateAckResponse("ack-emg-codi", ERROR_COMANDO_NO_ENCONTRADO);
        rspJson = true;
        return;
    }

    // Limpiar comandos previos
    cmdSalidas.clear();
    cmdAudio = "";

    // Separar comandos usando "##" como delimitador
    int startPos = 0;
    int endPos = 0;

    while ((endPos = comandosConcatenados.indexOf("##", startPos)) != -1)
    {
        // Extraer comando individual (incluyendo ##)
        String comando = comandosConcatenados.substring(startPos, endPos + 2);
        LOG("\r\nProcesando comando individual: " + comando);

        // Procesar según tipo
        if (comando.startsWith("*SA*"))
        {
            // Comando de audio
            cmdAudio = comando;
            cmdAudioPendiente = true;
            LOG("\r\nComando de audio asignado: " + cmdAudio);
        }
        else if (comando.startsWith("*SD*"))
        {

            cmdSalidas.push_back(comando);
            LOG("\r\nComando digital válido agregado: " + comando);
        }
        else
        {
            LOG("\r\nComando con formato desconocido: " + comando);
        }

        // Mover al siguiente comando
        startPos = endPos + 2;
    }

    // Indicar que hay comandos pendientes si se agregaron
    if (cmdSalidas.size() > 0)
        cmdSalidasPendiente = true;

    if (cmdAudio != "")
        cmdAudioPendiente = true; // Indicar que hay un comando de audio pendiente
    delay(100);                   // Pequeña pausa para evitar problemas de sincronización

    generateAckResponse("ack-emg-codi", ERROR_NINGUNO);
    rspJson = true;

    LOG("\r\nprocessEmgCoDi completado. Audio pendiente: " + String(cmdAudioPendiente) +
        ", Salidas pendientes: " + String(cmdSalidasPendiente) +
        ", Comandos audio: " + (cmdAudio != "" ? "1" : "0") +
        ", Comandos digitales: " + String(cmdSalidas.size()));
}

void GestorCMD::processHandShakeRspJson(void)
{
    LOG("\r\n\r\nprocessHandShakeRspJson.");

    // Limpiar el vector de emergencias actual
    emgCoDi.clear();

    // Verificar que existe el objeto params
    if (!docJson.containsKey("params") || !docJson["params"].containsKey("emgs"))
    {
        LOG("\r\nError: No se encontró 'params.emgs' en el JSON.");
        generateHandShakeAckResponse(ERROR_FORMATO_INVALIDO);
        rspJson = true; // Indicar que se debe enviar una respuesta JSON
        return;
    }

    // Extraer las emergencias
    JsonObject emgs = docJson["params"]["emgs"];
    int emergenciasRecibidas = 0;
    int emergenciasValidas = 0;

    for (JsonPair emergencia : emgs)
    {
        String key = emergencia.key().c_str();          // emg1, emg2, etc.
        String value = emergencia.value().as<String>(); // Código de emergencia

        LOG("\r\nValidando " + key + ": " + value);

        bool emergenciaValida = true;
        emergenciasRecibidas++;

        // Separar y validar cada comando individual
        int startPos = 0;
        int endPos = 0;

        while ((endPos = value.indexOf("##", startPos)) != -1 && emergenciaValida)
        {
            // Extraer comando individual (incluyendo ##)
            String comando = value.substring(startPos, endPos + 2);
            Serial.println("\r\n  Comando a validar: " + comando);

            ComandoProcesadoInfo info;
            if (!revisarCMD(comando, info))
            {
                LOG("\r\nError: Comando inválido en " + key + ": " + comando);
                LOG("\r\nMotivo: " + String(info.msg));
                emergenciaValida = false;
            }
            startPos = endPos + 2;
        }

        if (emergenciaValida)
        {
            emgCoDi.push_back(value); // Agregar al vector global solo si es válida
            emergenciasValidas++;
            LOG("\r\nEmergencia " + key + " validada y agregada.");
        }
        else
        {
            LOG("\r\nEmergencia " + key + " descartada por errores de formato.");
        }
    }

    LOG("\r\nTotal emergencias recibidas: " + String(emergenciasRecibidas));
    LOG("\r\nTotal emergencias válidas almacenadas: " + String(emergenciasValidas));
    LOG("\r\nTotal emergencias en vector: " + String(emgCoDi.size()));

    // Generar respuesta de confirmación basada en la validación
    if (emergenciasValidas == emergenciasRecibidas)
    {
        // Todas las emergencias son válidas
        generateHandShakeAckResponse(ERROR_NINGUNO);
        LOG("\r\nTodas las emergencias validadas correctamente.");
    }
    else
    {
        // Al menos una emergencia tiene errores
        generateHandShakeAckResponse(ERROR_PARAMETROS_INVALIDOS);
        LOG("\r\nAlgunas emergencias contienen errores de formato.");
    }

    rspJson = true; // Indicar que se debe enviar una respuesta JSON
}

void GestorCMD::processCmdJson(void)
{
    std::vector<ComandoProcesadoInfo> localResponsesInfo; // Vector para acumular la información
    ComandoProcesadoInfo info;

    localResponsesInfo.clear(); // Limpiar el vector antes de llenarlo
    cmdSalidas.clear();         // Limpiar el vector de comandos de salida

    LOG("\r\n\r\nGestorCMD::processCmdJson.\r\n");

    JsonArray datos = docJson["datos"].as<JsonArray>();

    for (JsonVariant v : datos)
    {
        String comando = v.as<String>();

        if (revisarCMD(comando, info))      // comando válido
            cmd_totales.push_back(comando); // Agregar el comando a la lista de comandos totales

        localResponsesInfo.push_back(info); // Agregar la información al vector
    }

    generateDeviceJsonResponse(localResponsesInfo); // Generar la respuesta JSON

    serializeJsonPretty(docJson, Serial); // Imprimir el JSON en formato legible

    for (size_t i = 0; i < localResponsesInfo.size(); ++i)
    {
        ComandoProcesadoInfo &response = localResponsesInfo[i];
        Serial.print("\r\nComando: ");
        Serial.print(response.rsp_cmd);
        Serial.print("\r\nError: ");
        Serial.print(response.error);
        Serial.print("\r\nMensaje: ");
        Serial.println(response.msg);
    }

    LOG("\r\n\r\nGestorCMD::processCmdJson. Comandos totales: " + String(cmd_totales.size()));
    LOG("\r\n\r\ncmdAudio: " + cmdAudio + "\r\n");
    LOG("\r\n\r\ncmdSalidas: " + String(cmdSalidas.size()) + "\r\n");

    if (cmdSalidas.size() > 0 || cmdAudio != "")
        rspJson = true; // Indicar que se debe enviar una respuesta JSON

    if (cmdSalidas.size() > 0)
        cmdSalidasPendiente = true; // Indicar que hay comandos de salida pendientes

    if (cmdAudio != "")
        cmdAudioPendiente = true; // Indicar que hay un comando de audio pendiente
}
