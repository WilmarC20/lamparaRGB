# Librerías y estructura — LamparaV3

## Sketchbook

```
H:\Programacion\Arduino\lamparaV3
```

El IDE debe usar este sketchbook para que `libraries/` del repo tenga prioridad sobre copias globales (LAMPARAV2, Espacio1, etc.).

## Librerías en `libraries/` (solo las que compila el firmware)

| Librería | Uso | Notas |
|----------|-----|--------|
| **lvgl** 8.3.11 | UI táctil LVGL | Config en `ui/lv_conf.h` |
| **TFT_eSPI** 2.5.43 | Display ILI9341 320×240 | `User_Setup.h` para ESP32-2432S024C |
| **WS2812FX** 1.4.6 | Tira WS2812 (90 LEDs) | Requiere **Adafruit_NeoPixel** (IDE global) |
| **minimp3** | Radio online MP3 | Header-only; decode en `radio_player.cpp` |

### Del core ESP32 (Arduino15, no en repo)

RainMaker, WiFiProv, WiFi, Wire, Preferences, RMT, DAC HAL.

### Eliminadas / no usar

| Carpeta | Motivo |
|---------|--------|
| ~~ESP8266Audio~~ | Sustituida por minimp3 (menos IRAM) |
| ~~ESP32-audioI2S~~ | Eliminada |
| ~~ui/libraries/~~ | Eliminada; era duplicado de `lv_conf.h` |

## minimp3 (radio)

Decoder MP3 en un solo header. La implementacion vive en `radio_mp3.cpp` (`MINIMP3_IMPLEMENTATION`).
En `minimp3.h`: `mp3dec_scratch_t` es **static** dentro de `mp3dec_decode_frame` (~16 KB; no cabe en stack de tarea).
Stream HTTP via `NetworkClient` (GET manual + redirects; sin HTTPClient para ahorrar IRAM). Salida `dacWrite` GPIO 26.

## Particiones flash (RainMaker + BLE)

RainMaker necesita partición **`fctry`**. Archivo: **`ui/partitions.csv`**.

| Herramienta | Valor |
|-------------|--------|
| Board | ESP32 Dev Module |
| Flash Size | **4MB** |
| Partition Scheme | **Custom** |

Tras cambiar particiones: **Erase All Flash** y volver a subir.

Ver también **`ui/PROV_ESTADO.md`** para provisioning BLE/QR.

## `build_opt.h` (IRAM / RAM)

Flags en `ui/build_opt.h`:

- WiFi sin optimización IRAM (`CONFIG_ESP_WIFI_IRAM_OPT=0`) — libera IRAM para radio + LVGL
- BLE en flash cuando el core lo permite
- Buffers WiFi mínimos, logs desactivados

## Estructura del sketch `ui/`

```
ui/
├── ui.ino                 Entrada Arduino
├── config.h               Flags de compilación
├── build_opt.h            Flags linker/compilador
├── lv_conf.h              Config LVGL
├── partitions.csv         Tabla flash RainMaker
│
├── ui_app.cpp/h           Orquestador (setup/loop UI)
├── lamp_state.c/h         Estado lámpara
├── display.cpp/h          TFT + LVGL flush
├── touch_cst820.*         Touch I2C
├── CST820.*               Driver touch
│
├── ui_init.c              Init pantallas
├── lampara_ui.h           Globals LVGL
├── ui_control_screen.c/h  Tabs COLOR / EFX / RADIO / AJUSTES
├── ui_config_screen.c/h   Contenido tab Ajustes
├── ui_radio_screen.c/h    Tab radio Colombia
├── ui_effects.c/h         Catálogo efectos
├── ui_theme.h             Estilos UI
├── ui_settings.cpp/h      Timer, modo noche
├── ui_prov_screen.c/h     Prov LVGL (legacy si RM_PROV_UI_NO_LVGL=0)
├── ui_prov_raw.cpp/h      Prov TFT directo (activo)
├── ui_qr_bitmap.c/h       QR provisioning
│
├── led_controller.cpp/h   WS2812FX
├── led_calib.h              Calibración LEDs
├── music_effects.cpp/h    Efectos reactivos al mic
├── audio_input.cpp/h      Micrófono ADC
├── audio_output.cpp/h     Prueba tono altavoz
├── radio_player.cpp/h     Streaming emisoras CO
├── rainmaker_app.cpp/h    RainMaker + WiFiProv
├── ir_control.cpp/h       Mando IR
├── IRControlLite.cpp/h    RMT IR
│
├── pins.h, led_layout.h, lamp_log.h, music_fx.h
├── LIBRERIAS.md           Este archivo
└── PROV_ESTADO.md         Depuración prov
```

## Referencia de diseño (no compilada)

`SquareLineStudio/` — proyecto SquareLine exportado; la UI activa está reescrita en `ui_*`.

## lv_conf.h

Solo **`ui/lv_conf.h`**. `LV_MEM_CUSTOM=1` (heap, no buffer estático de 64 KB).

`ui.ino` comprueba en compile-time que `LV_MEM_CUSTOM==1`.
