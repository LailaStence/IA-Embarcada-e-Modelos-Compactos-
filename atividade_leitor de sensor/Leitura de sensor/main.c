#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#define DHT_GPIO GPIO_NUM_4
#define TAG "DHT22"

static bool wait_for_level(int level, int timeout_us)
{
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(DHT_GPIO) != level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return false;
        }
    }
    return true;
}

static esp_err_t dht22_read(uint8_t *humidity, uint8_t *temperature)
{
    uint8_t data[5] = {0};

    // Sinal de início enviado pelo microcontrolador.
    gpio_set_direction(DHT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_GPIO, 0);
    esp_rom_delay_us(20000); // mínimo de 18 ms para o DHT22
    gpio_set_level(DHT_GPIO, 1);
    esp_rom_delay_us(40);

    // Libera o barramento para o sensor responder.
    gpio_set_direction(DHT_GPIO, GPIO_MODE_INPUT);

    // Resposta do sensor: aproximadamente 80 us em nível baixo e 80 us em alto.
    if (!wait_for_level(0, 100)) return ESP_ERR_TIMEOUT;
    if (!wait_for_level(1, 100)) return ESP_ERR_TIMEOUT;
    if (!wait_for_level(0, 100)) return ESP_ERR_TIMEOUT;

    // Cada bit começa com aproximadamente 50 us em nível baixo.
    // O tempo do pulso alto diferencia 0 (~26 us) de 1 (~70 us).
    for (int byte = 0; byte < 5; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            if (!wait_for_level(1, 100)) return ESP_ERR_TIMEOUT;

            int64_t pulse_start = esp_timer_get_time();
            if (!wait_for_level(0, 120)) return ESP_ERR_TIMEOUT;
            int pulse_width = (int)(esp_timer_get_time() - pulse_start);

            if (pulse_width > 45) {
                data[byte] |= (1 << (7 - bit));
            }
        }
    }

    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Checksum invalido: calculado=0x%02X recebido=0x%02X",
                 checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }

    *humidity = data[0];
    *temperature = data[2];
    return ESP_OK;
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "Leitor DHT22 iniciado no GPIO %d", DHT_GPIO);
    ESP_LOGI(TAG, "Aguardando leituras...\n");

    while (true) {
        uint8_t humidity = 0;
        uint8_t temperature = 0;
        esp_err_t result = dht22_read(&humidity, &temperature);

        if (result == ESP_OK) {
            printf("Temperatura: %d C | Umidade: %d %%\n",
                   temperature, humidity);
        } else {
            ESP_LOGW(TAG, "Falha ao ler o DHT22: %s", esp_err_to_name(result));
        }

        // O DHT22 recomenda intervalo de pelo menos 1 segundo entre leituras.
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
