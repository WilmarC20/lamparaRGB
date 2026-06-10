#ifndef LAMPARA_CONFIG_H
#define LAMPARA_CONFIG_H

#define USE_LVGL_UI 1

/* RainMaker: 1 = activo, 0 = compilar sin cloud (útil para depurar UI/LED) */
#define ENABLE_RAINMAKER 1

/* RainMaker ligero: desactiva OTA, escenas, schedule, insights y system service */
#define RAINMAKER_MINIMAL 1

/* Espera tras pintar la UI antes de arrancar WiFi/BLE (ms) */
#define RAINMAKER_START_DELAY_MS 1500

/* 0 = no iniciar BLE/QR al boot; prov solo desde Ajustes -> WiFi */
#define RM_AUTO_PROVISION_ON_BOOT 0

/* 1 = beginProvision en setup() ANTES de lv_init (solo si compila y no reinicia) */
#define RAINMAKER_BLE_BEFORE_UI 0
/* 0 = no usar boton / loop para arrancar BLE (falla con LVGL activo) */
#define RAINMAKER_AUTO_BLE_PROVISION 0

/* Audio sin RainMaker: retardo fijo tras boot */
#define AUDIO_START_DELAY_MS 2500

/* Con RainMaker: no arrancar mic hasta WiFi OK (claim CSR necesita ~10 KB de stack) */
#define AUDIO_START_AFTER_WIFI 1
/* Si no hay WiFi, arrancar audio igual tras este tiempo (modo fiesta local) */
#define AUDIO_START_FALLBACK_MS 120000
/* 1 = arrancar mic al activar cualquier modo musical (sin esperar WiFi) */
#define AUDIO_START_ON_PARTY 1
#define AUDIO_START_ON_MUSIC_BAR 1

/* Mic: bloque mas corto + EMA por muestra = respuesta mas rapida */
#define AUDIO_SAMPLE_MS      1
#define AUDIO_BLOCK_LEN      32
#define AUDIO_EMA_SHIFT        3

/* Serial Plotter mic: desactivado (niveles en pantalla) */
#define AUDIO_SERIAL_PLOTTER   0
#define MIC_PLOTTER_FREEZE_LEDS 0
/* 1 = arrancar mic al boot (plotter). 0 = usar boton Config -> Probar mic */
#define MIC_START_AT_BOOT      0
/* Resta ruido de fondo ADC; mas bajo = mas sensible */
#define AUDIO_MIC_THRESHOLD  5
/* Valor L en silencio (Config -> Mic). Ajusta hasta que en silencio L ~= este numero. */
#define AUDIO_SILENCE_LEVEL  200

/* 0 = sin WS2812 (GPIO 1 queda libre para Serial; usar mientras pruebas radio) */
#define ENABLE_LED_STRIP       0

/* Altavoz onboard ESP32-2432S024C (GPIO 26 -> FM8002A) */
#define ENABLE_SPEAKER         1
#define SPEAKER_TEST_AT_BOOT   0

/* Radio en linea Colombia (minimp3 + NetworkClient en tarea aparte) */
#define ENABLE_RADIO           1

/* Salida Serial del firmware (0 = silencio total en nuestro codigo) */
#define LAMP_SERIAL_LOG      1

/* Log en Serial al pulsar mando IR */
#define IR_DEBUG_LOG         0

/* Nombre BLE y PoP (igual que el ejemplo RMakerSwitch que te funciono) */
#define RM_SERVICE_NAME "PROV_ESP32"
#define RM_POP          "12345678"

/*
 * Provisioning BLE (como RMakerSwitch oficial).
 * RMAKER_PROV_BOOT_RESET: reinicia sin LVGL antes de BLE (evita panic con BLE).
 */
#define RMAKER_USE_SOFTAP_PROVISION 0
#define RM_BOOT_PROV_USE_SOFTAP     0
/* 0 = pantalla prov con QR; 1 = reinicio sin pantalla (legacy) */
#define RMAKER_PROV_BOOT_RESET      0
/* Pantalla dedicada: QR + Salir + BLE activo */
#define RM_PROV_UI_MODE             1
/* 1 = pantalla prov usa SoftAP (no admite Assisted Claiming). 0 = BLE */
#define RM_PROV_UI_USE_SOFTAP       0
/* 1 = pantalla prov SIN LVGL (dibujo directo TFT, max RAM para BLE) */
#define RM_PROV_UI_NO_LVGL          1
/* 1 = mostrar QR en pantalla prov. 0 = solo texto BLE (prueba sin QR) */
#define RM_PROV_UI_SHOW_QR          1
/* QR en pantalla prov (100 px escaneable, ~22 KB buffer vs 29 KB) */
#define RM_PROV_QR_SIZE_PX          100
#define RMAKER_PROV_BLE_HANDLER     NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM
/* Requerido por la app RainMaker actual para enlazar cuenta */
#define RM_ENABLE_USER_MAPPING      1
/* 0 = nombre fijo RM_SERVICE_NAME en BLE y QR (como tu ejemplo) */
#define RM_USE_PROV_MAC_NAME        0
/* Logs serial: eventos prov, mapping y protocomm/BLE (depuracion) */
#define RM_PROV_DEBUG_LOG           0
/* Tiempo max esperando mapping nube tras WiFi (ms) */
#define RM_PROV_MAPPING_TIMEOUT_MS  180000
/* 0 = no borrar fctry cada prov (CLAIM OK; borrar solo con Erase All Flash) */
#define RM_PROV_ERASE_FCTRY_ON_SESSION 0
/* 0 = liberar RAM BT en GOT_IP para MQTT (app puede desconectar; nodo si llega a DONE) */
#define RM_PROV_KEEP_BLE_ACTIVE         0
/* Tras mapping DONE: reinicio a UI normal (la app Home lista el nodo ahi) */
#define RM_PROV_AUTO_RESTART_ON_DONE    1
#define RM_PROV_AUTO_RESTART_DELAY_MS   12000

/* 1 = QR grafico LVGL. 0 = solo texto JSON (depuracion) */
/* QR grafico: lv_qrcode_create + lv_qrcode_update (2 pasos en cfg_load) */
#define CONFIG_USE_LVGL_QR 0
/* 120 px en pantalla 320x240 (~3 px/modulo; escaneable). Buffer estatico ~29 KB */
#define CONFIG_QR_SIZE_PX  120
#define CONFIG_QR_FRAME_PX 6
/* Tarea QR en core 1 (core 0 = BLE/RainMaker). Stack pequeño: solo qrcodegen */
#define CONFIG_QR_ENCODE_TASK_STACK 4096
#define CONFIG_QR_ENCODE_CORE       1

#endif
