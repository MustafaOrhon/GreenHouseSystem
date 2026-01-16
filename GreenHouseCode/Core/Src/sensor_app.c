/*
 * sensor_app.c
 *
 * Created on: [Date]
 * Author: Mustafa
 */

#include "sensor_app.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "dhtxx.h"
#include <stdio.h>
#include <string.h>


// External handle declarations (adjust these according to your setup)
extern I2C_HandleTypeDef hi2c1;
extern ADC_HandleTypeDef hadc1;
// Sensor data variables
SensorData g_sensor_app_current_data = {0};
sSenosorThresholds_t g_sensor_app_sensor_threshold = {
        .temperature_min_threshold = 20,
		.temperature_max_threshold = 35,
	    .soil_moisture_min_adc = 1500,
	    .soil_moisture_max_adc = 2000,
	    .air_quality_baseline_adc = 2500,
	    .air_quality_threshold_adc = 2700,
	    .water_level_min_adc = 50,
	    .water_level_max_adc = 1400,
	    .water_level_threshold_adc = 450
};
// Sensor data string buffer
osEventFlagsId_t sensorDataEventFlag;
static uint16_t adc_values[3] = {0};
static uint8_t adc_count = 0;
DHTxx_Drv_t dht22;
osMutexId_t sensorDataMutex;  // Mutex to protect sensor data
static const osMutexAttr_t sensorDataMutex_attributes = {
    .name = "SensorDataMutex"
};
// Task handle (if needed)
static osThreadId_t sensorTaskHandle;

// Sensor data collection task
void SENSOR_APP_Task(void *argument) {
    while (1) {
    	osMutexAcquire(sensorDataMutex, osWaitForever);
        //BH1750_ReadLight(&g_sensor_app_current_data.lightIntensity);  // Read light intensity from BH1750
        DHT_GetData(&dht22);
        g_sensor_app_current_data.temperature = dht22.Temperature;
        g_sensor_app_current_data.humidity = dht22.Humidity;
        HAL_ADC_Start_IT(&hadc1);
        osMutexRelease(sensorDataMutex);
        osEventFlagsSet(sensorDataEventFlag, 0x0001);  // Set the event flag
        osDelay(2000);
    }
}


static const osThreadAttr_t sensorTask_attributes = {
    .name = "SENSOR_APP_Task",
    .stack_size = 128 * 4,  // Adjust as necessary
    .priority = osPriorityLow,  // Set appropriate priority
};

void SENSOR_APP_Init(void) {
    // Initialize BH1750 light sensor
	DwtInit();
	dht22.DataPin = GPIO_PIN_0;
	dht22.DataPort = GPIOC;
	dht22.Type = DHT22;
    //BH1750_Init(&hi2c1);
    //BH1750_SetMode(CONTINUOUS_HIGH_RES_MODE);  // Set mode as needed
    sensorDataMutex = osMutexNew(&sensorDataMutex_attributes);
    sensorTaskHandle = osThreadNew(SENSOR_APP_Task, NULL, &sensorTask_attributes);
    sensorDataEventFlag = osEventFlagsNew(NULL);  // Create the event flag

}

#include <stdbool.h>
#include <stdio.h>

bool SENSOR_APP_GetFormattedData(eSensortType_t sensor, char* buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return false;
    }
    int written = 0;
    switch (sensor) {
        case SENSOR_LIGHT:
            written = snprintf(buffer, buffer_size, "Light Intensity: %.2f lx\r", g_sensor_app_current_data.lightIntensity);
            break;
        case SENSOR_TEMPERATURE:
            written = snprintf(buffer, buffer_size, "Temperature: %.2f Celcius\r", g_sensor_app_current_data.temperature);
            break;
        case SENSOR_HUMIDITY:
            written = snprintf(buffer, buffer_size, "Humidity: %.2f%%\r", g_sensor_app_current_data.humidity);
            break;
        case SENSOR_SOIL:
            if (g_sensor_app_current_data.adc_soilmoisture <= 850) {
                written = snprintf(buffer, buffer_size, "Soil Moisture: Waterlogged - No watering needed.\r");
            } else if (g_sensor_app_current_data.adc_soilmoisture <= 900) {
                written = snprintf(buffer, buffer_size, "Soil Moisture: Wet - No watering needed.\r");
            } else if (g_sensor_app_current_data.adc_soilmoisture <= 1250) {
                written = snprintf(buffer, buffer_size, "Soil Moisture: Moist - Optimal condition.\r");
            } else if (g_sensor_app_current_data.adc_soilmoisture <= 1450) {
                written = snprintf(buffer, buffer_size, "Soil Moisture: Dry - Consider watering.\r");
            } else {
                written = snprintf(buffer, buffer_size, "Soil Moisture: Very Dry - Watering needed.\r");
            }
            break;
        case SENSOR_AIRQUALITY:
            if (g_sensor_app_current_data.adc_airquality >= g_sensor_app_sensor_threshold.air_quality_threshold_adc) {
                written = snprintf(buffer, buffer_size, "Air Quality: Gas Leakage!\r");
            } else {
                written = snprintf(buffer, buffer_size, "Air Quality: Good (%u)\r", g_sensor_app_current_data.adc_airquality);
            }
            break;
        case SENSOR_WATERLEVEL:
            if (g_sensor_app_current_data.adc_waterlevel <= g_sensor_app_sensor_threshold.water_level_min_adc) {
                written = snprintf(buffer, buffer_size, "Water Level: Run-out\r");
            } else if (g_sensor_app_current_data.adc_waterlevel <= g_sensor_app_sensor_threshold.water_level_threshold_adc) {
                written = snprintf(buffer, buffer_size, "Water Level: Low\r");
            } else {
                written = snprintf(buffer, buffer_size, "Water Level: Adequate (%u)\r", g_sensor_app_current_data.adc_waterlevel);
            }
            break;
        default:
            written = snprintf(buffer, buffer_size, "Unknown sensor data requested");
    }
    return ((written > 0) && (written < buffer_size));
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    if(__HAL_ADC_GET_FLAG(&hadc1,ADC_FLAG_EOC) == SET){
        adc_values[adc_count] = HAL_ADC_GetValue(&hadc1);
        adc_count++;
        if(adc_count >= 3){
            adc_count = 0;
            g_sensor_app_current_data.adc_soilmoisture = adc_values[0];
            g_sensor_app_current_data.adc_airquality = adc_values[1];
            g_sensor_app_current_data.adc_waterlevel = adc_values[2];
        }
    }
}
