#include "command_prase.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


bool Command_Parse(const char *line, Command_t *out) {



    if (strncmp(line, "PING", 4) == 0) {
        out->type = 'P';
        out->value = 0;
        out->reciving_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        return true;
    }
    else if (strncmp(line, "THROTTLE", 8) == 0) {
        int val;
        sscanf(line, "THROTTLE %d", &val);
            if(val >= 0 && val <= 100 ) {
                out->type = 'T';
                out->value = val;
                out->reciving_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                return true;
            }
        return false;
    }
    else if (strncmp(line, "STEER", 5) == 0) {
        int val;
        sscanf(line, "STEER %d", &val);
            if(val >= -100 && val <= 100 ) {
                out->type = 'S';
                out->value = val;
                out->reciving_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                return true;
            }
        return false;
    }
    else if (strncmp(line, "BRAKE", 5) == 0) {
        int val;    
        sscanf(line, "BRAKE %d", &val);
            if(val >= 0 && val <= 100 ) {
                out->type = 'B';
                out->value = val;
                out->reciving_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                return true;
            }
        return false;
    }
    
    return false;
}