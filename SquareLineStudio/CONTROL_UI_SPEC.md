# Pantalla Control — especificación SquareLine Studio

Resolución: **320 × 240** | LVGL **8.3.11** | Export Arduino → `libraries/ui/src`

## Widgets requeridos (nombres exactos para el firmware)

| Objeto LVGL | Nombre C | Evento |
|-------------|----------|--------|
| lv_colorwheel | `ui_ColorWheel` | OnColorChanged |
| lv_slider 0-255 | `ui_SliderBrillo` | OnBrightnessChanged |
| lv_dropdown | `ui_DropdownEfectos` | OnEffectChanged |
| lv_switch | `ui_SwitchFiesta` | OnPartyModeChanged |
| lv_switch | `ui_SwitchPower` | OnPowerChanged |
| lv_label | `ui_LabelEstado` | — |

## Opciones dropdown (orden fijo)

```
Solido
Arcoiris
Persecucion
Onda
Respiracion
Estrobo
```

## Export paths (AlarmaV3.slp)

- UI: `H:\Programacion\Arduino\lamparaV3\libraries\LamparaUI\src`
- Proyecto: `H:\Programacion\Arduino\lamparaV3\ui`

## Nota

La UI programática en `libraries/LamparaUI/src/screens/ui_Control.c` ya implementa estos widgets. Al exportar desde SquareLine, conservar los mismos nombres de objetos globales en `ui.h`.
