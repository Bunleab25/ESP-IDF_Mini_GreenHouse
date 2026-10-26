# Greenhouse Monitoring System - ESP-IDF v5.5

This project implements a greenhouse monitoring and control system using ESP32 with ESP-IDF v5.5.

## Features

- **Temperature and Humidity Monitoring**: Uses DHT22 sensor
- **Soil Moisture Monitoring**: Analog soil moisture sensor
- **Automatic Control**: Fan and water pump control based on thresholds
- **IoT Connectivity**: Sends data to ThingsBoard via MQTT
- **Wi-Fi Connectivity**: Connects to your home/office network

## Hardware Requirements

- ESP32 development board
- DHT22 temperature and humidity sensor
- Soil moisture sensor (analog)
- 2-channel relay module
- 12V fan
- Water pump
- Jumper wires and breadboard

## Pin Configuration

- **DHT22**: GPIO 4
- **Soil Moisture Sensor**: GPIO 34 (ADC1_CHANNEL_6)
- **Fan Relay**: GPIO 14
- **Pump Relay**: GPIO 27

## Software Requirements

- ESP-IDF v5.5 or later
- Python 3.8+

## Setup Instructions

### 1. Install ESP-IDF v5.5

Follow the official ESP-IDF installation guide:
https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/get-started/index.html

### 2. Set up the environment

On Windows (PowerShell):
```powershell
cd C:\path\to\esp-idf
.\export.ps1
```

On Linux/macOS:
```bash
cd ~/esp/esp-idf
source ./export.sh
```

### 3. Configure the project

```bash
cd greenhouse_idf
idf.py menuconfig
```

Configure your Wi-Fi credentials and other settings.

### 4. Build and flash

```bash
# Build the project
idf.py build

# Flash to ESP32
idf.py -p COM3 flash  # Replace COM3 with your port

# Monitor serial output
idf.py -p COM3 monitor
```

## Configuration

### Wi-Fi Settings
Update the Wi-Fi credentials in `main/main.c`:
```c
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"
```

### ThingsBoard Configuration
Update the ThingsBoard settings in `main/main.c`:
```c
#define MQTT_URI "mqtt://your_thingsboard_server:1883"
#define ACCESS_TOKEN "your_device_access_token"
```

### Control Thresholds
Adjust the control thresholds as needed:
```c
#define TEMP_THRESHOLD 30.0       // Celsius
#define HUM_THRESHOLD 80.0        // Percentage
#define SOIL_THRESHOLD 2000       // ADC raw value
```

## Project Structure

```
greenhouse_idf/
├── main/
│   ├── main.c              # Main application code
│   └── CMakeLists.txt      # Main component build file
├── components/
│   └── dht/                # DHT22 sensor driver
│       ├── include/
│       │   └── dht.h       # DHT driver header
│       ├── dht.c           # DHT driver implementation
│       └── CMakeLists.txt  # DHT component build file
├── CMakeLists.txt          # Project build file
├── sdkconfig.defaults      # Default configuration
└── README.md               # This file
```

## Key Changes from Arduino to ESP-IDF v5.5

1. **Wi-Fi Management**: Uses ESP-IDF's WiFi station mode with event handling
2. **MQTT Client**: Uses ESP-IDF's MQTT client component
3. **ADC Reading**: Updated to use new ADC oneshot API in ESP-IDF v5.5
4. **GPIO Control**: Uses ESP-IDF GPIO driver
5. **Task Management**: Uses FreeRTOS tasks instead of Arduino loop()
6. **DHT22 Driver**: Custom component for DHT22 sensor reading

## Troubleshooting

### Build Issues
- Ensure ESP-IDF v5.5 is properly installed and environment is set up
- Check that all required components are included in CMakeLists.txt

### Connection Issues
- Verify Wi-Fi credentials
- Check that ThingsBoard server is accessible
- Ensure MQTT broker settings are correct

### Sensor Issues
- Check wiring connections
- Verify pin configurations in code
- Monitor serial output for error messages

## Data Format

The system sends telemetry data to ThingsBoard in JSON format:
```json
{
  "temperature": 25.6,
  "humidity": 65.2,
  "soil_moisture": 45.8
}
```

## License

This project is open source and available under the MIT License.

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates how to blink a LED by using the GPIO driver or using the [led_strip](https://components.espressif.com/component/espressif/led_strip) library if the LED is addressable e.g. [WS2812](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf). The `led_strip` library is installed via [component manager](main/idf_component.yml).

## How to Use Example

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

### Hardware Required

* A development board with normal LED or addressable LED on-board (e.g., ESP32-S3-DevKitC, ESP32-C6-DevKitC etc.)
* A USB cable for Power supply and programming

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

### Configure the Project

Open the project configuration menu (`idf.py menuconfig`).

In the `Example Configuration` menu:

* Select the LED type in the `Blink LED type` option.
  * Use `GPIO` for regular LED
  * Use `LED strip` for addressable LED
* If the LED type is `LED strip`, select the backend peripheral
  * `RMT` is only available for ESP targets with RMT peripheral supported
  * `SPI` is available for all ESP targets
* Set the GPIO number used for the signal in the `Blink GPIO number` option.
* Set the blinking period in the `Blink period in ms` option.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

As you run the example, you will see the LED blinking, according to the previously defined period. For the addressable LED, you can also change the LED color by setting the `led_strip_set_pixel(led_strip, 0, 16, 16, 16);` (LED Strip, Pixel Number, Red, Green, Blue) with values from 0 to 255 in the [source file](main/blink_example_main.c).

```text
I (315) example: Example configured to blink addressable LED!
I (325) example: Turning the LED OFF!
I (1325) example: Turning the LED ON!
I (2325) example: Turning the LED OFF!
I (3325) example: Turning the LED ON!
I (4325) example: Turning the LED OFF!
I (5325) example: Turning the LED ON!
I (6325) example: Turning the LED OFF!
I (7325) example: Turning the LED ON!
I (8325) example: Turning the LED OFF!
```

Note: The color order could be different according to the LED model.

The pixel number indicates the pixel position in the LED strip. For a single LED, use 0.

## Troubleshooting

* If the LED isn't blinking, check the GPIO or the LED type selection in the `Example Configuration` menu.

For any technical queries, please open an [issue](https://github.com/espressif/esp-idf/issues) on GitHub. We will get back to you soon.

