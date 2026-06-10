#include "music_effects.h"
#include "audio_input.h"
#include "config.h"
#include "led_controller.h"
#include "led_layout.h"
#include "pins.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

#define MUSIC_LEVEL_MAX   400
#define MUSIC_ATTACK      0.55f
#define MUSIC_RELEASE     0.08f

static float s_smoothLevel = 0.0f;
static float s_chasePos = 0.0f;
static float s_wavePhase = 0.0f;
static float s_breathPhase = 0.0f;
static bool s_strobeOn = false;
static uint32_t s_strobeUntilMs = 0;

static const char *kFxNames[] = {
    "",
    "Barra",
    "Fiesta",
    "Persecucion",
    "Onda",
    "Respiracion",
    "Estrobo"
};

const char *music_fx_name(music_fx_t fx)
{
    if (fx >= MUSIC_FX_COUNT) return "";
    return kFxNames[fx];
}

static void hsv_to_rgb(uint8_t hue, uint8_t sat, uint8_t val, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t region = hue / 43;
    uint8_t remainder = (hue - (region * 43)) * 6;
    uint8_t p = (val * (255 - sat)) >> 8;
    uint8_t q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
    uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;
    switch (region) {
        case 0: *r = val; *g = t; *b = p; break;
        case 1: *r = q; *g = val; *b = p; break;
        case 2: *r = p; *g = val; *b = t; break;
        case 3: *r = p; *g = q; *b = val; break;
        case 4: *r = t; *g = p; *b = val; break;
        default: *r = val; *g = p; *b = q; break;
    }
}

static float music_target_level(void)
{
    return (float)audio_input_get_music_level();
}

static void music_smooth_level(void)
{
    const float target = music_target_level();
    if (target > s_smoothLevel) {
        s_smoothLevel += (target - s_smoothLevel) * MUSIC_ATTACK;
    } else {
        s_smoothLevel += (target - s_smoothLevel) * MUSIC_RELEASE;
    }
    if (s_smoothLevel < 0.5f) {
        s_smoothLevel = 0.0f;
    }
}

static void set_pixel_rgb(uint8_t *rgb, uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (idx >= LED_COUNT) return;
    rgb[idx * 3] = r;
    rgb[idx * 3 + 1] = g;
    rgb[idx * 3 + 2] = b;
}

static void fx_bar(lamp_state_t *state, uint8_t *rgb)
{
    const uint8_t baseHue = (uint8_t)((state->hue * 255U) / 360U);
    const uint8_t sat = state->saturation;

    int totalFill = (int)((s_smoothLevel * (float)LED_COUNT) / (float)MUSIC_LEVEL_MAX);
    if (totalFill > (int)LED_COUNT) totalFill = (int)LED_COUNT;
    if (totalFill < 0) totalFill = 0;

    for (int idx = 0; idx < totalFill; idx++) {
        const uint8_t val = (uint8_t)((state->brightness * (idx + 1)) / LED_COUNT);
        uint8_t r, g, b;
        const uint8_t hue = (uint8_t)(baseHue + (uint8_t)((idx * 60) / LED_COUNT));
        hsv_to_rgb(hue, sat, val, &r, &g, &b);
        set_pixel_rgb(rgb, (uint16_t)idx, r, g, b);
    }
}

static void fx_party(lamp_state_t *state, uint8_t *rgb)
{
    audio_levels_t levels;
    if (!audio_input_get_levels(&levels)) return;

    const int soundLevel = audio_input_get_music_level();

    float maxBand = (float)soundLevel;
    if (maxBand < 1.0f) maxBand = 1.0f;

    for (int i = 0; i < AUDIO_BAND_COUNT; i++) {
        if (levels.bands[i] > maxBand) maxBand = levels.bands[i];
    }

    const uint16_t ledsPerBand = LEDS_PER_LEVEL / AUDIO_BAND_COUNT;
    const uint16_t bandRemainder = LEDS_PER_LEVEL % AUDIO_BAND_COUNT;

    for (int level = 0; level < LED_LEVEL_COUNT; level++) {
        const uint16_t base = led_level_start((uint8_t)level);
        uint16_t ringOffset = 0;

        for (int band = 0; band < AUDIO_BAND_COUNT; band++) {
            const uint16_t bandLen = ledsPerBand + ((band < (int)bandRemainder) ? 1U : 0U);
            float norm = levels.bands[band] / maxBand;
            if (norm > 1.0f) norm = 1.0f;
            if (norm < 0.05f && soundLevel > 0) {
                norm = (float)soundLevel / maxBand;
            }

            uint8_t hue = (uint8_t)((band * 255) / AUDIO_BAND_COUNT);
            uint8_t val = (uint8_t)(norm * state->brightness);
            if (val < 4 && soundLevel > 0) {
                val = (uint8_t)min((int)state->brightness, soundLevel / 2);
            }

            uint8_t r, g, b;
            hsv_to_rgb(hue, 255, val, &r, &g, &b);

            for (uint16_t i = 0; i < bandLen; i++) {
                set_pixel_rgb(rgb, base + ringOffset + i, r, g, b);
            }
            ringOffset += bandLen;
        }
    }
}

static void fx_chase(lamp_state_t *state, uint8_t *rgb)
{
    const uint8_t baseHue = (uint8_t)((state->hue * 255U) / 360U);
    const uint8_t sat = state->saturation;
    const float speed = 0.35f + (s_smoothLevel * 0.004f);

    s_chasePos += speed;
    while (s_chasePos >= (float)LED_COUNT) s_chasePos -= (float)LED_COUNT;
    while (s_chasePos < 0.0f) s_chasePos += (float)LED_COUNT;

    const int head = (int)s_chasePos;
    int tailLen = 4 + (int)(s_smoothLevel / 40.0f);
    if (tailLen > 14) tailLen = 14;

    for (int dist = 0; dist <= tailLen; dist++) {
        int idx = head - dist;
        while (idx < 0) idx += LED_COUNT;
        idx %= LED_COUNT;

        const uint8_t val = (uint8_t)((state->brightness * (tailLen - dist + 1)) / (tailLen + 1));
        uint8_t r, g, b;
        hsv_to_rgb((uint8_t)(baseHue + (uint8_t)(dist * 8)), sat, val, &r, &g, &b);
        set_pixel_rgb(rgb, (uint16_t)idx, r, g, b);
    }
}

static void fx_wave(lamp_state_t *state, uint8_t *rgb)
{
    const uint8_t baseHue = (uint8_t)((state->hue * 255U) / 360U);
    const uint8_t sat = state->saturation;
    const float amp = 0.25f + (s_smoothLevel / MUSIC_LEVEL_MAX);

    s_wavePhase += 0.12f + (s_smoothLevel * 0.0004f);

    for (uint16_t idx = 0; idx < LED_COUNT; idx++) {
        const float w = sinf(((float)idx * 0.22f) + s_wavePhase);
        const float norm = (w + 1.0f) * 0.5f;
        const uint8_t val = (uint8_t)(norm * amp * (float)state->brightness);
        uint8_t r, g, b;
        hsv_to_rgb((uint8_t)(baseHue + (uint8_t)((idx * 80) / LED_COUNT)), sat, val, &r, &g, &b);
        set_pixel_rgb(rgb, idx, r, g, b);
    }
}

static void fx_breath(lamp_state_t *state, uint8_t *rgb)
{
    const uint8_t baseHue = (uint8_t)((state->hue * 255U) / 360U);
    const uint8_t sat = state->saturation;

    s_breathPhase += 0.05f;
    const float breath = (sinf(s_breathPhase) + 1.0f) * 0.5f;
    const float audio = s_smoothLevel / MUSIC_LEVEL_MAX;
    const uint8_t val = (uint8_t)(breath * (0.25f + 0.75f * audio) * (float)state->brightness);

    uint8_t r, g, b;
    hsv_to_rgb(baseHue, sat, val, &r, &g, &b);
    for (uint16_t idx = 0; idx < LED_COUNT; idx++) {
        set_pixel_rgb(rgb, idx, r, g, b);
    }
}

static void fx_strobe(lamp_state_t *state, uint8_t *rgb)
{
    const uint8_t baseHue = (uint8_t)((state->hue * 255U) / 360U);
    const uint8_t sat = state->saturation;
    const uint32_t now = millis();

    if (s_smoothLevel > 35.0f) {
        s_strobeOn = true;
        s_strobeUntilMs = now + 60U;
    } else if (now > s_strobeUntilMs) {
        s_strobeOn = false;
    }

    if (s_strobeOn) {
        uint8_t r, g, b;
        hsv_to_rgb(baseHue, sat, state->brightness, &r, &g, &b);
        for (uint16_t idx = 0; idx < LED_COUNT; idx++) {
            set_pixel_rgb(rgb, idx, r, g, b);
        }
    }
}

void music_effects_reset(void)
{
    s_smoothLevel = 0.0f;
    s_chasePos = 0.0f;
    s_wavePhase = 0.0f;
    s_breathPhase = 0.0f;
    s_strobeOn = false;
    s_strobeUntilMs = 0;
}

void music_effects_update(lamp_state_t *state)
{
    if (!state || state->musicFx == MUSIC_FX_NONE || !state->power) return;
    if (!audio_input_is_running()) return;

    music_smooth_level();

    uint8_t rgb[LED_COUNT * 3];
    memset(rgb, 0, sizeof(rgb));

    switch (state->musicFx) {
        case MUSIC_FX_BAR:    fx_bar(state, rgb); break;
        case MUSIC_FX_PARTY:  fx_party(state, rgb); break;
        case MUSIC_FX_CHASE:  fx_chase(state, rgb); break;
        case MUSIC_FX_WAVE:   fx_wave(state, rgb); break;
        case MUSIC_FX_BREATH: fx_breath(state, rgb); break;
        case MUSIC_FX_STROBE: fx_strobe(state, rgb); break;
        default: return;
    }

    led_controller_set_party_pixels(rgb, LED_COUNT);
}
