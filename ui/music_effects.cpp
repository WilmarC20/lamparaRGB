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
static float s_breathPhase = 0.0f;
static bool s_strobeOn = false;
static uint32_t s_strobeUntilMs = 0;

static const char *kFxNames[] = {
    "",
    "Barra",
    "Fiesta",
    "Persecucion",
    "Colombia",
    "Respiracion",
    "Estrobo",
    "Solido"
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

/* Bandera Colombia — franjas por indice (90 LEDs espiral):
 *   LED  0-29  rojo mas oscuro  (30 LEDs)
 *   LED 30-59  azul oscuro      (30 LEDs)
 *   LED 60-89  amarillo         (30 LEDs)
 * Efecto: onda + audio (amarillo=volumen, azul=agudos, rojo=graves+beat)
 */
#define FLAG_IDX_RED_END    30U
#define FLAG_IDX_BLUE_END   60U
#define FLAG_BEAT_FLASH_MS  220

#define FLAG_RGB_YELLOW_R  252U
#define FLAG_RGB_YELLOW_G  209U
#define FLAG_RGB_YELLOW_B   22U
#define FLAG_RGB_BLUE_R      0U
#define FLAG_RGB_BLUE_G     42U
#define FLAG_RGB_BLUE_B    118U
#define FLAG_RGB_RED_R      45U
#define FLAG_RGB_RED_G       2U
#define FLAG_RGB_RED_B       6U

typedef enum {
    FLAG_ZONE_YELLOW = 0,
    FLAG_ZONE_BLUE,
    FLAG_ZONE_RED,
} flag_zone_t;

static float s_flagPhase = 0.0f;
static float s_flagBassSm = 0.0f;
static float s_flagHighSm = 0.0f;
static uint32_t s_flagBeatSeen = 0;
static uint32_t s_flagFlashMs = 0;

static float flag_norm01(float norm)
{
    float n = norm / (float)MUSIC_LEVEL_MAX;
    if (n < 0.0f) {
        n = 0.0f;
    }
    if (n > 1.0f) {
        n = 1.0f;
    }
    return n;
}

/* Curva agresiva: sonido bajo casi apagado, sube fuerte con el nivel */
static float flag_dyn(float n)
{
    if (n <= 0.0f) {
        return 0.0f;
    }
    return powf(n, 2.6f);
}

static flag_zone_t flag_zone_of(uint16_t idx)
{
    if (idx >= FLAG_IDX_BLUE_END) {
        return FLAG_ZONE_YELLOW;
    }
    if (idx >= FLAG_IDX_RED_END) {
        return FLAG_ZONE_BLUE;
    }
    return FLAG_ZONE_RED;
}

static void flag_base_rgb(flag_zone_t zone, uint8_t *r, uint8_t *g, uint8_t *b)
{
    switch (zone) {
        case FLAG_ZONE_YELLOW:
            *r = FLAG_RGB_YELLOW_R;
            *g = FLAG_RGB_YELLOW_G;
            *b = FLAG_RGB_YELLOW_B;
            break;
        case FLAG_ZONE_BLUE:
            *r = FLAG_RGB_BLUE_R;
            *g = FLAG_RGB_BLUE_G;
            *b = FLAG_RGB_BLUE_B;
            break;
        default:
            *r = FLAG_RGB_RED_R;
            *g = FLAG_RGB_RED_G;
            *b = FLAG_RGB_RED_B;
            break;
    }
}

static void fx_flag_colombia(lamp_state_t *state, uint8_t *rgb)
{
    const float tgtBass = (float)audio_input_get_norm_bass();
    const float tgtHigh = (float)audio_input_get_norm_high();

    s_flagBassSm += (tgtBass - s_flagBassSm) * (tgtBass > s_flagBassSm ? 0.65f : 0.12f);
    s_flagHighSm += (tgtHigh - s_flagHighSm) * (tgtHigh > s_flagHighSm ? 0.60f : 0.12f);

    s_flagPhase += 0.03f + (flag_dyn(flag_norm01(s_smoothLevel)) * 0.16f);
    if (s_flagPhase > 6.28318f) {
        s_flagPhase -= 6.28318f;
    }

    const uint32_t beats = audio_input_get_beat_count();
    const uint32_t now = millis();
    if (beats != s_flagBeatSeen) {
        s_flagBeatSeen = beats;
        s_flagFlashMs = now;
    }
    float beatFlash = 0.0f;
    if (s_flagFlashMs != 0U) {
        const uint32_t since = now - s_flagFlashMs;
        if (since < FLAG_BEAT_FLASH_MS) {
            beatFlash = 1.0f - ((float)since / (float)FLAG_BEAT_FLASH_MS);
        }
    }

    const uint8_t maxV = state->brightness > 0 ? state->brightness : 1;
    const float lvlN = flag_norm01(s_smoothLevel);
    const float bassN = flag_norm01(s_flagBassSm);
    const float highN = flag_norm01(s_flagHighSm);
    const float lvlD = flag_dyn(lvlN);
    const float bassD = flag_dyn(bassN);
    const float highD = flag_dyn(highN);
    const float masterD = flag_dyn(fmaxf(lvlN, fmaxf(bassN, highN)));
    const float waveMix = 0.08f + 0.92f * masterD;

    for (uint16_t i = 0; i < LED_COUNT; i++) {
        const flag_zone_t zone = flag_zone_of(i);
        uint8_t r0, g0, b0;
        flag_base_rgb(zone, &r0, &g0, &b0);

        const float wave = 0.5f + 0.5f * sinf(s_flagPhase + (float)i * 0.28f);
        float energy = 0.02f + (0.05f * masterD);

        if (zone == FLAG_ZONE_YELLOW) {
            energy += 0.95f * lvlD * (0.05f + 0.95f * wave * waveMix);
        } else if (zone == FLAG_ZONE_BLUE) {
            energy += 0.95f * highD * (0.05f + 0.95f * wave * waveMix);
        } else {
            energy += 0.95f * bassD * (0.05f + 0.95f * wave * waveMix);
            energy += 0.75f * beatFlash * bassD;
        }

        energy *= 0.12f + (0.88f * masterD);

        if (energy > 1.0f) {
            energy = 1.0f;
        }

        const uint8_t br = (uint8_t)((float)maxV * energy);
        set_pixel_rgb(rgb, i,
                      (uint8_t)(((uint16_t)r0 * br) / maxV),
                      (uint8_t)(((uint16_t)g0 * br) / maxV),
                      (uint8_t)(((uint16_t)b0 * br) / maxV));
    }
}

static void fx_solid_music(lamp_state_t *state, uint8_t *rgb)
{
    music_smooth_level();
    float lvl = s_smoothLevel / (float)MUSIC_LEVEL_MAX;
    if (lvl < 0.0f) {
        lvl = 0.0f;
    }
    if (lvl > 1.0f) {
        lvl = 1.0f;
    }
    lvl = powf(lvl, 1.8f);

    const uint8_t baseHue = (uint8_t)((state->hue * 255U) / 360U);
    const uint8_t sat = state->saturation;
    const uint8_t val = (uint8_t)((float)state->brightness * (0.10f + 0.90f * lvl));

    uint8_t r, g, b;
    hsv_to_rgb(baseHue, sat, val, &r, &g, &b);
    for (uint16_t idx = 0; idx < LED_COUNT; idx++) {
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
    s_breathPhase = 0.0f;
    s_strobeOn = false;
    s_strobeUntilMs = 0;
    s_flagPhase = 0.0f;
    s_flagBassSm = 0.0f;
    s_flagHighSm = 0.0f;
    s_flagBeatSeen = audio_input_get_beat_count();
    s_flagFlashMs = 0;
}

void music_effects_update(lamp_state_t *state)
{
    if (!state || state->musicFx == MUSIC_FX_NONE || !state->power) return;
    if (!audio_input_is_running() && !audio_input_external_active()) return;

    music_smooth_level();

    uint8_t rgb[LED_COUNT * 3];
    memset(rgb, 0, sizeof(rgb));

    switch (state->musicFx) {
        case MUSIC_FX_BAR:    fx_bar(state, rgb); break;
        case MUSIC_FX_PARTY:  fx_party(state, rgb); break;
        case MUSIC_FX_CHASE:  fx_chase(state, rgb); break;
        case MUSIC_FX_WAVE:   fx_flag_colombia(state, rgb); break;
        case MUSIC_FX_BREATH: fx_breath(state, rgb); break;
        case MUSIC_FX_STROBE: fx_strobe(state, rgb); break;
        case MUSIC_FX_SOLID:  fx_solid_music(state, rgb); break;
        default: return;
    }

    led_controller_set_party_pixels(rgb, LED_COUNT);
}

void music_effects_colombia_static(const lamp_state_t *state, uint8_t *rgb)
{
    if (!state || !rgb) {
        return;
    }
    (void)state;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        const flag_zone_t zone = flag_zone_of(i);
        uint8_t r0, g0, b0;
        flag_base_rgb(zone, &r0, &g0, &b0);
        set_pixel_rgb(rgb, i, r0, g0, b0);
    }
}

lamp_effect_t music_fx_static_lamp_effect(music_fx_t fx)
{
    switch (fx) {
        case MUSIC_FX_PARTY:
            return LAMP_EFFECT_RAINBOW;
        case MUSIC_FX_CHASE:
            return LAMP_EFFECT_CHASE;
        case MUSIC_FX_BREATH:
            return LAMP_EFFECT_BREATH;
        case MUSIC_FX_STROBE:
            return LAMP_EFFECT_STROBE;
        case MUSIC_FX_BAR:
        default:
            return LAMP_EFFECT_SOLID;
    }
}
