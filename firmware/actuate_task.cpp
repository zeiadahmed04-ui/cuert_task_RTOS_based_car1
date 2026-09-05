#include "actuate_task.h"
#include "Uart.h"
#include "command_prase.h"
#include "shared.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <Arduino.h>

#define LED_PIN 2

void ACTUATE_Task(void *pv) {
    Command_t cmd;
    char msg[64];

    while(1){
        if(xQueueReceive(commandQueue, &cmd, portMAX_DELAY) == pdTRUE){

            if(xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE){

                last_valid_command = cmd;

                if(cmd.type == 'T'){
                    current_throttle = cmd.value;
                }
                else if(cmd.type == 'B'){
                    if(cmd.value > 0)
                        brake_active = true;
                    else
                        brake_active = false;
                }
                // STEER doesn't change throttle or brake

                int duty;
                if(brake_active)
                    duty = 0;
                else
                    duty = (current_throttle * 255) / 100;

                ledcWrite(LED_PIN, duty);

                xSemaphoreGive(stateMutex);

                snprintf(msg, sizeof(msg), "ACTUATE: type=%c value=%d duty=%d brake=%d", cmd.type, cmd.value, duty, brake_active);
                UART_Display(msg);
            }
        }
    }
}