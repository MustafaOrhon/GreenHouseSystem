# SmartGreen 🌱

A real-time smart greenhouse monitoring and control system built on STM32F4 with FreeRTOS. The system monitors environmental conditions through multiple sensors and provides automated actuator control with Bluetooth connectivity for remote monitoring.

## Features

- **Real-time Environmental Monitoring**
  - Temperature & Humidity (DHT22)
  - Light Intensity (BH1750 via I2C)
  - Soil Moisture (Capacitive sensor via ADC)
  - Air Quality / Gas Detection (MQ sensor via ADC)
  - Water Level (Analog sensor via ADC)

- **Automated Control System**
  - Threshold-based actuator triggering
  - Configurable min/max thresholds for all sensors
  - Automatic alerts for critical conditions

- **Bluetooth CLI Interface**
  - Remote sensor data queries
  - Real-time threshold configuration
  - System status monitoring

## Hardware

| Component | Description |
|-----------|-------------|
| MCU | STM32F401RE (Nucleo-64) |
| Temp/Humidity | DHT22 |
| Light Sensor | BH1750 (I2C) |
| Soil Moisture | Capacitive Soil Moisture Sensor |
| Air Quality | MQ-series Gas Sensor |
| Water Level | Analog Water Level Sensor |
| Communication | HC-05/HC-06 Bluetooth Module |


## CLI Commands

### Sensor Queries
| Command | Description |
|---------|-------------|
| `temperature` | Get current temperature reading |
| `humidity` | Get current humidity reading |
| `light` | Get light intensity in lux |
| `soil` | Get soil moisture status |
| `airquality` | Get air quality reading |
| `waterlevel` | Get water level status |

### Threshold Configuration
| Command | Description |
|---------|-------------|
| `set_temp_min <value>` | Set minimum temperature threshold |
| `set_temp_max <value>` | Set maximum temperature threshold |
| `set_soil_min <value>` | Set soil moisture minimum ADC |
| `set_soil_max <value>` | Set soil moisture maximum ADC |
| `set_air_thresh <value>` | Set air quality threshold |
| `set_water_min <value>` | Set water level minimum ADC |
| `set_water_max <value>` | Set water level maximum ADC |
| `set_water_thresh <value>` | Set water level alert threshold |
