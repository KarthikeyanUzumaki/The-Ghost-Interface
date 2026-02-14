#include <Arduino.h>

#define PIEZO_PIN 34
const unsigned long SAMPLE_INTERVAL_US = 250; // 4000Hz = 250 microseconds
unsigned long lastSampleTime = 0;

// Filter variables
float filteredValue = 0;
const float alpha = 0.15; // Smoothing factor (0.0 to 1.0). Lower = smoother but slower.

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
}

void loop() {
    unsigned long currentTime = micros();

    // The Non-Blocking Timer: Only run this every 250us
    if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_US) {
        lastSampleTime = currentTime;

        int rawValue = analogRead(PIEZO_PIN);

        // Step 2.2: Digital Low-Pass Filter (IIR)
        // Formula: y[n] = α * x[n] + (1 - α) * y[n-1]
        filteredValue = (alpha * rawValue) + ((1.0 - alpha) * filteredValue);

        // Print raw vs filtered to see the difference
        Serial.print("Raw:");
        Serial.print(rawValue);
        Serial.print(",Filtered:");
        Serial.println((int)filteredValue);
    }

    // You can do other things here (like BLE) and the sampling won't stop!
}