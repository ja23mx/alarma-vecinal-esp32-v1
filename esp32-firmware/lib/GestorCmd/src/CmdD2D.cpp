#include "GestorCmd.h"

#include <vector>

#include "DataManager.h"
#include "CnfTarjeta.h"
#include "LogSistema.h"
#include "ErroresCMD.h"
#include "EstructurasGlobales.h"
#include "VariablesGlobales.h"
#include "ConfigSistema.h"

void GestorCMD::datosD2D(void)
{
    std::vector<ComandoProcesadoInfo> localResponsesInfo; // Vector para acumular la información

    localResponsesInfo.clear(); // Limpiar el vector antes de llenarlo

    cmdLocal.clear();   // Limpiar el vector antes de llenarlo
    cmdExterno.clear(); // Limpiar el vector antes de llenarlo

    ComandoProcesadoInfo info;

    CmdOrigen origenInfo = CmdOrigen::D2D;

    separarInstrucciones(lastPayload); // Separar las instrucciones del lastPayload
    if (cmd_totales.empty())
    {
        LOG("\r\n\r\nPayload inválido.");

        if (origenInfo == CmdOrigen::SRV_EXT)
        {
            info.rsp_cmd = "error. " + lastPayload;
            info.error = ERROR_FORMATO_INVALIDO;
            info.msg = "Formato invalido.";
            localResponsesInfo.push_back(info);             // Agregar la información al vector
            generateDeviceJsonResponse(localResponsesInfo); // Generar la respuesta JSON
            return;                                         // Formato inválido
        }
    }

    uint16_t cmd_ok = 0; // Contador de comandos válidos

    // Imprimir las instrucciones separadas
    for (const auto &instruccion : cmd_totales)
    {

        procesarCmd(instruccion); // separar el comando

        LOG("\r\n\r\nCMD:\r\nMD: " + cmdDatos.modelo + "\r\nNS: " + cmdDatos.numeroSerie + "\r\nCMD: " + cmdDatos.comando);

        // Validar formatos y coincidencia en un solo bloque optimizado
        if (validarFormato(cmdDatos.modelo, "modelo") &&
            validarFormato(cmdDatos.numeroSerie, "numeroSerie") &&
            validarFormato(cmdDatos.comando, "comandos"))
        {
            // coincidencia de modelo y número de serie
            if (cmdDatos.modelo == BOARD_ID || cmdDatos.numeroSerie == Data.numeroSerie)
            {

                // 3434

                cmd_ok++;
                cmdLocal.push_back(cmdDatos.comando);
            }
            // el comando no es para este dispositivo pero es válido
            else if (cmdDatos.modelo != BOARD_ID && cmdDatos.numeroSerie != Data.numeroSerie)
            {
                cmd_ok++;
                continue; // Continuar con el siguiente comando
            }
        }
        else
        {
            LOG("\r\nFormato inválido o modelo/número de serie no coinciden.");
            continue;
        }
    }

    if (!localResponsesInfo.empty() && cmd_ok > 0)
    {
        // Hubo instrucciones válidas y generaron respuestas
        // origen = SERVIDOR EXTERNO, SERVIDOR LOCAL y CONSOLA
        if (origenInfo == CmdOrigen::SRV_EXT && origenInfo == CmdOrigen::SRV_LOC_CMD && origenInfo == CmdOrigen::CONSOLA)
        {
            generateDeviceJsonResponse(localResponsesInfo);
            // Ejemplo: clienteMqtt.publish("topic/respuesta", this->jsonBuffer);
            LOGF("\r\n\r\nRSP: %s", this->jsonBuffer);

            if (origenInfo == CmdOrigen::CONSOLA)
            {
                LOG("\r\n\r\n");
                serializeJsonPretty(docJson, Serial);
            }
        }
        // informacion de lora o protolo similar que requiere pocos bytes para
        // formar el paquete
        else if (origenInfo == CmdOrigen::D2D)
        {
            LOGF("\r\n\r\nRAW_PACKET: %s", this->jsonBuffer);
        }
    }
    else if (localResponsesInfo.empty() == true && cmd_totales.size() == 0)
    {
        // Hubo instrucciones, pero ninguna generó una respuesta (ej. todas para otros dispositivos o
        // todas con formato inválido no manejado arriba)
        if (origenInfo == CmdOrigen::SRV_EXT && origenInfo == CmdOrigen::SRV_LOC_CMD && origenInfo == CmdOrigen::CONSOLA)
        {
            info.error = ERROR_FORMATO_INVALIDO;
            info.msg = "Formato invalido.";
            localResponsesInfo.push_back(info);             // Agregar la información al vector
            generateDeviceJsonResponse(localResponsesInfo); // Generar la respuesta JSON

            LOGF("\r\n\r\nRSP: %s", this->jsonBuffer);

            if (origenInfo == CmdOrigen::CONSOLA)
            {
                LOG("\r\n\r\n");
                serializeJsonPretty(docJson, Serial);
            }
        }
    }

    docJson.clear();     // Limpiar el documento JSON
    cmd_totales.clear(); // Limpiar el vector de comandos totales
    cmdExterno.clear();  // Limpiar el vector de comandos externos
    cmdLocal.clear();    // Limpiar el vector de comandos locales
    return;              // Formato válido
}

void GestorCMD::procesarCmd(const String &lastPayload)
{
    cmdDatos = {};

    int modeloEnd = lastPayload.indexOf('-');
    int serieEnd = lastPayload.indexOf('*', modeloEnd + 1); // Buscar el siguiente '*' después del modelo
    int comandoEnd = lastPayload.lastIndexOf("##");         // Buscar el final del comando

    if (modeloEnd != -1 && serieEnd != -1)
    {
        cmdDatos.modelo = lastPayload.substring(1, modeloEnd);                 // Extraer modelo
        cmdDatos.numeroSerie = lastPayload.substring(modeloEnd + 1, serieEnd); // Extraer número de serie

        // Extraer el comando completo desde el número de serie hasta antes de "##"
        cmdDatos.comando = lastPayload.substring(serieEnd, comandoEnd); // Excluir "##"
        cmdDatos.comando += "*";
    }
}

void GestorCMD::separarInstrucciones(const String &input)
{

    cmd_totales.clear(); // Limpiar el vector antes de llenarlo

    int start = 0, end;

    // Dividir el string en partes usando "##" como delimitador
    while ((end = input.indexOf("##", start)) != -1)
    {
        String instruccion = input.substring(start, end + 2); // Incluir "##" en cada instrucción
        cmd_totales.push_back(instruccion);                   //
        start = end + 2;                                      // Mover el inicio al siguiente segmento
    }
}

bool GestorCMD::validarFormato(const String &input, const String &tipo)
{
    if (tipo == "modelo")
    {
        // Validar modelo: Mayúsculas, minúsculas, números y guion medio (-)
        for (char c : input)
        {
            if (!isalnum(c))
                return false; // Caracter inválido
        }
    }
    else if (tipo == "numeroSerie")
    {
        // Validar número de serie: Hexadecimal (A-F, a-f, 0-9)
        for (char c : input)
        {
            if (!isxdigit(c))
                return false; // Caracter inválido
        }
    }
    else if (tipo == "comandos")
    {
        // Validar comandos: Mayúsculas, minúsculas, números y símbolos permitidos (-, :, *, @, #)
        for (char c : input)
        {
            if (!isalnum(c) && c != '-' && c != ':' && c != '*' && c != '@' && c != '#')
                return false; // Caracter inválido
        }
    }
    else
    {
        // Tipo desconocido
        return false;
    }

    return true; // Formato válido
}

