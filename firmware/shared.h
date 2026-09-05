#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "command_prase.h"

extern QueueHandle_t commandQueue;
extern SemaphoreHandle_t stateMutex;

extern Command_t last_valid_command;
extern unsigned long last_command_time;

extern bool brake_active;
extern int current_throttle;