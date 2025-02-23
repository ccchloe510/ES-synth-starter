#include "globals.h"
#include "scanKeys.h"
#include <FreeRTOS.h>
#include <task.h>
#include <Arduino.h>
#include <stdint.h>

// Suppose we have a stepSizes array somewhere
extern const uint32_t stepSizes[12];
extern volatile uint32_t currentStepSize;

void decodeTask(void *pvParameters) {
    uint8_t localMsg[8];

    while(1) {
        // Wait until a CAN message arrives
        xQueueReceive(msgInQ, localMsg, portMAX_DELAY);

        // localMsg now has 8 bytes from the CAN frame
        // e.g. Byte0 = 'P'/'R', Byte1 = octave, Byte2 = note
        char status = (char)localMsg[0];
        uint8_t octave = localMsg[1];
        uint8_t note   = localMsg[2];

        // If you want to store for display, do so with a mutex
        // For quick test, we can just do:
        for (int i=0; i<8; i++) {
            RX_Message_Global[i] = localMsg[i];
        }

        // Example: If 'P' => set currentStepSize
        // If 'R' => release => set stepSize=0
        if (status == 'R') {
            currentStepSize = 0;
        } else if (status == 'P') {
            // compute from note + octave
            // shift left/right by (octave-4) if needed
            // simplified example:
            currentStepSize = stepSizes[note]; 
        }

        // Print for debug
        Serial.print("Decoded: ");
        Serial.write(status);
        Serial.print(" Oct=");
        Serial.print(octave);
        Serial.print(" Note=");
        Serial.println(note);
    }
}
