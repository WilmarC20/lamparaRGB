#include "config.h"
#include "ui_app.h"
#include "lv_conf.h"
#include "lamp_log.h"
#include <Arduino.h>

#if LV_MEM_CUSTOM != 1
#error "lv_conf.h incorrecto: LV_MEM_CUSTOM debe ser 1. Usa ui/lv_conf.h (sketch ui/)"
#endif

void setup()
{
#if LAMP_SERIAL_LOG
    Serial.begin(115200);
    delay(500);
#endif
    LAMP_LOG("LamparaV3 iniciando... heap=%u\n", ESP.getFreeHeap());
#if USE_LVGL_UI
    ui_app_setup();
    LAMP_LOG("Setup OK heap=%u\n", ESP.getFreeHeap());
#endif
}

void loop()
{
#if USE_LVGL_UI
    ui_app_loop();
#else
    delay(1000);
#endif
}
