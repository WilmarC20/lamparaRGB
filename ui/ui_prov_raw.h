#ifndef UI_PROV_RAW_H
#define UI_PROV_RAW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Pantalla provisioning sin LVGL: dibujo directo al TFT.
 * Libera todo el RAM de LVGL para maximizar BLE GATT. */

void ui_prov_raw_init(void);
void ui_prov_raw_set_status(const char *text, uint16_t color565);

/* QR: alloc -> encode (sync ahorra task/stack) -> paint -> free */
bool ui_prov_raw_qr_alloc(int *out_px);
bool ui_prov_raw_qr_encode_sync(void);
void ui_prov_raw_qr_paint(void);
void ui_prov_raw_qr_free(void);

/* Polling touch directo al boton Salir; true 1 sola vez por toque. */
bool ui_prov_raw_check_exit_pressed(void);

#ifdef __cplusplus
}
#endif

#endif
