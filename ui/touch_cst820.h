#ifndef LAMPARA_TOUCH_CST820_H
#define LAMPARA_TOUCH_CST820_H

#include <stdint.h>

void touch_cst820_init(void);
void touch_cst820_register_indev(void);
/** Coordenadas pantalla 320x240 rotacion 1; true si hay toque */
bool touch_cst820_read_screen(uint16_t *x, uint16_t *y);
/** Zona boton Salir en pantalla prov (sin LVGL) */
bool touch_cst820_hit_prov_exit(uint16_t x, uint16_t y);

#endif
