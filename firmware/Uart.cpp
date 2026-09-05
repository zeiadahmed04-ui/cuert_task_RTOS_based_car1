#include "Uart.h"
#include <Arduino.h>

void UART_Init(unsigned long baud) {
    Serial.begin(baud); // set the speed of read and write
}

size_t UART_GetLine(char *buffer, size_t maxLen) {
    size_t idx = 0;
    for (;;) {
        if (Serial.available()) {
            char c = Serial.read();

            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                buffer[idx] = '\0';
                return idx;
            }
            if (idx >= maxLen - 1) {
                idx = 0;
                continue;
            }
            buffer[idx] = c;
            idx++;

        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void UART_Display(const char *message) { // print the required messages
    Serial.println(message);
}