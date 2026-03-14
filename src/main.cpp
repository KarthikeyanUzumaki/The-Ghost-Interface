#include <Arduino.h>

// Sensor Pin Definitions
const int PIN_TR = 34; // Top Right
const int PIN_BR = 35; // Bottom Right
const int PIN_BL = 32; // Bottom Left
const int PIN_TL = 33; // Top Left

// Timing variables (micros)
volatile uint32_t t_TR = 0, t_BR = 0, t_BL = 0, t_TL = 0;
uint32_t last_event_time = 0;

// Configurable Thresholds
const uint32_t CAPTURE_WINDOW = 5000; // 5ms window to catch all waves
const uint32_t DEBOUNCE_TIME = 200;    // 200ms lock after a success

// ISRs (Interrupt Service Routines)
void IRAM_ATTR isr_TR() { if (t_TR == 0) t_TR = micros(); }
void IRAM_ATTR isr_BR() { if (t_BR == 0) t_BR = micros(); }
void IRAM_ATTR isr_BL() { if (t_BL == 0) t_BL = micros(); }
void IRAM_ATTR isr_TL() { if (t_TL == 0) t_TL = micros(); }

void process_touch() {
    // Calculate relative time differences (Deltas)
    // We treat the first sensor hit as the reference point
    long deltaX = (int32_t)((t_TR + t_BR) - (t_TL + t_BL));
    long deltaY = (int32_t)((t_BL + t_BR) - (t_TL + t_TR));

    Serial.print("--- TOUCH DETECTED ---");
    Serial.print("  X_Delta: "); Serial.print(deltaX);
    Serial.print("  Y_Delta: "); Serial.println(deltaY);

    // Reset timestamps for next capture
    t_TR = t_BR = t_BL = t_TL = 0;
    last_event_time = millis();
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_TR, INPUT);
    pinMode(PIN_BR, INPUT);
    pinMode(PIN_BL, INPUT);
    pinMode(PIN_TL, INPUT);

    attachInterrupt(digitalPinToInterrupt(PIN_TR), isr_TR, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_BR), isr_BR, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_BL), isr_BL, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_TL), isr_TL, RISING);

    Serial.println("System Online. Tap surface to calibrate.");
}

void loop() {
    // Detect if any sensor has started a timing sequence
    uint32_t first_hit = 0;
    if (t_TR > 0) first_hit = t_TR;
    else if (t_BR > 0) first_hit = t_BR;
    else if (t_BL > 0) first_hit = t_BL;
    else if (t_TL > 0) first_hit = t_TL;

    if (first_hit > 0) {
        // Wait for waves to travel across the whole surface
        delayMicroseconds(CAPTURE_WINDOW);
        
        // Ensure we aren't within the debounce lockout period
        if (millis() - last_event_time > DEBOUNCE_TIME) {
            process_touch();
        } else {
            // Clear noise/echoes if still in debounce period
            t_TR = t_BR = t_BL = t_TL = 0;
        }
    }
}