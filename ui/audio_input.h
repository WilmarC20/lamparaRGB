#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_BAND_COUNT 8

typedef struct {
    float rms;
    float bands[AUDIO_BAND_COUNT];
} audio_levels_t;

void audio_input_init(void);
void audio_input_start(void);
void audio_input_stop(void);
bool audio_input_is_running(void);
bool audio_input_get_levels(audio_levels_t *out);

/** Media |AC| tras umbral (como soundLevel en RepisaHotWheels). */
int audio_input_get_sound_level(void);
/** Nivel util para efectos musicales (sound_level - silencio, min 0). */
int audio_input_get_music_level(void);
/** Max-min del bloque ADC (dinamica del mic). */
int audio_input_get_sound_span(void);
/** Ultima lectura ADC cruda 0-4095. */
int audio_input_get_raw_adc(void);
/** Modo prueba en tiempo de ejecucion (barra en pantalla Config). */
void audio_input_set_test_mode(bool enable);
bool audio_input_is_test_mode(void);

#ifdef __cplusplus
}
#endif

#endif
