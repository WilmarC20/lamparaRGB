#include "ui_settings.h"
#include "config.h"
#include <Arduino.h>
#include <stdio.h>
#include <time.h>

static const uint16_t kTimerPresets[] = {0, 15, 30, 60, 120};
static const uint8_t kTimerPresetCount =
    (uint8_t)(sizeof(kTimerPresets) / sizeof(kTimerPresets[0]));

static uint8_t s_timerPresetIdx = 0;
static uint32_t s_timerDeadlineMs = 0;
static bool s_nightMode = false;
static bool s_nightForcedOff = false;

static bool settings_time_available(void)
{
#if ENABLE_RAINMAKER
    struct tm ti;
    return getLocalTime(&ti, 0);
#else
    return false;
#endif
}

static bool settings_is_night_hour(int hour)
{
    return hour >= 22 || hour < 7;
}

void ui_settings_init(void)
{
    s_timerPresetIdx = 0;
    s_timerDeadlineMs = 0;
    s_nightMode = false;
    s_nightForcedOff = false;
}

void ui_settings_timer_cycle(void)
{
    s_timerPresetIdx = (uint8_t)((s_timerPresetIdx + 1U) % kTimerPresetCount);
    const uint16_t mins = kTimerPresets[s_timerPresetIdx];
    if (mins == 0) {
        s_timerDeadlineMs = 0;
    } else {
        s_timerDeadlineMs = millis() + ((uint32_t)mins * 60000UL);
    }
}

void ui_settings_set_night_mode(bool enabled)
{
    s_nightMode = enabled;
    if (!enabled) {
        s_nightForcedOff = false;
    }
}

bool ui_settings_get_night_mode(void)
{
    return s_nightMode;
}

uint8_t ui_settings_get_timer_preset_idx(void)
{
    return s_timerPresetIdx;
}

void ui_settings_restore(bool nightMode, uint8_t timerPresetIdx)
{
    s_nightMode = nightMode;
    if (timerPresetIdx >= kTimerPresetCount) {
        timerPresetIdx = 0;
    }
    s_timerPresetIdx = timerPresetIdx;
    s_timerDeadlineMs = 0;
    s_nightForcedOff = false;
}

bool ui_settings_timer_active(void)
{
    return s_timerDeadlineMs != 0;
}

void ui_settings_format_timer_label(char *buf, size_t len)
{
    if (!buf || len == 0) {
        return;
    }

    if (s_timerDeadlineMs == 0) {
        snprintf(buf, len, "Desactivado");
        return;
    }

    const int32_t remMs = (int32_t)(s_timerDeadlineMs - millis());
    if (remMs <= 0) {
        snprintf(buf, len, "Desactivado");
        return;
    }

    const uint32_t remMin = (uint32_t)(remMs / 60000L);
    const uint32_t remSec = (uint32_t)((remMs % 60000L) / 1000L);
    if (remMin > 0) {
        snprintf(buf, len, "%lum", (unsigned long)remMin);
    } else {
        snprintf(buf, len, "%lus", (unsigned long)remSec);
    }
}

bool ui_settings_service(lamp_state_t *state)
{
    if (!state) {
        return false;
    }

    bool changed = false;

    if (s_timerDeadlineMs != 0 && (int32_t)(millis() - s_timerDeadlineMs) >= 0) {
        if (state->power) {
            state->power = false;
            changed = true;
        }
        s_timerDeadlineMs = 0;
        s_timerPresetIdx = 0;
    }

    if (s_nightMode && settings_time_available()) {
        struct tm ti;
        getLocalTime(&ti, 0);
        if (settings_is_night_hour(ti.tm_hour)) {
            if (!s_nightForcedOff) {
                s_nightForcedOff = true;
            }
            if (state->power) {
                state->power = false;
                changed = true;
            }
        } else if (s_nightForcedOff) {
            s_nightForcedOff = false;
        }
    } else if (s_nightForcedOff) {
        s_nightForcedOff = false;
    }

    if (changed) {
        lamp_state_mark_dirty(state);
    }
    return changed;
}
