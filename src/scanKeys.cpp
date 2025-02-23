#include <Arduino.h>
#include <bitset>
#include <STM32FreeRTOS.h>
#include "scanKeys.h"
#include "globals.h"
#include "hardware.h"
#include "LockGuard.h"
#include "knob.h"
#include <atomic>
#include <cmath>
#include <ES_CAN.h>

// Audio and key constants
const uint32_t SAMPLE_RATE = 22000;
constexpr float BASE_FREQ = 440.0;
constexpr uint8_t NUM_KEYS = 12;

// Helper to calculate step size
constexpr uint32_t calcStepSize(float freq) {
  return (uint32_t)((pow(2, 32) * freq) / SAMPLE_RATE);
}

// Precalculate step sizes for each musical key
constexpr uint32_t stepSizes[NUM_KEYS] = {
  calcStepSize(BASE_FREQ * pow(2, -9.0/12)),  // C
  calcStepSize(BASE_FREQ * pow(2, -8.0/12)),  // C#
  calcStepSize(BASE_FREQ * pow(2, -7.0/12)),  // D
  calcStepSize(BASE_FREQ * pow(2, -6.0/12)),  // D#
  calcStepSize(BASE_FREQ * pow(2, -5.0/12)),  // E
  calcStepSize(BASE_FREQ * pow(2, -4.0/12)),  // F
  calcStepSize(BASE_FREQ * pow(2, -3.0/12)),  // F#
  calcStepSize(BASE_FREQ * pow(2, -2.0/12)),  // G
  calcStepSize(BASE_FREQ * pow(2, -1.0/12)),  // G#
  calcStepSize(BASE_FREQ * pow(2,  0.0/12)),  // A (440 Hz)
  calcStepSize(BASE_FREQ * pow(2,  1.0/12)),  // A#
  calcStepSize(BASE_FREQ * pow(2,  2.0/12))   // B
};

// Knob externs (defined in main.cpp)
extern std::atomic<int8_t> knob3Rotation;
extern Knob knob3Class;

void scanKeysTask(void *pvParameters) {
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Keep track of previous state to detect changes
    static std::bitset<32> previousInputs;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        std::bitset<32> localInputs;
        uint8_t lastPressedKey = 255;
        bool keyPressed = false;
        uint32_t localStepSize = 0;

        // 1) Scan rows 0..2 for key presses
        for (uint8_t row = 0; row < 3; row++) {
            setRow(row);
            delayMicroseconds(3);
            std::bitset<4> cols = readCols();
            for (uint8_t col = 0; col < 4; col++) {
                uint8_t keyIndex = row * 4 + col;
                if (keyIndex < NUM_KEYS) {
                    localInputs[keyIndex] = cols[col];
                    if (cols[col] == 0) {  // active-low => key is pressed
                        lastPressedKey = keyIndex;
                        keyPressed = true;
                    }
                }
            }
        }

        // 2) Update localStepSize if a key is pressed
        if (keyPressed) {
            localStepSize = stepSizes[lastPressedKey];
        }

        // 3) Read knob state on row 3
        setRow(3);
        delayMicroseconds(3);
        std::bitset<4> knobCols = readCols();
        uint8_t knobA = knobCols[0];
        uint8_t knobB = knobCols[1];
        knob3Class.updateState(knobA, knobB);
        knob3Class.constrainRotation();
        Serial.println(knob3Class.getRotation());

        // 4) Compare with previousInputs to detect key changes
        for (uint8_t i = 0; i < NUM_KEYS; i++) {
            bool wasPressed = (previousInputs[i] == 0); // active-low
            bool isPressed  = (localInputs[i]   == 0);

            if (wasPressed != isPressed) {
                // Key state changed
                // Create a local message
                uint8_t TX_Message[8] = {0};

                // Byte 0: 'P' (0x50) if pressed, 'R' (0x52) if released
                TX_Message[0] = isPressed ? 'P' : 'R';

                // Byte 1: Octave (example: fixed 4 or do your own mapping)
                TX_Message[1] = 4;

                // Byte 2: Note number 0..11
                TX_Message[2] = i;

                // Send over CAN with ID 0x123
                CAN_TX(0x123, TX_Message);
            }
        }

        // 5) Update the global shared state
        {
            LockGuard lock(sysState.mutex);
            sysState.inputs = localInputs;
            currentStepSize = localStepSize;
        }

        // 6) Store current as previous for next iteration
        previousInputs = localInputs;
    }
}

