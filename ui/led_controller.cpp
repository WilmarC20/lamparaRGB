#include "music_effects.h"
#include "led_controller.h"
#include "led_calib.h"
#include "config.h"
#include "pins.h"
#include <Arduino.h>
#include <string.h>

#if ENABLE_LED_STRIP

#include <WS2812FX.h>

static WS2812FX ws2812fx(LED_COUNT, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);
static bool partyActive = false;
static bool outputPaused = false;
static bool s_staticColombia = false;
static bool calibActive = false;
static uint16_t calibIndex = 0;
static uint32_t solidColor = 0xFF0000;
static uint16_t effectSpeed = 200;
static bool effectReverse = false;

static const char *kEffectNames[] = {
    "Solido",
    "Arcoiris",
    "Persecucion",
    "Onda",
    "Respiracion",
    "Estrobo"
};

static const uint8_t kEffectModes[] = {
    FX_MODE_STATIC,
    FX_MODE_RAINBOW_CYCLE,
    FX_MODE_THEATER_CHASE_RAINBOW,
    FX_MODE_RUNNING_LIGHTS,
    FX_MODE_BREATH,
    FX_MODE_STROBE
};

const char *led_effect_name(lamp_effect_t effect)
{
    if (effect >= LAMP_EFFECT_COUNT) return "Solido";
    return kEffectNames[effect];
}

static uint32_t hsv_to_rgb32(uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t r, g, b;
    uint8_t region = hue / 43;
    uint8_t remainder = (hue - (region * 43)) * 6;
    uint8_t p = (val * (255 - sat)) >> 8;
    uint8_t q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
    uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;
    switch (region) {
        case 0: r = val; g = t; b = p; break;
        case 1: r = q; g = val; b = p; break;
        case 2: r = p; g = val; b = t; break;
        case 3: r = p; g = q; b = val; break;
        case 4: r = t; g = p; b = val; break;
        default: r = val; g = p; b = q; break;
    }
    return ws2812fx.Color(r, g, b);
}

void led_controller_init(void)
{
    ws2812fx.init();
    ws2812fx.setBrightness(128);
    ws2812fx.setColor(hsv_to_rgb32(0, 255, 255));
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.start();
}

void led_controller_set_output_paused(bool paused)
{
    outputPaused = paused;
}

bool led_controller_output_paused(void)
{
    return outputPaused;
}

static void led_all_off(void)
{
    partyActive = false;
    s_staticColombia = false;
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.setColor(0);
    ws2812fx.setBrightness(0);
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        ws2812fx.setPixelColor(i, 0);
    }
    ws2812fx.show();
}

void led_controller_apply(lamp_state_t *state)
{
    if (calibActive) {
        state->dirty = false;
        return;
    }
    if (outputPaused) {
        state->dirty = false;
        return;
    }
    if (!state->power) {
        led_all_off();
        state->dirty = false;
        return;
    }
    if (lamp_state_music_active(state)) {
        s_staticColombia = false;
        partyActive = true;
        ws2812fx.setBrightness(state->brightness);
        state->dirty = false;
        return;
    }

    partyActive = false;
    ws2812fx.setBrightness(state->brightness);

    if (!state->musicMode && state->musicFx == MUSIC_FX_WAVE) {
        if (!s_staticColombia) {
            ws2812fx.stop();
            ws2812fx.setMode(FX_MODE_STATIC);
            s_staticColombia = true;
        }
        uint8_t rgb[LED_COUNT * 3];
        memset(rgb, 0, sizeof(rgb));
        music_effects_colombia_static(state, rgb);
        for (uint16_t i = 0; i < LED_COUNT; i++) {
            ws2812fx.setPixelColor(i,
                                   ws2812fx.Color(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]));
        }
        ws2812fx.show();
        state->dirty = false;
        return;
    }

    s_staticColombia = false;
    ws2812fx.start();
    lamp_effect_t eff = state->effect;
    if (!state->musicMode && state->musicFx != MUSIC_FX_NONE && state->musicFx != MUSIC_FX_WAVE) {
        eff = music_fx_static_lamp_effect(state->musicFx);
    }
    if (eff >= LAMP_EFFECT_COUNT) {
        eff = LAMP_EFFECT_SOLID;
    }

    solidColor = hsv_to_rgb32(state->hue, state->saturation, 255);
    ws2812fx.setMode(kEffectModes[eff]);
    ws2812fx.setColor(solidColor);
    ws2812fx.setSpeed(effectSpeed);
    ws2812fx.setOptions(0, effectReverse ? REVERSE : NO_OPTIONS);
    state->dirty = false;
}

void led_controller_set_speed(uint16_t speed)
{
    if (speed < 10) speed = 10;
    effectSpeed = speed;
    ws2812fx.setSpeed(speed);
}

void led_controller_set_reverse(bool reverse)
{
    effectReverse = reverse;
}

uint16_t led_controller_get_speed(void)
{
    return effectSpeed;
}

bool led_controller_get_reverse(void)
{
    return effectReverse;
}

void led_controller_service(void)
{
    if (outputPaused || calibActive || s_staticColombia) return;
    if (!partyActive) {
        ws2812fx.service();
    }
}

void led_controller_set_party_pixels(const uint8_t *rgb, uint16_t count)
{
    if (outputPaused || calibActive || !partyActive) return;
    if (count > LED_COUNT) count = LED_COUNT;
    for (uint16_t i = 0; i < count; i++) {
        ws2812fx.setPixelColor(i, ws2812fx.Color(rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]));
    }
    ws2812fx.show();
}

static void led_calib_render(void)
{
    ws2812fx.setBrightness(255);
    ws2812fx.setMode(FX_MODE_STATIC);
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        ws2812fx.setPixelColor(i, (i == calibIndex) ? 0xFFFFFF : 0);
    }
    ws2812fx.show();
}

extern "C" {

bool led_calib_is_active(void)
{
    return calibActive;
}

void led_calib_set_active(bool active)
{
    calibActive = active;
    if (active) {
        partyActive = false;
        calibIndex = 0;
        led_calib_render();
    }
}

uint16_t led_calib_get_index(void)
{
    return calibIndex;
}

uint16_t led_calib_get_count(void)
{
    return LED_COUNT;
}

bool led_calib_step(int delta)
{
    if (!calibActive) return false;
    int next = (int)calibIndex + delta;
    if (next < 0) next = 0;
    if (next >= (int)LED_COUNT) next = (int)LED_COUNT - 1;
    if (next == (int)calibIndex) return false;
    calibIndex = (uint16_t)next;
    led_calib_render();
    return true;
}

void led_calib_set_index(uint16_t idx)
{
    if (!calibActive) return;
    if (idx >= LED_COUNT) idx = LED_COUNT - 1;
    calibIndex = idx;
    led_calib_render();
}

} /* extern "C" */

#else /* !ENABLE_LED_STRIP */

const char *led_effect_name(lamp_effect_t effect)
{
    (void)effect;
    return "Solido";
}

void led_controller_init(void) {}
void led_controller_set_output_paused(bool paused) { (void)paused; }
bool led_controller_output_paused(void) { return true; }
void led_controller_apply(lamp_state_t *state) { if (state) state->dirty = false; }
void led_controller_set_speed(uint16_t speed) { (void)speed; }
void led_controller_set_reverse(bool reverse) { (void)reverse; }
uint16_t led_controller_get_speed(void) { return 200; }
bool led_controller_get_reverse(void) { return false; }
void led_controller_service(void) {}
void led_controller_set_party_pixels(const uint8_t *rgb, uint16_t count)
{
    (void)rgb;
    (void)count;
}

extern "C" {

bool led_calib_is_active(void) { return false; }
void led_calib_set_active(bool active) { (void)active; }
uint16_t led_calib_get_index(void) { return 0; }
uint16_t led_calib_get_count(void) { return LED_COUNT; }
bool led_calib_step(int delta) { (void)delta; return false; }
void led_calib_set_index(uint16_t idx) { (void)idx; }

} /* extern "C" */

#endif
