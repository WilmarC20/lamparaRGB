#ifndef UI_EFFECTS_H
#define UI_EFFECTS_H

#include "lamp_state.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_EFFECT_OPTION_COUNT 12

typedef enum {
    UI_FX_COLOR_SINGLE = 0,
    UI_FX_COLOR_RAINBOW,
    UI_FX_COLOR_GRADIENT,
} ui_fx_color_mode_t;

const char *ui_effect_dropdown_options(void);
const char *ui_effect_name_at(uint16_t idx);
const char *ui_effect_icon_at(uint16_t idx);
bool ui_effect_uses_flag_icon(uint16_t idx);
uint32_t ui_effect_icon_color(uint16_t idx);
ui_fx_color_mode_t ui_effect_color_mode(uint16_t idx);
void ui_effect_color_stops(uint16_t idx, uint16_t base_hue, uint8_t sat,
                             uint16_t *hues_out, uint8_t *count_out, uint8_t max_count);
uint16_t ui_effect_hue_from_bar_x(int16_t x, int16_t bar_width);
uint16_t ui_effect_dropdown_index(const lamp_state_t *state);
void ui_effect_apply_dropdown_index(lamp_state_t *state, uint16_t idx);

#ifdef __cplusplus
}
#endif

#endif
