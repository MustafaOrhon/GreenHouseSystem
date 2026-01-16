/*
 * actuator.c
 *
 *  Created on: May 7, 2024
 *      Author: Mustafa
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "sensor_app.h"
#include "cli.h"
#include "actuator_app.h"

// Task handle for the actuator task (if needed for reference outside task creation)
static osThreadId_t g_actuator_app_thread_handle = NULL;
static char g_actuator_app_response_buffer[512] = {0};
static const osThreadAttr_t g_actuator_app_thread_attr = {
    .name = "ACTUATOR_APP_Thread",
    .stack_size = 128 * 4,  // Adjust the stack size as necessary
    .priority = osPriorityNormal,  // Set appropriate priority
};
// Define actuator control task
extern SensorData g_sensor_app_current_data;
extern osEventFlagsId_t sensorDataEventFlag;
extern sSenosorThresholds_t g_sensor_app_sensor_threshold;
static void ACTUATOR_APP_Thread(void *argument);

static void ACTUATOR_APP_Thread(void *argument) {
    while (1) {
        // Wait for the sensor data ready event
        osEventFlagsWait(sensorDataEventFlag, 0x0001, osFlagsWaitAny, osWaitForever);
        if (g_sensor_app_current_data.temperature >= g_sensor_app_sensor_threshold.temperature_max_threshold) {
            snprintf(g_actuator_app_response_buffer, sizeof(g_actuator_app_response_buffer),
                     "Cooling system activated at %.2f°C.\r", g_sensor_app_current_data.temperature);
            CLI_SendData(g_actuator_app_response_buffer, strlen(g_actuator_app_response_buffer));
        }

        // Soil moisture actuator logic
        if (g_sensor_app_current_data.adc_soilmoisture < g_sensor_app_sensor_threshold.soil_moisture_min_adc) {
            snprintf(g_actuator_app_response_buffer, sizeof(g_actuator_app_response_buffer),
                     "Irrigation system activated due to low soil moisture.\r");
            CLI_SendData(g_actuator_app_response_buffer, strlen(g_actuator_app_response_buffer));
        }

        // Air quality actuator logic
        if (g_sensor_app_current_data.adc_airquality > g_sensor_app_sensor_threshold.air_quality_threshold_adc) {
            snprintf(g_actuator_app_response_buffer, sizeof(g_actuator_app_response_buffer),
                     "Air purifier activated due to high pollution levels.\r");
            CLI_SendData(g_actuator_app_response_buffer, strlen(g_actuator_app_response_buffer));
        }

        if (g_sensor_app_current_data.adc_waterlevel < g_sensor_app_sensor_threshold.water_level_threshold_adc) {
            snprintf(g_actuator_app_response_buffer, sizeof(g_actuator_app_response_buffer),"Water refill needs to be activated\r");
            CLI_SendData(g_actuator_app_response_buffer, strlen(g_actuator_app_response_buffer));
        }

//        snprintf(g_actuator_app_response_buffer, sizeof(g_actuator_app_response_buffer),"Actuator Worked\r");
//        CLI_SendData(g_actuator_app_response_buffer, strlen(g_actuator_app_response_buffer));
    }
}

void ACTUATOR_APP_Init(void) {

    g_actuator_app_thread_handle = osThreadNew(ACTUATOR_APP_Thread, NULL, &g_actuator_app_thread_attr);
}
