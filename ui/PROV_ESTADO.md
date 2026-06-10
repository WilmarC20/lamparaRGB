# Estado provisioning RainMaker (2026-05-26)

## Modo activo: **BLE + pantalla RAW (sin LVGL)**

Configurable en `config.h`:

```c
#define RM_PROV_UI_MODE             1   // pantalla dedicada QR + Salir
#define RM_PROV_UI_USE_SOFTAP       0   // 0=BLE (con Assisted Claiming), 1=SoftAP
#define RM_PROV_UI_NO_LVGL          1   // 1=TFT directo (sin LVGL), max RAM
#define RM_PROV_QR_SIZE_PX          120 // tamano QR en pixeles
```

## Por que sin LVGL

LVGL inicializado consumia ~20 KB extras. En modo RAW solo `display_init_tft_only()`
+ QR/boton con TFT_eSPI directo.

## Orden (el que llego a mapping DONE en pruebas)

1. BLE activo mientras app envia cuenta + WiFi
2. En **GOT_IP**: liberar RAM BT (la app puede mostrar desconexion)
3. MQTT + mapping DONE en nube
4. Pantalla: **OK! Reinicio 12s o Salir** -> arranca UI normal con **Luz LED**
5. **ESP Insights** muestra el nodo (claiming); la **app RainMaker Home** solo si el mapping enlazo tu cuenta
6. Panel: https://dashboard.rainmaker.espressif.com (misma cuenta Google/Apple que la app)

Si la app dice error pero serial muestra `mapping DONE`, el dispositivo **si** esta en tu cuenta.

**No** `esp_rmaker_user_mapping_endpoint_create()` manual (duplica endpoint).

La caida de ~5 KB al conectar GATT puede ser normal si el heap queda >35 KB.

## Por que BLE y no SoftAP

La app RainMaker oficial (Android/iOS) requiere BLE para **Assisted Claiming**
(linking automatico del dispositivo a la cuenta del usuario). Con SoftAP la
app muestra: "Assisted Claiming not supported for SoftAP. Cannot Proceed."

## Funciona

- Pantalla Config solo boton **RainMaker** -> reinicio a pantalla QR + **Salir**
- LED controller omitido en modo prov (mas RAM)
- LVGL omitido en pantalla prov (mas RAM)
- Boton **Salir** centrado abajo, polling touch directo

## Layout pantalla prov (320x240 landscape)

```
+-------------------------------------------+
| RainMaker - Provisioning                  |   <- titulo y=2
| [ Status: Preparando QR / Escanea ...  ]  |   <- banner amarillo
|                                           |
|         +------------------+              |   <- QR centrado y=42
|         |                  |              |
|         |    QR 120x120    |              |
|         |                  |              |
|         +------------------+              |
|                                           |
| PROV_9B7C44                               |   <- nombre y=180
| PoP: lampara1234                          |   <- pop y=198
|         [    SALIR    ]                   |   <- boton y=210
+-------------------------------------------+
```

## Flujo esperado en serial (BLE + RAW)

1. `RM: reinicio -> pantalla RainMaker (QR + BLE)`
2. `Display TFT-only OK heap=~125 KB`
3. `PROV RAW: UI dibujada`
4. `PROV RAW: encode QR payload len=...`
5. `PROV RAW: QR pintado 120x120`
6. `PROV UI: heap tras liberar QR=~130 KB`
7. `RM inicio BLE en pantalla prov: heap=~130 KB`
8. `RM: anunciando BLE 'PROV_XXXXXX' PoP 'lampara1234'`
9. `>>> BLE: PROV_XXXXXX PoP: lampara1234 <<<`
10. App movil: escanear QR -> seleccionar WiFi -> ingresar cuenta
11. `WIFI_CRED_RECV` -> `mapping STARTED` -> `mapping DONE`

## QR

Payload: `{"ver":"v1","name":"PROV_XXXXXX","pop":"lampara1234","transport":"ble"}`

URL test: `https://rainmaker.espressif.com/qrcode.html?data=<payload>`

## Volver a modo LVGL completo

```c
#define RM_PROV_UI_NO_LVGL 0
```

(menos RAM pero touch via LVGL y pantalla mas vistosa)

## Debug activo (`RM_PROV_DEBUG_LOG 1`)

- `RM DBG [...]` heap, max_blk, min, prov, wifi, segundos desde inicio
- `RM *** CAIDA RAM` si heap baja >3.5 KB de golpe
- Logs IDF `protocomm_ble` en DEBUG (mira si fallan asignaciones BLE)

## Si falla

Pegar serial desde escanear QR hasta error, buscar:

- `CAIDA RAM` con `max_blk` <12 KB -> la app no podra conectarse
- `Endpoint already exists`
- Reinicio `rst:0xc` / `Guru Meditation` al conectar
- `Error allocating version string` en protocomm -> aun falta RAM
