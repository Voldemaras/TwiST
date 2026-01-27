/* ============================================================================
 * TwiST Framework v1.2.0 | Basic Servo Example
 * ============================================================================
 *
 * MINIMAL EXAMPLE: One servo, one movement, clean architecture showcase.
 *
 * CRITICAL: Calibrate YOUR servo first!
 *   1. Open test/test.ino
 *   2. Run validation suite
 *   3. Update TwiST_Config.h with YOUR values
 *
 * This example demonstrates v1.2.0 features:
 *   - App::initializeSystem() single entry point
 *   - Name-based device access
 *   - Logger for structured output
 *   - Config-driven device management
 *
 * Author: Voldemaras Birskys
 * License: MIT
 * ============================================================================ */

#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/ApplicationConfig.h"  // Device abstraction

using namespace TwiST;

// ============================================================================
// FRAMEWORK - ONLY global object needed!
// ============================================================================

TwiSTFramework framework;

void setup() {
    Serial.begin(115200);
    delay(1000);

// ============================================================================
// SETUP SEQUENCE - v1.2.0 API
// ============================================================================

    framework.initialize();          // 1. Framework core (includes Logger::begin)
    App::initializeSystem(framework); // 2. All devices (init + calibrate + register)

    Logger::info("MAIN", "Setup complete!");
    Logger::logf(Logger::Level::INFO, "MAIN", "Servos: %d", App::getServoCount());
    Logger::logf(Logger::Level::INFO, "MAIN", "Joysticks: %d", App::getJoystickCount());
}

// ============================================================================
// Arduino Loop
// ============================================================================

void loop() {


    // ========================================================================
    // TemuServo: Manual control with joystick (PRODUCTION STYLE)
    // ========================================================================

    float x = App::joystick("MainJoystick").getX();
    float gripperAngle = x * 180.0;
    App::servo("TemuServo").setAngle(gripperAngle);


    
    auto& baseServo = App::servo("BaseServo");

    // Smooth sweep: 0° → 180° → 0° with easing
    static enum { MOVING_TO_180, MOVING_TO_0 } state = MOVING_TO_180;

    if (!baseServo.isMoving()) {
        switch (state) {
            case MOVING_TO_180:
                Logger::info("DEMO", "Moving to 180° (3 seconds, S-curve)");
                baseServo.moveToWithEasing(180, 3000, Devices::Servo::EASE_IN_OUT_QUAD);
                state = MOVING_TO_0;
                break;

            case MOVING_TO_0:
                Logger::info("DEMO", "Moving to 0° (3 seconds, S-curve)");
                baseServo.moveToWithEasing(0, 3000, Devices::Servo::EASE_IN_OUT_QUAD);
                state = MOVING_TO_180;
                break;
        }
    }

    // CRITICAL: framework.update() processes animations
    framework.update();
    delay(20);
}