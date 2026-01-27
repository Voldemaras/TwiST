/*
 * TwiST_ConfigValidator.cpp
 * System configuration safety validation implementation
 *
 * CRITICAL: This is the last line of defense before hardware initialization.
 * If validation fails, system MUST halt - no exceptions.
 *
 * Author: Voldemaras Birskys
 * Version: 1.2.0 (Memory Safety: Eliminated std::vector, using std::array + Logger integration)
 */

#include "TwiST_ConfigValidator.h"
#include "TwiST_Config.h"
#include "Core/Logger.h"              // For centralized logging (v1.2.0)
#include <Arduino.h>
#include <array>
#include <cstring>

namespace TwiST {

// ============================================================================
// Maximum Array Sizes for Validation (compile-time constants)
// ============================================================================
// These must be >= maximum possible device counts in any configuration
// Using reasonable limits for embedded systems (deterministic memory)

constexpr uint8_t MAX_PWM_DRIVERS = 4;        // Typical: 1-2, maximum reasonable: 4
constexpr uint8_t MAX_TOTAL_DEVICES = 32;     // Matches MAX_DEVICES from TwiST_Config.h
constexpr uint8_t MAX_GPIO_PINS = 64;         // Typical ESP32: ~40 pins, buffer for safety

bool runSystemConfigSafetyCheck() {
    bool valid = true;

    // ========================================================================
    // Check 1: PWM Driver I2C Address Uniqueness
    // ========================================================================
    std::array<uint8_t, MAX_PWM_DRIVERS> pwmAddresses = {};
    uint8_t pwmAddressCount = 0;

    for (uint8_t i = 0; i < PWM_DRIVER_COUNT; i++) {
        uint8_t addr = PWM_DRIVER_CONFIGS[i].i2cAddress;

        // Check for duplicates in collected addresses
        for (uint8_t j = 0; j < pwmAddressCount; j++) {
            if (pwmAddresses[j] == addr) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "PWM driver I2C address collision: 0x%02X (driver indices %d and %d)",
                            addr, j, i);
                valid = false;
            }
        }

        // Add to collected addresses
        if (pwmAddressCount < MAX_PWM_DRIVERS) {
            pwmAddresses[pwmAddressCount++] = addr;
        }
    }

    // ========================================================================
    // Check 2: Device ID Uniqueness (across ALL device types)
    // ========================================================================
    std::array<uint16_t, MAX_TOTAL_DEVICES> deviceIds = {};
    uint8_t deviceIdCount = 0;

    // Collect servo IDs
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        uint16_t id = SERVO_CONFIGS[i].deviceId;

        for (uint8_t j = 0; j < deviceIdCount; j++) {
            if (deviceIds[j] == id) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "Device ID collision: %d (servo '%s' conflicts with earlier device)",
                            id, SERVO_CONFIGS[i].name);
                valid = false;
            }
        }

        if (deviceIdCount < MAX_TOTAL_DEVICES) {
            deviceIds[deviceIdCount++] = id;
        }
    }

    // Collect joystick IDs
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        uint16_t id = JOYSTICK_CONFIGS[i].deviceId;

        for (uint8_t j = 0; j < deviceIdCount; j++) {
            if (deviceIds[j] == id) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "Device ID collision: %d (joystick '%s' conflicts with earlier device)",
                            id, JOYSTICK_CONFIGS[i].name);
                valid = false;
            }
        }

        if (deviceIdCount < MAX_TOTAL_DEVICES) {
            deviceIds[deviceIdCount++] = id;
        }
    }

    // Collect distance sensor IDs
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        uint16_t id = DISTANCE_SENSOR_CONFIGS[i].deviceId;

        for (uint8_t j = 0; j < deviceIdCount; j++) {
            if (deviceIds[j] == id) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "Device ID collision: %d (distance sensor '%s' conflicts with earlier device)",
                            id, DISTANCE_SENSOR_CONFIGS[i].name);
                valid = false;
            }
        }

        if (deviceIdCount < MAX_TOTAL_DEVICES) {
            deviceIds[deviceIdCount++] = id;
        }
    }

    // ========================================================================
    // Check 3: Device Name Uniqueness (across ALL device types)
    // ========================================================================
    std::array<const char*, MAX_TOTAL_DEVICES> deviceNames = {};
    uint8_t deviceNameCount = 0;

    // Collect servo names
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        const char* name = SERVO_CONFIGS[i].name;

        for (uint8_t j = 0; j < deviceNameCount; j++) {
            if (strcmp(deviceNames[j], name) == 0) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "Device name collision: '%s' (servo conflicts with earlier device)", name);
                valid = false;
            }
        }

        if (deviceNameCount < MAX_TOTAL_DEVICES) {
            deviceNames[deviceNameCount++] = name;
        }
    }

    // Collect joystick names
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        const char* name = JOYSTICK_CONFIGS[i].name;

        for (uint8_t j = 0; j < deviceNameCount; j++) {
            if (strcmp(deviceNames[j], name) == 0) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "Device name collision: '%s' (joystick conflicts with earlier device)", name);
                valid = false;
            }
        }

        if (deviceNameCount < MAX_TOTAL_DEVICES) {
            deviceNames[deviceNameCount++] = name;
        }
    }

    // Collect distance sensor names
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        const char* name = DISTANCE_SENSOR_CONFIGS[i].name;

        for (uint8_t j = 0; j < deviceNameCount; j++) {
            if (strcmp(deviceNames[j], name) == 0) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "Device name collision: '%s' (distance sensor conflicts with earlier device)", name);
                valid = false;
            }
        }

        if (deviceNameCount < MAX_TOTAL_DEVICES) {
            deviceNames[deviceNameCount++] = name;
        }
    }

    // ========================================================================
    // Check 4: PWM Frequency Consistency (50Hz for servos)
    // ========================================================================
    for (uint8_t i = 0; i < PWM_DRIVER_COUNT; i++) {
        if (PWM_DRIVER_CONFIGS[i].frequency != 50) {
            Logger::logf(Logger::Level::ERROR, "CONFIG", "PWM driver %d has frequency %dHz (servos require 50Hz)",
                        i, PWM_DRIVER_CONFIGS[i].frequency);
            valid = false;
        }
    }

    // ========================================================================
    // Check 5: GPIO Pin Collision Detection
    // ========================================================================
    std::array<uint8_t, MAX_GPIO_PINS> usedPins = {};
    uint8_t usedPinCount = 0;

    // Collect joystick pins
    for (uint8_t i = 0; i < JOYSTICK_COUNT; i++) {
        uint8_t xPin = JOYSTICK_CONFIGS[i].xPin;
        uint8_t yPin = JOYSTICK_CONFIGS[i].yPin;

        // Check xPin collision
        for (uint8_t j = 0; j < usedPinCount; j++) {
            if (usedPins[j] == xPin) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "GPIO pin collision: %d (joystick '%s' X-axis conflicts with earlier pin)",
                            xPin, JOYSTICK_CONFIGS[i].name);
                valid = false;
            }
        }
        if (usedPinCount < MAX_GPIO_PINS) {
            usedPins[usedPinCount++] = xPin;
        }

        // Check yPin collision
        for (uint8_t j = 0; j < usedPinCount; j++) {
            if (usedPins[j] == yPin) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "GPIO pin collision: %d (joystick '%s' Y-axis conflicts with earlier pin)",
                            yPin, JOYSTICK_CONFIGS[i].name);
                valid = false;
            }
        }
        if (usedPinCount < MAX_GPIO_PINS) {
            usedPins[usedPinCount++] = yPin;
        }
    }

    // Collect distance sensor pins
    for (uint8_t i = 0; i < DISTANCE_SENSOR_COUNT; i++) {
        uint8_t trigPin = DISTANCE_SENSOR_CONFIGS[i].trigPin;
        uint8_t echoPin = DISTANCE_SENSOR_CONFIGS[i].echoPin;

        // Check trigPin collision
        for (uint8_t j = 0; j < usedPinCount; j++) {
            if (usedPins[j] == trigPin) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "GPIO pin collision: %d (distance sensor '%s' TRIG conflicts with earlier pin)",
                            trigPin, DISTANCE_SENSOR_CONFIGS[i].name);
                valid = false;
            }
        }
        if (usedPinCount < MAX_GPIO_PINS) {
            usedPins[usedPinCount++] = trigPin;
        }

        // Check echoPin collision
        for (uint8_t j = 0; j < usedPinCount; j++) {
            if (usedPins[j] == echoPin) {
                Logger::logf(Logger::Level::ERROR, "CONFIG", "GPIO pin collision: %d (distance sensor '%s' ECHO conflicts with earlier pin)",
                            echoPin, DISTANCE_SENSOR_CONFIGS[i].name);
                valid = false;
            }
        }
        if (usedPinCount < MAX_GPIO_PINS) {
            usedPins[usedPinCount++] = echoPin;
        }
    }

    // ========================================================================
    // Check 6: Servo pwmDriverIndex Range Validation
    // ========================================================================
    for (uint8_t i = 0; i < SERVO_COUNT; i++) {
        uint8_t driverIdx = SERVO_CONFIGS[i].pwmDriverIndex;
        if (driverIdx >= PWM_DRIVER_COUNT) {
            Logger::logf(Logger::Level::ERROR, "CONFIG", "Servo '%s' references pwmDriverIndex %d but only %d drivers configured",
                        SERVO_CONFIGS[i].name, driverIdx, PWM_DRIVER_COUNT);
            valid = false;
        }
    }

    // ========================================================================
    // Final Result
    // ========================================================================
    if (valid) {
        Logger::info("CONFIG", "Validation passed - all checks OK");
    } else {
        Logger::error("CONFIG", "Validation FAILED - system cannot proceed");
    }

    return valid;
}

}  // namespace TwiST
