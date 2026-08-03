
#include "EstructurasGlobales.h"

void crearTareaProcesosCmd(void);

void cambiar_estado_tarea_proc_cmd(uint8_t estado);

void tareaProcesosCmd(void *pvParameters);

void call_async_eventos(void);

uint8_t get_estado_alarma(void);
void reset_estado_alarma(void);

bool get_alarma_pendiente_ack(void);
void ack_alarma_pendiente(void);

void rev_async_evento_ctrl_av(void);
void async_evento_ctrl_av(uint8_t tipoCtrl, uint8_t estadoAlarma);

void rev_sync_evento_mqtt(void);
void sync_evento_mqtt(String payload);

void proc_cmd_play_pista_msg(uint8_t pista);
void async_play_pista_led_atm(uint8_t pista, uint8_t config, uint8_t cargaPeriferico);
void rev_async_pista_led_atm(void);


void sync_proc_cmd_play_pista_led_atm(uint8_t pista, uint8_t config, uint8_t cargaPeriferico);