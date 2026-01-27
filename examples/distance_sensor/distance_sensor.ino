/* ============================================================================
 * TwiST Framework v1.2.0 | Distance Sensor Example
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
 * NOTE: This is a BASIC example with direct device instantiation.
 *       For PRODUCTION code patterns, see main.ino which demonstrates:
 *         - ApplicationConfig.cpp for centralized device management
 *         - Name-based access: App::distanceSensor("ObstacleSensor").getDistanceCm()
 *         - Logger for structured output (v1.2.0)
 *
 * Author: Voldemaras Birskys
 * License: MIT
 * ============================================================================ */

#include "../../src/TwiST_Framework/TwiST.h"
#include "../../src/TwiST_Framework/Drivers/Distance/HCSR04.h"

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
// NOTE: Device name "ProximitySensor" added for production-style name-based access
DistanceSensor sensor(distanceDriver, 300, "ProximitySensor", framework.eventBus(), 100);  // 100ms interval

// ============================================================================
// Arduino Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize framework
    framework.initialize();

    // Initialize hardware
    hcsr04.begin();

    // Initialize device
    sensor.initialize();

    // Register device
    framework.registry()->registerDevice(&sensor);

    Logger::info("DISTANCE", "HC-SR04 Ultrasonic Sensor initialized");
    Logger::info("DISTANCE", "Range: 2cm - 400cm");
    Logger::info("DISTANCE", "Resolution: 0.3cm");
    Logger::warning("DISTANCE", "Ensure ECHO pin uses voltage divider (5V -> 3.3V protection)");
    Logger::info("DISTANCE", "Starting distance measurement...");
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
        Logger::logf(Logger::Level::INFO, "DISTANCE", "Distance: %.1f cm (%.2f m)",
                    distance, distance / 100.0f);
    } else {
        Logger::info("DISTANCE", "Out of range (> 400cm)");
    }

    delay(100);  // Match measurement interval
}
