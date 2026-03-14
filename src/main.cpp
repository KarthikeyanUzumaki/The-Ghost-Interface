#include <Arduino.h>

// Define ADC Pins
#define SENSOR_TL 32
#define SENSOR_TR 33
#define SENSOR_BL 34
#define SENSOR_BR 35

void setup() {
    Serial.begin(115200);
    
    pinMode(SENSOR_TL, INPUT);
    pinMode(SENSOR_TR, INPUT);
    pinMode(SENSOR_BL, INPUT);
    pinMode(SENSOR_BR, INPUT);

    Serial.println("--- Acoustic Interface Diagnostic Started ---");
}

void loop() {
    // Read raw ADC values (0 - 4095)
    int valTL = analogRead(SENSOR_TL);
    int valTR = analogRead(SENSOR_TR);
    int valBL = analogRead(SENSOR_BL);
    int valBR = analogRead(SENSOR_BR);

    // Format for PlatformIO / Arduino Serial Plotter
    // Format: ">Label:Value" or "Label:Value"
    Serial.print("TopLeft:");     Serial.print(valTL); Serial.print(",");
    Serial.print("TopRight:");    Serial.print(valTR); Serial.print(",");
    Serial.print("BottomLeft:");  Serial.print(valBL); Serial.print(",");
    Serial.print("BottomRight:"); Serial.println(valBR);

    // 10ms delay gives us a 100Hz sample rate, enough for basic tap detection
    delay(500); 
}