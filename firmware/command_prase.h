#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char type;          // ping / brake / thresold / steer
    signed int value;       // values each type can hold 
    unsigned long int reciving_time; // time the both type and values were recived
} Command_t;

bool Command_Parse(const char *line, Command_t *out); // the handle of the input