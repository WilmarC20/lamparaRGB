# Librerías de LamparaV3

## Solo estas 3 (+ ESP32 core)

| Librería | Uso | Ubicación activa |
|----------|-----|------------------|
| **lvgl** | UI táctil | `ui/libraries/lvgl` |
| **TFT_eSPI** | Display ILI9341 | `ui/libraries/TFT_eSPI` |
| **WS2812FX** | Tira LED | `ui/libraries/WS2812FX` |
| **RainMaker / WiFi** | Cloud | Paquete ESP32 (Arduino15) |

`ui/libraries/` tiene **prioridad** sobre otras carpetas del PC (p. ej. LAMPARAV2).

## Particiones flash (RainMaker + BLE)

RainMaker necesita la partición **`fctry`**. Sin ella, `beginProvision` reinicia el ESP32.

### Opción recomendada: Custom + `partitions.csv`

El archivo debe llamarse **`partitions.csv`** (con **s**), en la carpeta del sketch `ui/`.

| Herramienta | Valor |
|-------------|--------|
| Board | ESP32 Dev Module |
| Flash Size | **4MB** |
| Partition Scheme | **Custom** |
| Sketchbook | `H:\Programacion\Arduino\lamparaV3` |

### Opción alternativa: menú RainMaker

| Herramienta | Valor |
|-------------|--------|
| Partition Scheme | **RainMaker 4MB No OTA** (no "RainMaker 4MB") |

Si usas el menú RainMaker, **renombra o borra** `ui/partitions.csv` para que no pise la tabla del IDE.

### Tras cambiar particiones

**Tools → Erase All Flash Before Sketch Upload → Enabled**, luego sube el sketch.

## Provisioning RainMaker (depuracion)

Ver **`ui/PROV_ESTADO.md`** — flujo pantalla QR, BLE `PROV_xxx`, logs `RM DBG` y errores conocidos.

En Serial debe aparecer: `RM: fctry OK @ 0x3EA000`

## Si "Custom" no compila (exit status 1)

1. Comprueba que existe `ui/partitions.csv` (no `partition.csv`).
2. Flash Size = **4MB**.
3. Abre **Archivo → Preferencias → Compilación**: activa salida detallada y lee la **primera línea de error** (no solo "exit status 1").
4. Errores frecuentes:
   - `Sketch too big` → elegiste "RainMaker 4MB" (con OTA); usa **RainMaker 4MB No OTA** o **Custom**.
   - `dram0_0 overflow` → sketchbook debe ser `lamparaV3`, no `LAMPARAV2`.
   - `lv_conf.h incorrecto` → ver abajo.

## lv_conf.h

Configuración LVGL en **`ui/lv_conf.h`** con `LV_MEM_CUSTOM 1` (heap, no 64 KB estáticos).

`build_opt.h` fuerza `-DLV_CONF_INCLUDE_SIMPLE` y `-DLV_MEM_CUSTOM=1`.

## Sketchbook recomendado

```
H:\Programacion\Arduino\lamparaV3
```

Si el sketchbook es `H:\Programacion\Arduino`, el IDE indexa cientos de librerías de otros proyectos y puede elegir las incorrectas.
