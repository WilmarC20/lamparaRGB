#include "ui_effects.h"
#include "music_effects.h"

static const char *kUiEffectOptions =
    "Solido\n"
    "Arcoiris\n"
    "Persecucion\n"
    "Onda\n"
    "Respiracion\n"
    "Estrobo\n"
    "Musica Barra\n"
    "Musica Fiesta\n"
    "Musica Persecucion\n"
    "Musica Onda\n"
    "Musica Respiracion\n"
    "Musica Estrobo";

const char *ui_effect_dropdown_options(void)
{
    return kUiEffectOptions;
}

const char *ui_effect_name_at(uint16_t idx)
{
    static const char *kNames[UI_EFFECT_OPTION_COUNT] = {
        "Solido",
        "Arcoiris",
        "Persecucion",
        "Onda",
        "Respiracion",
        "Estrobo",
        "Musica Barra",
        "Musica Fiesta",
        "Musica Persecucion",
        "Musica Onda",
        "Musica Respiracion",
        "Musica Estrobo"
    };
    if (idx >= UI_EFFECT_OPTION_COUNT) {
        return "Solido";
    }
    return kNames[idx];
}

uint16_t ui_effect_dropdown_index(const lamp_state_t *state)
{
    if (!state) return 0;
    if (state->musicFx != MUSIC_FX_NONE) {
        return (uint16_t)(LAMP_EFFECT_COUNT + (int)state->musicFx - 1);
    }
    if (state->effect >= LAMP_EFFECT_COUNT) return 0;
    return (uint16_t)state->effect;
}

void ui_effect_apply_dropdown_index(lamp_state_t *state, uint16_t idx)
{
    if (!state || idx >= UI_EFFECT_OPTION_COUNT) return;

    if (idx >= LAMP_EFFECT_COUNT) {
        const music_fx_t fx = (music_fx_t)((int)idx - LAMP_EFFECT_COUNT + 1);
        state->musicFx = fx;
        music_effects_reset();
        return;
    }

    state->musicFx = MUSIC_FX_NONE;
    state->effect = (lamp_effect_t)idx;
}
