#include <Arduino.h>
#include <U8g2lib.h>
#include <bitset>
#include "display.h"
#include "globals.h"
#include "hardware.h"
#include "LockGuard.h"
#include <ES_CAN.h>

// We'll assume NUM_KEYS = 12
constexpr uint8_t NUM_KEYS = 12;

// Note names for local scanning display
const char* noteNames[NUM_KEYS] = {
  "C", "C#", "D", "D#", "E", "F",
  "F#", "G", "G#", "A", "A#", "B"
};

// We'll store the "last received" CAN message so we can display it
static uint8_t lastCANMsg[8] = {0};

void displayUpdateTask(void *pvParameters) {
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // 1) Poll for any new CAN messages
        while (CAN_CheckRXLevel() > 0) {
            uint32_t rxID = 0;
            uint8_t RX_Message[8] = {0};
            CAN_RX(rxID, RX_Message);

            // If ID matches 0x123, save it
            if (rxID == 0x123) {
                for (int i = 0; i < 8; i++) {
                    lastCANMsg[i] = RX_Message[i];
                }
            }
        }

        // 2) Read localInputs from global state (mutex protected)
        std::bitset<32> localInputs;
        {
            LockGuard lock(sysState.mutex);
            localInputs = sysState.inputs;
        }

        // 3) Clear and update the display buffer
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);

        // Example text
        u8g2.drawStr(2, 10, "Hello World!");
        u8g2.setCursor(2, 20);
        u8g2.print(localInputs.to_ulong(), HEX);

        // Show the first pressed key name
        for (uint8_t i = 0; i < NUM_KEYS; i++) {
            if (localInputs[i] == 0) { // active-low => pressed
                u8g2.setCursor(2, 30);
                u8g2.print(noteNames[i]);
                break;
            }
        }

        // 4) Show the "last received" CAN message
        //    Byte 0 is a character ('P' or 'R'), so cast to char
        //    Byte 1 is octave, Byte 2 is note
        u8g2.setCursor(66, 30);
        u8g2.print((char) lastCANMsg[0]); // 'P' or 'R'
        u8g2.print(lastCANMsg[1]);       // Octave
        u8g2.print(lastCANMsg[2]);       // Note

        // 5) Send to display
        u8g2.sendBuffer();
        digitalToggle(LED_BUILTIN);
    }
}

