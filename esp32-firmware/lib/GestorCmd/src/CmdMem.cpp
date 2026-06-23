/**
 * @file CmdMem.cpp
 * @brief Comandos MQTT de lectura y escritura de memoria del dispositivo.
 *
 * Implementa los handlers de `tipo: "mem"` para leer y modificar:
 *   - Configuración del sistema (config)
 *   - Redes WiFi guardadas (wifi)
 *   - Controles RF registrados (rf)
 *
 * El dispatcher principal es processMemJson(). Cada operación serializa
 * su respuesta en jsonBuffer y activa rspJson para que mqtt_loop() publique el ACK.
 */
#include "GestorCmd.h"
#include "DataManager.h"
#include "LogSistema.h"
#include "ErroresCMD.h"

// ---------------------------------------------------------------------------
// Helper interno
// ---------------------------------------------------------------------------

void GestorCMD::buildMemHeader(const char *target, uint8_t status)
{
    docJson.clear();
    docJson["dsp"] = Data.numeroSerie;
    docJson["tipo"] = "ack-mem";
    docJson["target"] = target;
    docJson["status"] = status;
}

// ---------------------------------------------------------------------------
// Lectura
// ---------------------------------------------------------------------------

void GestorCMD::readMemConfig(void)
{
    buildMemHeader("config", ERROR_NINGUNO);
    JsonObject datos = docJson.createNestedObject("datos");
    datos["num_serie"] = Data.numeroSerie;
    datos["pin"] = Data.pinDispositivo;
    datos["timer"] = Data.timerAlarma;
    datos["tamper"] = Data.tamper;
    JsonObject per = datos.createNestedObject("per");
    per["rf"] = Data.perifericos[0];
    per["wifi"] = Data.perifericos[1];
    per["voz"] = Data.perifericos[2];
    per["per"] = Data.perifericos[3];
    serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    LOG("\r\nreadMemConfig ok.");
}

void GestorCMD::readMemWifi(void)
{
    buildMemHeader("wifi", ERROR_NINGUNO);
    JsonArray datos = docJson.createNestedArray("datos");
    for (const auto &red : Data.redesGuardadas)
    {
        JsonObject entry = datos.createNestedObject();
        entry["ssid"] = red.ssid;
        entry["pass"] = red.password;
    }
    serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    LOG("\r\nreadMemWifi ok. Redes: " + String(Data.redesGuardadas.size()));
}

/**
 * Lee los controles RF registrados con paginación (20 por página).
 * La página se indica con el campo opcional "page" (0-based) en el request.
 * La página 0 incluye también el array "modelos" con los tipos disponibles.
 */
void GestorCMD::readMemRF(void)
{
    const uint8_t PAGE_SIZE = 20;
    uint8_t page = docJson["page"] | 0;

    size_t total = Data.controlValues.size();
    uint8_t pages = total == 0 ? 1 : (uint8_t)((total + PAGE_SIZE - 1) / PAGE_SIZE);
    size_t start = page * PAGE_SIZE;
    size_t end = start + PAGE_SIZE < total ? start + PAGE_SIZE : total;

    buildMemHeader("rf", ERROR_NINGUNO);
    docJson["page"] = page;
    docJson["pages"] = pages;
    docJson["total"] = (int)total;

    if (page == 0)
    {
        JsonArray modelos = docJson.createNestedArray("modelos");
        for (const auto &modelo : Data.CtrlModelos)
        {
            JsonObject m = modelos.createNestedObject();
            char id_str[2] = {modelo.id_cnf_ctrl, '\0'};
            m["id_ct"] = id_str;
            m["nombre"] = modelo.nombre;
        }
    }

    JsonArray datos = docJson.createNestedArray("datos");
    for (size_t i = start; i < end; i++)
    {
        const auto &cv = Data.controlValues[i];
        char id_ct = std::get<0>(cv);
        uint16_t num = std::get<1>(cv);
        uint8_t ctrlStatus = std::get<2>(cv);
        unsigned long signal = std::get<3>(cv);

        JsonObject entry = datos.createNestedObject();
        entry["num"] = num;
        char id_str[2] = {id_ct, '\0'};
        entry["id_ct"] = id_str;
        entry["status"] = ctrlStatus;
        entry["sig"] = signal;
    }

    serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    LOG("\r\nreadMemRF. page " + String(page) + "/" + String(pages) +
        " entries: " + String(end - start));
}

// ---------------------------------------------------------------------------
// Escritura
// ---------------------------------------------------------------------------

void GestorCMD::writeMemConfig(void)
{
    JsonObject datos = docJson["datos"].as<JsonObject>();
    bool ok = true;

    if (datos.containsKey("timer"))
        ok &= Data.setTimerAlarma(datos["timer"].as<uint16_t>());
    if (datos.containsKey("tamper"))
        ok &= Data.setTamper(datos["tamper"].as<uint8_t>());
    if (datos.containsKey("pin"))
        ok &= Data.setClaveDispositivo(datos["pin"].as<uint16_t>());
    if (datos.containsKey("rf"))
        ok &= Data.setPerifericos(1, datos["rf"].as<uint8_t>());
    if (datos.containsKey("wifi"))
        ok &= Data.setPerifericos(2, datos["wifi"].as<uint8_t>());
    if (datos.containsKey("voz"))
        ok &= Data.setPerifericos(3, datos["voz"].as<uint8_t>());
    if (datos.containsKey("per"))
        ok &= Data.setPerifericos(4, datos["per"].as<uint8_t>());

    buildMemHeader("config", ok ? ERROR_NINGUNO : 1);
    serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    LOG("\r\nwriteMemConfig. ok: " + String(ok));
}

void GestorCMD::writeMemWifi(void)
{
    String op = docJson["op"] | "";
    bool ok = false;

    if (op == "add")
    {
        String ssid = docJson["datos"]["ssid"] | "";
        String pass = docJson["datos"]["pass"] | "";
        if (ssid.length() > 0)
        {
            ok = Data.guardarRedWiFi(ssid.c_str(), pass.c_str());
            if (ok)
                Data.redesGuardadas = Data.leerTodasLasRedesGuardadas();
        }
    }
    else if (op == "del")
    {
        String ssid = docJson["datos"]["ssid"] | "";
        if (ssid.length() > 0)
        {
            ok = Data.borrar1RedWiFi(ssid.c_str());
            if (ok)
                Data.redesGuardadas = Data.leerTodasLasRedesGuardadas();
        }
    }

    buildMemHeader("wifi", ok ? ERROR_NINGUNO : 1);
    serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    LOG("\r\nwriteMemWifi. op: " + op + " ok: " + String(ok));
}

/**
 * Registra o borra controles RF.
 *
 * add: por cada grupo {"id_ct":"A","sig":[...]} resuelve el nombre del modelo
 *      desde Data.CtrlModelos y llama guardarDspRFFull() para cada señal.
 *      El número de control se asigna automáticamente con espacioVacioDspRF().
 *
 * del: borra por número de control (campo "num" del read rf).
 */
void GestorCMD::writeMemRF(void)
{
    String op = docJson["op"] | "";
    bool ok = true;

    if (op == "add")
    {
        JsonArray grupos = docJson["datos"].as<JsonArray>();
        for (JsonObject grupo : grupos)
        {
            String id_ct_str = grupo["id_ct"] | "";
            if (id_ct_str.length() == 0)
            {
                ok = false;
                continue;
            }
            char id_ct = id_ct_str[0];

            const char *nombre = nullptr;
            for (const auto &modelo : Data.CtrlModelos)
            {
                if (modelo.id_cnf_ctrl == id_ct)
                {
                    nombre = modelo.nombre;
                    break;
                }
            }
            if (nombre == nullptr)
            {
                LOG("\r\nwriteMemRF: id_ct no encontrado: " + id_ct_str);
                ok = false;
                continue;
            }

            JsonArray sigs = grupo["sig"].as<JsonArray>();
            for (JsonVariant v : sigs)
            {
                unsigned long signal = v.as<unsigned long>();
                uint16_t num = Data.espacioVacioDspRF();
                if (!Data.guardarDspRFFull(nombre, num, 1, signal))
                    ok = false;
            }
        }
    }
    else if (op == "del")
    {
        JsonArray nums = docJson["datos"].as<JsonArray>();
        for (JsonVariant v : nums)
        {
            uint16_t num = v.as<uint16_t>();
            if (!Data.borrar1DspRFAV(num))
                ok = false;
        }
    }
    else
    {
        ok = false;
    }

    buildMemHeader("rf", ok ? ERROR_NINGUNO : 1);
    serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    LOG("\r\nwriteMemRF. op: " + op + " ok: " + String(ok));
}

// ---------------------------------------------------------------------------
// Dispatcher principal
// ---------------------------------------------------------------------------

void GestorCMD::processMemJson(void)
{
    String accion = docJson["accion"] | "";
    String target = docJson["target"] | "";

    LOG("\r\n\r\nprocessMemJson. accion: " + accion + " target: " + target);

    if (accion == "" || target == "")
    {
        buildMemHeader(target.c_str(), ERROR_FORMATO_INVALIDO);
        serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
        rspJson = true;
        return;
    }

    if (accion == "read")
    {
        if (target == "config")
            readMemConfig();
        else if (target == "wifi")
            readMemWifi();
        else if (target == "rf")
            readMemRF();
        else
        {
            buildMemHeader(target.c_str(), ERROR_PARAMETROS_INVALIDOS);
            serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
        }
    }
    else if (accion == "write")
    {
        if (target == "config")
            writeMemConfig();
        else if (target == "wifi")
            writeMemWifi();
        else if (target == "rf")
            writeMemRF();
        else
        {
            buildMemHeader(target.c_str(), ERROR_PARAMETROS_INVALIDOS);
            serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
        }
    }
    else
    {
        buildMemHeader(target.c_str(), ERROR_PARAMETROS_INVALIDOS);
        serializeJson(docJson, jsonBuffer, sizeof(jsonBuffer));
    }

    rspJson = true;
}
