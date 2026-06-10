#ifndef UI_APP_H
#define UI_APP_H

#include "lamp_state.h"

void ui_app_setup(void);
void ui_app_loop(void);
lamp_state_t *ui_app_get_state(void);
void ui_app_sync_from_state(void);
/** Aplica LEDs + UI tras cambio externo (p. ej. mando IR). */
void ui_app_notify_state_changed(void);
/** Arranca el mic si hay un modo musical activo. */
void ui_app_ensure_music_audio(void);
/** false durante pantalla prov / BLE activo (IR RMT interfiere). */
bool ui_app_ir_allowed(void);
bool ui_app_led_calib_active(void);
void ui_app_led_calib_step(int delta);

/** true si se puede llamar beginProvision (no durante pintado del QR) */
bool ui_app_allow_ble_provision(void);

/** Pausar LVGL/SPI durante beginProvision (evita panic con BLE) */
void ui_app_pause_graphics(bool pause);
void ui_app_hold_graphics_ms(uint32_t ms);
bool ui_app_graphics_paused(void);

#endif
