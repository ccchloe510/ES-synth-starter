#include "globals.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <ES_CAN.h>

void CAN_TX_Task(void *pvParameters) {
    uint8_t msgOut[8];

    while(1) {
        // 1) Block until a message is available in msgOutQ
        xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);

        // 2) Wait for a free mailbox
        xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);

        // 3) Now it's safe to call CAN_TX
        CAN_TX(0x123, msgOut);

        // debug print
        Serial.print("Sent: ");
        for (int i=0; i<8; i++) {
            Serial.print(msgOut[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        
    }
}
