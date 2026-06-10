#include "ui_prov_raw.h"
#include "display.h"
#include "config.h"
#include "lamp_log.h"
#include "ui_qr_bitmap.h"
#include "touch_cst820.h"
#include <Arduino.h>
#include <esp_heap_caps.h>
#include <string.h>

#if ENABLE_RAINMAKER
#include "rainmaker_app.h"
#endif

/* 320x240 landscape */
#define PROV_BG          0x0000
#define PROV_TITLE_Y     4
#define PROV_STATUS_Y    20
#define PROV_STATUS_BG   0x1084
#define PROV_QR_FRAME_PX 3
#define PROV_INFO_Y      168
#define PROV_POP_Y       186
#define PROV_BTN_W       100
#define PROV_BTN_H       26
#define PROV_BTN_Y       212
#define PROV_BTN_X       ((320 - PROV_BTN_W) / 2)
#define PROV_BTN_COLOR   0xC966
#define PROV_BTN_COLOR_P 0xF800

#ifndef RM_PROV_QR_SIZE_PX
#define RM_PROV_QR_SIZE_PX 100
#endif

static uint16_t *s_qrPixels = NULL;
static const int s_qrPixelSize = RM_PROV_QR_SIZE_PX;
static char s_qrPayload[160];
static char s_currentStatus[48];
static bool s_initialized = false;
static bool s_btnPressed = false;
static bool s_btnExitLatched = false;

static void draw_exit_button(bool pressed)
{
    const uint16_t bg = pressed ? PROV_BTN_COLOR_P : PROV_BTN_COLOR;
    const int16_t x = (int16_t)PROV_BTN_X;
    const int16_t y = (int16_t)PROV_BTN_Y;
    display_fill_rect(x, y, PROV_BTN_W, PROV_BTN_H, bg);
    display_fill_rect(x, y, PROV_BTN_W, 2, 0xFFFF);
    display_fill_rect(x, y + PROV_BTN_H - 2, PROV_BTN_W, 2, 0xFFFF);
    display_fill_rect(x, y, 2, PROV_BTN_H, 0xFFFF);
    display_fill_rect(x + PROV_BTN_W - 2, y, 2, PROV_BTN_H, 0xFFFF);
    display_draw_text(x + PROV_BTN_W / 2, y + 7, "SALIR", 0xFFFF, bg, 1, 2);
}

void ui_prov_raw_init(void)
{
    if (s_initialized) {
        return;
    }
    s_initialized = true;
    s_btnPressed = false;
    s_btnExitLatched = false;
    memset(s_currentStatus, 0, sizeof(s_currentStatus));

    display_clear(PROV_BG);
    display_draw_text(160, PROV_TITLE_Y, "RainMaker", 0xFFFF, PROV_BG, 1, 2);
    display_fill_rect(0, PROV_STATUS_Y - 2, 320, 16, PROV_STATUS_BG);
    display_draw_text(160, PROV_STATUS_Y, "Preparando QR...", 0xFFE0, PROV_STATUS_BG, 1, 1);

#if ENABLE_RAINMAKER
    char svc[32];
    rainmaker_app_get_service_name(svc, sizeof(svc));
    display_draw_text(160, PROV_INFO_Y, svc, 0xCCCC, PROV_BG, 1, 1);
    char popLine[32];
    snprintf(popLine, sizeof(popLine), "PoP: %s", RM_POP);
    display_draw_text(160, PROV_POP_Y, popLine, 0xAAAA, PROV_BG, 1, 1);
#endif

    draw_exit_button(false);
    LAMP_LOG("PROV RAW: lista heap=%u\n", ESP.getFreeHeap());
}

void ui_prov_raw_set_status(const char *text, uint16_t color565)
{
    if (!text) {
        return;
    }
    if (strncmp(text, s_currentStatus, sizeof(s_currentStatus)) == 0) {
        return;
    }
    strncpy(s_currentStatus, text, sizeof(s_currentStatus) - 1);
    s_currentStatus[sizeof(s_currentStatus) - 1] = '\0';

    display_fill_rect(0, PROV_STATUS_Y - 2, 320, 16, PROV_STATUS_BG);
    display_draw_text(160, PROV_STATUS_Y, s_currentStatus, color565, PROV_STATUS_BG, 1, 1);
}

bool ui_prov_raw_qr_alloc(int *out_px)
{
    if (s_qrPixels) {
        heap_caps_free(s_qrPixels);
        s_qrPixels = NULL;
    }
    const size_t bytes = (size_t)s_qrPixelSize * (size_t)s_qrPixelSize * sizeof(uint16_t);
    s_qrPixels = (uint16_t *)heap_caps_malloc(bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!s_qrPixels) {
        LAMP_LOG_LN("PROV RAW: sin RAM para QR");
        return false;
    }
    memset(s_qrPixels, 0xFF, bytes);
    if (out_px) {
        *out_px = s_qrPixelSize;
    }
    return true;
}

bool ui_prov_raw_qr_encode_sync(void)
{
    if (!s_qrPixels) {
        return false;
    }
#if ENABLE_RAINMAKER
    if (!rainmaker_app_get_qr_payload(s_qrPayload, sizeof(s_qrPayload))) {
        return false;
    }
#else
    strncpy(s_qrPayload, "{}", sizeof(s_qrPayload) - 1);
#endif
    LAMP_LOG("PROV RAW: QR encode len=%u heap=%u\n",
                  (unsigned)strlen(s_qrPayload), ESP.getFreeHeap());
    return ui_qr_bitmap_render(s_qrPayload, s_qrPixels, s_qrPixelSize);
}

void ui_prov_raw_qr_paint(void)
{
    if (!s_qrPixels) {
        return;
    }
    const int total = s_qrPixelSize + PROV_QR_FRAME_PX * 2;
    const int x = (320 - total) / 2;
    const int y = 36;
    display_fill_rect((int16_t)x, (int16_t)y, (int16_t)total, (int16_t)total, 0xFFFF);
    display_draw_rgb565_rect((int16_t)(x + PROV_QR_FRAME_PX),
                             (int16_t)(y + PROV_QR_FRAME_PX),
                             (int16_t)s_qrPixelSize, (int16_t)s_qrPixelSize,
                             s_qrPixels);
    LAMP_LOG("PROV RAW: QR %dpx heap=%u\n", s_qrPixelSize, ESP.getFreeHeap());
}

void ui_prov_raw_qr_free(void)
{
    if (s_qrPixels) {
        heap_caps_free(s_qrPixels);
        s_qrPixels = NULL;
        LAMP_LOG("PROV RAW: QR buffer libre heap=%u\n", ESP.getFreeHeap());
    }
}

bool ui_prov_raw_check_exit_pressed(void)
{
    if (s_btnExitLatched) {
        return false;
    }

    uint16_t tx = 0;
    uint16_t ty = 0;
    const bool touched = touch_cst820_read_screen(&tx, &ty);
    const bool inBtn = touched &&
                       tx >= (uint16_t)PROV_BTN_X &&
                       tx < (uint16_t)(PROV_BTN_X + PROV_BTN_W) &&
                       ty >= (uint16_t)PROV_BTN_Y &&
                       ty < (uint16_t)(PROV_BTN_Y + PROV_BTN_H);

    if (inBtn) {
        if (!s_btnPressed) {
            s_btnPressed = true;
            draw_exit_button(true);
        }
        return false;
    }

    if (s_btnPressed) {
        s_btnPressed = false;
        draw_exit_button(false);
        s_btnExitLatched = true;
        LAMP_LOG_LN("PROV RAW: Salir");
        return true;
    }
    return false;
}
