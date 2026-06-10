#ifndef RAINMAKER_APP_H
#define RAINMAKER_APP_H

#include "lamp_state.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void rainmaker_app_setup(lamp_state_t *state);
/** Lee y borra flag NVS de reinicio para provisioning (llamar antes de setup). */
bool rainmaker_app_consume_boot_prov_request(void);
/** Reinicio a pantalla minima QR + provisioning activo. */
bool rainmaker_app_consume_prov_ui_boot(void);
bool rainmaker_app_prov_ui_active(void);
void rainmaker_app_request_prov_ui_restart(void);
/** Tras pintar QR en pantalla prov (bloqueante; preferir async). */
bool rainmaker_app_start_live_provision(void);
/** Arranca BLE en tarea FreeRTOS (no bloquea loop / evita reinicios). */
void rainmaker_app_ble_start_async(void);
/** true si la tarea termino; ok=exito BLE anunciado. */
bool rainmaker_app_ble_start_poll(bool *done, bool *ok);
void rainmaker_app_exit_prov_ui(void);
/** Actualiza etiqueta de estado en pantalla prov (desde ui_app_loop). */
bool rainmaker_app_consume_prov_status(char *buf, size_t buf_len, uint32_t *color_hex);
/** Tras setup RM: provisioning BLE sin LVGL. No inicia display. */
void rainmaker_app_run_boot_provision(void);
/** Solo antes de display_init(); no llamar con LVGL/SPI activos */
bool rainmaker_app_start_ble_before_ui(void);
void rainmaker_app_begin_provision(void);
void rainmaker_app_cancel_provision_request(void);
void rainmaker_app_print_qr_serial(void);
void rainmaker_app_print_qr_serial_async(void);
void rainmaker_app_loop(void);
void rainmaker_app_report_state(const lamp_state_t *state);

bool rainmaker_app_is_online(void);
bool rainmaker_app_provisioning_active(void);
bool rainmaker_app_init_done(void);
bool rainmaker_app_prov_started(void);
bool rainmaker_app_get_qr_payload(char *buf, size_t buf_len);
void rainmaker_app_get_service_name(char *buf, size_t buf_len);
bool rainmaker_app_setup_blocks_ui(void);
bool rainmaker_app_wifi_credentials_saved(void);
bool rainmaker_app_wifi_provisioned(void);
void rainmaker_app_on_ui_ready(void);
void rainmaker_app_wifi_reset(void);

#ifdef __cplusplus
}
#endif

#endif
