#ifndef MUSIC_EFFECTS_H
#define MUSIC_EFFECTS_H

#include "lamp_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void music_effects_reset(void);
void music_effects_update(lamp_state_t *state);
/** Paleta para Fiesta/Beat: 0=Arcoiris, 1=Tropical, 2=Rock, 3=Chill */
void music_effects_set_palette(uint8_t idx);
uint8_t music_effects_get_palette(void);
const char *music_effects_palette_name(uint8_t idx);
void music_effects_colombia_static(const lamp_state_t *state, uint8_t *rgb);
lamp_effect_t music_fx_static_lamp_effect(music_fx_t fx);
const char *music_fx_name(music_fx_t fx);

#ifdef __cplusplus
}
#endif

#endif
