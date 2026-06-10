#ifndef UI_EFFECTS_H
#define UI_EFFECTS_H

#include "lamp_state.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_EFFECT_OPTION_COUNT 12

const char *ui_effect_dropdown_options(void);
const char *ui_effect_name_at(uint16_t idx);
uint16_t ui_effect_dropdown_index(const lamp_state_t *state);
void ui_effect_apply_dropdown_index(lamp_state_t *state, uint16_t idx);

#ifdef __cplusplus
}
#endif

#endif
