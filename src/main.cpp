#include <Arduino.h>

#define PIEZO_PIN 34
const int SAMPLES_PER_GESTURE = 200; 
const int SAMPLING_PERIOD_MS = 2; // 500Hz sampling
const int THRESHOLD = 100;

int dataBuffer[SAMPLES_PER_GESTURE];
bool isRecording = false;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    Serial.println("READY TO LOG. Tap to start capture...");
}

void loop() {
    int val = analogRead(PIEZO_PIN);

    // Trigger capture when signal crosses threshold
    if (val > THRESHOLD && !isRecording) {
        isRecording = true;
        
        // Capture the window
        for (int i = 0; i < SAMPLES_PER_GESTURE; i++) {
            dataBuffer[i] = analogRead(PIEZO_PIN);
            delay(SAMPLING_PERIOD_MS); 
        }

        // Print the data in CSV format for Edge Impulse
        Serial.println("---START DATA---");
        for (int i = 0; i < SAMPLES_PER_GESTURE; i++) {
            Serial.println(dataBuffer[i]);
        }
        Serial.println("---END DATA---");

        isRecording = false;
        Serial.println("Capture complete. Ready for next...");
    }
}