#ifndef UI_PROV_SCREEN_H
#define UI_PROV_SCREEN_H

#include "config.h"
#include "lvgl.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_Prov_screen_init(void);
void ui_prov_set_status(const char *text, uint32_t color_hex);

#if CONFIG_USE_LVGL_QR
bool ui_prov_qr_mem_init(int *out_px);
void ui_prov_create_qr(void);
void ui_prov_qr_encode_async(void);
void ui_prov_qr_encode_reset(void);
bool ui_prov_qr_encode_pending(void);
bool ui_prov_qr_encode_finished(void);
bool ui_prov_qr_encode_ok(void);
void ui_prov_refresh_qr(void);
bool ui_prov_qr_ready(void);
void ui_prov_release_qr_for_ble(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
