#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "mqtt_client.h"

// DHT22 driver
#include "dht.h"

static const char *TAG = "GREENHOUSE";

// ===== Wi-Fi credentials =====
#define WIFI_SSID "bunleab"
#define WIFI_PASSWORD "12345678"

// ===== ThingsBoard MQTT settings =====
#define MQTT_URI "mqtt://demo.thingsboard.io:1883"
#define ACCESS_TOKEN "wapLUSjtz2LyLsjjN7kM"

// ===== Pins =====
#define DHT_GPIO 4
#define SOIL_ADC_CHANNEL ADC_CHANNEL_6  // GPIO 34
#define FAN_GPIO 14
#define PUMP_GPIO 27

// ===== Thresholds =====
#define TEMP_THRESHOLD 30.0       // Celsius
#define HUM_THRESHOLD 80.0        // Percentage
#define SOIL_THRESHOLD 2000       // ADC raw value

// ===== Soil moisture calibration (adjust these after measuring) =====
// Record a raw value with the probe COMPLETELY DRY, set here:
#define SOIL_DRY_RAW   3500
// Record a raw value with the probe FULLY WET (in water / saturated soil), set here:
#define SOIL_WET_RAW   1200
// Number of ADC samples to average per reading
#define SOIL_SAMPLES   8

// ===== Timing =====
#define MSG_INTERVAL_MS 5000      // 5 seconds

// ===== Event bits =====
#define WIFI_CONNECTED_BIT BIT0
#define MQTT_CONNECTED_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;
static esp_mqtt_client_handle_t mqtt_client;
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle;
static bool adc_calibrated = false;

// ===== Last known sensor values (retain if a read fails) =====
static float last_temp = 0.0f;
static float last_hum  = 0.0f;
static bool  have_valid_dht = false;

// Warm-up guard to skip DHT reads for first seconds (some sensors need stabilization)
static const TickType_t DHT_WARMUP_TICKS = 2000 / portTICK_PERIOD_MS; // 2s
static TickType_t start_tick = 0;

// ===== Wi-Fi event handler =====
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, retrying...");
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// ===== MQTT event handler =====
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // esp_mqtt_event_handle_t event = event_data; // Not needed currently, avoid unused variable warning
    (void)handler_args; (void)base; (void)event_data; // silence unused warnings for now

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected");
        xEventGroupSetBits(s_wifi_event_group, MQTT_CONNECTED_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT Disconnected");
        xEventGroupClearBits(s_wifi_event_group, MQTT_CONNECTED_BIT);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT Error");
        break;
    default:
        break;
    }
}

// ===== Wi-Fi initialization =====
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization finished.");
}

// ===== MQTT initialization =====
static void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_URI,
        .credentials.username = ACCESS_TOKEN,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// ===== ADC initialization =====
static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, // Use non-deprecated attenuation
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, SOIL_ADC_CHANNEL, &config));

    // Try line fitting calibration (available in ESP-IDF 5.4.x). Curve fitting not present here.
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_handle) == ESP_OK) {
        adc_calibrated = true;
        ESP_LOGI(TAG, "ADC calibration (line fitting) initialized");
    } else {
        ESP_LOGW(TAG, "ADC calibration not available; proceeding without calibrated values");
    }
}

// ===== GPIO initialization =====
static void gpio_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << FAN_GPIO) | (1ULL << PUMP_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // Initialize relays to OFF state (HIGH for active-low relays)
    gpio_set_level(FAN_GPIO, 1);
    gpio_set_level(PUMP_GPIO, 1);
}

// ===== DHT22 combined safe read =====
static void dht_read_safe(float *out_temp, float *out_hum)
{
    if ((xTaskGetTickCount() - start_tick) < DHT_WARMUP_TICKS && !have_valid_dht) {
        // Skip reads during warm-up to avoid bogus zeros
        *out_temp = last_temp;
        *out_hum  = last_hum;
        return;
    }

    float t = NAN, h = NAN;
    int attempts = 0;
    while (attempts < 5) {
        if (dht_read_float_data(DHT_TYPE_DHT22, DHT_GPIO, &h, &t) == ESP_OK) {
            if (!isnan(t) && !isnan(h) && t > -40 && t < 85 && h >= 0 && h <= 100) {
                last_temp = t;
                last_hum  = h;
                have_valid_dht = true;
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(250));
        attempts++;
    }

    if (!have_valid_dht) {
        ESP_LOGW(TAG, "DHT read failed after %d attempts (keeping last values)", attempts);
    }

    *out_temp = last_temp;
    *out_hum  = last_hum;
}

// ===== Read soil moisture =====
static int read_soil_moisture_raw(void)
{
    int acc = 0;
    for (int i = 0; i < SOIL_SAMPLES; ++i) {
        int sample = 0;
        if (adc_oneshot_read(adc1_handle, SOIL_ADC_CHANNEL, &sample) == ESP_OK) {
            acc += sample;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return acc / SOIL_SAMPLES;
}

static float soil_percent_from_raw(int raw)
{
    // Clamp raw within calibration range
    if (raw > SOIL_DRY_RAW) raw = SOIL_DRY_RAW;
    if (raw < SOIL_WET_RAW) raw = SOIL_WET_RAW;
    float span = (float)(SOIL_DRY_RAW - SOIL_WET_RAW);
    if (span < 50) { // avoid div-by-zero if user mis-configured
        return 0.0f;
    }
    // Map: dry => 0%, wet => 100%
    float percent = (float)(SOIL_DRY_RAW - raw) * 100.0f / span;
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
}

// ===== Control devices =====
static void control_fan(bool enable)
{
    gpio_set_level(FAN_GPIO, enable ? 0 : 1);  // Active low relay
    ESP_LOGI(TAG, "Fan: %s", enable ? "ON" : "OFF");
}

static void control_pump(bool enable)
{
    gpio_set_level(PUMP_GPIO, enable ? 0 : 1);  // Active low relay
    ESP_LOGI(TAG, "Pump: %s", enable ? "ON" : "OFF");
}

// ===== Main sensor and control task =====
static void greenhouse_task(void *pvParameters)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    
    while (1) {
        // Wait for both Wi-Fi and MQTT connection
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                               WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT,
                                               pdFALSE,
                                               pdTRUE,
                                               portMAX_DELAY);
        
        if ((bits & (WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT)) == (WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT)) {
            // Read sensors
            float temp = 0, humidity = 0;
            dht_read_safe(&temp, &humidity);
            int soil_raw = read_soil_moisture_raw();
            float soil_percent = soil_percent_from_raw(soil_raw);
            
            // Debug output
            ESP_LOGI(TAG, "Temperature: %.1f°C, Humidity: %.1f%%, Soil raw: %d, Soil: %.1f%%", 
                     temp, humidity, soil_raw, soil_percent);
            
            // Control logic
            bool fan_enable = (temp > TEMP_THRESHOLD) || (humidity > HUM_THRESHOLD);
            control_fan(fan_enable);
            
            bool pump_enable = (soil_raw < SOIL_THRESHOLD);
            control_pump(pump_enable);
            
            // Create and send telemetry
            char payload[200];
            snprintf(payload, sizeof(payload),
                     "{\"temperature\":%.1f,\"humidity\":%.1f,\"soil_moisture\":%.1f,\"soil_raw\":%d}",
                     temp, humidity, soil_percent, soil_raw);
            
            ESP_LOGI(TAG, "Sending payload: %s", payload);
            
            int msg_id = esp_mqtt_client_publish(mqtt_client, "v1/devices/me/telemetry", payload, 0, 0, 0);
            if (msg_id < 0) {
                ESP_LOGE(TAG, "Failed to send telemetry!");
            }
        }
        
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MSG_INTERVAL_MS));
    }
}

// ===== Main application =====
void app_main(void)
{
    ESP_LOGI(TAG, "Starting Greenhouse Monitoring System");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize components
    gpio_init();
    adc_init();
    wifi_init_sta();
    mqtt_init();
    
    // Wait for Wi-Fi connection
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
    // Create main task
    xTaskCreate(greenhouse_task, "greenhouse_task", 4096, NULL, 5, NULL);
}
