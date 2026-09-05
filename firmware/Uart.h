#pragma once // prevent multiple calling 
#include <stddef.h> 

void UART_Init(unsigned long speed);  // the speed of read and write 

size_t UART_GetLine(char *buffer, size_t maxLen); // line length i should read , maxlen so not overflow takes place 

void UART_Display(const char *message);     // the massage i print no the screen as a return 