#include "status_task.h"
#include "Uart.h"
#include "shared.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>

void STATUS_Task(void *pv) {
    char msg[96];

    while(1){
        unsigned long uptime = millis() / 1000; // seconds since boot

        if(xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE){
            snprintf(msg, sizeof(msg),
                "STATUS: uptime=%lus last_cmd_type=%c last_cmd_value=%d throttle=%d brake=%d",
                uptime, last_valid_command.type, last_valid_command.value, current_throttle, brake_active);
            xSemaphoreGive(stateMutex);
        }

        UART_Display(msg);
        vTaskDelay(pdMS_TO_TICKS(1000)); // exactly once per second
    }
}