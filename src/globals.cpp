#include "globals.h"

SystemState sysState;
volatile uint32_t currentStepSize = 0;

// runtime config
bool isSender = true;         // default is sender, can be changed
uint8_t moduleOctave = 4;     // default octave

// Queues and semaphores
QueueHandle_t msgInQ = NULL;
QueueHandle_t msgOutQ = NULL;
SemaphoreHandle_t CAN_TX_Semaphore = NULL;
uint8_t RX_Message_Global[8] = {0};


