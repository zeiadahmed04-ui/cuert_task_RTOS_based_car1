#include "watchdog_task.h"
#include "Uart.h"
#include "shared.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>

#define LED_PIN 2
#define TIMEOUT_MS 500

void WATCHDOG_Task(void *pv) {
    bool link_lost = false;
    bool blink_state = false;

    while(1){
        unsigned long now = millis();
        unsigned long elapsed = now - last_command_time;

        if (elapsed > TIMEOUT_MS) {
            if (!link_lost) {
                UART_Display("LINK LOST - failing safe");
                link_lost = true;
            }

            if(xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE){
                blink_state = !blink_state;
                ledcWrite(LED_PIN, blink_state ? 255 : 0);
                xSemaphoreGive(stateMutex);
            }

        } else {
            if (link_lost) {
                UART_Display("LINK RECOVERED");
                link_lost = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200)); 
    }
}