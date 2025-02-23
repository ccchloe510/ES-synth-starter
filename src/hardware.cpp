#include "hardware.h"

// ---------------------------------------------------------------------
//                        PIN DEFINITIONS
// ---------------------------------------------------------------------

// Row select and enable pins.
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

// Matrix input and output pins.
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

// Audio analogue output pins.
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

// Joystick analogue input pins.
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

// Output multiplexer bits.
const int DEN_BIT  = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

// ---------------------------------------------------------------------
//              HARDWARE PERIPHERALS INSTANCES
// ---------------------------------------------------------------------

// Construct display object (using hardware I2C).
U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C u8g2(U8G2_R0);

// Create audio timer object.
HardwareTimer sampleTimer(TIM1);

// ---------------------------------------------------------------------
//              HARDWARE INITIALIZATION FUNCTIONS
// ---------------------------------------------------------------------

void initHardware() {
  // Set output pin modes.
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Set input pin modes.
  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);
}

void initDisplay() {
  // Initialize display: reset and enable display power.
  setOutMuxBit(DRST_BIT, LOW);  // Assert display reset.
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH); // Release reset.
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  // Enable display power supply.
}

void initAudio() {
  // Initialize audio output timer.
  sampleTimer.setOverflow(22000, HERTZ_FORMAT);
  sampleTimer.resume();
}

void setOutMuxBit(const uint8_t bitIdx, const bool value) {
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA0_PIN, bitIdx & 0x01);
  digitalWrite(RA1_PIN, bitIdx & 0x02);
  digitalWrite(RA2_PIN, bitIdx & 0x04);
  digitalWrite(OUT_PIN, value);
  digitalWrite(REN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(REN_PIN, LOW);
}

// ---------------------------------------------------------------------
//            HELPER FUNCTIONS FOR KEY SCANNING
// ---------------------------------------------------------------------

void setRow(uint8_t rowIdx) {
  digitalWrite(REN_PIN, LOW);  // Disable row selection.
  digitalWrite(RA0_PIN, rowIdx & 0x01);
  digitalWrite(RA1_PIN, rowIdx & 0x02);
  digitalWrite(RA2_PIN, rowIdx & 0x04);
  digitalWrite(REN_PIN, HIGH); // Enable row selection.
}

std::bitset<4> readCols() {
  std::bitset<4> result;
  result[0] = digitalRead(C0_PIN);
  result[1] = digitalRead(C1_PIN);
  result[2] = digitalRead(C2_PIN);
  result[3] = digitalRead(C3_PIN);
  return result;
}

