/*
 * sensor_app.h
 *
 *  Created on: May 7, 2024
 *      Author: Mustafa
 */

#ifndef INC_SENSOR_APP_H_
#define INC_SENSOR_APP_H_

#include "stdint.h"
#include "bh1750.h"  // Include the BH1750 light sensor header
#include <stdbool.h>
typedef enum {
	SENSOR_TEMPERATURE,
    SENSOR_HUMIDITY,
    SENSOR_LIGHT,
	SENSOR_SOIL,
	SENSOR_AIRQUALITY,
    SENSOR_WATERLEVEL
    // Add other sensor types as needed
} eSensortType_t;

typedef struct {
    float temperature;
    float humidity;
    float lightIntensity;
    uint16_t adc_soilmoisture;
    uint16_t adc_airquality;
    uint16_t adc_waterlevel;
} SensorData;

typedef struct {
	uint16_t temperature_min_threshold;
	uint16_t temperature_max_threshold;
    uint16_t soil_moisture_min_adc;   // Minimum ADC value for soil moisture
    uint16_t soil_moisture_max_adc;   // Maximum ADC value for soil moisture
    uint16_t air_quality_baseline_adc; // Baseline ADC value for air quality
    uint16_t air_quality_threshold_adc; // Threshold ADC value for gas leakage or high pollution
    uint16_t water_level_min_adc;     // Minimum ADC value for water level
    uint16_t water_level_max_adc;     // Maximum ADC value for water level
    uint16_t water_level_threshold_adc; // Threshold ADC value at 70% of maximum
} sSenosorThresholds_t;


/* Initializes all sensors and the data collection task */
void SENSOR_APP_Init(void);

/* Retrieves sensor data as a formatted string based on sensor type */
bool SENSOR_APP_GetFormattedData(eSensortType_t sensor, char *buffer, size_t buffer_size);

#endif /* INC_SENSOR_APP_H_ */
