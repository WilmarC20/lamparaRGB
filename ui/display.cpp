#include "display.h"
#include "config.h"
#include "lamp_log.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_heap_caps.h>

const uint16_t kScreenWidth  = 320;
const uint16_t kScreenHeight = 240;

#if USE_LVGL_UI

/* 6 lineas: ahorra ~2.5 KB heap; flush trocea areas grandes */
#define DISP_BUF_LINES  6
#define DISP_BUF_PIXELS (kScreenWidth * DISP_BUF_LINES)

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_buf = NULL;
static bool s_lvglSuspended = false;
static bool s_backlightOn = true;
static TFT_eSPI tft = TFT_eSPI(kScreenWidth, kScreenHeight);

extern "C" void display_draw_rgb565_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                                         const uint16_t *rgb565)
{
    if (!rgb565 || w <= 0 || h <= 0) return;
    tft.startWrite();
    tft.setAddrWindow(x, y, (uint32_t)w, (uint32_t)h);
    tft.pushColors((uint16_t *)rgb565, (uint32_t)w * (uint32_t)h, true);
    tft.endWrite();
}

extern "C" void display_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color565)
{
    if (w <= 0 || h <= 0) return;
    tft.startWrite();
    tft.fillRect(x, y, w, h, color565);
    tft.endWrite();
}

extern "C" void display_suspend_lvgl(void)
{
    if (s_lvglSuspended || !disp_buf) return;
    s_lvglSuspended = true;
    heap_caps_free(disp_buf);
    disp_buf = NULL;
    LAMP_LOG("Display: LVGL suspendido heap=%u\n", ESP.getFreeHeap());
}

extern "C" void display_resume_lvgl(void)
{
    if (!s_lvglSuspended) return;
    disp_buf = (lv_color_t *)heap_caps_malloc(DISP_BUF_PIXELS * sizeof(lv_color_t),
                                              MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!disp_buf) {
        LAMP_LOG_LN("Display: no RAM para reanudar LVGL");
        return;
    }
    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, DISP_BUF_PIXELS);
    s_lvglSuspended = false;
}

extern "C" bool display_lvgl_suspended(void)
{
    return s_lvglSuspended;
}

extern "C" void display_prov_status_banner(const char *text, uint16_t color565)
{
    if (!text) return;
    tft.startWrite();
    tft.fillRect(0, 20, kScreenWidth, 22, 0x1084); /* fondo oscuro */
    tft.setTextColor(color565, 0x1084);
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(text, kScreenWidth / 2, 24, 1);
    tft.endWrite();
}

extern "C" void display_draw_text(int16_t x, int16_t y, const char *text,
                                  uint16_t color565, uint16_t bg565,
                                  uint8_t datum, uint8_t size)
{
    if (!text) return;
    tft.startWrite();
    tft.setTextColor(color565, bg565);
    tft.setTextSize(size ? size : 1);
    switch (datum) {
        case 1: tft.setTextDatum(TC_DATUM); break;
        case 2: tft.setTextDatum(TR_DATUM); break;
        default: tft.setTextDatum(TL_DATUM); break;
    }
    tft.drawString(text, x, y, 1);
    tft.endWrite();
}

extern "C" void display_clear(uint16_t color565)
{
    tft.startWrite();
    tft.fillScreen(color565);
    tft.endWrite();
}

extern "C" void display_set_backlight(bool on)
{
    s_backlightOn = on;
    digitalWrite(PIN_TFT_BL, on ? HIGH : LOW);
}

extern "C" bool display_backlight_on(void)
{
    return s_backlightOn;
}

extern "C" void display_init_tft_only(void)
{
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

extern "C" void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    if (!disp_buf) {
        lv_disp_flush_ready(disp);
        return;
    }
    const uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    const uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);
    if (w == 0 || h == 0) {
        lv_disp_flush_ready(disp);
        return;
    }

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);

    uint32_t y = 0;
    while (y < h) {
        uint32_t lines = h - y;
        const uint32_t max_lines = DISP_BUF_PIXELS / w;
        if (max_lines == 0) break;
        if (lines > max_lines) lines = max_lines;

        const uint32_t num_px = w * lines;
        tft.pushColors((uint16_t *)color_p, num_px, true);
        color_p += num_px;
        y += lines;
    }

    tft.endWrite();
    lv_disp_flush_ready(disp);
}

void display_init(void)
{
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_NAVY);

    disp_buf = (lv_color_t *)heap_caps_malloc(DISP_BUF_PIXELS * sizeof(lv_color_t),
                                              MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!disp_buf) {
        LAMP_LOG_LN("ERROR: sin RAM para buffer display");
        return;
    }

    lv_disp_draw_buf_init(&draw_buf, disp_buf, NULL, DISP_BUF_PIXELS);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = kScreenWidth;
    disp_drv.ver_res = kScreenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

#else

void display_init(void) {}

#endif
