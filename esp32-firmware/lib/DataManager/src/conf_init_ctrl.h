
#ifndef _CONF_INIT_CTRL_H_
#define _CONF_INIT_CTRL_H_


/* 
    estructura de modelo de control
    obligatorio                                 
        si        modelo: nombre del modelo
        si        nombre: nombre del control (por ejemplo: "CTRL BLANCO 4T AV M1") que aparece en la interfaz web
        si        codificador: codificador del control (por ejemplo: "EV1527")
        si        conf: configuracion del modelo de dispositivo RF (1: AV con identificacion, 2: AV sin identificacion, 3: Integrador)
        si        bt_lm: bits de limpieza
        si        nm_btn: numero de botones (por lo menos un boton)
        si        btn_des: boton de desactivacion
        si        sm_bt: suma botones 
        si        vl_btn: valor botones 
        si        tp_btn: tipo botones (1: silencioso, 2: alarma vecinal, 3: una pista)
        
        no        av_pst_1_lista: lista de pistas de botones de una pista (solo si hay al menos un boton de una pista)
        no        av_nm_pst: numero de pistas de botones av (solo si hay al menos un boton de alarma vecinal)
        no        av_pst_arr: lista de pistas de botones av (solo si hay al menos un boton de alarma vecinal)
        no        av_pst_us: pista de usuario (solo si hay al menos un boton de alarma vecinal)
        no        av_lista_pst_btn_av: lista de pistas de botones de alarma vecinal (solo si hay al menos un boton de alarma vecinal)
*/

// "sm_bt":[2,8,1,4], control blanco pequeno 4T EV1527 

const char json_EV1527_CTRL_BL_AV1[] PROGMEM = R"rawliteral(
{
 "modelo":"CTAV1",
 "nombre":"C BLANCO 4T AV IDENT",
 "codificador":"EV1527",
 "conf":1,
 "bt_lm":4,
 "nm_btn":4,
 "btn_des":4,
 "sm_bt":[8,4,2,1],   
 "vl_btn":["A","B","C","D"],
 "tp_btn":[2,2,3,0],
 "av_tamano_ciclo_btn_av":2,
 "av_lista_ciclo_btn_av":[0,-1],
 "av_pst_us_init":1001,
 "av_lista_pst_variable":[50,51],
 "av_una_pista_lista":[52]
}
)rawliteral";

const char json_EV1527_CTRL_BL_AV2[] PROGMEM = R"rawliteral(
{
 "modelo":"CTAV2",
 "nombre":"C BLANCO 4T AV SIN IDENT",
 "codificador":"EV1527",
 "conf":1,
 "bt_lm":4,
 "nm_btn":4,
 "btn_des":4,
 "sm_bt":[8,4,2,1],   
 "vl_btn":["A","B","C","D"],
 "tp_btn":[2,2,2,0],
 "av_tamano_ciclo_btn_av":1,
 "av_lista_ciclo_btn_av":[0],
 "av_lista_pst_variable":[60,61,62]
}
)rawliteral";


const char json_EV1527_CTRL_BL_GUARDIAN[] PROGMEM = R"rawliteral(
{
 "modelo":"CTGD1",
 "nombre":"C BLANCO 4T GUARDIAN M1",
 "codificador":"EV1527",
 "conf":2,
 "bt_lm":4,
 "nm_btn":4,
 "btn_des":4,
 "sm_bt":[8,4,2,1],
 "vl_btn":["A","B","C","D"],
 "tp_btn":[4,5,5,6],
 "as_lista_pst_gral":[70, 71, 72, 0]
}
)rawliteral";


const char json_EV1527_CTRL_BL_INT[] PROGMEM = R"rawliteral(
{
"modelo":"CTINT1",
"nombre":"C BLANCO 4T INTEG M1",
"codificador":"EV1527",
"conf":3,
"bt_lm":4,
"nm_btn":4,
"btn_des":4,
"sm_bt":[8,4,2,1],     
"vl_btn":["A","B","C","D"],
"tp_btn":[4,7,8,9],
"int_una_pista_lista":[80],
"int_ini_prog_pista":81,
"int_fin_prog_pista":82,
"int_tamper_off_pista":83,
"int_tamper_on_pista":84,
"int_reset_pista":85
}
)rawliteral";
#endif // _CONF_INIT_CTRL_H_