#ifndef GLOBALS_H
#define GLOBALS_H

#include <bitset>
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

// Our system state
struct SystemState {
  std::bitset<32> inputs;
  SemaphoreHandle_t mutex;
};

// Global system state
extern SystemState sysState;
extern volatile uint32_t currentStepSize;

// runtime config --
extern bool isSender;         // true => sender, false => receiver
extern uint8_t moduleOctave;  // e.g. 4, 5, etc.

// Queues and semaphores
extern QueueHandle_t msgInQ;
extern QueueHandle_t msgOutQ;
extern SemaphoreHandle_t CAN_TX_Semaphore;
extern uint8_t RX_Message_Global[8];

#endif // GLOBALS_H


