#include "ui_qr_bitmap.h"
#include "../libraries/lvgl/src/extra/libs/qrcode/qrcodegen.h"
#include <stdlib.h>
#include <string.h>

bool ui_qr_bitmap_render(const char *text, uint16_t *rgb565, int display_px)
{
    if (!text || !rgb565 || display_px <= 0) return false;

    const size_t len = strlen(text);
    int version = qrcodegen_getMinFitVersion(qrcodegen_Ecc_MEDIUM, len);
    if (version < qrcodegen_VERSION_MIN) return false;

    const size_t buf_len = qrcodegen_BUFFER_LEN_FOR_VERSION(version);
    uint8_t *temp = (uint8_t *)malloc(buf_len);
    uint8_t *qr = (uint8_t *)malloc(buf_len);
    if (!temp || !qr) {
        free(temp);
        free(qr);
        return false;
    }

    const bool ok = qrcodegen_encodeText(text, temp, qr, qrcodegen_Ecc_MEDIUM,
                                         version, version, qrcodegen_Mask_AUTO, true);
    if (!ok) {
        free(temp);
        free(qr);
        return false;
    }

    const int qr_size = qrcodegen_getSize(qr);
    const int scale = display_px / qr_size;
    if (scale < 1) {
        free(temp);
        free(qr);
        return false;
    }

    const int draw_size = qr_size * scale;
    const int margin = (display_px - draw_size) / 2;
    const uint16_t white = 0xFFFF;
    const uint16_t black = 0x0000;

    const int pixels = display_px * display_px;
    for (int i = 0; i < pixels; i++) {
        rgb565[i] = white;
    }

    for (int py = 0; py < display_px; py++) {
        for (int px = 0; px < display_px; px++) {
            if (px < margin || py < margin ||
                px >= margin + draw_size || py >= margin + draw_size) {
                continue;
            }
            const int qx = (px - margin) / scale;
            const int qy = (py - margin) / scale;
            if (qrcodegen_getModule(qr, qx, qy)) {
                rgb565[py * display_px + px] = black;
            }
        }
    }

    free(temp);
    free(qr);
    return true;
}
