#ifndef DISPLAY_H
#define DISPLAY_H

#include <STM32FreeRTOS.h>

// Task function for updating the display.
void displayUpdateTask(void *pvParameters);

#endif // DISPLAY_H
