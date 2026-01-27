/*
 * ApplicationConfig.cpp
 * Application device configuration implementation
 *
 * ARCHITECTURE:
 * - This file implements device initialization from TwiST_Config.h
 * - Device topology (COUNT, names, IDs, channels) configured in TwiST_Config.h
 * - For-loop driven initialization - NO hardcoded device instantiation
 * - main.ino NEVER knows device count or types
 *
 * TO ADD/REMOVE DEVICES:
 * 1. Edit TwiST_Config.h - modify SERVO_CONFIGS, JOYSTICK_CONFIGS, etc.
 * 2. COUNT auto-computed from array size
 * 3. This file and main.ino stay UNCHANGED!
 *
 * Author: Voldemaras Birskys
 * Version: 1.2.0 (Memory Safety: std::unique_ptr for automatic cleanup)
 */

#include "ApplicationConfig.h"
#include "TwiST_Config.h"              // For device configuration structs
#include "TwiST_ConfigValidator.h"     // For runSystemConfigSafetyCheck()
#include "TwiST.h"                     // For TwiSTFramework class definition
#include "Core/Logger.h"               // For centralized logging (v1.2.0)
#include "Drivers/PWM/PCA9685.h"       // Concrete PWM driver
#include "Drivers/ADC/ESP32ADC.h"      // Concrete ADC driver
#include "Drivers/Distance/HCSR04.h"   // Concrete distance sensor driver
#include <Arduino.h>                   // For Serial debugging
#include <memory>                      // For std::unique_ptr, std::make_unique

namespace TwiST {
namespace App {

// Import device counts from TwiST_Config.h
using TwiST::PWM_DRIVER_COUNT;
using TwiST::SERVO_COUNT;
using TwiST::JOYSTICK_COUNT;
using TwiST::DISTANCE_SENSOR_COUNT;

// Import config structs from TwiST_Config.h
using TwiST::PWM_DRIVER_CONFIGS;
using TwiST::SERVO_CONFIGS;
using TwiST::JOYSTICK_CONFIGS;
using TwiST::DISTANCE_SENSOR_CONFIGS;
using TwiST::PWMDriverType;
using TwiST::CalibrationMode;

// Import validation function
using TwiST::runSystemConfigSafetyCheck;

// ============================================================================
// Private device and driver storage (hidden from outside)
// ============================================================================

namespace {
    // Driver storage - std::unique_ptr for automatic memory management (v1.2.0)
    // RAII (Resource Acquisition Is Initialization) - automatic cleanup
    std::array<std::unique_ptr<Drivers::PCA9685>, PWM_DRIVER_COUNT> pwmDrivers;
    std::array<std::unique_ptr<Drivers::ESP32ADC>, JOYSTICK_COUNT * 2> adcDrivers;  // 2 per joystick (X and Y)
    std::array<std::unique_ptr<Drivers::HCSR04>, DISTANCE_SENSOR_COUNT> ultrasonicDrivers;

    // Device storage - std::unique_ptr for clear ownership semantics
    // Memory: Allocated at startup, automatically freed on shutdown (RAII)
    std::array<std::unique_ptr<Devices::Servo>, SERVO_COUNT> servos;
    std::array<std::unique_ptr<Devices::Joystick>, JOYSTICK_COUNT> joysticks;
    std::array<std::unique_ptr<Devices::DistanceSensor>, DISTANCE_SENSOR_COUNT> distanceSensors;
}

// ============================================================================
// Public API Implementation
// ============================================================================

void initializeDevices(EventBus& eventBus) {
    Logger::info("APP", "Initializing devices...");

    // ========================================================================
    // CRITICAL: Pre-flight safety check
    // ========================================================================
    Logger::info("APP", "Running system config safety check...");
    if (!runSystemConfigSafetyCheck()) {
        Logger::fatal("APP", "Safety check failed - fix TwiST_Config.h and recompile");
        // Logger::fatal() halts MCU internally
    }

    // ========================================================================
    // Create PWM drivers dynamically from config (FULLY CONFIG-DRIVEN)
    // ========================================================================
    Logger::info("APP", "Creating PWM drivers...");
    for (uint8_t i = 0; i < PWM_DRIVER_COUNT; i++) {
        const auto& cfg = PWM_DRIVER_CONFIGS[i];

        // Factory pattern: Create driver based on type from config (v1.2.0: std::make_unique)
        switch (cfg.type) {
            case PWMDriverType::PCA9685:
                pwmDrivers[i] = std::make_unique<Drivers::PCA9685>(cfg.i2cAddress);
                pwmDrivers[i]->begin(XIAO_SDA_PIN, XIAO_SCL_PIN);
                pwmDrivers[i]->setFrequency(cfg.frequency);
                Logger::logf(Logger::Level::INFO, "PWM", "PCA9685 driver %d at 0x%02X, %dHz",
                            i, cfg.i2cAddress, cfg.frequency);
                break;

            case PWMDriverType::ESP32_LEDC:
                // Future: ESP32 native LEDC driver
                Logger::fatal("PWM", "ESP32_LEDC not implemented - use PCA9685 or implement ESP32_LEDC driver");
                break;

            default:
                Logger::logf(Logger::Level::FATAL, "PWM", "Unknown driver type: %d - fix TwiST_Config.h",
                            static_cast<uint8_t>(cfg.type));
        }
    }

    // ========================================================================
    // Create ADC drivers dynamically (2 per joystick) - v1.2.0: std::make_unique
    // ========================================================================
    Logger::info("APP", "Creating ADC drivers...");
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        const auto& cfg = JOYSTICK_CONFIGS[i];
        adcDrivers[i * 2] = std::make_unique<Drivers::ESP32ADC>(cfg.xPin);
        adcDrivers[i * 2 + 1] = std::make_unique<Drivers::ESP32ADC>(cfg.yPin);
        Logger::logf(Logger::Level::INFO, "ADC", "Joystick '%s': X=GPIO%d, Y=GPIO%d",
                    cfg.name, cfg.xPin, cfg.yPin);
    }

    // ========================================================================
    // Create ultrasonic drivers dynamically - v1.2.0: std::make_unique
    // ========================================================================
    Logger::info("APP", "Creating ultrasonic drivers...");
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        const auto& cfg = DISTANCE_SENSOR_CONFIGS[i];
        ultrasonicDrivers[i] = std::make_unique<Drivers::HCSR04>(cfg.trigPin, cfg.echoPin);
        Logger::logf(Logger::Level::INFO, "ULTRASONIC", "'%s': TRIG=GPIO%d, ECHO=GPIO%d",
                    cfg.name, cfg.trigPin, cfg.echoPin);
    }

    // ========================================================================
    // Initialize servos from config - v1.2.0: std::make_unique
    // ========================================================================
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        const auto& cfg = SERVO_CONFIGS[i];
        servos[i] = std::make_unique<Devices::Servo>(
            *pwmDrivers[cfg.pwmDriverIndex],  // Use dynamic driver
            cfg.pwmChannel,
            cfg.deviceId,
            cfg.name,
            eventBus
        );
        Logger::logf(Logger::Level::INFO, "SERVO", "Initializing %s (ID %d, PWM driver %d, channel %d)",
                    cfg.name, cfg.deviceId, cfg.pwmDriverIndex, cfg.pwmChannel);
        servos[i]->initialize();
    }

    // ========================================================================
    // Initialize joysticks from config - v1.2.0: std::make_unique
    // ========================================================================
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        const auto& cfg = JOYSTICK_CONFIGS[i];
        joysticks[i] = std::make_unique<Devices::Joystick>(
            *adcDrivers[i * 2],      // X-axis driver
            *adcDrivers[i * 2 + 1],  // Y-axis driver
            cfg.deviceId,
            cfg.name,
            eventBus
        );
        Logger::logf(Logger::Level::INFO, "JOYSTICK", "Initializing %s (ID %d)",
                    cfg.name, cfg.deviceId);
        joysticks[i]->initialize();
    }

    // ========================================================================
    // Initialize distance sensors from config - v1.2.0: std::make_unique
    // ========================================================================
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        const auto& cfg = DISTANCE_SENSOR_CONFIGS[i];
        distanceSensors[i] = std::make_unique<Devices::DistanceSensor>(
            *ultrasonicDrivers[i],  // Use dynamic driver
            cfg.deviceId,
            cfg.name,
            eventBus,
            cfg.measurementIntervalMs
        );
        Logger::logf(Logger::Level::INFO, "DISTANCE", "Initializing %s (ID %d)",
                    cfg.name, cfg.deviceId);
        distanceSensors[i]->initialize();
    }

    Logger::info("APP", "All devices created");
}

void calibrateDevices() {
    Logger::info("APP", "Calibrating devices...");

    // Calibrate servos based on mode
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        const auto& cfg = SERVO_CONFIGS[i];

        if (cfg.calMode == CalibrationMode::STEPS) {
            servos[i]->calibrateBySteps(cfg.minSteps, cfg.maxSteps);
            Logger::logf(Logger::Level::INFO, "APP", "%s: calibrateBySteps(%d, %d)",
                        cfg.name, cfg.minSteps, cfg.maxSteps);
        } else {
            servos[i]->calibrate(cfg.minUs, cfg.maxUs, cfg.angleMin, cfg.angleMax);
            Logger::logf(Logger::Level::INFO, "APP", "%s: calibrate(%d, %d, %d, %d)",
                        cfg.name, cfg.minUs, cfg.maxUs, cfg.angleMin, cfg.angleMax);
        }
    }

    // Calibrate joysticks
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        const auto& cfg = JOYSTICK_CONFIGS[i];
        joysticks[i]->calibrate(
            cfg.xMin, cfg.xCenter, cfg.xMax,
            cfg.yMin, cfg.yCenter, cfg.yMax
        );
        joysticks[i]->setDeadzone(cfg.deadzone);
        Logger::logf(Logger::Level::INFO, "APP", "%s: calibrated", cfg.name);
    }

    // Calibrate distance sensors
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        const auto& cfg = DISTANCE_SENSOR_CONFIGS[i];
        distanceSensors[i]->setFilterStrength(cfg.filterStrength);
        Logger::logf(Logger::Level::INFO, "APP", "%s: setFilterStrength(%.2f)",
                    cfg.name, cfg.filterStrength);
    }

    Logger::info("APP", "All devices calibrated");
}

void registerAllDevices(DeviceRegistry* registry) {
    Logger::info("APP", "Registering devices to framework...");

    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        registry->registerDevice(servos[i].get());
        Logger::logf(Logger::Level::INFO, "APP", "Registered: %s", servos[i]->getName());
    }

    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        registry->registerDevice(joysticks[i].get());
        Logger::logf(Logger::Level::INFO, "APP", "Registered: %s", joysticks[i]->getName());
    }

    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        registry->registerDevice(distanceSensors[i].get());
        Logger::logf(Logger::Level::INFO, "APP", "Registered: %s", distanceSensors[i]->getName());
    }

    Logger::logf(Logger::Level::INFO, "APP", "Total devices registered: %d",
                SERVO_COUNT + JOYSTICK_COUNT + DISTANCE_SENSOR_COUNT);
}

Devices::Servo& getServo(uint8_t index) {
    if (index >= SERVO_COUNT) {
        Logger::logf(Logger::Level::FATAL, "APP", "Invalid servo index %d (valid: 0-%d) - fix application code",
                    index, SERVO_COUNT - 1);
        // Logger::fatal() halts MCU internally
    }
    return *servos[index];
}

Devices::Servo& getServoByName(const char* name) {
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        if (strcmp(servos[i]->getName(), name) == 0) {
            return *servos[i];
        }
    }

    // Not found - CRITICAL ERROR, halt system
    Logger::logf(Logger::Level::ERROR, "APP", "Servo not found: '%s'", name);
    Logger::error("APP", "Available servos:");
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        Logger::logf(Logger::Level::ERROR, "APP", "  - '%s'", servos[i]->getName());
    }
    Logger::fatal("APP", "System halted - fix application code (check device name)");
    // Logger::fatal() halts MCU internally
}

Devices::Servo& servo(const char* name) {
    return getServoByName(name);
}

Devices::Joystick& getJoystick(uint8_t index) {
    if (index >= JOYSTICK_COUNT) {
        Logger::logf(Logger::Level::FATAL, "APP", "Invalid joystick index %d (valid: 0-%d) - fix application code",
                    index, JOYSTICK_COUNT - 1);
        // Logger::fatal() halts MCU internally
    }
    return *joysticks[index];
}

Devices::Joystick& getJoystickByName(const char* name) {
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        if (strcmp(joysticks[i]->getName(), name) == 0) {
            return *joysticks[i];
        }
    }

    // Not found - CRITICAL ERROR, halt system
    Logger::logf(Logger::Level::ERROR, "APP", "Joystick not found: '%s'", name);
    Logger::error("APP", "Available joysticks:");
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        Logger::logf(Logger::Level::ERROR, "APP", "  - '%s'", joysticks[i]->getName());
    }
    Logger::fatal("APP", "System halted - fix application code (check device name)");
    // Logger::fatal() halts MCU internally
}

Devices::Joystick& joystick(const char* name) {
    return getJoystickByName(name);
}

Devices::DistanceSensor& getDistanceSensor(uint8_t index) {
    if (index >= DISTANCE_SENSOR_COUNT) {
        Logger::logf(Logger::Level::FATAL, "APP", "Invalid distance sensor index %d (valid: 0-%d) - fix application code",
                    index, DISTANCE_SENSOR_COUNT - 1);
        // Logger::fatal() halts MCU internally
    }
    return *distanceSensors[index];
}

Devices::DistanceSensor& getDistanceSensorByName(const char* name) {
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        if (strcmp(distanceSensors[i]->getName(), name) == 0) {
            return *distanceSensors[i];
        }
    }

    // Not found - CRITICAL ERROR, halt system
    Logger::logf(Logger::Level::ERROR, "APP", "Distance sensor not found: '%s'", name);
    Logger::error("APP", "Available distance sensors:");
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        Logger::logf(Logger::Level::ERROR, "APP", "  - '%s'", distanceSensors[i]->getName());
    }
    Logger::fatal("APP", "System halted - fix application code (check device name)");
    // Logger::fatal() halts MCU internally
}

Devices::DistanceSensor& distanceSensor(const char* name) {
    return getDistanceSensorByName(name);
}

uint8_t getServoCount() {
    return SERVO_COUNT;
}

uint8_t getJoystickCount() {
    return JOYSTICK_COUNT;
}

uint8_t getDistanceSensorCount() {
    return DISTANCE_SENSOR_COUNT;
}

// ============================================================================
// Phase 2: Single Entry Point API (v1.1.0)
// ============================================================================

void initializeSystem(TwiSTFramework& framework) {
    // Step 1: Initialize drivers and devices (includes fail-fast validation)
    initializeDevices(framework.eventBus());

    // Step 2: Calibrate devices (config-driven: STEPS/MICROSECONDS modes)
    calibrateDevices();

    // Step 3: Register devices to framework (enables framework.loop() updates)
    registerAllDevices(framework.registry());
}

}  // namespace App
}  // namespace TwiST
