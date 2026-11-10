#ifndef CREDENCIALES_DEV_H
#define CREDENCIALES_DEV_H

/**
 * Credenciales WiFi SOLO PARA DESARROLLO (PROG_LOCAL=1 y red por defecto tras reset).
 *
 * Este archivo se versiona SIN credenciales. Las reales van en
 * include/CredencialesDev.local.h (ignorado por git), con este formato:
 *
 *   #define DEV_WIFI_SSID "mi-red"
 *   #define DEV_WIFI_PASS "mi-clave"
 *
 * Sin el archivo local, las macros quedan vacias: PROG_LOCAL=1 no conecta y
 * el reset de datos no guarda ninguna red por defecto.
 */
#if __has_include("CredencialesDev.local.h")
#include "CredencialesDev.local.h"
#endif

#ifndef DEV_WIFI_SSID
#define DEV_WIFI_SSID ""
#endif
#ifndef DEV_WIFI_PASS
#define DEV_WIFI_PASS ""
#endif

#endif // CREDENCIALES_DEV_H
