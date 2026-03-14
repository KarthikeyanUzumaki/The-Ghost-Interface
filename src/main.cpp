#include <Arduino.h>
#include <BleMouse.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// --- Pins ---
const int PIN_TR = 34, PIN_BR = 35, PIN_BL = 32, PIN_TL = 33;

// --- Advanced Config ---
const float BASE_SENSITIVITY = 5.0;   // Base speed multiplier
const float ACCELERATION_PWR = 1.8;   // Curve: 1.0 is linear, 2.0 is quadratic (highly exponential)
const float GYRO_DEADZONE = 0.02;     // Lowered because we now have filtering
const float FILTER_ALPHA = 0.3;       // 0.0 to 1.0. Lower = smoother but slight latency.
const uint32_t CLICK_DEBOUNCE = 300;  // ms

// --- Objects ---
BleMouse bleMouse("Karthik-AirMouse", "EEE", 100);
Adafruit_MPU6050 mpu;

// --- State ---
volatile bool clickDetected = false;
volatile int activeSensor = -1;
uint32_t lastClickTime = 0;

// Signal Processing State
float gyroXOffset = 0, gyroZOffset = 0;
float filteredX = 0, filteredZ = 0;
float remainderX = 0, remainderY = 0;

// --- ISRs ---
void IRAM_ATTR isr_TR() { if(!clickDetected) { activeSensor = 34; clickDetected = true; } }
void IRAM_ATTR isr_TL() { if(!clickDetected) { activeSensor = 33; clickDetected = true; } }

void calibrateMPU() {
    Serial.println("Calibrating MPU... DO NOT MOVE DEVICE!");
    float sumX = 0, sumZ = 0;
    const int numSamples = 200;
    
    for (int i = 0; i < numSamples; i++) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        sumX += g.gyro.x;
        sumZ += g.gyro.z;
        delay(5);
    }
    
    gyroXOffset = sumX / numSamples;
    gyroZOffset = sumZ / numSamples;
    Serial.println("Calibration complete.");
}

void setup() {
    Serial.begin(115200);
    
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) delay(10);
    }
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    
    calibrateMPU(); // Remove baseline drift

    pinMode(PIN_TR, INPUT);
    pinMode(PIN_TL, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_TR), isr_TR, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_TL), isr_TL, RISING);

    bleMouse.begin();
    Serial.println("Hybrid Mouse Ready!");
}

void loop() {
    if (!bleMouse.isConnected()) return;

    // 1. HANDLE CONTINUOUS MOVEMENT
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Remove zero-rate offset
    float rawX = g.gyro.x - gyroXOffset;
    float rawZ = g.gyro.z - gyroZOffset;

    // Apply EMA Low-Pass Filter
    filteredX = (FILTER_ALPHA * rawX) + ((1.0 - FILTER_ALPHA) * filteredX);
    filteredZ = (FILTER_ALPHA * rawZ) + ((1.0 - FILTER_ALPHA) * filteredZ);

    // Apply Deadzone to filtered signal
    float finalX = (abs(filteredX) < GYRO_DEADZONE) ? 0 : filteredX;
    float finalZ = (abs(filteredZ) < GYRO_DEADZONE) ? 0 : filteredZ;

    // Apply Acceleration Curve & Sensitivity
    // Using pow() creates a curve, preserving the directional sign
    float moveX_f = -1.0 * (finalZ > 0 ? 1 : -1) * pow(abs(finalZ), ACCELERATION_PWR) * BASE_SENSITIVITY;
    float moveY_f = -1.0 * (finalX > 0 ? 1 : -1) * pow(abs(finalX), ACCELERATION_PWR) * BASE_SENSITIVITY;

    // Sub-pixel Accumulation to prevent integer truncation of slow movements
    moveX_f += remainderX;
    moveY_f += remainderY;

    int moveX = (int)moveX_f;
    int moveY = (int)moveY_f;

    // Store the fractional part for the next loop
    remainderX = moveX_f - moveX;
    remainderY = moveY_f - moveY;

    if (moveX != 0 || moveY != 0) {
        bleMouse.move(moveX, moveY);
    }

    // 2. HANDLE DISCRETE CLICKS
    if (clickDetected && (millis() - lastClickTime > CLICK_DEBOUNCE)) {
        if (activeSensor == 33) { 
            bleMouse.click(MOUSE_LEFT);
            Serial.println("Left Click!");
        } 
        else if (activeSensor == 34) { 
            bleMouse.click(MOUSE_RIGHT);
            Serial.println("Right Click!");
        }

        clickDetected = false;
        activeSensor = -1;
        lastClickTime = millis();
    }

    delay(10); // 100Hz Refresh
}