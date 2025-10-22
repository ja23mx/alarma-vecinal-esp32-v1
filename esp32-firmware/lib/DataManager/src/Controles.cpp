#include "DataManager.h"
#include "LogSistema.h"

#include "Constantes.h"
#include "ConfigSistema.h"
#include "VariablesGlobales.h"
#include <conf_init_ctrl.h>
#include <algorithm>
#include <vector>
#include <string>

bool DataManager::beginCnfCtrl(void)
{

    // rstDataCnfCtrl();

    if (!cargarModelosRF(true))
    {
        LOG("\r\nError al cargar las configuraciones de control.");
        return false;
    }

    // Llamar al método guardarControl
    // guardarDspRFFull("CTAV1", 1, 1, 13255936);
    // guardarDspRFFull("EV1527_M2", 23, 2, 10569896);

    if (!cargarDatosDspRF())
    {
        LOG("\r\nError al cargar los datos RF.");
        return false;
    }

    /* for (const auto &control : CtrlModelos)
    {
        // Crear un objeto JSON para cada modelo 3434
        JsonObject modeloObj = datosArray.createNestedObject();
        modeloObj["modelo"] = control.nombre; // Nombre del modelo
    }*/

    return true;
}

bool DataManager::rstDataCnfCtrl(void)
{
    if (!limpiarNamespace(NAME_SPACE_CTRL_CNF))
    {
        LOG("\r\nError al limpiar el espacio de config_control.");
        return false;
    }

    if (!limpiarNamespace(NAME_SPACE_CTRL_DATA))
    {
        LOG("\r\nError al limpiar el espacio de data_control.");
        return false;
    }

    LOGF("\r\n\r\nNamespace: %s limpieza ok", NAME_SPACE_CTRL_CNF);
    LOGF("\r\n\r\nNamespace: %s limpieza ok", NAME_SPACE_CTRL_DATA);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // CONTROLES AV
    if (!guardarModeloCtrl(json_EV1527_CTRL_BL_AV1))
    {
        LOG("\r\nError al guardar: json_EV1527_CTRL_BL_AV1");
        return false;
    }
    LOG("\r\nConfiguracion ok: json_EV1527_CTRL_BL_AV1");
    if (!guardarModeloCtrl(json_EV1527_CTRL_BL_AV2))
    {
        LOG("\r\nError al guardar: json_EV1527_CTRL_BL_AV2");
        return false;
    }
    LOG("\r\nConfiguracion ok: json_EV1527_CTRL_BL_AV2");
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // CONTROLES GUARDIAN
    if (!guardarModeloCtrl(json_EV1527_CTRL_BL_GUARDIAN))
    {
        LOG("\r\nError al guardar la configuracion del control 1");
        return false;
    }
    LOG("\r\nConfiguracion ok: json_EV1527_CTRL_BL_GUARDIAN");
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // CONTROL INTEGRADOR
    if (!guardarModeloCtrl(json_EV1527_CTRL_BL_INT))
    {
        LOG("\r\nError al guardar la configuracion del control 3");
        return false;
    }
    LOG("\r\nConfiguracion ok: json_EV1527_CTRL_BL_INT");
    ///////////////////////////////////////////////////////////////////////////////////////////////

    return true;
}

bool DataManager::cargarModelosRF(bool log)
{
    if (!nvsData.begin(NAME_SPACE_CTRL_CNF, true))
    {
        controlesCnfFlag = false; // Indicar que no se cargaron las configuraciones
        LOG("\r\nError al abrir el espacio de config_control.");
        return false;
    }

    String config_list;
    size_t count = 0;
    loadConfigList(&config_list, count);

    if (count == 0)
    {
        controlesCnfFlag = false; // Indicar que no se cargaron las configuraciones
        // LOG("\r\nNo hay configuraciones de control guardadas.");
        nvsData.end();
        return false;
    }

    // Limpiar el vector de controles cargados antes de llenarlo
    CtrlModelos.clear();

    // Separar las configuraciones de la lista y cargarlas
    int start = 0;
    int end = config_list.indexOf(',');

    LOG("\r\nCargando configuraciones:\r\n");
    LOG(config_list);

    while (end != -1)
    {
        String tipo = config_list.substring(start, end);

        // Crear una estructura CTRL_MODELO_AV para almacenar la configuración
        // la estructura es limpiada al ser creada
        CTRL_MODELO_AV control = {};

        // Cargar la configuración desde la NVS
        if (cargar1ModeloControl(tipo.c_str(), control))
        {
            // Agregar el control cargado al vector
            CtrlModelos.push_back(control);
            if (log)
            {
                LOG("\r\nModelo Dsp RF: " + tipo + " cargada\r\n");
                if (String(tipo) == "CTINT1")
                    infoCtrlModeloLog(control); // Imprimir la información del control
            }
        }
        else
            LOG("\r\nError al cargar la configuración: " + tipo);

        start = end + 1;
        end = config_list.indexOf(',', start);
    }

    nvsData.end();
    return true;
}

void DataManager::loadConfigList(String *config_list, size_t &count)
{
    // Verificar si la clave existe en NVS
    if (!nvsData.isKey(config_list_key)) // Si no existe la clave, retornar un string vacío
    {
        *config_list = "";
        count = 0;
        return;
    }

    // Obtener el valor de la clave
    *config_list = nvsData.getString(config_list_key, ""); // Obtener la lista de configuraciones
    count = 0;                                             //
    for (int i = 0; i < config_list->length(); i++)        // Contar el número de configuraciones
    {
        if ((*config_list)[i] == ',')
            count++;
    }
}

void DataManager::updateConfigList(const String &tipo)
{
    String config_list;
    size_t count = 0;

    loadConfigList(&config_list, count);

    if (config_list.indexOf(tipo) == -1) // Cambiado de 'contains' a 'indexOf'
    {
        config_list += tipo + ",";
        nvsData.putString(config_list_key, config_list);
    }

    // LOG("\r\nConfig_list actualizada: " + config_list);
}

bool DataManager::guardarModeloCtrl(const char *json)
{
    // Primero, cargar todas las configuraciones para verificar si ya existe el ID
    bool modelos_existentes = cargarModelosRF(0); // No imprimir información

    // Reserva espacio para el deserializador
    StaticJsonDocument<1024> doc;

    // Carga el JSON desde PROGMEM
    DeserializationError error = deserializeJson(doc, json);

    // Serial.print(F("\r\n\r\nTamaño del JSON: "));
    // Serial.println(strlen(json));

    // Verifica si hubo errores
    if (error)
    {
        LOG("\r\n\r\ndeserializeJson() failed: ");
        LOG(error.f_str());
        return false;
    }

    char id_cnf_ctrl = 'x';

    if (modelos_existentes == 0) // Si no hay modelos cargados
    {
        id_cnf_ctrl = 'A';
    }
    else // asignar un nuevo ID
    {
        char nuevoID = 'A';
        bool nuevoIDEncontrado = false;
        while (!nuevoIDEncontrado)               // Buscar un nuevo ID disponible
        {                                        //
            nuevoIDEncontrado = true;            // Suponer que el nuevo ID es válido
            for (const auto &ctrl : CtrlModelos) // Recorrer el vector CtrlModelos
            {                                    //
                if (ctrl.id_cnf_ctrl == nuevoID) // Si el ID ya existe
                {                                //
                    nuevoID++;                   // Incrementar el ID
                    nuevoIDEncontrado = false;
                    break;
                }
            }
            if (nuevoID == 'Z')
            {
                LOG("\r\n\r\nNo hay espacio para mas controles.");
                return false;
            }
        }
        id_cnf_ctrl = nuevoID;
    }

    if (!nvsData.begin(NAME_SPACE_CTRL_CNF, false))
    {
        LOG("\r\nError al abrir el espacio de config_control. guardarModeloCtrl");
        return false;
    }

    doc["id_ct"] = String(id_cnf_ctrl);
    const char *modelo = doc["modelo"];
    String modelo_str = String(modelo);

    // Convierte el documento JSON modificado a una cadena
    String output;
    serializeJson(doc, output);

    // Usar un prefijo más corto para evitar la longitud máxima de clave
    String prefix = modelo_str + "_";

    nvsData.putString((prefix + "nm").c_str(), output);

    // Imprime el JSON string
    /* Serial.print("\r\n\r\nJSON modificado: ");
    Serial.println(output); */

    // LOG("\r\ninit updateConfigList");
    updateConfigList(modelo_str);

    output.clear();
    nvsData.end();
    doc.clear();

    // Limpiar el documento después de usarlo
    // LOG("\r\nConfiguracion de control guardada correctamente.");
    LOG("\r\n fin guardado");
    revModeloGuardado(modelo_str.c_str());

    modelo_str.clear();
    return true;
}

bool DataManager::revModeloGuardado(const char *modelo)
{

    CTRL_MODELO_AV control = {}; // limpia la estructura
    if (cargar1ModeloControl(modelo, control))
    {
        infoCtrlModeloLog(control); // Imprimir la información del control
        return true;
    }

    LOG("\r\nError al cargar la informacion. " + String(modelo));

    return false;
}

void DataManager::infoCtrlModeloLog(CTRL_MODELO_AV &control)
{
    LOG("\r\n\r\n\r\n************************");
    LOG("\r\n\r\ninfoCtrlModeloLog: \r\nHEADER:");
    LOGF("\r\nModelo: %s", control.modelo);
    LOGF("\r\nNombre: %s", control.nombre);
    LOGF("\r\nCodificador: %s", control.codificador);
    LOGF("\r\nConfiguracion: %d", control.configuracion);
    LOGF("\r\nID: %c", control.id_cnf_ctrl);

    LOG("\r\n*********\r\nConfig:");
    LOG("\r\nbits_limpieza: " + String(control.bits_limpieza));
    LOGF("\r\nNumero de botones: %d", control.numero_botones);
    LOGF("\r\nBoton desactivacion: %d", control.boton_desactivacion);

    LOG("\r\nsuma botones: ");
    for (size_t i = 0; i < control.numero_botones; i++)
    {
        if (control.suma_botones[i] != 0)
            LOG(String(control.suma_botones[i]) + ". ");
    }

    LOG("\r\nvalor botones: ");
    for (size_t i = 0; i < control.numero_botones; i++)
    {
        if (control.valor_botones[i] != 0)
            LOG(String(control.valor_botones[i]) + ". ");
    }

    LOG("\r\nTipo de botones: ");
    for (size_t i = 0; i < control.numero_botones; i++)
    {
        if (control.tipo_botones[i] != 0)
            LOG(String(control.tipo_botones[i]) + ". ");
    }
    LOG("\r\n*********\r\n");

    if (control.configuracion == CTRL_CNF_AV)
    {
        LOG("\r\n\r\nCONTROL ALARMA VECINAL");

        LOGF("\r\nTamano pistas ciclo AV: %d", control.av_tamano_ciclo_btn_av);

        LOG("\r\nLista de ciclo AV: ");
        for (size_t i = 0; i < control.av_tamano_ciclo_btn_av; i++)
            LOG(String(control.av_lista_ciclo_btn_av[i]) + ". ");

        LOGF("\r\nInicio de pista de usuario AV: %d", control.av_pst_us_init);

        LOG("\r\nLista de pistas varibles de ciclo AV:\r\n");
        for (size_t i = 0; i < MAX_BOTONES; i++)
        {
            if (control.av_lista_pst_variable[i] != 0)
                LOG(String(control.av_lista_pst_variable[i]) + ". ");
        }

        LOG("\r\nLista de una pista AV:\r\n");
        for (size_t i = 0; i < control.numero_botones; i++)
        {
            if (control.av_una_pista_lista[i] != 0)
                LOG(String(control.av_una_pista_lista[i]) + ". ");
        }
        LOG("\r\n*********\r\n");
    }
    else if (control.configuracion == CTRL_CNF_ALERTA_SON_MSG)
    {
        LOG("\r\n\r\nCONTROL ALERTA SONORA Y MENSAJE");
        LOG("\r\nLista de pistas Alerta Sonora y Msg:\r\n");
        for (size_t i = 0; i < control.numero_botones; i++)
            LOG(String(control.as_lista_pst_gral[i]) + ". ");
        LOG("\r\n*********\r\n");
    }
    else if (control.configuracion == CTRL_CNF_INTEGRADOR)
    {
        LOG("\r\n\r\nCONTROL INTEGRADOR");
        LOG("\r\nLista de una pista:\r\n");
        for (size_t i = 0; i < control.numero_botones; i++)
        {
            if (control.int_una_pista_lista[i] != 0)
                LOG(String(control.int_una_pista_lista[i]) + ". ");
        }
        LOGF("\r\nInicio de pista de programacion: %d", control.int_ini_prog_pista);
        LOGF("\r\nFin de pista de programacion: %d", control.int_fin_prog_pista);
        LOGF("\r\nTamper off pista: %d", control.int_tamper_off_pista);
        LOGF("\r\nTamper on pista: %d", control.int_tamper_on_pista);
        LOGF("\r\nReset pista: %d", control.int_reset_pista);
        LOG("\r\n*********\r\n");
    }
}

bool DataManager::cargar1ModeloControl(const char *tipo, CTRL_MODELO_AV &control)
{

    nvsData.begin(NAME_SPACE_CTRL_CNF, true);

    // Construir el prefijo para las claves
    String prefix = String(tipo) + "_nm";

    // LOG("\r\n\r\nCargando configuracion de control:\r\n" + prefix);

    // Obtener el JSON almacenado
    String json = nvsData.getString(prefix.c_str(), "");
    nvsData.end();

    // LOG("\r\n\r\nJSON: " + json);

    if (json.isEmpty())
    {
        LOG("\r\nNo se encontró la configuración para el tipo: " + String(tipo));
        return false;
    }

    // Deserializar el JSON
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error)
    {
        LOG("\r\nError al deserializar el JSON: ");
        LOG(error.f_str());
        return false;
    }

    // Asignar los valores del JSON a la estructura CTRL_MODELO_AV
    // valores obligatorios ///////////////////////////////
    // modelo, por ejemplo: "EVAV4TM1"
    strlcpy(control.modelo, doc["modelo"] | "", sizeof(control.modelo));
    // nombre, por ejemplo: "CTRL BLANCO 4T AV M1"
    strlcpy(control.nombre, doc["nombre"] | "", sizeof(control.nombre));
    // codificador, por ejemplo: "EV1527"
    strlcpy(control.codificador, doc["codificador"] | "", sizeof(control.codificador));

    control.configuracion = doc["conf"] | 0;          // configuracion del modelo de dispositivo RF
    control.id_cnf_ctrl = (doc["id_ct"] | " ")[0];    // id de configuracion de control
    control.bits_limpieza = doc["bt_lm"] | 0;         // bits de limpieza
    control.numero_botones = doc["nm_btn"] | 0;       // numero de botones
    control.boton_desactivacion = doc["btn_des"] | 0; // boton de desactivacion

    // Asignar arrays
    JsonArray sm_bt = doc["sm_bt"].as<JsonArray>();   // suma botones
    JsonArray vl_btn = doc["vl_btn"].as<JsonArray>(); // valor botones
    JsonArray tp_btn = doc["tp_btn"].as<JsonArray>(); // tipo botones

    for (size_t i = 0; i < sm_bt.size() && i < MAX_BOTONES; i++)
        control.suma_botones[i] = sm_bt[i] | 0;

    for (size_t i = 0; i < vl_btn.size() && i < MAX_BOTONES; i++)
    {
        String temp = vl_btn[i].as<String>();            // Convertir a String
        control.valor_botones[i] = strdup(temp.c_str()); // Duplicar la cadena como char*
    }

    for (size_t i = 0; i < tp_btn.size() && i < MAX_BOTONES; i++)
        control.tipo_botones[i] = tp_btn[i] | 0;

    // fin valores obligatorios ///////////////////////////

    String key;

    switch (control.configuracion)
    {
    // Dispositivo de Alarma Vecinal
    case CTRL_CNF_AV:
    {
        // Asignar valores específicos de alarma vecinal
        control.av_tamano_ciclo_btn_av = doc["av_tamano_ciclo_btn_av"] | 0; // numero de pistas

        if (control.av_tamano_ciclo_btn_av > 0) //
        {
            JsonArray av_lista_ciclo_btn_av = doc["av_lista_ciclo_btn_av"].as<JsonArray>(); // lista de pistas
            for (size_t i = 0; i < av_lista_ciclo_btn_av.size() && i < MAX_BOTONES; i++)    //
                control.av_lista_ciclo_btn_av[i] = av_lista_ciclo_btn_av[i] | 0;            //
        }

        control.av_pst_us_init = doc["av_pst_us_init"] | 0; // init pista de usuario

        key = "av_lista_pst_variable";
        if (doc.containsKey(key))
        {
            // LOG("\r\n\r\nConfiguracion de control ok. Cargando lista de pistas de alarma vecinal.\r\n");
            JsonArray av_pst_ind = doc[key].as<JsonArray>();
            for (size_t i = 0; i < av_pst_ind.size() && i < MAX_BOTONES; i++)
            {
                control.av_lista_pst_variable[i] = av_pst_ind[i] | 0;
                // LOGF("%d. ", control.av_lista_pst_variable[i]);
            }
        }

        key = "av_una_pista_lista";
        if (doc.containsKey(key))
        {
            JsonArray av_una_pista_lista = doc[key].as<JsonArray>();
            for (size_t j = 0; j < av_una_pista_lista.size() && j < MAX_BOTONES; j++)
                control.av_una_pista_lista[j] = av_una_pista_lista[j] | 0;
        }
        break;
    } //  fin Dispositivo de Alarma Vecinal

    // Dispositivo de Alerta Sonora y Mensaje
    case CTRL_CNF_ALERTA_SON_MSG:
    {
        key = "as_lista_pst_gral";
        if (doc.containsKey(key))
        {
            // lista de las pistas a reproducir, si esta en 0 significa que solo
            // manda mensaje
            JsonArray as_lista_pst_gral = doc[key].as<JsonArray>();
            for (size_t j = 0; j < as_lista_pst_gral.size() && j < MAX_BOTONES; j++)
                control.as_lista_pst_gral[j] = as_lista_pst_gral[j] | 0;
        }
        break;
    }
    // Dispositivo de Alerta Sonora y Mensaje
    case CTRL_CNF_INTEGRADOR:
    {
        key = "int_una_pista_lista";
        if (doc.containsKey(key))
        {
            // lista de las pistas a reproducir, si esta en 0 significa que solo
            // manda mensaje
            JsonArray int_una_pista_lista = doc[key].as<JsonArray>();
            for (size_t j = 0; j < int_una_pista_lista.size() && j < MAX_BOTONES; j++)
                control.int_una_pista_lista[j] = int_una_pista_lista[j] | 0;
        }

        control.int_ini_prog_pista = doc["int_ini_prog_pista"] | 0;     // init programacion
        control.int_fin_prog_pista = doc["int_fin_prog_pista"] | 0;     // fin programacion
        control.int_tamper_off_pista = doc["int_tamper_off_pista"] | 0; // tamper off
        control.int_tamper_on_pista = doc["int_tamper_on_pista"] | 0;   // tamper on
        control.int_reset_pista = doc["int_reset_pista"] | 0;           // reset
        break;
    }
    }

    nvsData.end(); // Liberar la memoria utilizada por el documento JSON
    doc.clear();   // Limpiar el documento después de usarlo

    return true;
}

bool DataManager::existeDspRFRAM(uint16_t num_ctrl)
{
    // Verificar si el número de control está dentro del rango válido
    if (num_ctrl == 0 || num_ctrl > CTRL_LIMITE_CTRL)
    {
        LOGF("\r\nError: El número de control AV debe estar entre 1 y %d.", CTRL_LIMITE_CTRL);
        return false;
    }

    // Recorrer el vector controlValues para buscar el número de control
    for (const auto &controlValue : controlValues)
    {
        if (std::get<1>(controlValue) == num_ctrl) // Comparar el segundo elemento de la tupla
        {
            return true; // Control encontrado
        }
    }

    return false; // Control no encontrado
}

bool DataManager::borrar1DspRFAV(uint16_t num_ctrl)
{

    uint16_t limite = CTRL_LIMITE_CTRL; // Obtener el número de control del parámetro
    if (num_ctrl == 0 || num_ctrl > CTRL_LIMITE_CTRL)
    {
        LOGF("\r\nError: El número de control debe estar entre 1 y %d.", limite);
        return false;
    }

    // Variable para almacenar el iterador del elemento encontrado
    auto it = controlValues.end();

    // Recorrer el vector controlValues para buscar el número de control
    for (auto iter = controlValues.begin(); iter != controlValues.end(); ++iter)
    {
        if (std::get<1>(*iter) == num_ctrl)
        {              // Comparar el segundo elemento de la tupla
            it = iter; // Guardar el iterador del elemento encontrado
            break;     // Salir del bucle
        }
    }

    // Si no se encuentra el control, registrar un mensaje y retornar false
    if (it == controlValues.end())
    {
        LOGF("\r\nControl con numero %d no encontrado en controlValues.", num_ctrl);
        return false;
    }

    // Eliminar el control del vector
    controlValues.erase(it);
    LOGF("\r\nControl con numero %d eliminado del vector controlValues.", num_ctrl);

    // Crear la clave para NVS
    char key[10];
    snprintf(key, sizeof(key), "DSRF%03d", num_ctrl);

    // Verificar si la clave existe en la NVS
    if (!nvsData.begin(NAME_SPACE_CTRL_DATA, false))
    {
        LOG("\r\nError al abrir el espacio de config_control en NVS.");
        return false;
    }

    removeKey(key); // Eliminar la clave de la lista de claves

    if (nvsData.isKey(key))
    {
        // Eliminar la clave de la NVS
        nvsData.remove(key);
        LOGF("\r\nClave %s eliminada de la NVS.", key);
    }
    else
    {
        LOGF("\r\nClave %s no encontrada en la NVS.", key);
    }

    nvsData.end();
    return true;
}

bool DataManager::guardarDspRFLoop(uint8_t status, uint16_t num_ctrl, const char *nombre)
{

    LOG("\r\n\r\nGuardando control RF del loop...");
    LOGF("\r\nname config: %s", nombre);
    LOGF("\r\ncontrol number: %d", num_ctrl);
    LOGF("\r\nstatus: %d", status);
    LOGF("\r\nsignal: %lu", ctrlRfLoopSignal);

    if (guardarDspRFFull(nombre, num_ctrl, status, ctrlRfLoopSignal))
    {
        LOG("\r\n\r\nControl RF Loop guardado correctamente");
        return true;
    }
    else
    {
        LOG("\r\n\r\nError al guardar el control RF en el loop.");
        return false;
    }
}

bool DataManager::guardarDspRFFull(const char *nombre, uint16_t num_ctrl, uint8_t status, unsigned long signal)
{
    // Buscar el control correspondiente en el vector CtrlModelos
    CTRL_MODELO_AV *controlEncontrado = nullptr;
    for (auto &control : CtrlModelos)
    {
        if (strcmp(control.nombre, nombre) == 0)
        {
            controlEncontrado = &control;
            break;
        }
    }

    // Si no se encuentra el control, retornar false
    if (controlEncontrado == nullptr)
    {
        LOGF("\r\n\r\nControl name no encontrado: %s", nombre);
        return false;
    }

    // LOG("\r\n\r\nConfiguracion de control ok");
    // LOGF("\r\nNombre: %s", controlEncontrado->name);
    //  LOG("\r\nbits_limpieza: " + String(controlEncontrado->bits_limpieza));
    //   Paso 1: Obtener la máscara de bits de limpieza
    unsigned long bitsLimpieza = (1UL << controlEncontrado->bits_limpieza) - 1;
    /* Serial.print("\r\nBits de limpieza: ");
    Serial.print(bitsLimpieza, BIN); */

    // Paso 2: Aplicar operación AND de bits para limpiar los bits especificados
    unsigned long signalProcesada = signal & ~bitsLimpieza;
    /* Serial.print("\r\nSignal original: ");
    Serial.print(signal, BIN);
    Serial.print("\r\nSignal procesada: ");
    Serial.print(signalProcesada, BIN); */

    // Crear la string de configuración
    char configuracion[30];
    snprintf(configuracion, sizeof(configuracion), "%c,%d,%lu,", controlEncontrado->id_cnf_ctrl, status, signalProcesada);

    // Paso 3: Guardar la configuración en el vector de controlValues
    // Verificar si el control ya existe en el vector
    controlValues.push_back(std::make_tuple(controlEncontrado->id_cnf_ctrl, num_ctrl, status, signalProcesada));

    // Guardar la configuración en la NVS
    // Aquí se implementaría la lógica para guardar la configuración en la NVS
    // Ejemplo: nvsData.putString(key, configuracion);

    // Para este ejemplo, simplemente imprimimos la configuración
    // LOG("\r\n\r\nNVS DATA: ");
    // LOG(configuracion);

    // Crear la clave para NVS
    char key[10];
    snprintf(key, sizeof(key), "DSRF%03d", num_ctrl);

    // Abrir el namespace en modo de escritura (false)
    if (!nvsData.begin(NAME_SPACE_CTRL_DATA, false))
    {
        LOG("\r\nError al abrir el espacio de config_control. guardarControl");
        return false;
    }

    LOGF("\r\n\r\n\r\nKEY: %s. data: %s\r\n\r\n\r\n", key, configuracion);

    // Guardar la configuración en la NVS
    nvsData.putString(key, configuracion);
    addKey(key);
    nvsData.end();

    if (controlesCargadosSize < CTRL_LIMITE_CTRL)
        controlesCargadosSize++; // Incrementar el tamaño de controles cargados
    else
        LOG("\r\n\r\nLimite de controles alcanzado. No se puede guardar mas controles.");

    // Imprimir los controles guardados
    LOG("\r\nControles guardados:");
    for (const auto &controlValue : controlValues)
    {
        char id_cnf_ctrl = std::get<0>(controlValue);
        uint16_t controlNumber = std::get<1>(controlValue);
        uint8_t controlStatus = std::get<2>(controlValue);
        unsigned long baseSignal = std::get<3>(controlValue);

        LOGF("\r\nID Config Control: %c", id_cnf_ctrl);
        LOGF("\r\nControl Number: %d", controlNumber);
        LOGF("\r\nControl Status: %d", controlStatus);
        LOGF("\r\nBase Signal: %lu", baseSignal);
    }

    // LOG("\r\n\r\nCONTROL GUARDADO CORRECTAMENTE");
    // LOGF("\r\nname config: %s", controlEncontrado->tipo);
    // LOG("\r\nkey: " + String(key) + ". data: " + String(configuracion));

    // Retornar true si la operación fue exitosa, false de lo contrario
    return true;
}

bool DataManager::cargarDatosDspRF(void)
{
    String keys;

    // Abrir el namespace en modo de lectura (true)
    if (!nvsData.begin(NAME_SPACE_CTRL_DATA, true))
    {
        LOG("\r\nError al abrir el espacio de config_control.");
        return false;
    }

    controlValues.clear();     // Limpiar el vector de configuraciones de RF
    cargandoCtrlRF = true;     // Indicar que se está cargando la configuración de RF
    controlesCargadosSize = 0; // Inicializar el tamaño de controles cargados
    controlesNVSok = false;    // Indicar que no se han cargado los controles

    unsigned long time = millis();

    // Obtener todas las claves almacenadas
    if (nvsData.isKey("keys"))
    {
        keys = nvsData.getString("keys", "");
        LOG("\r\nClave 'keys' encontrada: " + keys);
    }
    else
    {
        cargandoCtrlRF = false; // Indicar que se acabo la carga de los controles
        controlesNVSok = true;  // Indicar que se han cargado los controles
        LOG("\r\n\r\nClave 'keys' error. SIN CONTROLES CARGADOS");
        nvsData.end();
        return true;
    }

    // Separar las claves y cargar los valores correspondientes
    int start = 0;
    int end = keys.indexOf(',');

    LOG("\r\n\r\n\r\nCargando datos de RF...");

    while (end != -1)
    {
        String key = keys.substring(start, end);
        LOG("\r\n\r\nkey: " + key);
        if (key.length() > 0)
        {
            // Extraer el número de control del key
            uint16_t controlNumber = key.substring(4).toInt();

            String value = nvsData.getString(key.c_str(), "");
            LOGF("\r\n\r\nkey: %s. value: %s", key.c_str(), value.c_str());
            if (value.length() > 0)
            {
                controlesCargadosSize++; // Incrementar el tamaño de controles cargados
                char id_cnf_ctrl;
                uint8_t status;
                unsigned long signal;
                sscanf(value.c_str(), "%c,%d,%lu,", &id_cnf_ctrl, &status, &signal);                  // Leer la configuración de control
                controlValues.push_back(std::make_tuple(id_cnf_ctrl, controlNumber, status, signal)); // Añadir la configuración al vector
                LOGF("\r\nCtrl Num: %d: %lu. Tp: %c", controlNumber, signal, id_cnf_ctrl);            // Imprimir la configuración
            }
        }
        start = end + 1;
        end = keys.indexOf(',', start);
    }

    nvsData.end();

    timerCargaCtrlRF = millis() - time;

    LOGF("\r\nControles tot: %d. \r\nTiempo: %d ms", controlesCargadosSize, timerCargaCtrlRF);

    cargandoCtrlRF = false; // Indicar que se acabo la carga de los controles
    controlesNVSok = true;  // Indicar que se han cargado los controles
    return true;
}

bool DataManager::busqDspRF(unsigned long senalRF)
{

    estadoCompRFAv.status = 0;
    estadoCompRFAv.control = 0;
    estadoCompRFAv.btnIndice = 0;
    estadoCompRFAv.valido = 0;

    if (controlesCargadosSize == 0)
    {
        LOG("\r\n\r\nNo hay controles cargados.");
        return false;
    }

    /* LOG("\r\n\r\n\r\nbusqDspRF\r\nControles guardados:\r\n");
    for (const auto &controlValue : controlValues)
    {
        char id_cnf_ctrl = std::get<0>(controlValue);
        uint16_t controlNumber = std::get<1>(controlValue);
        uint8_t controlStatus = std::get<2>(controlValue);
        unsigned long baseSignal = std::get<3>(controlValue);

        LOGF("\r\nID Config Control: %c", id_cnf_ctrl);
        LOGF("\r\nControl Number: %d", controlNumber);
        LOGF("\r\nBase Signal: %lu", baseSignal);
    }

    LOG("\r\n\r\n\r\nControles cargados:\r\n");
    for (const auto &control : CtrlModelos)
    {
        LOGF("\r\n\r\nModelo: %s", control.modelo);
        LOGF("\r\nNombre: %s", control.nombre);
        LOGF("\r\nID Config Control: %c", control.id_cnf_ctrl);
        LOGF("\r\nConfiguracion: %d", control.configuracion);
        LOGF("\r\nNumero de botones: %d", control.numero_botones);
        LOGF("\r\nBits de limpieza: %d", control.bits_limpieza);
        LOGF("\r\nBoton de desactivacion: %d", control.boton_desactivacion);
        LOG("\r\nSuma botones: ");
        for (int i = 0; i < control.numero_botones; i++)
        {
            LOGF("%lu ", control.suma_botones[i]);
        }
    }

    LOG("\r\n\r\n\r\nFOR:\r\n"); */

    // Iterar sobre las configuraciones cargadas
    for (const auto &controlValue : controlValues)
    {
        char id_cnf_ctrl = std::get<0>(controlValue);
        uint16_t controlNumber = std::get<1>(controlValue);
        uint8_t controlStatus = std::get<2>(controlValue);
        unsigned long baseSignal = std::get<3>(controlValue);

        /* LOGF("\r\n\r\nID Config Control: %c", id_cnf_ctrl);
        LOGF("\r\nControl Number: %d", controlNumber);
        LOGF("\r\nControl Status: %d", controlStatus);
        LOGF("\r\nBase Signal: %lu", baseSignal); */

        // Buscar el control correspondiente en el vector CtrlModelos
        for (const auto &control : CtrlModelos)
        {
            if (control.id_cnf_ctrl == id_cnf_ctrl)
            {
                // Iterar sobre los botones del control
                for (int i = 0; i < control.numero_botones; i++)
                {
                    unsigned long signal = baseSignal + control.suma_botones[i];
                    // LOGF("\r\n\r\nComparando señal RF: %lu con %lu", senalRF, signal);
                    if (senalRF == signal)
                    {
                        modeloCtrlAVRx = control;
                        estadoCompRFAv.configuracion = control.configuracion;
                        estadoCompRFAv.status = controlStatus;
                        estadoCompRFAv.control = controlNumber;
                        strncpy(estadoCompRFAv.btnStr, control.valor_botones[i], sizeof(estadoCompRFAv.btnStr) - 1);
                        estadoCompRFAv.btnStr[sizeof(estadoCompRFAv.btnStr) - 1] = '\0';
                        estadoCompRFAv.btnIndice = i;
                        estadoCompRFAv.valido = 1;
                        return true;
                    }
                }
            }
        }
    }

    Serial.print("\r\n\r\nNo se encontro ninguna coincidencia.");
    return false;
}

void DataManager::addKey(const String &key) /// 3434
{
    // Verificar si la clave "keys" existe
    if (!nvsData.isKey("keys"))
    {
        // Serial.print("\r\n\r\nInicializando la clave 'keys'");
        nvsData.putString("keys", key + ",");
        return;
    }

    // Obtener la clave "keys"
    String currentKeys = nvsData.getString("keys", "");
    if (currentKeys.indexOf(key) == -1)
    {
        currentKeys += key + ",";
        nvsData.putString("keys", currentKeys);
    }

    nvsData.putString("keys", currentKeys);
}

void DataManager::removeKey(const String &key)
{

    // Verificar si la clave "keys" existe
    if (!nvsData.isKey("keys"))
    {
        LOG("\r\nLa lista de claves 'keys' no existe en la NVS.");
        return;
    }

    // Obtener la lista actual de claves
    String currentKeys = nvsData.getString("keys", "");

    // Verificar si la clave está en la lista
    int keyIndex = currentKeys.indexOf(key + ",");
    if (keyIndex == -1)
    {
        LOGF("\r\nLa clave '%s' no existe en la lista.", key.c_str());
        return;
    }

    // Eliminar la clave de la lista
    currentKeys.remove(keyIndex, key.length() + 1); // +1 para eliminar también la coma

    // LOGF("\r\n\r\ncurrentKeys: %s", currentKeys.c_str());

    // Actualizar la lista en la NVS
    nvsData.putString("keys", currentKeys);
    LOGF("\r\nClave '%s' eliminada de la lista.", key.c_str());

    nvsData.end(); // Cerrar el espacio de nombres
}

uint16_t DataManager::espacioVacioDspRF(void)
{
    if (controlValues.empty())
    {
        // Si no hay controles grabados, el primer espacio vacío es 1
        LOG("\r\n\r\nEspacio vacio encontrado: 1");
        return 1;
    }

    // Extraer los valores de controlNumber
    std::vector<uint16_t> controlNumbers;
    for (const auto &controlValue : controlValues)
    {
        controlNumbers.push_back(std::get<1>(controlValue));
    }

    // Ordenar los valores en orden ascendente
    std::sort(controlNumbers.begin(), controlNumbers.end());

    // Verificar si el número 1 está disponible
    if (controlNumbers.front() != 1)
    {
        LOG("\r\n\r\nEspacio vacio encontrado: 1");
        return 1;
    }

    // Buscar el primer espacio vacío en la secuencia
    for (size_t i = 0; i < controlNumbers.size() - 1; ++i)
    {
        if (controlNumbers[i] + 1 != controlNumbers[i + 1])
        {
            LOGF("\r\n\r\nEspacio vacio encontrado: %d", controlNumbers[i] + 1);
            return controlNumbers[i] + 1;
        }
    }

    // Si no hay espacios vacíos, retornar el siguiente número después del mayor
    LOGF("\r\n\r\nEspacio vacio encontrado: %d", controlNumbers.back() + 1);
    return controlNumbers.back() + 1;
}
