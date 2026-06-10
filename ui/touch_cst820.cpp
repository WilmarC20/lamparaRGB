#include "touch_cst820.h"
#include "display.h"
#include "pins.h"
#include "CST820.h"
#include <lvgl.h>

static CST820 touch(PIN_TP_SDA, PIN_TP_SCL, PIN_TP_RST, PIN_TP_INT);

static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    (void)indev_driver;
    uint16_t touchX = 0, touchY = 0;
    uint8_t gesture = 0;

    if (!touch.getTouch(&touchX, &touchY, &gesture)) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }

    data->state = LV_INDEV_STATE_PR;
    /* Landscape rotation 1 (display.setRotation(1)) */
    uint16_t temp = touchX;
    touchX = touchY;
    touchY = kScreenHeight - temp;
    data->point.x = touchX;
    data->point.y = touchY;
}

void touch_cst820_init(void)
{
    touch.begin();
}

void touch_cst820_register_indev(void)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

bool touch_cst820_read_screen(uint16_t *x, uint16_t *y)
{
    uint16_t rawX = 0, rawY = 0;
    uint8_t gesture = 0;
    if (!touch.getTouch(&rawX, &rawY, &gesture)) {
        return false;
    }
    const uint16_t temp = rawX;
    const uint16_t sx = rawY;
    const uint16_t sy = kScreenHeight - temp;
    if (x) *x = sx;
    if (y) *y = sy;
    return true;
}

bool touch_cst820_hit_prov_exit(uint16_t x, uint16_t y)
{
    /* ui_BtnProvExit: 140x36, bottom mid, offset -6 */
    const int16_t bx = (int16_t)((kScreenWidth - 140) / 2);
    const int16_t by = (int16_t)(kScreenHeight - 6 - 36);
    return (x >= (uint16_t)bx && x < (uint16_t)(bx + 140) &&
            y >= (uint16_t)by && y < (uint16_t)(by + 36));
}
