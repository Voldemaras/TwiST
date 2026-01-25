/* ============================================================================
 * TwiST Framework | Basic Servo Example
 * ============================================================================
 *
 * MINIMAL EXAMPLE: One servo, one movement, clean architecture showcase.
 *
 * This demonstrates TwiST's core principle:
 *   Device (Servo) → Interface (IPWMDriver) → Hardware (PCA9685)
 *
 * Hardware Required:
 *   - ESP32-C6 (XIAO or compatible)
 *   - PCA9685 16-channel PWM driver (I2C: 0x40)
 *   - One servo motor on channel 0
 *   - External 5V power supply for servo
 *
 * Wiring:
 *   ESP32 GPIO22 (D4) → PCA9685 SDA
 *   ESP32 GPIO23 (D5) → PCA9685 SCL
 *   ESP32 GND         → PCA9685 GND
 *   ESP32 3.3V        → PCA9685 VCC
 *   External 5V       → PCA9685 V+ (servo power)
 *   Servo signal      → PCA9685 channel 0
 *
 * CRITICAL: Calibrate YOUR servo first!
 *   1. Open: PROJEKTAS/test/test.ino
 *   2. Run validation suite
 *   3. Update line 35 with YOUR values
 *
 * Author: Voldemaras Birskys
 * License: MIT
 * ============================================================================ */

#include "../../main/src/TwiST_Framework/TwiST.h"
#include "../../main/src/TwiST_Framework/Drivers/PWM/PCA9685.h"

using namespace TwiST;
using namespace TwiST::Devices;
using namespace TwiST::Drivers;

// ============================================================================
// Hardware Setup - Clean Architecture
// ============================================================================

// Layer 1: Framework
TwiSTFramework framework;

// Layer 2: Hardware driver (concrete)
PCA9685 pca9685(0x40);

// Layer 3: Abstraction layer (dependency injection)
IPWMDriver& pwm = pca9685;  // Device sees only interface, not hardware

// Layer 4: Logical device (hardware-independent)
Servo servo(pwm, 0, 100, framework.eventBus());


// Constructor injection - all dependencies provided

// ============================================================================
// Arduino Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("   TwiST Framework - Basic Servo");
    Serial.println("========================================\n");

    // Initialize framework
    framework.initialize();

    // Initialize hardware
    pca9685.begin(22, 23);        // I2C pins: SDA=22, SCL=23
    pca9685.setFrequency(50);     // 50Hz for servos

    // Initialize device
    servo.initialize();

    // CRITICAL: Use YOUR calibrated values!
    servo.calibrateBySteps(110, 540);  // Default SG90 values

    // Register device
    framework.registry()->registerDevice(&servo);

    Serial.println("Servo initialized");
    Serial.println("Calibration: 110-540 steps (0-180°)");
    Serial.println("\nStarting sweep animation...\n");
}

// ============================================================================
// Arduino Loop
// ============================================================================

void loop() {
    // Smooth sweep: 0° → 180° → 0° with easing
    static enum { MOVING_TO_180, MOVING_TO_0 } state = MOVING_TO_180;

    if (!servo.isMoving()) {
        switch (state) {
            case MOVING_TO_180:
                Serial.println("→ Moving to 180° (3 seconds, S-curve)");
                servo.moveToWithEasing(180, 3000, Servo::EASE_IN_OUT_QUAD);
                state = MOVING_TO_0;
                break;

            case MOVING_TO_0:
                Serial.println("→ Moving to 0° (3 seconds, S-curve)");
                servo.moveToWithEasing(0, 3000, Servo::EASE_IN_OUT_QUAD);
                state = MOVING_TO_180;
                break;
        }
    }

    // CRITICAL: framework.update() processes animations
    framework.update();
    delay(20);
}
