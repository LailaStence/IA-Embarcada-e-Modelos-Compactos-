#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#define DHT11_GPIO              GPIO_NUM_4
#define DHT11_TIMEOUT_US        120
#define DHT11_SAMPLE_PERIOD_MS  2000

static const char *TAG = "DHT11";

static esp_err_t dht11_read(int *temperature_c, int *humidity_percent)
{
    uint8_t data[5] = {0};

    // Start signal: the host pulls the data line low for at least 18 ms.
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT11_GPIO, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_INPUT);

    // Sensor response: approximately 80 us low, followed by 80 us high.
    uint32_t deadline = esp_log_timestamp() * 1000U + 100000U;
    while (gpio_get_level(DHT11_GPIO) == 1) {
        if (esp_log_timestamp() * 1000U > deadline) return ESP_ERR_TIMEOUT;
    }
    while (gpio_get_level(DHT11_GPIO) == 0) {
        if (esp_log_timestamp() * 1000U > deadline) return ESP_ERR_TIMEOUT;
    }
    while (gpio_get_level(DHT11_GPIO) == 1) {
        if (esp_log_timestamp() * 1000U > deadline) return ESP_ERR_TIMEOUT;
    }

    // 40 bits: each bit starts low; the length of the high pulse encodes 0/1.
    for (int bit = 0; bit < 40; bit++) {
        uint32_t wait_us = 0;
        while (gpio_get_level(DHT11_GPIO) == 0) {
            esp_rom_delay_us(1);
            if (++wait_us > DHT11_TIMEOUT_US) return ESP_ERR_TIMEOUT;
        }

        uint32_t high_us = 0;
        while (gpio_get_level(DHT11_GPIO) == 1) {
            esp_rom_delay_us(1);
            if (++high_us > DHT11_TIMEOUT_US) return ESP_ERR_TIMEOUT;
        }

        data[bit / 8] <<= 1;
        if (high_us > 40) data[bit / 8] |= 1;
    }

    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (checksum != data[4]) {
        ESP_LOGW(TAG, "Checksum invalido: calculado=0x%02X recebido=0x%02X", checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }

    *humidity_percent = data[0];
    *temperature_c = data[2];
    return ESP_OK;
}

void app_main(void)
{
    int temperature = 0;
    int humidity = 0;

    gpio_set_pull_mode(DHT11_GPIO, GPIO_PULLUP_ONLY);
    ESP_LOGI(TAG, "Atividade Pratica 2/6 - Leitura de sensor");
    ESP_LOGI(TAG, "ESP32-S3 + DHT11 | GPIO%d | periodo=%d ms", DHT11_GPIO, DHT11_SAMPLE_PERIOD_MS);

    while (true) {
        esp_err_t err = dht11_read(&temperature, &humidity);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Leitura: temperatura=%d C | umidade=%d %%", temperature, humidity);
        } else {
            ESP_LOGE(TAG, "Falha na leitura do DHT11: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(DHT11_SAMPLE_PERIOD_MS));
    }
}
