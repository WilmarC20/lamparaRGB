#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include "lamp_state.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_settings_init(void);
void ui_settings_timer_cycle(void);
void ui_settings_set_night_mode(bool enabled);
bool ui_settings_get_night_mode(void);
uint8_t ui_settings_get_timer_preset_idx(void);
void ui_settings_restore(bool nightMode, uint8_t timerPresetIdx);
bool ui_settings_timer_active(void);
void ui_settings_format_timer_label(char *buf, size_t len);
bool ui_settings_service(lamp_state_t *state);

#ifdef __cplusplus
}
#endif

#endif
