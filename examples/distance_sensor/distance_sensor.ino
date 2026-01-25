/* ============================================================================
 * TwiST Framework | Distance Sensor Example
 * ============================================================================
 *
 * MINIMAL EXAMPLE: One HC-SR04 ultrasonic sensor, clean architecture showcase.
 *
 * This demonstrates TwiST's core principle:
 *   Device (DistanceSensor) → Interface (IDistanceDriver) → Hardware (HCSR04)
 *
 * Hardware Required:
 *   - ESP32-C6 (XIAO or compatible)
 *   - HC-SR04 ultrasonic distance sensor
 *   - Voltage divider for ECHO pin (1kΩ + 2kΩ resistors)
 *
 * Wiring:
 *   ESP32 5V          → HC-SR04 VCC
 *   ESP32 GPIO21 (D7) → HC-SR04 TRIG
 *   ESP32 GPIO22 (D8) → HC-SR04 ECHO (via voltage divider!)
 *   ESP32 GND         → HC-SR04 GND
 *
 *   CRITICAL: ECHO pin outputs 5V - ESP32 is 3.3V logic!
 *   Voltage divider circuit:
 *     ECHO → 1kΩ → GPIO22 → 2kΩ → GND
 *     (5V × 2kΩ/(1kΩ+2kΩ) = 3.3V)
 *
 * Expected Behavior:
 *   - Distance printed every 100ms
 *   - Range: 2cm - 400cm
 *   - Resolution: 0.3cm
 *
 * Author: Voldemaras Birskys
 * License: MIT
 * ============================================================================ */

#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/Drivers/Distance/HCSR04.h"

using namespace TwiST;
using namespace TwiST::Devices;
using namespace TwiST::Drivers;

// ============================================================================
// Hardware Setup - Clean Architecture
// ============================================================================

// Layer 1: Framework
TwiSTFramework framework;

// Layer 2: Hardware driver (concrete)
HCSR04 hcsr04(16, 17);  // TRIG=GPIO16 (D6), ECHO=GPIO17 (D7)

// Layer 3: Abstraction layer (dependency injection)
IDistanceDriver& distanceDriver = hcsr04;  // Device sees only interface, not hardware

// Layer 4: Logical device (hardware-independent)
DistanceSensor sensor(distanceDriver, 300, framework.eventBus(), 100);  // 100ms interval

// ============================================================================
// Arduino Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("   TwiST Framework - Distance Sensor");
    Serial.println("========================================\n");

    // Initialize framework
    framework.initialize();

    // Initialize hardware
    hcsr04.begin();

    // Initialize device
    sensor.initialize();

    // Register device
    framework.registry()->registerDevice(&sensor);

    Serial.println("HC-SR04 Ultrasonic Sensor initialized");
    Serial.println("Range: 2cm - 400cm");
    Serial.println("Resolution: 0.3cm");
    Serial.println("\nWARNING: Ensure ECHO pin uses voltage divider!");
    Serial.println("         (5V → 3.3V protection)\n");
    Serial.println("Starting distance measurement...\n");
}

// ============================================================================
// Arduino Loop
// ============================================================================

void loop() {
    // Update framework (triggers automatic measurements)
    framework.update();

    // Read current distance
    float distance = sensor.getDistance();

    // Print distance
    if (sensor.isInRange()) {
        Serial.print("Distance: ");
        Serial.print(distance, 1);
        Serial.print(" cm (");
        Serial.print(distance / 100.0f, 2);
        Serial.println(" m)");
    } else {
        Serial.println("Out of range (> 400cm)");
    }

    delay(100);  // Match measurement interval
}
