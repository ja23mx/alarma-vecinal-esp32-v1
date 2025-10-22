#ifndef TAREA_NEOPIXEL_H
#define TAREA_NEOPIXEL_H

#include <Arduino.h>

// Declaraciones de funciones de la tarea
void crearTareaNeoPixel(void);

void cambiar_estado_neopixel(uint8_t estado);

void tareaNeoPixel(void *pvParameters);

void async_stop_carga_sistema(void);
void check_stop_carga_sistema(void);

// Funciones para control asíncrono del servidor
void async_servidor_conectado(void);
void async_servidor_desconectado(void);
void async_servidor_deshabilitar(void);
void check_servidor_status(void);

// Funciones para control asíncrono del error MP3
void async_error_mp3_activar(void);
void async_error_mp3_desactivar(void);
void check_error_mp3(void);

// Funciones para control asíncrono del LED MP3
void async_led_mp3_on(void);
void async_led_mp3_off(void);
void async_led_mp3_reproduciendo(void);
void async_led_mp3_stop_reproduciendo(int8_t estado_final);
void check_led_mp3_status(void);

// Funciones para control de LED y recepción de datos de servidor
void check_led_server_data(void);
void async_led_server_data(void);

void async_led_rf_rx(void);
void check_led_rf_rx(void);
void rf_led_rx(void);

// Función para verificar estado busy del NeoPixel
bool is_neopixel_busy(void);

// Función encapsulada para detener carga sistema y esperar
void async_stop_carga_sistema_and_wait(unsigned long timeout_ms = 5000);
#endif
