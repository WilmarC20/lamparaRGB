#ifndef MUSIC_EFFECTS_H
#define MUSIC_EFFECTS_H

#include "lamp_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void music_effects_reset(void);
void music_effects_update(lamp_state_t *state);
const char *music_fx_name(music_fx_t fx);

#ifdef __cplusplus
}
#endif

#endif
