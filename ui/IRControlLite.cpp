#include <Arduino.h>
#include "IRControlLite.h"
#include "config.h"
#include "lamp_log.h"
#include "driver/gpio.h"
#include "driver/rmt_rx.h"

extern void IRControlLite_inicio(int valor);
extern void IRControlLite_presionado(int valor);
extern void IRControlLite_solto(int valor);

#define RMT_RX_SYMBOLS 256

static rmt_channel_handle_t rx_channel = NULL;
static rmt_receive_config_t rx_config;
static rmt_symbol_word_t raw_buffer[RMT_RX_SYMBOLS];
static const size_t kRawBufferBytes = sizeof(raw_buffer);

static volatile bool data_ready = false;
static size_t received_size = 0;
static bool boton_presionado = false;

static bool rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
    (void)channel;
    (void)user_ctx;
    received_size = edata->num_symbols * sizeof(rmt_symbol_word_t);
    data_ready = true;
    return false;
}

static uint32_t decodeNEC(rmt_symbol_word_t *symbols, int count)
{
    if (count < 30) return 0;
    if (symbols[0].duration0 < 8000 || symbols[0].duration1 < 4000) {
        return 0;
    }

    uint32_t data = 0;
    int bits = 0;

    for (int i = 1; i < count && bits < 32; i++) {
        uint32_t low = symbols[i].duration1;
        if (low > 1000) {
            data |= (1UL << bits);
        }
        bits++;
    }

    if (bits < 32) return 0;

    uint8_t command = (data >> 16) & 0xFF;
    uint8_t inv_command = (data >> 24) & 0xFF;

    if ((command ^ inv_command) != 0xFF) {
        return 0;
    }

    return command + 1;
}

static bool isNECRepeat(rmt_symbol_word_t *symbols, int count)
{
    if (count < 2) return false;
    const uint32_t h0 = symbols[0].duration0;
    const uint32_t h1 = symbols[0].duration1;
    if (h0 < 8000 || h0 > 10000) return false;
    if (h1 < 1800 || h1 > 3000) return false;
    return true;
}

IRControlLite::IRControlLite(int pin) : pin(pin) {}

bool IRControlLite::iniciar()
{
    ready = false;
    rx_channel = NULL;
    data_ready = false;
    boton_presionado = false;

    gpio_reset_pin((gpio_num_t)pin);
    pinMode(pin, INPUT);

    rmt_rx_channel_config_t rx_chan_config = {
        .gpio_num = (gpio_num_t)pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .mem_block_symbols = 64,
        .flags = {
            .invert_in = false,
            .with_dma = false,
        }
    };

    esp_err_t err = rmt_new_rx_channel(&rx_chan_config, &rx_channel);
    if (err != ESP_OK) {
        LAMP_LOG("IR RMT channel error: %d (GPIO %d)\n", (int)err, pin);
        return false;
    }

    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = rx_done_callback,
    };
    err = rmt_rx_register_event_callbacks(rx_channel, &cbs, NULL);
    if (err != ESP_OK) {
        LAMP_LOG("IR RMT callback error: %d\n", (int)err);
        return false;
    }

    rx_config.signal_range_min_ns = 1000;
    rx_config.signal_range_max_ns = 10000000;

    err = rmt_enable(rx_channel);
    if (err != ESP_OK) {
        LAMP_LOG("IR RMT enable error: %d\n", (int)err);
        return false;
    }

    err = rmt_receive(rx_channel, raw_buffer, kRawBufferBytes, &rx_config);
    if (err != ESP_OK) {
        LAMP_LOG("IR RMT receive error: %d\n", (int)err);
        return false;
    }

    ready = true;
    LAMP_LOG("IR: listo GPIO %d idle=%d\n", pin, digitalRead(pin));
    return true;
}

bool IRControlLite::listo() const
{
    return ready && rx_channel != NULL;
}

uint32_t IRControlLite::procesar()
{
    uint32_t retorno = (uint32_t)-1;

    if (!ready || !rx_channel) {
        return retorno;
    }

    if (data_ready) {
        rmt_symbol_word_t *symbols = raw_buffer;
        int num = (int)(received_size / sizeof(rmt_symbol_word_t));

        uint32_t comando = decodeNEC(symbols, num);
        const bool repeatFrame = (comando == 0) && isNECRepeat(symbols, num);

        if (comando != 0 || (repeatFrame && boton_presionado)) {
            if (millis() - ultimoTiempo > t_pulsacion) {
                if (comando != 0) {
                    retorno = comando - 1;
                    ultimoCodigo = retorno;
                } else {
                    retorno = ultimoCodigo;
                }

                if (!boton_presionado) {
                    IRControlLite_inicio((int)retorno);
                } else {
                    IRControlLite_presionado((int)retorno);
                }

                boton_presionado = true;
                ultimoTiempo = millis();
            }

            ultimoTiempoGral = millis();
        }

        data_ready = false;
        vTaskDelay(1);
        const esp_err_t err = rmt_receive(rx_channel, raw_buffer, kRawBufferBytes, &rx_config);
        if (err != ESP_OK) {
            LAMP_LOG("IR RMT re-arm error: %d\n", (int)err);
        }
    } else if (boton_presionado && millis() - ultimoTiempoGral > 200) {
        boton_presionado = false;
        IRControlLite_solto((int)ultimoCodigo);
    }

    return retorno;
}

uint32_t IRControlLite::getUltimoComando()
{
    return ultimoCodigo;
}
