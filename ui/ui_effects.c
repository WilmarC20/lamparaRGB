#include "ui_effects.h"
#include "music_effects.h"
#include "lvgl.h"

#define UI_EFFECT_COLOMBIA_IDX 9U

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
    "Musica Colombia\n"
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
        "Musica Colombia",
        "Musica Respiracion",
        "Musica Estrobo"
    };
    if (idx >= UI_EFFECT_OPTION_COUNT) {
        return "Solido";
    }
    return kNames[idx];
}

const char *ui_effect_icon_at(uint16_t idx)
{
    static const char *kIcons[UI_EFFECT_OPTION_COUNT] = {
        LV_SYMBOL_TINT,
        LV_SYMBOL_IMAGE,
        LV_SYMBOL_NEXT,
        LV_SYMBOL_LOOP,
        LV_SYMBOL_EYE_OPEN,
        LV_SYMBOL_WARNING,
        LV_SYMBOL_VOLUME_MID,
        LV_SYMBOL_AUDIO,
        LV_SYMBOL_NEXT,
        LV_SYMBOL_AUDIO,
        LV_SYMBOL_EYE_OPEN,
        LV_SYMBOL_WARNING
    };
    if (idx >= UI_EFFECT_OPTION_COUNT) {
        return LV_SYMBOL_TINT;
    }
    return kIcons[idx];
}

bool ui_effect_uses_flag_icon(uint16_t idx)
{
    return idx == UI_EFFECT_COLOMBIA_IDX;
}

uint32_t ui_effect_icon_color(uint16_t idx)
{
    static const uint32_t kColors[UI_EFFECT_OPTION_COUNT] = {
        0xE8E8F0,
        0xFF6B9D,
        0xFF8800,
        0x4488FF,
        0xAA66FF,
        0xFFCC00,
        0x00CCCC,
        0x00CCCC,
        0x00CCCC,
        0xCE1126,   /* Musica Colombia: rojo bandera */
        0x00CCCC,
        0x00CCCC
    };
    if (idx >= UI_EFFECT_OPTION_COUNT) {
        return 0xE8E8F0;
    }
    return kColors[idx];
}

ui_fx_color_mode_t ui_effect_color_mode(uint16_t idx)
{
    switch (idx) {
        case 1:  /* Arcoiris */
        case 2:  /* Persecucion */
        case 7:  /* Musica Fiesta */
        case 9:  /* Musica Colombia (colores fijos de bandera) */
            return UI_FX_COLOR_RAINBOW;
        case 6:  /* Musica Barra */
        case 8:  /* Musica Persecucion */
            return UI_FX_COLOR_GRADIENT;
        default:
            return UI_FX_COLOR_SINGLE;
    }
}

void ui_effect_color_stops(uint16_t idx, uint16_t base_hue, uint8_t sat,
                             uint16_t *hues_out, uint8_t *count_out, uint8_t max_count)
{
    if (!hues_out || !count_out || max_count == 0) {
        return;
    }

    const ui_fx_color_mode_t mode = ui_effect_color_mode(idx);
    uint8_t n = 0;

    if (idx == 9) {
        /* Musica Colombia: amarillo, azul, rojo fijos */
        static const uint16_t kFlagHues[] = { 51, 235, 0 };
        n = (uint8_t)(sizeof(kFlagHues) / sizeof(kFlagHues[0]));
        if (n > max_count) {
            n = max_count;
        }
        for (uint8_t i = 0; i < n; i++) {
            hues_out[i] = kFlagHues[i];
        }
        *count_out = n;
        return;
    }

    if (mode == UI_FX_COLOR_RAINBOW) {
        static const uint16_t kRainbowHues[] = { 0, 30, 60, 120, 180, 240, 300 };
        n = (uint8_t)(sizeof(kRainbowHues) / sizeof(kRainbowHues[0]));
        if (n > max_count) {
            n = max_count;
        }
        for (uint8_t i = 0; i < n; i++) {
            hues_out[i] = kRainbowHues[i];
        }
    } else if (mode == UI_FX_COLOR_GRADIENT) {
        static const uint16_t kSpread[] = { 0, 20, 40, 60, 80 };
        n = (uint8_t)(sizeof(kSpread) / sizeof(kSpread[0]));
        if (idx == 8) {
            n = 3;
        }
        if (n > max_count) {
            n = max_count;
        }
        for (uint8_t i = 0; i < n; i++) {
            hues_out[i] = (uint16_t)((base_hue + kSpread[i]) % 360U);
        }
        (void)sat;
    } else {
        hues_out[0] = base_hue % 360U;
        n = 1;
        (void)sat;
    }

    *count_out = n;
}

uint16_t ui_effect_hue_from_bar_x(int16_t x, int16_t bar_width)
{
    if (bar_width <= 0) {
        return 0;
    }
    if (x < 0) {
        x = 0;
    }
    if (x > bar_width) {
        x = bar_width;
    }
    return (uint16_t)((uint32_t)x * 360U / (uint32_t)bar_width);
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
        state->musicMode = true;
        music_effects_reset();
        return;
    }

    state->musicMode = false;
    state->musicFx = MUSIC_FX_NONE;
    state->effect = (lamp_effect_t)idx;
}
