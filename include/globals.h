#ifndef GLOBALS_H
#define GLOBALS_H

#include <bitset>
#include <Arduino.h>
#include <STM32FreeRTOS.h>

// Global system state structure
struct SystemState {
  std::bitset<32> inputs;
  SemaphoreHandle_t mutex;
};

// Declare the global system state instance
extern SystemState sysState;

// Declare a global step size variable (used for audio synthesis)
extern volatile uint32_t currentStepSize;

#endif // GLOBALS_H
