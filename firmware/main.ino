#include "Uart.h"
#include "command_prase.h"
#include "shared.h"
#include "command_rx_task.h"
#include "actuate_task.h"
#include "watchdog_task.h"
#include "status_task.h"

QueueHandle_t commandQueue;
SemaphoreHandle_t stateMutex;
Command_t last_valid_command;
unsigned long last_command_time = 0;
bool brake_active = false;
int current_throttle = 0;


void setup() {
    UART_Init(115200);
    commandQueue = xQueueCreate(10, sizeof(Command_t));
    stateMutex = xSemaphoreCreateMutex();

    if (commandQueue == NULL) UART_Display("QUEUE CREATE FAILED");
    if (stateMutex == NULL) UART_Display("MUTEX CREATE FAILED");

    ledcAttach(2, 5000, 8); 

    xTaskCreate(COMMAND_RX_Task, "COMMAND_RX", 2048, NULL, 4, NULL);
    xTaskCreate(ACTUATE_Task, "ACTUATE", 2048, NULL, 3, NULL);
    xTaskCreate(WATCHDOG_Task, "WATCHDOG", 2048, NULL, 2, NULL);
    xTaskCreate(STATUS_Task,   "STATUS",   2048, NULL, 1, NULL);
}

void loop() {
}