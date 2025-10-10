#ifndef DHT_H
#define DHT_H

#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DHT_TYPE_DHT11,
    DHT_TYPE_DHT22
} dht_sensor_type_t;

/**
 * @brief Read float data from DHT sensor
 * 
 * @param sensor_type DHT sensor type (DHT11 or DHT22)
 * @param pin GPIO pin connected to DHT sensor
 * @param humidity Pointer to store humidity value (can be NULL if not needed)
 * @param temperature Pointer to store temperature value (can be NULL if not needed)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin, 
                              float *humidity, float *temperature);

/**
 * @brief Read integer data from DHT sensor
 * 
 * @param sensor_type DHT sensor type (DHT11 or DHT22)
 * @param pin GPIO pin connected to DHT sensor
 * @param humidity Pointer to store humidity value (can be NULL if not needed)
 * @param temperature Pointer to store temperature value (can be NULL if not needed)
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t dht_read_data(dht_sensor_type_t sensor_type, gpio_num_t pin, 
                        int16_t *humidity, int16_t *temperature);

#ifdef __cplusplus
}
#endif

#endif // DHT_H