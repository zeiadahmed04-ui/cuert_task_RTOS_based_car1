#include "command_rx_task.h"
#include "Uart.h"
#include "command_prase.h"
#include "shared.h"  
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

void COMMAND_RX_Task(void *pv) {
    char lineBuf[64];
    char msg[64];
    Command_t cmd;

    while(1){
        size_t len = UART_GetLine(lineBuf, sizeof(lineBuf));

        if(len == 0){
            continue; // if nothing or empty value is entered
        }
        if(!Command_Parse(lineBuf, &cmd)) {
            UART_Display("incorrect command entered");
            continue;
        }
        last_command_time = cmd.reciving_time;
        
        if(cmd.type == 'P'){
            UART_Display("PONG");  // tell me that the task is alive and still working
            continue;
        }
        else if(cmd.type == 'S'){
            snprintf(msg, sizeof(msg), "STEER received, value=%d, recived at time = %lu", cmd.value, cmd.reciving_time);
            xQueueSend(commandQueue, &cmd, 0);
            UART_Display(msg);
            continue;
        }
        else if(cmd.type == 'T'){
            snprintf(msg, sizeof(msg), "THROTTLE received, value=%d, recived at time = %lu", cmd.value, cmd.reciving_time);
            xQueueSend(commandQueue, &cmd, 0);
            UART_Display(msg);
            continue;
        }
        else if(cmd.type == 'B'){
            snprintf(msg, sizeof(msg), "BRAKE received, value=%d, recived at time = %lu", cmd.value, cmd.reciving_time);
            UART_Display(msg);

            BaseType_t sent = xQueueSend(commandQueue, &cmd, 0);
            if (sent != pdTRUE) {
                Command_t oldest_val;
                if (xQueueReceive(commandQueue, &oldest_val, 0) == pdTRUE) {
                    sent = xQueueSend(commandQueue, &cmd, 0);
                }
            }

            if (sent != pdTRUE) {
            UART_Display("WARNING: BRAKE COULD NOT BE QUEUED");
            }
            continue;
        }
    }
}
