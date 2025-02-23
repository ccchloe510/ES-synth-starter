#include <Arduino.h>
#include <U8g2lib.h>
#include <bitset>
#include <atomic>
#include <cmath>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>
#include "globals.h"
#include "LockGuard.h"
#include "hardware.h"
#include "scanKeys.h"
#include "display.h"
#include "knob.h"  
#include "can_tx_task.h"
#include "decodeTask.h"


// ---------------------------------------------------------------------
//                     AUDIO & KEY PARAMETERS
// ---------------------------------------------------------------------

const uint32_t SAMPLE_RATE = 22000;
constexpr float BASE_FREQ = 440.0;
constexpr uint8_t NUM_KEYS = 12;
constexpr uint8_t MAX_VOLUME = 8;

// Create an atomic variable and a knob instance.
std::atomic<int8_t> knob3Rotation(0);
Knob knob3Class(knob3Rotation);

// Audio sampling ISR – calculates waveform value and writes to the analogue output.
void sampleISR() {
  static uint32_t phaseAcc = 0;
  phaseAcc += currentStepSize;
  int32_t Vout = (phaseAcc >> 24) - 128;
  Vout = Vout >> (MAX_VOLUME - knob3Class.getRotation());
  // Convert from signed (-128..127) to unsigned (0..255) and write output.
  analogWrite(OUTR_PIN, (Vout + 128)/20);
}

// For receiving:
void CAN_RX_ISR(void) {
  uint32_t rxID = 0;
  uint8_t rxData[8] = {0};

  // Read from hardware
  CAN_RX(rxID, rxData);

  // Push to msgInQ
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(msgInQ, rxData, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// For transmitting:
void CAN_TX_ISR(void) {
  // A mailbox just freed up
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(CAN_TX_Semaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void setup() {
  // 1) Initialize hardware, display, etc. as before
  initHardware();
  initDisplay();

  // 2) Serial
  Serial.begin(9600);
  Serial.println("Hello World");

  // 3) Audio timer
  sampleTimer.attachInterrupt(sampleISR);
  initAudio();

  // 4) Create the global mutex for shared state
  sysState.mutex = xSemaphoreCreateMutex();
  if (sysState.mutex == NULL) {
      Serial.println("Mutex creation failed!");
      while (1);
  }

  // -------------------- NEW CODE BELOW --------------------
  // 5) Create the incoming CAN queue (36 items, each 8 bytes)
  msgInQ = xQueueCreate(36, 8);
  if (!msgInQ) {
    Serial.println("msgInQ creation failed!");
    while(1);
  }

  // 6) Create the outgoing CAN queue
  msgOutQ = xQueueCreate(36, 8);
  if (!msgOutQ) {
    Serial.println("msgOutQ creation failed!");
    while(1);
  }

  // 7) Create the counting semaphore for 3 Tx mailboxes
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);
  if (!CAN_TX_Semaphore) {
    Serial.println("CAN_TX_Semaphore creation failed!");
    while(1);
  }

  // 8) Initialize and start CAN
  CAN_Init(true);
  setCANFilter(0x123,0x7ff);

  // 9) Register the Rx and Tx ISRs
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_RegisterTX_ISR(CAN_TX_ISR);

  CAN_Start();

  // 10) Create tasks
  // Existing tasks
  TaskHandle_t scanKeysHandle, displayUpdateHandle;
  xTaskCreate(scanKeysTask, "scanKeys", 214, NULL, 1, &scanKeysHandle);
  xTaskCreate(displayUpdateTask, "displayUpdate", 512, NULL, 2, &displayUpdateHandle);

  // NEW tasks:
  // decodeTask to handle incoming messages from msgInQ
  xTaskCreate(decodeTask, "decodeTask", 256, NULL, 2, NULL);

  // CAN_TX_Task to handle outgoing messages from msgOutQ
  xTaskCreate(CAN_TX_Task, "canTxTask", 256, NULL, 3, NULL);

  // 11) Start scheduler
  vTaskStartScheduler();
}


void loop() {

}



