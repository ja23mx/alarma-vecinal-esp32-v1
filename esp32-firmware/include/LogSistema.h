#ifndef LOG_SISTEMA_H
#define LOG_SISTEMA_H

#include <Arduino.h>

#define DEBUG_MODE 1 

#ifdef DEBUG_MODE
    #define LOG(x) Serial.print(x)                          // Mensajes sin formato
    #define LOGF(fmt, ...) Serial.printf(fmt, __VA_ARGS__) // Mensajes con formato
#else
    #define LOG(x)
    #define LOGF(fmt, ...)
#endif


#endif // LOG_SISTEMA_H
