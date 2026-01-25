/* ============================================================================
 * TwiST Framework | Main Sketch
 * ============================================================================
 * @file      main.ino
 * @brief     Entry point for TwiST framework demo:
 *            - ESP32 + PCA9685 servos + analog joystick
 *            - Hardware drivers injected into logical devices
 *            - Fully hardware-independent device layer
 *
 * ARCHITECTURE ROLE:
 * - Layer 1: Framework (EventBus, DeviceRegistry, ConfigManager)
 * - Layer 2: Hardware Drivers (concrete: PCA9685, ESP32ADC)
 * - Layer 3: Abstraction Layer (IPWMDriver, IADCDriver)
 * - Layer 4: Logical Devices (Servo, Joystick)
 *
 * PRINCIPLES:
 * - Constructor Injection only
 * - Devices never know concrete hardware
 * - Calibration applied at initialization
 * - JSON config & runtime registry supported
 *
 * DEVICES INITIALIZED:
 * - Servos: servo_0 (ID 100, PWM CH0), servo_1 (ID 101, PWM CH2)
 * - Joystick: X/Y axes (ID 200)
 *
 * AUTHOR:    Voldemaras Birskys
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================ */

/* TwiST.h already includes:
#include "Devices/Servo.h"        // Automatic
#include "Devices/Joystick.h"     // Automatic

// But does NOT include:
// #include "Drivers/<DRIVER>/<driver>.h" 
*/

#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/Drivers/PWM/PCA9685.h"   //Sketch MUST includes drivers
#include "src/TwiST_Framework/Drivers/ADC/ESP32ADC.h"  

using namespace TwiST;
using namespace TwiST::Devices;
using namespace TwiST::Drivers;

/*============================================================================
LAYER 1: FRAMEWORK (must initialize first - provides EventBus)
============================================================================*/

TwiSTFramework framework;

// ============================================================================
// LAYER 2: HARDWARE DRIVERS (concrete implementations)
// ============================================================================
PCA9685 pca9685(PCA9685_ADDRESS);  // From TwiST_Config.h
ESP32ADC adcX(0);                  // Specific ADC for X-axis
ESP32ADC adcY(1);                  // Specific ADC for Y-axis

// ============================================================================
// LAYER 3: ABSTRACTION LAYER (dependency injection)
// ============================================================================
IPWMDriver& pwm = pca9685;  // Devices will use this abstraction
IADCDriver& xAxis = adcX;   // Not concrete ESP32ADC
IADCDriver& yAxis = adcY;   // Not concrete ESP32ADC

// ============================================================================
// LAYER 4: LOGICAL DEVICES (hardware-independent, FULLY INITIALIZED)
// ============================================================================

// Driver: control via abstraction. Channel: physical connection (locked). Device ID: framework identifier. EventBus: inter-device communication.
Servo servo_0(pwm, 0, 100, framework.eventBus());  // ALL dependencies in constructor
Servo servo_1(pwm, 2, 101, framework.eventBus());

Joystick joystick(xAxis, yAxis, 200, framework.eventBus());  // No setters needed

void setup() {
    Serial.begin(115200);

    // Initialize framework
    framework.initialize();

    // Initialize hardware drivers
    pca9685.begin(XIAO_SDA_PIN, XIAO_SCL_PIN);  // From TwiST_Config.h
    pca9685.setFrequency(PWM_FREQUENCY);        // From TwiST_Config.h
    adcX.begin();
    adcY.begin();

    // Initialize devices
    servo_0.initialize();
    servo_1.initialize();
    joystick.initialize();

    // Configure logical devices
    // Servo calibration comparison: microseconds vs steps
    servo_1.calibrate(500, 2500, 0, 180);      // Legacy: theoretical microseconds
    servo_0.calibrateBySteps(110, 540);        // Modern: measured PWM steps 

    // Joystick calibration from RAW measurements:78555
    // Left extreme: X=3, Center: X=1677, Right extreme: X=3290
    joystick.calibrate(3, 1677, 3290, 3, 1677, 3290);
    joystick.setDeadzone(50);

    // Register to framework
    framework.registry()->registerDevice(&servo_0);
    framework.registry()->registerDevice(&servo_1);

    framework.registry()->registerDevice(&joystick);
}

void loop() {
    // ========================================================================
    // SERVO_0: Manual control with joystick
    // ========================================================================
    float x = joystick.getX();
    float servo_0_angle = x * 180.0;
    servo_0.setAngle(servo_0_angle);

    // ========================================================================
    // servo_1: Autonomous motion demo - Robotic arm control examples
    // ========================================================================
    static unsigned long lastDemo = 0;
    static uint8_t demoStep = 0;

    if (millis() - lastDemo > 100) {  // Check every 100ms
        lastDemo = millis();

        // Execute demo sequence
        if (!servo_1.isMoving()) {
            switch (demoStep) {
                case 0:
                    Serial.println("[DEMO] Step 1: Fast start, slow end (EASE_OUT_CUBIC)");
                    servo_1.moveToWithEasing(160, 3000, Servo::EASE_OUT_CUBIC);
                    demoStep++;
                    break;

                case 1:
                    Serial.println("[DEMO] Step 2: Slow start, fast end (EASE_IN_CUBIC)");
                    servo_1.moveToWithEasing(20, 3000, Servo::EASE_IN_CUBIC);
                    demoStep++;
                    break;

                case 2:
                    Serial.println("[DEMO] Step 3: Smooth S-curve (EASE_IN_OUT_QUAD)");
                    servo_1.moveToWithEasing(90, 2000, Servo::EASE_IN_OUT_QUAD);
                    demoStep++;
                    break;

                case 3:
                    Serial.println("[DEMO] Step 4: Speed-based movement (30°/sec)");
                    servo_1.setSpeed(30.0);  // 30 degrees per second
                    servo_1.moveWithSpeed(170);
                    demoStep++;
                    break;

                case 4:
                    Serial.println("[DEMO] Step 5: Incremental steps (+10° every step)");
                    servo_1.moveBySteps(10, 500);  // +10° in 500ms
                    if (servo_1.getCurrentAngle() >= 180) {
                        demoStep++;
                    }
                    break;

                case 5:
                    Serial.println("[DEMO] Step 6: Incremental steps (-15° every step)");
                    servo_1.moveBySteps(-15, 400);  // -15° in 400ms
                    if (servo_1.getCurrentAngle() <= 10) {
                        demoStep++;
                    }
                    break;

                case 6:
                    Serial.println("[DEMO] Sequence complete! Restarting...\n");
                    delay(2000);
                    demoStep = 0;
                    break;
            }
        }
    }

    // Update framework (handles animations)
    framework.update();

    delay(20);
}