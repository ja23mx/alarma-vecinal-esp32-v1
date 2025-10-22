

void crearTareaGestionSalida(void);
void tareaGestionSalida(void *pvParameters);

void revTimerSalida(void);
void cnfSalidas(void);

void async_gestion_salida(void);

void reconfigurar_pin_para_fade(uint8_t salida_idx);
void setup_led_fade(void);
void rev_pin_fade(uint8_t salida_idx);
void cambio_fade(uint8_t salida_idx);
void config_pin_fade(uint8_t salida_idx, uint8_t nivel_fade, bool flaco_fade);
bool init_pin_fade(uint8_t salida_idx, unsigned long fade_time, bool flanco_fade);
void apagar_salida_fade(uint8_t salida_idx);