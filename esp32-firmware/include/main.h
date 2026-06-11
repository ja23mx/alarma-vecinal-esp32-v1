void blink_led_mqtt(bool revision);

void gestion_fin_ap(uint8_t origenConfig);
void manejarBotonProg();

int leerSwitchConAntirrebote(int pinADC, int numLecturas, int retardoMs);
void get_config_red(void);