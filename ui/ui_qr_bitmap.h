#ifndef UI_QR_BITMAP_H
#define UI_QR_BITMAP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Renderiza payload en buffer RGB565 (LV_COLOR_DEPTH 16, sin swap) */
bool ui_qr_bitmap_render(const char *text, uint16_t *rgb565, int display_px);

#ifdef __cplusplus
}
#endif

#endif
