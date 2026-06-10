#include "ui_prov_screen.h"
#include "lampara_ui.h"
#include "display.h"
#include "config.h"
#include "ui_qr_bitmap.h"
#include "lvgl.h"
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>

#if ENABLE_RAINMAKER
#include "rainmaker_app.h"
#endif

static lv_obj_t *ui_QrPlaceholder;
static lv_obj_t *ui_LabelProvStatus;
static lv_obj_t *ui_LabelProvName;

#if CONFIG_USE_LVGL_QR
#ifndef RM_PROV_QR_SIZE_PX
#define RM_PROV_QR_SIZE_PX CONFIG_QR_SIZE_PX
#endif

static uint16_t *s_qrPixels = NULL;
static const int s_qrPixelSize = RM_PROV_QR_SIZE_PX;
static char s_qrPayload[150];
static bool s_qrOnPanel = false;

typedef enum {
    QR_ENC_IDLE = 0,
    QR_ENC_RUNNING,
    QR_ENC_OK,
    QR_ENC_FAIL
} qr_encode_state_t;

static volatile qr_encode_state_t s_qrEncodeState = QR_ENC_IDLE;
static TaskHandle_t s_qrEncodeTask = NULL;

static void qr_encode_task(void *arg)
{
    (void)arg;
    const bool ok = s_qrPixels && ui_qr_bitmap_render(s_qrPayload, s_qrPixels, s_qrPixelSize);
    s_qrEncodeState = ok ? QR_ENC_OK : QR_ENC_FAIL;
    s_qrEncodeTask = NULL;
    vTaskDelete(NULL);
}
#endif

void ui_Prov_screen_init(void)
{
    if (ui_Prov) return;

    ui_Prov = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Prov, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Prov, lv_color_hex(0x0F0F1A), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(ui_Prov);
    lv_label_set_text(title, "RainMaker");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 4);

    ui_QrPlaceholder = lv_label_create(ui_Prov);
    lv_label_set_text(ui_QrPlaceholder, "Preparando QR...");
    lv_obj_set_style_text_color(ui_QrPlaceholder, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_align(ui_QrPlaceholder, LV_ALIGN_CENTER, 0, -12);

    ui_LabelProvStatus = lv_label_create(ui_Prov);
    lv_label_set_text(ui_LabelProvStatus, "Esperando...");
    lv_obj_set_style_text_color(ui_LabelProvStatus, lv_color_hex(0xFFAA44), LV_PART_MAIN);
    lv_obj_set_width(ui_LabelProvStatus, 300);
    lv_obj_set_style_text_align(ui_LabelProvStatus, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(ui_LabelProvStatus, LV_ALIGN_TOP_MID, 0, 26);

#if ENABLE_RAINMAKER
    char svc[32];
    char info[64];
    rainmaker_app_get_service_name(svc, sizeof(svc));
    snprintf(info, sizeof(info), "PoP: %s", RM_POP);
    lv_obj_t *sub = lv_label_create(ui_Prov);
    lv_label_set_text(sub, info);
    lv_obj_set_style_text_color(sub, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_align(sub, LV_ALIGN_BOTTOM_MID, 0, -44);

    ui_LabelProvName = lv_label_create(ui_Prov);
    lv_label_set_text(ui_LabelProvName, svc);
    lv_obj_set_style_text_color(ui_LabelProvName, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(ui_LabelProvName, LV_ALIGN_BOTTOM_MID, 0, -58);
#endif

    ui_BtnProvExit = lv_btn_create(ui_Prov);
    lv_obj_set_size(ui_BtnProvExit, 140, 36);
    lv_obj_align(ui_BtnProvExit, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_t *lblExit = lv_label_create(ui_BtnProvExit);
    lv_label_set_text(lblExit, LV_SYMBOL_POWER " Salir");
    lv_obj_center(lblExit);
}

void ui_prov_set_status(const char *text, uint32_t color_hex)
{
    if (!ui_LabelProvStatus) return;
    lv_label_set_text(ui_LabelProvStatus, text ? text : "");
    lv_obj_set_style_text_color(ui_LabelProvStatus, lv_color_hex(color_hex), LV_PART_MAIN);
}

#if CONFIG_USE_LVGL_QR
bool ui_prov_qr_mem_init(int *out_px)
{
    if (s_qrPixels) {
        heap_caps_free(s_qrPixels);
        s_qrPixels = NULL;
    }
    const size_t bytes = (size_t)s_qrPixelSize * (size_t)s_qrPixelSize * sizeof(uint16_t);
    s_qrPixels = (uint16_t *)heap_caps_malloc(bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!s_qrPixels) {
        printf("PROV UI: sin RAM para buffer QR\n");
        return false;
    }
    memset(s_qrPixels, 0xFF, bytes);
    if (out_px) *out_px = s_qrPixelSize;
    return true;
}

bool ui_prov_qr_ready(void)
{
    return s_qrEncodeState == QR_ENC_OK && s_qrPixels != NULL;
}

void ui_prov_create_qr(void)
{
    if (ui_QrPlaceholder) {
        lv_obj_add_flag(ui_QrPlaceholder, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_prov_qr_encode_async(void)
{
    if (s_qrEncodeState == QR_ENC_RUNNING || s_qrEncodeTask) return;
    if (!s_qrPixels) {
        s_qrEncodeState = QR_ENC_FAIL;
        return;
    }

#if ENABLE_RAINMAKER
    if (!rainmaker_app_get_qr_payload(s_qrPayload, sizeof(s_qrPayload))) {
        s_qrEncodeState = QR_ENC_FAIL;
        return;
    }
#else
    strncpy(s_qrPayload, "{}", sizeof(s_qrPayload) - 1);
#endif

    s_qrEncodeState = QR_ENC_RUNNING;
    BaseType_t ok = xTaskCreatePinnedToCore(
        qr_encode_task, "qr_prov", CONFIG_QR_ENCODE_TASK_STACK,
        NULL, 1, &s_qrEncodeTask, CONFIG_QR_ENCODE_CORE);
    if (ok != pdPASS) {
        s_qrEncodeState = QR_ENC_FAIL;
        s_qrEncodeTask = NULL;
    }
}

void ui_prov_qr_encode_reset(void)
{
    if (s_qrEncodeState == QR_ENC_RUNNING) return;
    s_qrEncodeState = QR_ENC_IDLE;
}

bool ui_prov_qr_encode_pending(void)
{
    return s_qrEncodeState == QR_ENC_RUNNING;
}

bool ui_prov_qr_encode_finished(void)
{
    return s_qrEncodeState == QR_ENC_OK || s_qrEncodeState == QR_ENC_FAIL;
}

bool ui_prov_qr_encode_ok(void)
{
    return s_qrEncodeState == QR_ENC_OK;
}

void ui_prov_refresh_qr(void)
{
    if (!s_qrPixels || s_qrEncodeState != QR_ENC_OK) return;

    const int frame = CONFIG_QR_FRAME_PX;
    const int total = s_qrPixelSize + frame * 2;
    const int x = ((int)kScreenWidth - total) / 2;
    const int y = ((int)kScreenHeight - total) / 2 - 8;

    display_fill_rect((int16_t)x, (int16_t)y, (int16_t)total, (int16_t)total, 0xFFFF);
    display_draw_rgb565_rect((int16_t)(x + frame), (int16_t)(y + frame),
                             (int16_t)s_qrPixelSize, (int16_t)s_qrPixelSize, s_qrPixels);
    s_qrOnPanel = true;
    printf("PROV UI: QR en TFT %dx%d heap=%u\n", s_qrPixelSize, s_qrPixelSize,
           (unsigned)esp_get_free_heap_size());
}

void ui_prov_release_qr_for_ble(void)
{
    if (s_qrPixels) {
        heap_caps_free(s_qrPixels);
        s_qrPixels = NULL;
        printf("PROV UI: buffer QR liberado heap=%u\n", (unsigned)esp_get_free_heap_size());
    }
    s_qrEncodeState = QR_ENC_IDLE;
    s_qrOnPanel = false;
}
#endif
