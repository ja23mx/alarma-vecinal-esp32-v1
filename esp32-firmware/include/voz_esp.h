void async_config_mp3_cmd(void);

void mp3_init(void);
void mp3_init_comandos(void);
void mp3_play_pst_cmd(uint16_t pista);

void mp3_loop(void);

void rev_reproduccion_temporizada(void);
void loop_1_pista_cmd(void);

void mp3_pista_prueba_init(uint16_t pista, bool habilitar_amplificador);

bool mp3_reproducion_audio(uint16_t pista, bool stop_en);

void lectura_pto(void);

bool mp3_pista_play_comunic_1(uint16_t pista, bool stop_en);
bool mp3_pista_play_comunic_0(uint16_t pista, bool stop_en);

void async_ejec_mp3_play_una_pista(void);
void async_cnf_mp3_play_una_pista(uint16_t pista);

void async_ejec_mp3_play_lista(void);
void async_cnf_mp3_play_lista(const uint16_t lista[], size_t size);

void async_ejec_pista_prueba_pendiente(void);

void mp3_serial_info(uint8_t type, int value);

void mp3_amplificador_on(void);
void mp3_amplificador_off(void);
