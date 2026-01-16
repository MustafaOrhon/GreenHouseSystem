/*
 * cli.c
 *
 * Created on: May 7, 2024
 * Author: Mustafa
 */

#include "cli.h"
#include <string.h> // For strcmp
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "sensor_app.h"
#include "cmsis_os.h" // Include the CMSIS-RTOS API

#define CLI_MENU \
"Available commands:\n" \
"- temperature: Measure the temperature\n" \
"- humidity: Measure the humidity\n" \
"- light: Control the light system\n" \
"- soil: Check soil moisture\n" \
"- airquality: Check air quality\n" \
"- waterlevel: Check water level\n" \
"- set_temp_min [value]: Set minimum temperature threshold\n" \
"- set_temp_max [value]: Set maximum temperature threshold\n" \
"- set_soil_min [value]: Set minimum soil moisture ADC\n" \
"- set_soil_max [value]: Set maximum soil moisture ADC\n" \
"- set_air_thresh [value]: Set air quality threshold ADC\n" \
"- set_water_min [value]: Set minimum water level ADC\n" \
"- set_water_max [value]: Set maximum water level ADC\n" \
"- set_water_thresh [value]: Set water level threshold ADC\n"

#define CLI_MENU_SIZE (sizeof(CLI_MENU) - 1)
#define RESPONSE_BUFFER_SIZE   256
/*STATIC FUNCTION Declarations*/
static void CLI_HandleTemperature(void);
static void CLI_HandleHumidity(void);
static void CLI_HandleLight(void);
static void CLI_HandleSoil(void);
static void CLI_HandleAirQuality(void);
static void CLI_HandleWaterLevel(void);
static void CLI_HandleSetTemperatureMin(void);
static void CLI_HandleSetTemperatureMax(void);
static void CLI_HandleSetSoilMoistureMin(void);
static void CLI_HandleSetSoilMoistureMax(void);
static void CLI_HandleSetAirQualityThreshold(void);
static void CLI_HandleSetWaterLevelMin(void);
static void CLI_HandleSetWaterLevelMax(void);
static void CLI_HandleSetWaterLevelThreshold(void);


/* UART handle extern declaration (adjust as per your setup) */
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;
extern sSenosorThresholds_t g_sensor_app_sensor_threshold;

#define CLI_WORKING_UART_PORT huart2

/* Macro to define commands more easily, including the command length automatically */
#define DEFINE_CMD(cmd, handler_func) { \
    .command = #cmd, \
    .command_size = sizeof(#cmd) - 1, \
    .handler = handler_func \
}

typedef enum {
    eCommandList_First = 0,
    eCommandList_ShowTemperature = eCommandList_First,
    eCommandList_ShowHumidity,
    eCommandList_ShowLight,
    eCommandList_ShowSoil,
    eCommandList_ShowAirQuality,
    eCommandList_ShowWaterLevel,
    eCommandList_SetTempMin,
    eCommandList_SetTempMax,
    eCommandList_SetSoilMin,
    eCommandList_SetSoilMax,
    eCommandList_SetAirThresh,
    eCommandList_SetWaterMin,
    eCommandList_SetWaterMax,
    eCommandList_SetWaterThresh,
    eCommandList_Last // Always last
} eCommandList_t;

static const sCLICommand_t g_command_table[eCommandList_Last] = {
    DEFINE_CMD(temperature, CLI_HandleTemperature),
    DEFINE_CMD(humidity, CLI_HandleHumidity),
    DEFINE_CMD(light, CLI_HandleLight),
    DEFINE_CMD(soil, CLI_HandleSoil),
    DEFINE_CMD(airquality, CLI_HandleAirQuality),
    DEFINE_CMD(waterlevel, CLI_HandleWaterLevel),
	DEFINE_CMD(set_temp_min, CLI_HandleSetTemperatureMin),
	DEFINE_CMD(set_temp_max, CLI_HandleSetTemperatureMax),
	DEFINE_CMD(set_soil_min, CLI_HandleSetSoilMoistureMin),
	DEFINE_CMD(set_soil_max, CLI_HandleSetSoilMoistureMax),
	DEFINE_CMD(set_air_thresh, CLI_HandleSetAirQualityThreshold),
	DEFINE_CMD(set_water_min, CLI_HandleSetWaterLevelMin),
	DEFINE_CMD(set_water_max, CLI_HandleSetWaterLevelMax),
	DEFINE_CMD(set_water_thresh, CLI_HandleSetWaterLevelThreshold),
    // Add additional commands here
};

static const osThreadAttr_t task_attributes = {
    .name = "CLI_Task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal
};
/* Calculate the number of commands automatically */
static const int command_count = sizeof(g_command_table) / sizeof(g_command_table[0]);

/* Message queue declaration */
static osMessageQueueId_t g_cli_queue;
static osMutexId_t g_uart_mutex_handle;
static const osMutexAttr_t uartMutexAttributes = {
    .name = "uartMutex"
};
static char g_received_command[128] = {0}; // Adjust based on expected input length
static char g_uart_tx_buffer[RESPONSE_BUFFER_SIZE] = {0};
static void CLI_Task(void *argument);
static bool CLI_ProcessReceivedData(const char *data);
/*STATIC FUNCTION Definitions*/

static void CLI_HandleTemperature(void) {
    if (!SENSOR_APP_GetFormattedData(SENSOR_TEMPERATURE, g_uart_tx_buffer, RESPONSE_BUFFER_SIZE)) {
        CLI_SendData(g_uart_tx_buffer, snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Internal Error\r"));
    } else {
        CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
    }
}

static void CLI_HandleHumidity(void) {
    if (!SENSOR_APP_GetFormattedData(SENSOR_HUMIDITY, g_uart_tx_buffer, RESPONSE_BUFFER_SIZE)) {
        CLI_SendData(g_uart_tx_buffer, snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Internal Error\r"));
    } else {
        CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
    }
}

static void CLI_HandleLight(void) {
    if (!SENSOR_APP_GetFormattedData(SENSOR_LIGHT, g_uart_tx_buffer, RESPONSE_BUFFER_SIZE)) {
        CLI_SendData(g_uart_tx_buffer, snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Internal Error\r"));
    } else {
        CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
    }
}

static void CLI_HandleSoil(void) {
    if (!SENSOR_APP_GetFormattedData(SENSOR_SOIL, g_uart_tx_buffer, RESPONSE_BUFFER_SIZE)) {
        CLI_SendData(g_uart_tx_buffer, snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Internal Error\r"));
    } else {
        CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
    }
}

static void CLI_HandleAirQuality(void) {
    if (!SENSOR_APP_GetFormattedData(SENSOR_AIRQUALITY, g_uart_tx_buffer, RESPONSE_BUFFER_SIZE)) {
        CLI_SendData(g_uart_tx_buffer, snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Internal Error\r"));
    } else {
        CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
    }
}

static void CLI_HandleWaterLevel(void) {
    if (!SENSOR_APP_GetFormattedData(SENSOR_WATERLEVEL, g_uart_tx_buffer, RESPONSE_BUFFER_SIZE)) {
        CLI_SendData(g_uart_tx_buffer, snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Internal Error\r"));
    } else {
        CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
    }
}

static void CLI_HandleSetTemperatureMin(void) {
    uint16_t newThreshold;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetTempMin].command_size + 1, "%hu", &newThreshold) == 1) {
        if (newThreshold <= g_sensor_app_sensor_threshold.temperature_max_threshold) {
            g_sensor_app_sensor_threshold.temperature_min_threshold = newThreshold;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Temperature minimum threshold set to %hu.\r\n", newThreshold);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or range error for temperature minimum threshold.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input format for temperature minimum threshold.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}

static void CLI_HandleSetTemperatureMax(void) {
    uint16_t newThreshold;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetTempMax].command_size + 1, "%hu", &newThreshold) == 1) {
        if (newThreshold >= g_sensor_app_sensor_threshold.temperature_min_threshold) {
            g_sensor_app_sensor_threshold.temperature_max_threshold = newThreshold;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Temperature maximum threshold set to %hu.\r\n", newThreshold);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or range error for temperature maximum threshold.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input format for temperature maximum threshold.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}

static void CLI_HandleSetSoilMoistureMin(void) {
    uint16_t newAdc;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetSoilMin].command_size + 1, "%hu", &newAdc) == 1) {
        if (newAdc >= 0 && newAdc <= 4095 && newAdc <= g_sensor_app_sensor_threshold.soil_moisture_max_adc) {
            g_sensor_app_sensor_threshold.soil_moisture_min_adc = newAdc;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Soil moisture minimum ADC set to %hu.\r\n", newAdc);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or logical inconsistency for soil moisture minimum ADC.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input for soil moisture minimum ADC.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}

static void CLI_HandleSetSoilMoistureMax(void) {
    uint16_t newAdc;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetSoilMax].command_size + 1, "%hu", &newAdc) == 1) {
        if (newAdc >= 0 && newAdc <= 4095 && newAdc >= g_sensor_app_sensor_threshold.soil_moisture_min_adc) {
            g_sensor_app_sensor_threshold.soil_moisture_max_adc = newAdc;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Soil moisture maximum ADC set to %hu.\r\n", newAdc);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or logical inconsistency for soil moisture maximum ADC.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input for soil moisture maximum ADC.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}


static void CLI_HandleSetAirQualityThreshold(void) {
    uint16_t newAdc;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetAirThresh].command_size + 1, "%hu", &newAdc) == 1) {
        if (newAdc >= 0 && newAdc <= 4095) {
            g_sensor_app_sensor_threshold.air_quality_threshold_adc = newAdc;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Air quality threshold ADC set to %hu.\r\n", newAdc);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input for air quality threshold ADC.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input format for air quality threshold ADC.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}

static void CLI_HandleSetWaterLevelMin(void) {
    uint16_t newAdc;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetWaterMin].command_size + 1, "%hu", &newAdc) == 1) {
        if (newAdc >= 0 && newAdc <= 4095 && newAdc <= g_sensor_app_sensor_threshold.water_level_threshold_adc) {
            g_sensor_app_sensor_threshold.water_level_min_adc = newAdc;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Water level minimum ADC set to %hu.\r\n", newAdc);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or range error for water level minimum ADC.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input format for water level minimum ADC.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}

static void CLI_HandleSetWaterLevelMax(void) {
    uint16_t newAdc;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetWaterMax].command_size + 1, "%hu", &newAdc) == 1) {
        if (newAdc >= 0 && newAdc <= 4095 && newAdc >= g_sensor_app_sensor_threshold.water_level_min_adc) {
            g_sensor_app_sensor_threshold.water_level_max_adc = newAdc;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Water level maximum ADC set to %hu.\r\n", newAdc);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or range error for water level maximum ADC.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input format for water level maximum ADC.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}

static void CLI_HandleSetWaterLevelThreshold(void) {
    uint16_t newAdc;
    if (sscanf(g_received_command + g_command_table[eCommandList_SetWaterThresh].command_size + 1, "%hu", &newAdc) == 1) {
        if (newAdc >= 0 && newAdc <= 4095 && newAdc >= g_sensor_app_sensor_threshold.water_level_min_adc) {
            g_sensor_app_sensor_threshold.water_level_threshold_adc = newAdc;
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Water level threshold ADC set to %hu.\r\n", newAdc);
        } else {
            snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input or logical inconsistency for water level threshold ADC.\r\n");
        }
    } else {
        snprintf(g_uart_tx_buffer, RESPONSE_BUFFER_SIZE, "Invalid input format for water level threshold ADC.\r\n");
    }
    CLI_SendData(g_uart_tx_buffer, strlen(g_uart_tx_buffer));
}


void CLI_SendData(const char *data, uint16_t size) {
    // Acquire the mutex to ensure exclusive access to UART DMA
    if (osMutexAcquire(g_uart_mutex_handle, 100) == osOK) {
        // Transmit data using DMA
        if (HAL_UART_Transmit(&CLI_WORKING_UART_PORT, (uint8_t *)data, size,1000) != HAL_OK) {
            // Handle possible error
        }
    }
    osMutexRelease(g_uart_mutex_handle);
}

void CLI_Init(void) {
    g_cli_queue = osMessageQueueNew(10, sizeof(g_received_command), NULL);
    osThreadNew(CLI_Task, NULL, &task_attributes);
    g_uart_mutex_handle = osMutexNew(&uartMutexAttributes);
}

static void CLI_Task(void *argument) {
	CLI_SendData(CLI_MENU, CLI_MENU_SIZE);
	while (1) {
		if (osMessageQueueGet(g_cli_queue, g_received_command, NULL,osWaitForever) == osOK) {
			if (CLI_ProcessReceivedData(g_received_command) == false) {
				CLI_SendData(g_uart_tx_buffer,snprintf(g_uart_tx_buffer,RESPONSE_BUFFER_SIZE,"Command Not Found\r\n"));
			}
		}
	}
}

bool CLI_ProcessReceivedData(const char *data) {
    for (int i = 0; i < command_count; i++) {
        // Check if the command matches the beginning of the data string
        if (strncmp(g_command_table[i].command, data, g_command_table[i].command_size) == 0) {
            g_command_table[i].handler();
            return true; // Command was found and handled
        }
    }
    return false; // No command matched
}

void CLI_PostCommand(const char *data) {
    osMessageQueuePut(g_cli_queue, data, 0, 0);
}

