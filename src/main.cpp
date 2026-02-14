#include <Arduino.h>
#include <BleKeyboard.h>

/**
 * THE GHOST INTERFACE - FULL HID VERSION
 * Hardware: ESP32 + 4 Piezo Sensors
 * Features: Quadrant Detection, Software Calibration, BLE Media Control
 */

// 1. Initialize BLE Keyboard (Name, Manufacturer, Battery Level)
BleKeyboard bleKeyboard("The Ghost Interface", "Karthik R&D", 100);

// 2. Pin Definitions (ADC1 Only)
const int S_TL = 34; // Top-Left
const int S_TR = 35; // Top-Right
const int S_BL = 32; // Bottom-Left
const int S_BR = 33; // Bottom-Right

const int SENSORS[] = {S_TL, S_TR, S_BL, S_BR};
const char* NAMES[] = {"PLAY/PAUSE", "NEXT TRACK", "VOL DOWN", "VOL UP"};

// 3. CALIBRATION WEIGHTS 
// Decrease BL (0.6) and increase BR (1.5) to balance the surface sensitivity
float weights[] = {1.0, 1.0, 0.6, 1.5}; 

// 4. Detection Constants
const int THRESHOLD = 180;        // Minimum strength after weighting
const int LOCKOUT_TIME_MS = 350;  // Prevents multiple triggers per tap
unsigned long lastTapTime = 0;

void setup() {
    Serial.begin(115200);
    
    // Configure ADC
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    // Start Bluetooth
    bleKeyboard.begin();
    
    Serial.println("======================================");
    Serial.println("  GHOST INTERFACE: BLE HID ACTIVE     ");
    Serial.println("  Search for Bluetooth Device...      ");
    Serial.println("======================================");
}

void loop() {
    // Only process taps if Bluetooth is connected to a host (Phone/PC)
    if (bleKeyboard.isConnected()) {
        
        int maxStrength = 0;
        int winner = -1;

        // 1. Scan and Weighting
        for (int i = 0; i < 4; i++) {
            // Apply the weight to balance the physical sensitivity
            int raw = analogRead(SENSORS[i]);
            int adjusted = (int)(raw * weights[i]);

            if (adjusted > maxStrength) {
                maxStrength = adjusted;
                winner = i;
            }
        }

        // 2. Threshold & Timing Check
        if (maxStrength > THRESHOLD) {
            unsigned long currentTime = millis();

            if (currentTime - lastTapTime > LOCKOUT_TIME_MS) {
                
                Serial.print("🎯 DETECTED: ");
                Serial.print(NAMES[winner]);
                Serial.print(" | Adjusted Strength: ");
                Serial.println(maxStrength);

                // 3. Execute Media Commands
                switch(winner) {
                    case 0: // Top-Left
                        bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
                        break;
                    case 1: // Top-Right
                        bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
                        break;
                    case 2: // Bottom-Left
                        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
                        break;
                    case 3: // Bottom-Right
                        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
                        break;
                }

                lastTapTime = currentTime;
            }
        }
    } else {
        // Optional: Slow blink an LED or print status if not connected
        static unsigned long lastMsg = 0;
        if (millis() - lastMsg > 2000) {
            Serial.println("Waiting for BLE connection...");
            lastMsg = millis();
        }
    }
}