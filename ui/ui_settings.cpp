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

/* Amanecer: minutos desde medianoche en que la rampa TERMINA (15 min antes
 * empieza). -1 = desactivado. Requiere hora NTP. */
static const int16_t kSunriseMins[] = { -1, 330, 360, 390, 420 };
static const char *kSunriseNames[] = { "Off", "5:30", "6:00", "6:30", "7:00" };
#define SUNRISE_PRESET_COUNT 5
#define SUNRISE_RAMP_SECS    900
#define SUNRISE_HUE          35U

static uint8_t s_sunriseIdx = 0;
static bool s_sunriseActive = false;
static bool s_sunriseHasLast = false;
static uint8_t s_sunriseLastBright = 0;
static int s_sunriseFiredDay = -1;

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

void ui_settings_restore(bool nightMode, uint8_t timerPresetIdx, uint8_t sunriseIdx)
{
    s_nightMode = nightMode;
    if (timerPresetIdx >= kTimerPresetCount) {
        timerPresetIdx = 0;
    }
    s_timerPresetIdx = timerPresetIdx;
    s_timerDeadlineMs = 0;
    s_nightForcedOff = false;
    s_sunriseIdx = (sunriseIdx < SUNRISE_PRESET_COUNT) ? sunriseIdx : 0;
    s_sunriseActive = false;
    s_sunriseHasLast = false;
    s_sunriseFiredDay = -1;
}

void ui_settings_sunrise_cycle(void)
{
    s_sunriseIdx = (uint8_t)((s_sunriseIdx + 1U) % SUNRISE_PRESET_COUNT);
    s_sunriseActive = false;
    s_sunriseHasLast = false;
    s_sunriseFiredDay = -1;
}

uint8_t ui_settings_get_sunrise_idx(void)
{
    return s_sunriseIdx;
}

void ui_settings_format_sunrise_label(char *buf, size_t len)
{
    if (!buf || len == 0) {
        return;
    }
    snprintf(buf, len, "%s", kSunriseNames[s_sunriseIdx]);
}

/* Rampa de amanecer: 15 min de blanco calido creciente terminando a la hora
 * elegida. Si el usuario cambia el brillo durante la rampa, se cancela por
 * hoy. Devuelve true si modifico el estado. */
static bool sunrise_service(lamp_state_t *state)
{
    if (s_sunriseIdx == 0 || !settings_time_available()) {
        return false;
    }

    struct tm ti;
    getLocalTime(&ti, 0);
    const int target = (int)kSunriseMins[s_sunriseIdx];
    const int nowSec = (ti.tm_hour * 60 + ti.tm_min) * 60 + ti.tm_sec;
    const int startSec = (target * 60) - SUNRISE_RAMP_SECS;
    const bool inWindow = nowSec >= startSec && nowSec < (target * 60);

    if (!inWindow) {
        if (s_sunriseActive) {
            /* rampa completada: no re-disparar hoy */
            s_sunriseActive = false;
            s_sunriseHasLast = false;
            s_sunriseFiredDay = ti.tm_yday;
        }
        return false;
    }
    if (s_sunriseFiredDay == ti.tm_yday) {
        return false;
    }

    /* usuario ajusto el brillo durante la rampa: cancelar por hoy */
    if (s_sunriseActive && s_sunriseHasLast && state->brightness != s_sunriseLastBright) {
        s_sunriseActive = false;
        s_sunriseHasLast = false;
        s_sunriseFiredDay = ti.tm_yday;
        return false;
    }
    s_sunriseActive = true;

    float p = (float)(nowSec - startSec) / (float)SUNRISE_RAMP_SECS;
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    const uint8_t b = (uint8_t)(10.0f + p * 190.0f);
    const uint8_t sat = (uint8_t)(220.0f - p * 130.0f);

    if (state->power && state->brightness == b && state->saturation == sat &&
        state->hue == SUNRISE_HUE && state->effect == LAMP_EFFECT_SOLID &&
        state->musicFx == MUSIC_FX_NONE) {
        return false;
    }

    state->power = true;
    state->musicMode = false;
    state->musicFx = MUSIC_FX_NONE;
    state->effect = LAMP_EFFECT_SOLID;
    state->hue = SUNRISE_HUE;
    state->saturation = sat;
    state->brightness = b;
    s_sunriseLastBright = b;
    s_sunriseHasLast = true;
    return true;
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

    if (sunrise_service(state)) {
        changed = true;
    }

    if (changed) {
        lamp_state_mark_dirty(state);
    }
    return changed;
}
