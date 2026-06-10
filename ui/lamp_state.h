#ifndef LAMP_STATE_H
#define LAMP_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "music_fx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LAMP_EFFECT_SOLID = 0,
    LAMP_EFFECT_RAINBOW,
    LAMP_EFFECT_CHASE,
    LAMP_EFFECT_WAVE,
    LAMP_EFFECT_BREATH,
    LAMP_EFFECT_STROBE,
    LAMP_EFFECT_COUNT
} lamp_effect_t;

typedef struct {
    bool power;
    uint8_t brightness;
    uint16_t hue;
    uint8_t saturation;
    lamp_effect_t effect;
    music_fx_t musicFx;
    bool dirty;
} lamp_state_t;

static inline bool lamp_state_music_active(const lamp_state_t *state)
{
    return state && state->musicFx != MUSIC_FX_NONE;
}

void lamp_state_init(lamp_state_t *state);
void lamp_state_set_color_hsv(lamp_state_t *state, uint16_t hue, uint8_t sat);
void lamp_state_mark_dirty(lamp_state_t *state);

#ifdef __cplusplus
}
#endif

#endif
