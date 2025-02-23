#include <Arduino.h>
#include <U8g2lib.h>
#include <bitset>
#include "display.h"
#include "globals.h"
#include "hardware.h"
#include "LockGuard.h"
#include <ES_CAN.h>

constexpr uint8_t NUM_KEYS = 12;
const char* noteNames[NUM_KEYS] = {
  "C", "C#", "D", "D#", "E", "F",
  "F#", "G", "G#", "A", "A#", "B"
};

void displayUpdateTask(void *pvParameters) {
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // No more polling for CAN here!

        // Read localInputs from global state
        std::bitset<32> localInputs;
        {
            LockGuard lock(sysState.mutex);
            localInputs = sysState.inputs;
        }

        // Update the display
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);

        u8g2.drawStr(2, 10, "Hello World!");
        u8g2.setCursor(2, 20);
        u8g2.print(localInputs.to_ulong(), HEX);

        // Show the first pressed key name
        for (uint8_t i = 0; i < NUM_KEYS; i++) {
            if (localInputs[i] == 0) {
                u8g2.setCursor(2, 30);
                u8g2.print(noteNames[i]);
                break;
            }
        }

        // Show the "last received" CAN message from RX_Message_Global
        // (Byte 0 => 'P'/'R', Byte 1 => octave, Byte 2 => note)
        u8g2.setCursor(66, 30);
        u8g2.print((char)RX_Message_Global[0]);
        u8g2.print(RX_Message_Global[1]);
        u8g2.print(RX_Message_Global[2]);

        u8g2.sendBuffer();
        digitalToggle(LED_BUILTIN);
    }
}


