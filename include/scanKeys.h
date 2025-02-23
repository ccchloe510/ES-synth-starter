#ifndef SCANKEYS_H
#define SCANKEYS_H

#include <STM32FreeRTOS.h>

extern const uint32_t stepSizes[12];

// Task function for scanning keys.
void scanKeysTask(void *pvParameters);

#endif // SCANKEYS_H
