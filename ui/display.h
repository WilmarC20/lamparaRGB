#ifndef LAMPARA_DISPLAY_H
#define LAMPARA_DISPLAY_H

#include <lvgl.h>
#include "pins.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const uint16_t kScreenWidth;
extern const uint16_t kScreenHeight;

void display_init(void);
/** Solo TFT, sin buffer LVGL (modo prov sin GUI completa) */
void display_init_tft_only(void);
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
/** Pinta RGB565 en panel (QR); coordenadas con rotation 1 */
void display_draw_rgb565_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                              const uint16_t *rgb565);
void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color565);
/** Texto directo al TFT (no LVGL). datum: 0=TL, 1=TC, 2=TR */
void display_draw_text(int16_t x, int16_t y, const char *text,
                       uint16_t color565, uint16_t bg565, uint8_t datum, uint8_t size);
/** Rellena toda la pantalla */
void display_clear(uint16_t color565);
/** Libera buffer LVGL durante BLE (el QR ya esta en el TFT) */
void display_suspend_lvgl(void);
void display_resume_lvgl(void);
bool display_lvgl_suspended(void);
/** Texto corto arriba cuando LVGL suspendido (pantalla prov) */
void display_prov_status_banner(const char *text, uint16_t color565);

/** Retroiluminacion TFT (GPIO PIN_TFT_BL). */
void display_set_backlight(bool on);
bool display_backlight_on(void);

#ifdef __cplusplus
}
#endif

#endif
