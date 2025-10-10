idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi esp_event nvs_flash driver mqtt esp_adc esp_netif dht)
