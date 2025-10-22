#include "GestorCmd.h"

#include <vector>

#include "ListaComandos.h"
#include "ConfigSistema.h"
#include "CnfTarjeta.h"
#include "DataManager.h"
#include "LogSistema.h"
#include "ErroresCMD.h"
#include "VariablesGlobales.h"

// Definición de la instancia global
GestorCMD GestorCmd;

GestorCMD::GestorCMD()
{
    // Inicializar jsonBuffer a una cadena vacía
    jsonBuffer[0] = '\0'; // <--- INICIALIZACIÓN AQUÍ
    docJson.clear();      // Limpiar el documento JSON
    cmd_totales.clear();  // Limpiar el vector de comandos totales
    cmdDatos = {};        // Inicializar la estructura de datos del comando
}

DATOS_INSTRUCCION comando_local;
std::vector<long> numSeparados;

void GestorCMD::process(const String &payload, CmdOrigen origenInfo)
{

    lastPayload = payload; // Guardar el último payload recibido

    if (origenInfo == CmdOrigen::D2D)
    {
        datosD2D();
    }
    else if (origenInfo == CmdOrigen::SRV_EXT)
    {
        datosJSON();
    }
}

/*
    @brief: Método para activar una pista de audio y salida led dependiente de la reproduccion
    @param pista: Número de la pista de audio a reproducir.
    @param config: Configuración del LED (1: ON-OFF, 2: PWM, 3: FADE).
    @param cargaPeriferico: Valor de carga al periférico (depende de la configuración).
*/
bool GestorCMD::playPistaLedAtm(uint16_t pista, uint8_t config, uint8_t cargaPeriferico)
{
    uint16_t numInstrucciones;
    String instruccionAudio;
    String instruccionSalida;
    String instruccionesTot[5];

    if (pista > 1000) // pista fuera de rango
        return false;

    // config ON-OFF
    if (config == 1 && cargaPeriferico > 1) // fuera de rango
        return false;

    // config PWM
    if (config == 2 && cargaPeriferico > 100) // fuera de rango
        return false;

    // config FADE
    if (config == 3 && cargaPeriferico > 10) // fuera de rango
        return false;

    // Construir instrucción de salida
    instruccionSalida = "*SD*1*" + String(config) + ":" + String(cargaPeriferico) + ":-1##";
    // Construir instrucción de audio
    instruccionAudio = "*SA*1*1:" + String(pista) + ":0##";

    numInstrucciones = 2;
    instruccionesTot->clear();               // Limpiar las instrucciones totales
    instruccionesTot[0] = instruccionSalida; // Instrucción de salida
    instruccionesTot[1] = instruccionAudio;  // Instrucción de audio

    procesarInstrucciones(numInstrucciones, instruccionesTot);

    return true; // Retornar true si las instrucciones se han preparado correctamente
}

bool GestorCMD::revisarCMD(const String &cmd, ComandoProcesadoInfo &info)
{
    info = {}; // Esto reinicia rsp_cmd, error, y msg

    if (!separarComando(cmd, comando_local)) /* 3434 */
    {
        // Si separarComando falla, el comando original 'cmd' es el que se reporta
        setErrorInfo(info, cmd, ERROR_FORMATO_INVALIDO, MSG_FORMATO_INVALIDO); // Asumiendo que tienes MSG_FORMATO_INVALIDO
        return false;
    }

    uint8_t numSalida;
    bool comandoValido = false;

    for (int i = 0; i < numComandos; i++)
    {
        if (comando_local.cmd == listaComandos[i])
        {
            comandoValido = true;
            break;
        }
    }

    // no es un comando valido
    if (!comandoValido)
    {
        setErrorInfo(info, cmd, ERROR_COMANDO_NO_ENCONTRADO, MSG_COMANDO_NO_ENCONTRADO);
        return false;
    }

    /* LOG("\r\nComando valido: " + comando_local.cmd);
    LOG("\r\nNumero de periferico: " + comando_local.numPeriferico);
    LOG("\r\nParametros: " + comando_local.parametros); */

    if (comando_local.cmd == "SD") // Si el comando es "SD" SALIDA DIGITAL
    {

        numSalida = comando_local.numPeriferico.toInt(); // Convertir el número de salida a entero

        // salida fuera de rango
        if (numSalida > GST_SALIDA_NUM_PRF)
        {
            // Reportar el comando extraído o el comando completo original
            setErrorInfo(info, cmd, ERROR_COMANDO_INVALIDO, MSG_COMANDO_INVALIDO);
            return false;
        }
        // Agregar el comando a la lista de comandos de salida

        if (!separarArrNumeros(comando_local.parametros, ':', numSeparados)) // Asumiendo que numSeparados se llena aquí
        {
            // Error al separar los parámetros
            setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS); // Asumiendo estas macros/constantes
            return false;
        }

        /* Serial.print("\r\n\r\n");
        for (size_t i = 0; i < numSeparados.size(); ++i)
        {
            Serial.print("\r\nnumSeparados[");
            Serial.print(i);
            Serial.print("] = ");
            Serial.println(numSeparados[i]);
        }*/

        // el comando de audio está ocupado y llego un comando de activación de salida
        /* if (cmdAudioBusy == true && numSeparados[0] != 0)
        {
            Serial.print("\r\n  Comando de audio ocupado: " + cmd);
            setErrorInfo(info, cmd, WARNING_PERIFERICO_OCUPADO, MSG_PERIFERICO_OCUPADO);
            return false; // Si el comando de audio está ocupado, no procesar más comandos
        } */

        switch (numSeparados[0])
        {
        case 0: // APAGADO 0:0:0
            if (numSeparados[1] == 0 && numSeparados[2] == 0)
            {
                cmdSalidas.push_back(cmd);                                  // Agregar el comando a la lista de comandos de salida
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); // Asumiendo estas macros/constantes
                return true;
            }
            break;
        case 1: // ENCENDIDO 1:1:[2]TIME
            if (numSeparados[1] == 1 && numSeparados[2] <= GST_SALIDA_SD_MAX_TM_1_SG)
            {
                cmdSalidas.push_back(cmd);                                  // Agregar el comando a la lista de comandos de salida
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); //
                return true;
            }
            break;
        case 2: // PWM 2:[1]PWM:[2]TIME
            if (numSeparados[1] >= GST_SALIDA_PWM_MIN_DUTY_CYCLE && numSeparados[1] <= 100 && numSeparados[2] <= GST_SALIDA_SD_MAX_TM_1_SG)
            {
                cmdSalidas.push_back(cmd);                                  // Agregar el comando a la lista de comandos de salida
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); //
                return true;
            }
            break;
        case 3: // FADE 3:[1]NIVEL:[2]TIME
            if (numSeparados[1] >= 1 && numSeparados[1] <= GST_SALIDA_FADE_OPCION && numSeparados[2] <= GST_SALIDA_SD_MAX_TM_1_SG)
            {
                cmdSalidas.push_back(cmd);                                  // Agregar el comando a la lista de comandos de salida
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); //
                return true;
            }
            break;
        case 4: // TONO 4:[1]NIVEL:[2]TIME
            if (numSeparados[1] >= 1 && numSeparados[1] <= GST_SALIDA_TONO_OPCION && numSeparados[2] <= GST_SALIDA_SD_MAX_TM_1_SG)
            {
                cmdSalidas.push_back(cmd);                                  // Agregar el comando a la lista de comandos de salida
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); //
                return true;
            }
            break;
        }

        // algun parametro erroneo
        setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
        return false;
    }
    else if (comando_local.cmd == "SA") // Si el comando es "SA" SALIDA DE AUDIO
    {
        numSalida = comando_local.numPeriferico.toInt(); // Convertir el número de salida a entero
        // salida fuera de rango
        if (numSalida == 0 || numSalida > 1)
        {
            // Reportar el comando extraído o el comando completo original
            setErrorInfo(info, cmd, ERROR_COMANDO_INVALIDO, MSG_COMANDO_INVALIDO);
            return false;
        }

        if (!separarArrNumeros(comando_local.parametros, ':', numSeparados)) // Asumiendo que numSeparados se llena aquí
        {
            // Error al separar los parámetros
            setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS); // Asumiendo estas macros/constantes
            return false;
        }

        // el comando de salida está ocupado y llego un comando de activación de salida
        if (cmdSalidasBusy == true && numSeparados[0] != 0)
        {
            setErrorInfo(info, cmd, WARNING_PERIFERICO_OCUPADO, MSG_PERIFERICO_OCUPADO);
            return false; // Si el comando de audio está ocupado, no procesar más comandos
        }

        switch (numSeparados[0])
        {
        case 0: // APAGADO 0:0:0
            if (numSeparados[1] == 0 && numSeparados[2] == 0)
            {
                cmdAudio = cmd;
                // Comando válido, llenar info con éxito
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); // Asumiendo estas macros/constantes
                return true;
            }
            break;
        case 1: // Pista individual una vez 1:[1]Pista:0
        case 2: // Pista individual indefinido 2:[1]Pista:0
            if (numSeparados[1] >= 2 && numSeparados[1] <= GST_SALIDA_MAX_PISTA_MP3 && numSeparados[2] == 0)
            {
                cmdAudio = cmd;
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO);
                return true;
            }
            break;
        case 3: // Pista individual temporizada 3:[1]Pista:[2]TIME
            if (numSeparados[1] >= 2 && numSeparados[1] <= GST_SALIDA_MAX_PISTA_MP3 && numSeparados[2] >= 0 && numSeparados[2] <= GST_SALIDA_SD_MAX_TM_1_SG)
            {
                cmdAudio = cmd;
                setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO);
                return true;
            }
            break;

        case 4: // lista de pistas reproduce una vez 4:[1]NUM_PISTAS:[2]Pista1:[3]Pista2:...
        case 5: // lista de pistas reproduce en bucle 5:[1]NUM_PISTAS:[2]Pista1:[3]Pista2:...
            // por lo menos debe haber dos pistas
            if (numSeparados.size() < 4)
            {
                setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
                return false;
            }

            // el numero de pistas totales debe ser igual al primer parametro
            if ((2 + numSeparados[1]) != numSeparados.size())
            {
                setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
                return false;
            }

            for (size_t i = 2; i < numSeparados.size(); i++)
            {
                if (numSeparados[i] < 2 || numSeparados[i] > GST_SALIDA_MAX_PISTA_MP3)
                {
                    // Comando válido, llenar info con éxito
                    setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO); // Asumiendo estas macros/constantes
                    return false;
                }
            }

            cmdAudio = cmd;
            setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO);
            return true;
            break;

        case 6: // lista de pistas temporizada 6:[1]NUM_PISTAS:[2]TIME:[3]Pista1:[4]Pista2:...

            // por lo menos debe haber dos pistas
            if (numSeparados.size() < 5)
            {
                setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
                return false;
            }

            // el numero de pistas totales debe ser igual al primer parametro
            if ((3 + numSeparados[1]) != numSeparados.size())
            {
                setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
                return false;
            }

            // el tiempo de espera debe ser mayor a 0 y menor al maximo
            if (numSeparados[1] == 0 || numSeparados[1] > GST_SALIDA_SD_MAX_TM_1_SG)
            {
                setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
                return false;
            }

            for (size_t i = 3; i < numSeparados.size(); i++)
            {
                if (numSeparados[i] < 2 || numSeparados[i] > GST_SALIDA_MAX_PISTA_MP3)
                {
                    setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
                    return false;
                }
            }

            cmdAudio = cmd;
            setErrorInfo(info, cmd, ERROR_NINGUNO, MSG_COMANDO_VALIDO);
            return true;
            break;
        }

        // algun parametro erroneo
        setErrorInfo(info, cmd, ERROR_PARAMETROS_INVALIDOS, MSG_PARAMETROS_INVALIDOS);
        return false;
    }

    // algun parametro erroneo
    // Si se llega aquí, significa que el comando estaba en la lista pero no se manejó
    // específicamente o falló alguna validación interna no cubierta.
    // Esto podría ser un punto para un error genérico de "parámetros no válidos para este comando"
    // o "lógica de comando no implementada completamente".
    setErrorInfo(info, cmd, ERROR_COMANDO_INVALIDO, MSG_COMANDO_INVALIDO);
    return false;
}

void GestorCMD::setErrorInfo(ComandoProcesadoInfo &info, const String &cmdStr, int errorCode, const String &errorMsg)
{
    info.rsp_cmd = cmdStr;
    info.error = errorCode;
    info.msg = errorMsg;
}

bool GestorCMD::separarComando(const String &comando, DATOS_INSTRUCCION &data)
{

    data = {}; // Inicializar la estructura de datos

    // Separar los campos del comando usando el delimitador '*'
    // y el terminador "##"
    int firstAsterisk = comando.indexOf('*', 1); // Primer asterisco después del inicial (entre CMD e ID)
    int secondAsterisk = -1;
    if (firstAsterisk != -1)
    {
        secondAsterisk = comando.indexOf('*', firstAsterisk + 1); // Asterisco entre ID y PARAMETROS
    }

    int endMarker = -1;
    if (secondAsterisk != -1)
    {
        endMarker = comando.indexOf("##", secondAsterisk + 1); // Posición del terminador "##"
    }

    // Verificar que el comando empiece con '*' y que todos los delimitadores se encontraron
    if (comando.startsWith("*") && firstAsterisk != -1 && secondAsterisk != -1 && endMarker != -1)
    {
        // Extraer el comando (entre el primer '*' y firstAsterisk)
        data.cmd = comando.substring(1, firstAsterisk);
        // Extraer el número de periférico (entre firstAsterisk y secondAsterisk)
        data.numPeriferico = comando.substring(firstAsterisk + 1, secondAsterisk);
        // Extraer los parámetros (entre secondAsterisk y endMarker)
        data.parametros = comando.substring(secondAsterisk + 1, endMarker);
        return true;
    }

    return false;
}

bool GestorCMD::separarArrNumeros(const String &input, char delimitador, std::vector<long> &numerosArray)
{
    numerosArray.clear(); // Limpiar el vector antes de llenarlo

    if (input.length() == 0)
    {
        return true; // Input vacío resulta en un array vacío, considerado éxito.
                     // Si prefieres que un input vacío sea un error, retorna false.
    }

    int startIndex = 0;
    int endIndex = 0;
    bool todosValidos = true;

    while (startIndex <= input.length())
    {
        endIndex = input.indexOf(delimitador, startIndex);
        String token;

        if (endIndex == -1)
        { // No se encontró más delimitador
            if (startIndex < input.length())
            {
                token = input.substring(startIndex);
            }
            else if (input.length() > 0 && input.charAt(input.length() - 1) == delimitador)
            {
                // Si el string original terminaba con el delimitador,
                // significa que hay un campo vacío después del último delimitador.
                token = ""; // Tratar como campo vacío
            }
            else
            {
                break; // No hay más tokens
            }
        }
        else
        {
            token = input.substring(startIndex, endIndex);
        }

        // Intentar convertir el token a número
        if (token.length() > 0)
        {
            bool esNumeroValido = true;
            size_t startIdx = 0;

            // Permitir signo negativo al inicio
            if (token.length() > 0 && token.charAt(0) == '-')
            {
                startIdx = 1;
                if (token.length() == 1)
                { // Solo "-" sin números
                    esNumeroValido = false;
                }
            }

            if (esNumeroValido)
            {
                for (size_t i = startIdx; i < token.length(); ++i)
                {
                    if (!isDigit(token.charAt(i)))
                    {
                        esNumeroValido = false;
                        break;
                    }
                }
            }

            if (esNumeroValido)
            {
                long valorLong = token.toInt();
                if (valorLong >= -32768 && valorLong <= 65535) // Rango extendido
                {
                    numerosArray.push_back(static_cast<long>(valorLong));
                }
                else
                {
                    todosValidos = false;
                    LOG("\r\nError: Token '" + token + "' fuera de rango.");
                    break;
                }
            }
        }
        else
        {
            // Token vacío (ej. "1::3" o "1:2:").
            // Decide cómo manejar esto. Aquí lo consideraremos un error si se espera un número.
            // Si un token vacío es aceptable y debe ser ignorado o representado por un valor (ej. 0), ajusta la lógica.
            LOG("\r\nAdvertencia: Token vacío encontrado.");
            // Si los tokens vacíos deben ser un error:
            // todosValidos = false;
            // break;
        }

        if (endIndex == -1)
        {
            break; // Salir del bucle si era el último token
        }
        startIndex = endIndex + 1;
    }

    if (!todosValidos)
    {
        numerosArray.clear(); // Si hubo un error, dejar el array vacío
        return false;
    }

    return true;
}

void GestorCMD::generateDeviceJsonResponse(const std::vector<ComandoProcesadoInfo> &responses)
{
    docJson.clear(); // Limpiar el documento JSON

    // Crear el objeto principal para el dispositivo
    JsonObject deviceObject = docJson.createNestedObject(Data.numeroSerie);

    // Añadir el campo "tipo"
    deviceObject["tipo"] = "rsp_cmd";

    // Crear el array anidado "respuestas"
    JsonArray respuestasArray = deviceObject.createNestedArray("respuestas");

    if (!responses.empty())
    {
        for (const auto &resp : responses)
        {
            JsonObject responseObj = respuestasArray.createNestedObject();
            responseObj["cmd"] = resp.rsp_cmd; // Usar el comando de respuesta original
            responseObj["error"] = resp.error;
            responseObj["msg"] = resp.msg;
        }
    }

    size_t len = serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));

    if (len == 0 && sizeof(jsonBuffer) > 0) // Verificar también que el buffer no sea de tamaño 0
    {
        LOG("\r\nGestorCMD: Error al serializar JSON para respuestas de dispositivo o buffer insuficiente.");
        // Crear una respuesta de error genérica si la serialización falla
        docJson.clear();
        JsonObject errorDeviceObject = docJson.createNestedObject(Data.numeroSerie);
        errorDeviceObject["tipo"] = "rsp_cmd_error"; // Tipo de respuesta de error
        JsonArray errorRespuestasArray = errorDeviceObject.createNestedArray("respuestas");
        JsonObject errorObj = errorRespuestasArray.createNestedObject();
        errorObj["cmd"] = "SERIALIZATION_ERROR";
        errorObj["error"] = -1; // Un código de error genérico para la serialización
        errorObj["msg"] = "Error al serializar JSON o buffer insuficiente.";
        serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer)); // Intentar serializar el mensaje de error
    }
    // En este punto, this->jsonBuffer CONTIENE el string JSON listo para ser enviado.
}

void GestorCMD::generateHandShakeAckResponse(uint8_t status)
{
    generateAckResponse("hd-ack", status);
}

const char* GestorCMD::getErrorMessage(uint8_t errorCode)
{
    switch (errorCode)
    {
        case ERROR_NINGUNO: 
            return MSG_COMANDO_VALIDO;
        case ERROR_FORMATO_INVALIDO: 
            return MSG_FORMATO_INVALIDO;
        case ERROR_MODELO_NO_COINCIDE: 
            return MSG_MODELO_NO_COINCIDE;
        case ERROR_NUMERO_SERIE_NO_COINCIDE: 
            return MSG_NUMERO_SERIE_NO_COINCIDE;
        case ERROR_COMANDO_INVALIDO: 
            return MSG_COMANDO_INVALIDO;
        case ERROR_COMANDO_NO_ENCONTRADO: 
            return MSG_COMANDO_NO_ENCONTRADO;
        case ERROR_SALIDA_ERRONEA: 
            return MSG_SALIDA_ERRONEA;
        case ERROR_PARAMETROS_INVALIDOS: 
            return MSG_PARAMETROS_INVALIDOS;
        case WARNING_PERIFERICO_OCUPADO: 
            return MSG_PERIFERICO_OCUPADO;
        case ERROR_INTERNO: 
            return MSG_ERROR_INTERNO;
        default: 
            return "Error desconocido";
    }
}

void GestorCMD::generateAckResponse(const String &responseType, uint8_t errorCode)
{
    docJson.clear(); // Limpiar el documento JSON

    // Crear el JSON de respuesta según el formato especificado
    docJson["dsp"] = Data.numeroSerie;           // Número de serie del dispositivo
    docJson["type"] = responseType;              // Tipo de respuesta (hd-ack, ack-emg-codi, etc.)
    docJson["error"] = errorCode;                // Código de error numérico
    docJson["msg"] = getErrorMessage(errorCode); // Mensaje descriptivo del error

    size_t len = serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));

    if (len == 0 && sizeof(jsonBuffer) > 0)
    {
        LOG("\r\nGestorCMD: Error al serializar JSON para respuesta " + responseType + ".");
        // Crear una respuesta de error genérica si la serialización falla
        docJson.clear();
        docJson["dsp"] = Data.numeroSerie;
        docJson["type"] = responseType;
        docJson["error"] = ERROR_INTERNO;
        docJson["msg"] = getErrorMessage(ERROR_INTERNO);
        serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    }
    
    LOG("\r\nACK Response generado (" + responseType + "): " + String(jsonBuffer));
}