#include "lamp_state.h"

void lamp_state_init(lamp_state_t *state)
{
    state->power = true;
    state->brightness = 128;
    state->hue = 0;
    state->saturation = 255;
    state->effect = LAMP_EFFECT_SOLID;
    state->musicFx = MUSIC_FX_NONE;
    state->dirty = true;
}

void lamp_state_set_color_hsv(lamp_state_t *state, uint16_t hue, uint8_t sat)
{
    state->hue = hue;
    state->saturation = sat;
    lamp_state_mark_dirty(state);
}

void lamp_state_mark_dirty(lamp_state_t *state)
{
    state->dirty = true;
}
