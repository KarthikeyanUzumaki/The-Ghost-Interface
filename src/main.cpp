#include <Arduino.h>

#define PIEZO_PIN 34
const unsigned long SAMPLE_INTERVAL_US = 250; 
unsigned long lastSampleTime = 0;

// Filter & Detection
float filteredValue = 0;
const float alpha = 0.15; 
const int THRESHOLD = 100;       // Adjust this based on your tap strength
const int LOCKOUT_TIME_MS = 150; // Ignore vibrations for 150ms after a tap
unsigned long lastTapTime = 0;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    Serial.println("System Ready. Tap the surface!");
}

void loop() {
    unsigned long currentTimeUS = micros();
    unsigned long currentTimeMS = millis();

    // 1. Fixed Frequency Sampling
    if (currentTimeUS - lastSampleTime >= SAMPLE_INTERVAL_US) {
        lastSampleTime = currentTimeUS;

        int rawValue = analogRead(PIEZO_PIN);
        filteredValue = (alpha * rawValue) + ((1.0 - alpha) * filteredValue);

        // 2. Peak Detection Logic
        if (filteredValue > THRESHOLD) {
            // Check if enough time has passed since the last tap (Lockout)
            if (currentTimeMS - lastTapTime > LOCKOUT_TIME_MS) {
                
                // --- ACTION START ---
                Serial.print("!!! GHOST TAP DETECTED !!! Strength: ");
                Serial.println((int)filteredValue);
                // --- ACTION END ---

                lastTapTime = currentTimeMS; // Reset the timer
            }
        }
    }
}