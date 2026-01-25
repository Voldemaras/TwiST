/* ============================================================================
 * TwiST Framework | Advanced: Distance Servo with Filtering & Hysteresis
 * ============================================================================
 *
 * ADVANCED EXAMPLE: Production-grade servo control with noise reduction.
 *
 * Behavior:
 *   - Distance < 5 cm:   Servo at 0° (too close)
 *   - Distance 5-30 cm:  Servo moves proportionally (5cm = 0°, 30cm = 180°)
 *   - Distance > 30 cm:  Servo at 180° (out of range)
 *
 * Sensitivity Control (reduces jitter):
 *   - Low-pass filter: 0.3 alpha (30% new, 70% previous value)
 *   - Hysteresis: 2 cm dead zone (servo moves only if change >= 2 cm)
 *   - Integer cm only: No decimal precision (getDistanceCm())
 *
 * This demonstrates:
 *   - Multi-device coordination (DistanceSensor + Servo)
 *   - Input-to-output mapping via application logic
 *   - Clean Architecture with TWO hardware abstractions
 *
 * Hardware Required:
 *   - ESP32-C6 (XIAO or compatible)
 *   - HC-SR04 ultrasonic distance sensor (3.3V version)
 *   - PCA9685 16-channel PWM driver (I2C: 0x40)
 *   - One servo motor on PCA9685 channel 0
 *   - External 5V power supply for servo
 *
 * Wiring:
 *   HC-SR04:
 *     ESP32 5V          → HC-SR04 VCC
 *     ESP32 GPIO16 (D6) → HC-SR04 TRIG
 *     ESP32 GPIO17 (D7) → HC-SR04 ECHO
 *     ESP32 GND         → HC-SR04 GND
 *
 *   PCA9685:
 *     ESP32 GPIO22 (D4) → PCA9685 SDA
 *     ESP32 GPIO23 (D5) → PCA9685 SCL
 *     ESP32 GND         → PCA9685 GND
 *     ESP32 3.3V        → PCA9685 VCC
 *     External 5V       → PCA9685 V+ (servo power)
 *
 *   Servo:
 *     PCA9685 Channel 0 → Servo signal
 *     External 5V       → Servo VCC
 *     GND               → Servo GND
 *
 * Author: Voldemaras Birskys
 * License: MIT
 * ============================================================================ */

#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/Drivers/Distance/HCSR04.h"
#include "src/TwiST_Framework/Drivers/PWM/PCA9685.h"

using namespace TwiST;
using namespace TwiST::Devices;
using namespace TwiST::Drivers;

// ============================================================================
// Hardware Setup - Clean Architecture with TWO abstractions
// ============================================================================

// Layer 1: Framework
TwiSTFramework framework;

// Layer 2: Hardware drivers (concrete)
HCSR04 hcsr04(16, 17);        // TRIG=GPIO16 (D6), ECHO=GPIO17 (D7)
PCA9685 pca9685(0x40);        // PCA9685 at I2C address 0x40

// Layer 3: Abstraction layer (dependency injection)
IDistanceDriver& distanceDriver = hcsr04;  // Distance abstraction
IPWMDriver& pwm = pca9685;                 // PWM abstraction

// Layer 4: Logical devices (hardware-independent)
DistanceSensor sensor(distanceDriver, 300, framework.eventBus(), 50);  // 50ms measurement interval
Servo servo(pwm, 0, 100, framework.eventBus());  // Channel 0, Device ID 100

// ============================================================================
// Application Constants - Control Parameters
// ============================================================================

// Distance range mapping (30 cm working range)
const float MIN_DISTANCE = 5.0f;    // Below 5 cm → servo at 0°
const float MAX_DISTANCE = 30.0f;   // Above 30 cm → servo at 180°

// Hysteresis (reduces jitter - servo moves only if distance changes > 2 cm)
const uint16_t HYSTERESIS_CM = 2;   // 2 cm dead zone

// Understanding sensitivity:
// HYSTERESIS = 2 cm means:
//   - If distance changes 9 cm → 10 cm: servo STAYS (change = 1 cm < 2 cm)
//   - If distance changes 9 cm → 11 cm: servo MOVES (change = 2 cm >= 2 cm)
//   - If distance changes 7 cm → 9 cm:  servo MOVES (change = 2 cm >= 2 cm)
//
// This creates "steps" in servo movement:
//   7-9 cm   → servo angle X
//   9-11 cm  → servo angle Y  (won't move between 9-10 cm)
//   11-13 cm → servo angle Z  (won't move between 11-12 cm)

// ============================================================================
// Arduino Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("   TwiST Framework");
    Serial.println("   Distance-Controlled Servo");
    Serial.println("========================================\n");

    // Initialize framework
    framework.initialize();

    // Initialize HC-SR04 driver
    hcsr04.begin();

    // Initialize PCA9685 driver
    pca9685.begin(22, 23);        // I2C pins: SDA=22, SCL=23
    pca9685.setFrequency(50);     // 50Hz for servos

    // Initialize devices
    sensor.initialize();
    servo.initialize();

    // Configure sensor filtering (reduces jitter)
    // alpha = 0.3 means: 30% new value, 70% previous value
    // Lower alpha = smoother but slower response
    sensor.setFilterStrength(0.3f);  // Moderate filtering

    // CRITICAL: Calibrate YOUR servo (use test suite to find values)
    servo.calibrateBySteps(110, 540);  // Default SG90 values

    // Register devices
    framework.registry()->registerDevice(&sensor);
    framework.registry()->registerDevice(&servo);

    Serial.println("Hardware initialized:");
    Serial.println("  HC-SR04: GPIO16(TRIG), GPIO17(ECHO)");
    Serial.println("  PCA9685: I2C 0x40, 50Hz");
    Serial.println("  Servo:   Channel 0, 110-540 steps\n");
    Serial.println("Control Logic:");
    Serial.println("  < 5 cm:   Servo at 0° (too close)");
    Serial.println("  5-30 cm:  Servo 0° to 180° (proportional)");
    Serial.println("  > 30 cm:  Servo at 180° (out of range)");
    Serial.println("\nSensitivity:");
    Serial.print("  Hysteresis: ");
    Serial.print(HYSTERESIS_CM);
    Serial.println(" cm (servo moves only if change >= 2 cm)");
    Serial.println("  Filter: 0.3 (30% new, 70% previous)\n");
    Serial.println("Starting distance-controlled servo...\n");
}

// ============================================================================
// Arduino Loop
// ============================================================================

void loop() {
    // Update framework (triggers sensor measurements and servo animations)
    framework.update();

    // Read current distance (whole centimeters only - reduces jitter)
    uint16_t distanceCm = sensor.getDistanceCm();  // Integer cm, no decimals

    // Static variable to track last commanded distance (for hysteresis)
    static uint16_t lastCommandedDistance = 0;

    // Calculate absolute change from last commanded position
    int16_t changeFromLast = abs(static_cast<int16_t>(distanceCm) - static_cast<int16_t>(lastCommandedDistance));

    // Hysteresis: Only move servo if distance changed more than threshold
    // OR if this is the first reading
    if (lastCommandedDistance == 0 || changeFromLast >= HYSTERESIS_CM) {
        // Distance changed enough - update servo position
        lastCommandedDistance = distanceCm;

        float angle;

        if (distanceCm < MIN_DISTANCE) {
            // Too close (< 5 cm): servo at 0°
            angle = 0.0f;
            Serial.print("Distance: ");
            Serial.print(distanceCm);
            Serial.println(" cm (TOO CLOSE) → Servo: 0°");

        } else if (distanceCm <= MAX_DISTANCE) {
            // Within active range (5-30 cm): proportional control
            // Map 5-30 cm → 0-180°
            // Formula: angle = ((distance - MIN) / (MAX - MIN)) * 180
            float normalizedDistance = static_cast<float>(distanceCm) - MIN_DISTANCE;
            float range = MAX_DISTANCE - MIN_DISTANCE;
            angle = (normalizedDistance / range) * 180.0f;

            // Clamp to valid range
            if (angle < 0.0f) angle = 0.0f;
            if (angle > 180.0f) angle = 180.0f;

            Serial.print("Distance: ");
            Serial.print(distanceCm);
            Serial.print(" cm → Servo: ");
            Serial.print(angle, 0);
            Serial.println("°");

        } else {
            // Out of range (> 30 cm): servo at 180°
            angle = 180.0f;
            Serial.print("Distance: ");
            Serial.print(distanceCm);
            Serial.println(" cm (OUT OF RANGE) → Servo: 180°");
        }

        servo.setAngle(angle);
    }
    // else: distance change < HYSTERESIS_CM, don't move servo (reduces jitter)

    delay(20);  // Smooth control loop
}
