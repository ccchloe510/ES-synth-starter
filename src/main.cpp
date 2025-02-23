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

void setup() {
  // Initialize hardware: pin configurations and peripherals.
  initHardware();
  initDisplay();

  // Initialize Serial communication.
  Serial.begin(9600);
  Serial.println("Hello World");

  // Attach the audio sampling ISR and initialize audio.
  sampleTimer.attachInterrupt(sampleISR);
  initAudio();

  // Create the global mutex for shared state.
  sysState.mutex = xSemaphoreCreateMutex();
  if (sysState.mutex == NULL) {
      Serial.println("Mutex creation failed!");
      while (1);
  }

  // Initialize the CAN bus
  CAN_Init(true);
  setCANFilter(0x123,0x7ff);
  CAN_Start();

  // Create tasks for scanning keys and updating the display.
  TaskHandle_t scanKeysHandle = NULL;
  xTaskCreate(scanKeysTask, "scanKeys", 214, NULL, 1, &scanKeysHandle);

  TaskHandle_t displayUpdateHandle = NULL;
  xTaskCreate(displayUpdateTask, "displayUpdate", 512, NULL, 2, &displayUpdateHandle);

  // Start the FreeRTOS scheduler.
  vTaskStartScheduler();
}

void loop() {
  // Empty loop – tasks manage all operations.
}



