#ifndef KNOB_H
#define KNOB_H

#include <atomic>
#include <stdint.h>

/**
 * Knob class that manages quadrature encoder state and rotation.
 * Provides:
 * - `updateState()`: Reads A & B inputs, detects rotation
 * - `constrainRotation()`: Clamps the value to 0..8
 * - `getRotation()`: Safely reads the rotation value (thread-safe)
 */
class Knob {
public:
    // Constructor, takes a reference to the global rotation variable
    Knob(std::atomic<int8_t>& knobRotationRef)
        : knob3Rotation(knobRotationRef), prevKnobState(0b00), lastValidRotation(0) {}

    /**
     * Reads A & B inputs and updates rotation direction.
     */
    void updateState(uint8_t knobA, uint8_t knobB) {
        uint8_t currentKnobState = (knobB << 1) | knobA;

        if (prevKnobState == 0b00 && currentKnobState == 0b01) lastValidRotation = +1;
        else if (prevKnobState == 0b00 && currentKnobState == 0b10) lastValidRotation = -1;
        else if (prevKnobState == 0b01 && currentKnobState == 0b11) lastValidRotation = +1;
        else if (prevKnobState == 0b10 && currentKnobState == 0b11) lastValidRotation = -1;
        else if (prevKnobState == 0b11 && currentKnobState == 0b10) lastValidRotation = +1;
        else if (prevKnobState == 0b11 && currentKnobState == 0b01) lastValidRotation = -1;
        else if ((prevKnobState == 0b11 && currentKnobState == 0b00) ||
                 (prevKnobState == 0b00 && currentKnobState == 0b11)) {
            // Impossible state transition, assume last valid direction
            knob3Rotation.fetch_add(lastValidRotation);
        }

        prevKnobState = currentKnobState;
    }

    /**
     * Ensures rotation value stays within 0..8 range.
     */
    void constrainRotation() {
        int8_t newRotation = knob3Rotation.load() + lastValidRotation;
        if (newRotation > 8) newRotation = 8;
        if (newRotation < 0) newRotation = 0;
        knob3Rotation.store(newRotation);
    }

    /**
     * Returns the current rotation value (thread-safe).
     */
    int8_t getRotation() {
        return knob3Rotation.load();
    }

private:
    std::atomic<int8_t>& knob3Rotation;
    uint8_t prevKnobState;
    int8_t lastValidRotation;
};

#endif // KNOB_H
