# Lámpara V3 — Control inteligente WS2812 con ESP32-2432S024C

**Proyecto:** [EpicFilm.co](https://epicfilm.co/)

Controlador de tira LED multicolor basado en la placa **ESP32-2432S024C** (pantalla táctil capacitiva 2,4″, 320×240). Incluye interfaz gráfica con pestañas (Color, Efectos, Ajustes), rueda cromática, presets de color, brillo, modos musicales con micrófono, calibración de LEDs, control por mando IR e integración **ESP RainMaker** para Google Home y Alexa.

**Stack:** Arduino IDE · ESP32 · LVGL 8 · TFT_eSPI · WS2812FX · ESP RainMaker

**Hardware:** pantalla ILI9341 + touch CST820 · WS2812 (90 LEDs) · micrófono analógico (MAX9814) · receptor IR

---

## Características

- **Interfaz táctil LVGL** optimizada para 320×240 px, tema oscuro con acento rojo
- **Pestaña COLOR:** rueda cromática, presets (frío / neutro / cálido + colores), brillo, modo fiesta y encendido
- **Pestaña EFECTOS:** lista de 12 efectos (sólido, arcoíris, persecución, onda, respiración, estrobo + variantes musicales), velocidad y dirección
- **Pestaña AJUSTES:** WiFi / RainMaker, brillo global, prueba de micrófono, calibración de LEDs, información de firmware
- **Audio reactivo:** modos musicales que responden al micrófono
- **Control IR** con mando a distancia
- **ESP RainMaker:** control remoto desde app móvil, Google Home y Amazon Alexa
- **Provisioning bajo demanda:** el QR de WiFi solo aparece al entrar manualmente en **Ajustes → WiFi** (no al arrancar)

---

## Requisitos

| Componente | Versión / notas |
|------------|-----------------|
| Arduino IDE | 2.x |
| ESP32 board package | ≥ 3.0.0 (incluye RainMaker y WiFiProv) |
| Placa | ESP32-2432S024**C** (touch capacitivo CST820) |
| Flash | 4 MB con partición `fctry` (RainMaker) |

### Abrir el proyecto

1. Clona este repositorio.
2. En Arduino IDE, configura el **Sketchbook** apuntando a la carpeta del repo (o abre directamente `ui/ui.ino`).
3. Las librerías prioritarias están en `ui/libraries/` y `libraries/` del repo.

> **Importante:** si el IDE encuentra librerías de otros proyectos (por ejemplo otro `lv_conf.h` con pool estático de 64 KB), la compilación puede fallar por falta de RAM. Consulta `ui/LIBRERIAS.md`.

---

## Configuración Arduino IDE

| Opción | Valor |
|--------|-------|
| Board | ESP32 Dev Module |
| Flash Size | 4 MB |
| Partition Scheme | **Custom** (`ui/partitions.csv`) o **RainMaker 4MB No OTA** |
| Erase All Flash Before Sketch Upload | **Enabled** (tras cambiar particiones) |
| CPU Frequency | 240 MHz |
| PSRAM | Disabled |

Tras cambiar el esquema de particiones, borra la flash una vez antes de flashear.

---

## Hardware y cableado

Referencia de la placa: [NoosaHydro/2.4inch_ESP32-2432S024](https://github.com/NoosaHydro/2.4inch_ESP32-2432S024) — demo `1_2_Factory_samples_Capacitive_touch`.

### Pantalla ESP32-2432S024C (integrada)

| Función | GPIO |
|---------|------|
| TFT MISO | 12 |
| TFT MOSI | 13 |
| TFT SCLK | 14 |
| TFT CS | 15 |
| TFT DC | 2 |
| Backlight | 27 |
| Touch SDA (CST820) | 33 |
| Touch SCL | 32 |
| Touch INT | 21 |
| Touch RST | 25 |

> La variante **2432S024** sin **C** usa touch resistivo XPT2046; los pines son distintos.

### Tira WS2812 (90 LEDs)

| WS2812 | ESP32 |
|--------|-------|
| DIN | GPIO **1** (P1 TXD) |
| VCC | 5 V (fuente externa recomendada) |
| GND | GND común |

### Micrófono MAX9814 (analógico)

| MAX9814 | ESP32 |
|---------|-------|
| VDD | 3.3 V |
| GND | GND |
| OUT | GPIO **35** (P3) |

### Receptor IR (TSOP4838)

| IR | ESP32 |
|----|-------|
| OUT | GPIO **22** (CN1) |

Detalle de conectores JST y layout de LEDs en `ui/pins.h` y `ui/led_layout.h`.

---

## Uso de la interfaz

Tras flashear, la lámpara arranca en la pestaña **COLOR**:

| Pestaña | Contenido |
|---------|-----------|
| **COLOR** | Rueda cromática, presets, brillo, modo fiesta, encendido |
| **EFECTOS** | Selección de efecto, velocidad, dirección |
| **AJUSTES** | WiFi, brillo global, micrófono, calibración LED, info |

La barra inferior permite cambiar de pestaña. El icono de engranaje en la cabecera abre **AJUSTES**.

### Efectos disponibles

- Sólido, Arcoíris, Persecución, Onda, Respiración, Estrobo
- Música: Barra, Fiesta, Persecución, Onda, Respiración, Estrobo

---

## ESP RainMaker (Google Home / Alexa)

### Provisioning WiFi (primera vez)

1. Instala la app **ESP RainMaker** (Android / iOS).
2. En la lámpara: **AJUSTES → WiFi**.
3. El dispositivo reinicia a la pantalla de provisioning con **QR en pantalla**.
4. Escanea el QR con la app RainMaker y sigue el asistente.
5. Tras conectar, el nodo **Luz LED** aparece en la app con: encendido, brillo, matiz, saturación, efecto y modo fiesta.

> El QR **no** se muestra al arrancar. Solo cuando entras manualmente en la opción WiFi de Ajustes. Si ya hay credenciales guardadas, el arranque normal intenta conectar sin activar BLE.

**Credenciales BLE por defecto** (configurables en `ui/config.h`):

- Servicio: `PROV_ESP32`
- PoP: `12345678`

### Google Home

1. Google Home → **+** → Configurar dispositivo → **Funciona con Google**
2. Busca **ESP RainMaker** e inicia sesión con la misma cuenta
3. Ejemplos: *"Ok Google, enciende la Luz LED"*, *"pon la Luz LED al 50%"*

### Amazon Alexa

1. Habilita la skill **ESP RainMaker**
2. Vincula tu cuenta RainMaker
3. Descubre dispositivos y controla encendido, brillo y color

Documentación adicional de provisioning: `ui/PROV_ESTADO.md`

---

## Depuración

### Compilar sin RainMaker

En `ui/config.h`:

```c
#define ENABLE_RAINMAKER 0
```

Compila solo UI + LEDs + audio, sin WiFi/BLE.

### Monitor serial

Con `LAMP_SERIAL_LOG 1` en `ui/config.h`, a 115200 baud deberías ver en arranque normal:

```
LamparaV3 iniciando... heap=...
RM setup heap=...
RM: sin WiFi guardado (prov desde Ajustes)   ← o conexión si ya hay WiFi
Display OK heap=...
UI init heap=...
UI lista heap=...
Setup OK heap=...
```

### Problemas de RAM

- Usa las librerías del repo (`ui/libraries/`, `libraries/`).
- Verifica `LV_MEM_CUSTOM 1` en `ui/lv_conf.h`.
- Revisa `ui/build_opt.h` (buffers WiFi/BLE reducidos).
- Consulta `ui/LIBRERIAS.md`.

---

## Estructura del repositorio

```
lamparaV3/
├── ui/                    Sketch principal (ui.ino)
│   ├── ui_control_screen.c   UI con pestañas COLOR / EFECTOS / AJUSTES
│   ├── ui_config_screen.c    Panel de ajustes
│   ├── rainmaker_app.cpp     Integración ESP RainMaker
│   ├── led_controller.cpp    Control WS2812FX
│   ├── config.h              Opciones de compilación
│   ├── partitions.csv        Particiones (incluye fctry)
│   └── lv_conf.h             Configuración LVGL
├── libraries/             lvgl, TFT_eSPI, WS2812FX
└── SquareLineStudio/      Proyecto SquareLine (referencia UI)
```

---

## Configuración principal (`ui/config.h`)

| Define | Descripción |
|--------|-------------|
| `ENABLE_RAINMAKER` | `1` = cloud activo, `0` = solo local |
| `RM_AUTO_PROVISION_ON_BOOT` | `0` = no QR/BLE al arrancar (recomendado) |
| `LED_COUNT` | Número de LEDs (en `ui/pins.h`, actualmente 90) |
| `LAMP_SERIAL_LOG` | Logs en monitor serial |
| `AUDIO_START_ON_PARTY` | Arrancar mic al activar modo musical |

---

## Licencia

Firmware y documentación del proyecto **Lámpara V3** · [epicfilm.co](https://epicfilm.co/)

Las librerías de terceros (LVGL, TFT_eSPI, WS2812FX, ESP RainMaker) mantienen sus propias licencias.

---

## Etiquetas sugeridas

`esp32` · `arduino` · `lvgl` · `ws2812` · `esp-rainmaker` · `smart-light` · `home-automation` · `google-home` · `alexa` · `esp32-2432s024c`
