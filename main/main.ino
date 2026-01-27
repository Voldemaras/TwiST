/* ============================================================================
 * TwiST Framework | Main Sketch
 * ============================================================================
 * @file      main.ino
 * @brief     Entry point for TwiST framework demo:
 *            - ESP32 + PCA9685 servos + analog joystick
 *            - FULL framework abstraction - ZERO device knowledge!
 *
 * CLEAN ARCHITECTURE:
 * - Layer 1: Framework (TwiSTFramework core)
 * - Layer 2: TwiST_Config.h (device topology - pure data)
 * - Layer 3: ApplicationConfig (device + driver initialization)
 * - Layer 4: main.ino (pure application logic)
 *
 * KEY BENEFITS:
 * - main.ino NEVER creates devices
 * - main.ino NEVER knows device count
 * - main.ino NEVER knows calibration values
 * - Change device count → modify ONLY TwiST_Config.h
 * - main.ino stays 100% UNCHANGED!
 *
 * DEVICES CONFIGURED IN ApplicationConfig.cpp:
 * - Servos: 2 servos (IDs 100, 101)
 * - Joysticks: 1 joystick (ID 200)
 * - Distance Sensors: 1 HC-SR04 sensor (ID 300)
 *
 * AUTHOR:    Voldemaras Birskys
 * PROJECT:   TwiST Framework
 * VERSION:   1.2.0 (Logger Integration + Memory Safety)
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

    // Framework initializes Logger automatically
    framework.initialize();          // 1. Framework core (includes Logger::begin)
    App::initializeSystem(framework); // 2. All devices (init + calibrate + register)

    Logger::info("MAIN", "Setup complete!");
    Logger::logf(Logger::Level::INFO, "MAIN", "Servos: %d", App::getServoCount());
    Logger::logf(Logger::Level::INFO, "MAIN", "Joysticks: %d", App::getJoystickCount());
    Logger::logf(Logger::Level::INFO, "MAIN", "Distance Sensors: %d", App::getDistanceSensorCount());
}

void loop() {

    // ========================================================================
    // GripperServo: Manual control with joystick (PRODUCTION STYLE)
    // ========================================================================

    float x = App::joystick("MainJoystick").getX();
    float gripperAngle = x * 180.0;
    App::servo("GripperServo").setAngle(gripperAngle);

    // ========================================================================
    // BaseServo: Autonomous motion demo - Robotic arm control examples (PRODUCTION STYLE)
    // ========================================================================
    static unsigned long lastDemo = 0;
    static uint8_t demoStep = 0;

    if (millis() - lastDemo > 100) {  // Check every 100ms
        lastDemo = millis();

        // Execute demo sequence with name-based access
        switch (demoStep) {
            case 0:
                if (!App::servo("BaseServo").isMoving()) {
                    Logger::info("DEMO", "Step 1: Fast start, slow end (EASE_OUT_CUBIC)");
                    App::servo("BaseServo").moveToWithEasing(160, 3000, Devices::Servo::EASE_OUT_CUBIC);
                    demoStep++;
                }
                break;

            case 1:
                if (!App::servo("BaseServo").isMoving()) {
                    Logger::info("DEMO", "Step 2: Slow start, fast end (EASE_IN_CUBIC)");
                    App::servo("BaseServo").moveToWithEasing(20, 3000, Devices::Servo::EASE_IN_CUBIC);
                    demoStep++;
                }
                break;

            case 2:
                if (!App::servo("BaseServo").isMoving()) {
                    Logger::info("DEMO", "Step 3: Smooth S-curve (EASE_IN_OUT_QUAD)");
                    App::servo("BaseServo").moveToWithEasing(90, 2000, Devices::Servo::EASE_IN_OUT_QUAD);
                    demoStep++;
                }
                break;

            case 3:
                if (!App::servo("BaseServo").isMoving()) {
                    Logger::info("DEMO", "Step 4: Speed-based movement (30°/sec)");
                    App::servo("BaseServo").setSpeed(30.0);  // 30 degrees per second
                    App::servo("BaseServo").moveWithSpeed(170);
                    demoStep++;
                }
                break;

            case 4:
                if (!App::servo("BaseServo").isMoving()) {
                    Logger::info("DEMO", "Step 5: Incremental steps (+10° every step)");
                    App::servo("BaseServo").moveBySteps(10, 500);  // +10° in 500ms
                }
                if (App::servo("BaseServo").getCurrentAngle() >= 180) {
                    demoStep++;
                }
                break;

            case 5:
                if (!App::servo("BaseServo").isMoving()) {
                    Logger::info("DEMO", "Step 6: Incremental steps (-15° every step)");
                    App::servo("BaseServo").moveBySteps(-15, 400);  // -15° in 400ms
                }
                if (App::servo("BaseServo").getCurrentAngle() <= 10) {
                    demoStep++;
                }
                break;

            case 6:
                Logger::info("DEMO", "Sequence complete! Restarting...");
                delay(2000);
                demoStep = 0;
                break;
        }
    }

    // Update framework (handles animations)
    framework.update();

    delay(20);
}
