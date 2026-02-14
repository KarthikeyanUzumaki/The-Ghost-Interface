#include <Arduino.h>

const int S_TL = 34; const int S_TR = 35;
const int S_BL = 32; const int S_BR = 33;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
}

void loop() {
    // Just print raw CSV lines: TL,TR,BL,BR
    Serial.print(analogRead(S_TL)); Serial.print(",");
    Serial.print(analogRead(S_TR)); Serial.print(",");
    Serial.print(analogRead(S_BL)); Serial.print(",");
    Serial.println(analogRead(S_BR));
    delay(1); // 10Hz sampling
}