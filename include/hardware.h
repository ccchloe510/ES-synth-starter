#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <bitset>

// ---------------------------------------------------------------------
//                        PIN CONFIGURATIONS
// ---------------------------------------------------------------------

// Row select and enable pins.
extern const int RA0_PIN;
extern const int RA1_PIN;
extern const int RA2_PIN;
extern const int REN_PIN;

// Matrix input and output pins.
extern const int C0_PIN;
extern const int C1_PIN;
extern const int C2_PIN;
extern const int C3_PIN;
extern const int OUT_PIN;

// Audio analogue output pins.
extern const int OUTL_PIN;
extern const int OUTR_PIN;

// Joystick analogue input pins.
extern const int JOYY_PIN;
extern const int JOYX_PIN;

// Output multiplexer bits.
extern const int DEN_BIT;
extern const int DRST_BIT;
extern const int HKOW_BIT;
extern const int HKOE_BIT;

// ---------------------------------------------------------------------
//                       HARDWARE PERIPHERALS
// ---------------------------------------------------------------------

// Display object (using hardware I2C).
extern U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C u8g2;

// Hardware timer for audio sampling.
extern HardwareTimer sampleTimer;

// ---------------------------------------------------------------------
//                 INITIALIZATION FUNCTIONS
// ---------------------------------------------------------------------

// Configure all microcontroller pins.
void initHardware();

// Initialize the display (reset and enable).
void initDisplay();

// Initialize the audio timer.
void initAudio();

// Set an output multiplexer bit (used by the display driver).
void setOutMuxBit(const uint8_t bitIdx, const bool value);

// ---------------------------------------------------------------------
//            HELPER FUNCTIONS FOR KEY SCANNING
// ---------------------------------------------------------------------

// Sets the key matrix row.
void setRow(uint8_t rowIdx);

// Reads the state of the key matrix columns.
std::bitset<4> readCols();

#endif // HARDWARE_H

