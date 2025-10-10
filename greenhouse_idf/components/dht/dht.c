#include "dht.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

static const char *TAG = "DHT";

#define DHT_TIMEOUT_US 85
#define DHT_DATA_BITS 40

static esp_err_t dht_await_pin_state(gpio_num_t pin, uint32_t timeout, int expected_pin_state, uint32_t *duration)
{
    uint32_t start_time = esp_timer_get_time();
    for (;;) {
        uint32_t elapsed_time = esp_timer_get_time() - start_time;
        if (gpio_get_level(pin) == expected_pin_state) {
            if (duration) {
                *duration = elapsed_time;
            }
            return ESP_OK;
        }
        if (elapsed_time > timeout) {
            return ESP_ERR_TIMEOUT;
        }
    }
}

static esp_err_t dht_fetch_data(gpio_num_t pin, uint8_t data[5])
{
    uint32_t low_duration;
    uint32_t high_duration;

    // Phase 'A' pulling signal low to initiate read sequence
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 0);
    ets_delay_us(20 * 1000);
    gpio_set_level(pin, 1);
    ets_delay_us(40);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // Step through Phase 'B', 40us
    if (dht_await_pin_state(pin, DHT_TIMEOUT_US, 0, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Initialization error, problem in phase 'B'");
        return ESP_ERR_TIMEOUT;
    }

    // Step through Phase 'C', 88us
    if (dht_await_pin_state(pin, DHT_TIMEOUT_US, 1, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Initialization error, problem in phase 'C'");
        return ESP_ERR_TIMEOUT;
    }

    // Step through Phase 'D', 88us
    if (dht_await_pin_state(pin, DHT_TIMEOUT_US, 0, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Initialization error, problem in phase 'D'");
        return ESP_ERR_TIMEOUT;
    }

    // Read in each bit of data
    for (int i = 0; i < DHT_DATA_BITS; i++) {
        // Initial data low level, 54us
        if (dht_await_pin_state(pin, DHT_TIMEOUT_US, 1, &low_duration) != ESP_OK) {
            ESP_LOGE(TAG, "LOW bit timeout");
            return ESP_ERR_TIMEOUT;
        }

        // Data high level, 26us or 70us
        if (dht_await_pin_state(pin, DHT_TIMEOUT_US, 0, &high_duration) != ESP_OK) {
            ESP_LOGE(TAG, "HIGH bit timeout");
            return ESP_ERR_TIMEOUT;
        }

        uint8_t b = i / 8;
        uint8_t m = i % 8;
        if (!m) {
            data[b] = 0;
        }

        data[b] |= (high_duration > low_duration) << (7 - m);
    }

    return ESP_OK;
}

static esp_err_t dht_parse_data(dht_sensor_type_t sensor_type, uint8_t data[5], 
                                int16_t *humidity, int16_t *temperature)
{
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        ESP_LOGE(TAG, "Checksum failed, got: %d, expected: %d", 
                 data[4], ((data[0] + data[1] + data[2] + data[3]) & 0xFF));
        return ESP_ERR_INVALID_CRC;
    }

    if (sensor_type == DHT_TYPE_DHT22) {
        if (humidity) {
            *humidity = (data[0] << 8) | data[1];
        }
        if (temperature) {
            *temperature = ((data[2] & 0x7F) << 8) | data[3];
            if (data[2] & 0x80) {
                *temperature = -*temperature;
            }
        }
    } else { // DHT11
        if (humidity) {
            *humidity = data[0] * 10;
        }
        if (temperature) {
            *temperature = data[2] * 10;
        }
    }

    return ESP_OK;
}

esp_err_t dht_read_data(dht_sensor_type_t sensor_type, gpio_num_t pin, 
                        int16_t *humidity, int16_t *temperature)
{
    uint8_t data[5] = {0};
    esp_err_t result;

    gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);

    result = dht_fetch_data(pin, data);
    if (result != ESP_OK) {
        return result;
    }

    return dht_parse_data(sensor_type, data, humidity, temperature);
}

esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin, 
                              float *humidity, float *temperature)
{
    int16_t i_humidity, i_temperature;
    esp_err_t result = dht_read_data(sensor_type, pin, 
                                     humidity ? &i_humidity : NULL, 
                                     temperature ? &i_temperature : NULL);
    
    if (result != ESP_OK) {
        return result;
    }

    if (humidity) {
        *humidity = i_humidity / 10.0;
    }
    if (temperature) {
        *temperature = i_temperature / 10.0;
    }

    return ESP_OK;
}