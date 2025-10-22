#ifndef ERRORES_MQTT_H
#define ERRORES_MQTT_H

// Códigos de error
const uint8_t ERROR_NINGUNO = 0;
const uint8_t ERROR_FORMATO_INVALIDO = 1;
const uint8_t ERROR_MODELO_NO_COINCIDE = 2;
const uint8_t ERROR_NUMERO_SERIE_NO_COINCIDE = 3;
const uint8_t ERROR_COMANDO_INVALIDO = 4;
const uint8_t ERROR_COMANDO_NO_ENCONTRADO = 5;
const uint8_t ERROR_SALIDA_ERRONEA = 6;
const uint8_t ERROR_PARAMETROS_INVALIDOS = 7;
const uint8_t WARNING_PERIFERICO_OCUPADO = 8;
const uint8_t ERROR_INTERNO = 9;

// Mensajes de error
const char *const MSG_COMANDO_VALIDO = "Comando valido";
const char *const MSG_FORMATO_INVALIDO = "Formato invalido";
const char *const MSG_MODELO_NO_COINCIDE = "Modelo no coincide";
const char *const MSG_NUMERO_SERIE_NO_COINCIDE = "Numero de serie no coincide";
const char *const MSG_COMANDO_INVALIDO = "Comando formato invalido";
const char *const MSG_COMANDO_NO_ENCONTRADO = "Comando invalido o no encontrado";
const char *const MSG_SALIDA_ERRONEA = "Salida fuera de rango";
const char *const MSG_PARAMETROS_INVALIDOS = "Parametros invalidos";
const char *const MSG_PERIFERICO_OCUPADO = "Periferico ocupado";
const char *const MSG_ERROR_INTERNO = "Error interno del sistema";

#endif // ERRORES_MQTT_H