/*
 * ApplicationConfig.h
 * Application-level device configuration
 *
 * PURPOSE: Manages device instances and their lifecycle
 *
 * ARCHITECTURE ROLE:
 * - Separates application (device instances) from hardware (drivers)
 * - main.ino NEVER creates devices directly
 * - Device count/configuration controlled in ONE place
 *
 * BENEFITS:
 * - Change device count without touching main.ino
 * - Automatic device registration
 * - Centralized calibration configuration
 * - Clean accessor-based API
 *
 * Author: Voldemaras Birskys
 * Version: 1.0.0
 */

#ifndef TWIST_APPLICATION_CONFIG_H
#define TWIST_APPLICATION_CONFIG_H

#include "Devices/Servo.h"
#include "Devices/Joystick.h"
#include "Devices/DistanceSensor.h"
#include "Core/EventBus.h"
#include "Core/DeviceRegistry.h"

// Forward declaration (global scope - TwiSTFramework is NOT in TwiST namespace)
class TwiSTFramework;

namespace TwiST {
namespace App {

/**
 * @brief Initialize all application devices (config-driven, fail-fast)
 * @param eventBus Reference to framework EventBus
 *
 * **Config-Driven Initialization** (v1.0.1+):
 * - Reads device configuration from TwiST_Config.h
 * - Creates drivers dynamically (PWM, ADC, ultrasonic)
 * - Initializes devices via for-loop
 *
 * **Fail-Fast Safety Check**:
 * - Runs runSystemConfigSafetyCheck() before hardware init
 * - Validates: I2C addresses, device IDs/names, GPIO pins
 * - **Halts MCU** on validation failure (prevents hardware corruption)
 *
 * **Usage**:
 * ```cpp
 * framework.initialize();
 * App::initializeDevices(framework.eventBus());
 * ```
 */
void initializeDevices(EventBus& eventBus);

/**
 * @brief Apply calibration to all devices (config-driven)
 *
 * **Config-Driven Calibration** (v1.0.1+):
 * - Reads calibration parameters from TwiST_Config.h
 * - Servo: STEPS or MICROSECONDS mode (CalibrationMode enum)
 * - Joystick: min/max/center/deadzone
 * - Distance sensor: filter strength
 *
 * **CRITICAL**: Must be called AFTER initializeDevices()
 *
 * **Usage**:
 * ```cpp
 * App::initializeDevices(framework.eventBus());
 * App::calibrateDevices();  // Apply config-based calibration
 * ```
 */
void calibrateDevices();

/**
 * @brief Register all devices to framework
 * @param registry Pointer to DeviceRegistry
 *
 * CRITICAL: Must be called AFTER initializeDevices()
 * Registers all created devices for framework.update()
 */
void registerAllDevices(DeviceRegistry* registry);

/**
 * @brief Get servo by index
 * @param index Servo index (0-based)
 * @return Reference to Servo instance
 *
 * Example: App::getServo(0).setAngle(90);
 * NOTE: Prefer getServoByName() for production code (name-based access is stable)
 */
Devices::Servo& getServo(uint8_t index);

/**
 * @brief Get servo by name (production-style access)
 * @param name Device name (e.g., "GripperServo")
 * @return Reference to Servo instance
 *
 * Example: App::getServoByName("GripperServo").setAngle(90);
 *
 * CRITICAL: Names are architectural contracts, indices are implementation details.
 * Use this for production code where device order may change.
 */
Devices::Servo& getServoByName(const char* name);

/**
 * @brief Clean alias for getServoByName() - production style
 * @param name Device name (e.g., "GripperServo")
 * @return Reference to Servo instance
 *
 * Example: App::servo("GripperServo").setAngle(90);
 */
Devices::Servo& servo(const char* name);

/**
 * @brief Get joystick by index
 * @param index Joystick index (0-based)
 * @return Reference to Joystick instance
 *
 * Example: float x = App::getJoystick(0).getX();
 * NOTE: Prefer getJoystickByName() for production code (name-based access is stable)
 */
Devices::Joystick& getJoystick(uint8_t index);

/**
 * @brief Get joystick by name (production-style access)
 * @param name Device name (e.g., "MainJoystick")
 * @return Reference to Joystick instance
 *
 * Example: App::getJoystickByName("MainJoystick").getX();
 */
Devices::Joystick& getJoystickByName(const char* name);

/**
 * @brief Clean alias for getJoystickByName() - production style
 * @param name Device name (e.g., "MainJoystick")
 * @return Reference to Joystick instance
 *
 * Example: float x = App::joystick("MainJoystick").getX();
 */
Devices::Joystick& joystick(const char* name);

/**
 * @brief Get distance sensor by index
 * @param index Distance sensor index (0-based)
 * @return Reference to DistanceSensor instance
 *
 * Example: int dist = App::getDistanceSensor(0).getDistanceCm();
 * NOTE: Prefer getDistanceSensorByName() for production code (name-based access is stable)
 */
Devices::DistanceSensor& getDistanceSensor(uint8_t index);

/**
 * @brief Get distance sensor by name (production-style access)
 * @param name Device name (e.g., "ObstacleSensor")
 * @return Reference to DistanceSensor instance
 *
 * Example: App::getDistanceSensorByName("ObstacleSensor").getDistanceCm();
 */
Devices::DistanceSensor& getDistanceSensorByName(const char* name);

/**
 * @brief Clean alias for getDistanceSensorByName() - production style
 * @param name Device name (e.g., "ObstacleSensor")
 * @return Reference to DistanceSensor instance
 *
 * Example: int dist = App::distanceSensor("ObstacleSensor").getDistanceCm();
 */
Devices::DistanceSensor& distanceSensor(const char* name);

/**
 * @brief Get number of servos configured
 * @return Servo count
 */
uint8_t getServoCount();

/**
 * @brief Get number of joysticks configured
 * @return Joystick count
 */
uint8_t getJoystickCount();

/**
 * @brief Get number of distance sensors configured
 * @return Distance sensor count
 */
uint8_t getDistanceSensorCount();

/**
 * @brief Single entry point: initialize, calibrate, and register all devices
 * @param framework Reference to TwiST framework instance
 *
 * **✅ RECOMMENDED** - Single Entry Point API (v1.1.0+)
 *
 * This function performs all device initialization in one call:
 * 1. `initializeDevices(framework.eventBus())` - Create drivers and devices
 * 2. `calibrateDevices()` - Apply config-based calibration
 * 3. `registerAllDevices(framework.registry())` - Register to framework
 *
 * **Benefits**:
 * - ✅ One-line device setup (reduces 3 calls to 1)
 * - ✅ Impossible to forget calibration or registration
 * - ✅ Future-proof (can add validation steps without breaking API)
 *
 * **Usage** (RECOMMENDED):
 * ```cpp
 * void setup() {
 *     Serial.begin(115200);
 *     framework.initialize();
 *     App::initializeSystem(framework);  // All 3 steps
 * }
 * ```
 *
 * **Backward Compatibility**: Individual functions remain available for advanced use
 */
void initializeSystem(TwiSTFramework& framework);

}  // namespace App
}  // namespace TwiST

#endif // TWIST_APPLICATION_CONFIG_H
