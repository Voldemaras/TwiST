/*
 * TwiST_Config.h
 * Central configuration for TwiST Framework
 *
 * Twin System Technology - Configuration Settings
 *
 * Author: Voldemaras Birskys
 * Version: 1.2.0
 *
 * ============================================================================
 * ARCHITECTURE: Configuration Philosophy
 * ============================================================================
 *
 * TwiST uses TWO configuration systems with DISTINCT roles:
 *
 * 1. TwiST_Config.h (THIS FILE) - COMPILE-TIME Hardware Topology
 *    - Device counts, types, GPIO pins
 *    - PWM driver I2C addresses
 *    - Calibration defaults (servo steps, joystick ADC ranges)
 *    - Immutable after compilation
 *    - Role: "What hardware exists and how is it wired?"
 *
 * 2. ConfigManager (Core/ConfigManager.cpp) - RUNTIME User Preferences
 *    - Calibration offsets, deadzone adjustments
 *    - Speed limits, easing curves
 *    - Behavior tuning (NOT topology changes)
 *    - Role: "How should the hardware behave?"
 *    - Status: Infrastructure - not active in v1.2.x (future feature)
 *
 * TwiST_Config.h = Hardware (geležis)
 * ConfigManager = Behavior (elgsena)
 *
 * This separation ensures:
 * - Compile-time safety for hardware config
 * - Runtime flexibility for user preferences
 * - No confusion between "what devices exist" vs "how devices behave"
 *
 * ============================================================================
 */

#ifndef TWIST_CONFIG_H
#define TWIST_CONFIG_H

// ============================================================================
// Framework Version
// ============================================================================
#define TWIST_VERSION_MAJOR   1
#define TWIST_VERSION_MINOR   2
#define TWIST_VERSION_PATCH   0
#define TWIST_VERSION_STRING  "1.2.0"

// ============================================================================
// Core Framework Limits
// ============================================================================

/**
 * @brief Maximum number of devices in registry
 *
 * Used by: DeviceRegistry.h/.cpp
 * Memory: 4 bytes per device (pointer)
 * Typical: 8-16 for simple projects, 32 for complex
 */
#ifndef MAX_DEVICES
#define MAX_DEVICES  32
#endif

/**
 * @brief Maximum number of bridges
 *
 * Used by: TwiST.h/.cpp (array allocated, but no Bridge implementation yet)
 * Status: Infrastructure ready, awaiting concrete Bridge class
 * Memory: 4 bytes per bridge
 */
#ifndef MAX_BRIDGES
#define MAX_BRIDGES  16
#endif

/**
 * @brief Maximum number of event listeners
 *
 * Used by: EventBus.h/.cpp
 * Memory: ~16 bytes per listener
 * Typical: 16-32 listeners
 */
#define MAX_EVENT_LISTENERS  32

// ============================================================================
// Hardware Pin Definitions (ESP32 - Example for XIAO Seed C6)
// ============================================================================

/**
 * @brief I2C pins for PCA9685 PWM drivers
 *
 * **ESP32-C6 (XIAO Seed)**: GPIO22 (SDA), GPIO23 (SCL)
 * **ESP32-S3**: Check your board pinout
 * **ESP32 DevKit**: GPIO21 (SDA), GPIO22 (SCL)
 *
 * Used by: ApplicationConfig.cpp (pwmDrivers[i]->begin(...))
 * Change these if switching ESP32 variant
 */
#define XIAO_SDA_PIN  22   // GPIO22 (D4) - I2C Data (XIAO Seed C6)
#define XIAO_SCL_PIN  23   // GPIO23 (D5) - I2C Clock (XIAO Seed C6)

// ============================================================================
// REMOVED: Legacy Hardware Defines (now configured in device config structs)
// ============================================================================

/*
 * The following defines were REMOVED in v1.0.1 (Phase 1 refactor):
 *
 * PCA9685_ADDRESS, PWM_FREQUENCY → Now in PWMDriverConfig struct
 * JOYSTICK_X_PIN, JOYSTICK_Y_PIN → Now in JoystickConfig.xPin/yPin
 * DISTANCE_SENSOR_TRIG_PIN, DISTANCE_SENSOR_ECHO_PIN → Now in DistanceSensorConfig.trigPin/echoPin
 *
 * **To configure devices**: Edit device config structs below
 */

// ============================================================================
// Device Configuration Structures
// ============================================================================

#include <cstdint>  // For uint8_t, uint16_t types
#include <array>    // C++11 std::array for type-safe, zero-size compatible arrays

namespace TwiST {

// PWM driver types (hardware abstraction)
enum class PWMDriverType : uint8_t {
    PCA9685,        // I2C PWM driver (16 channels, Adafruit)
    ESP32_LEDC      // ESP32 native LED PWM (16 channels, future)
    // Add more driver types here as needed
};

// Servo calibration modes
enum class CalibrationMode : uint8_t {
    STEPS,          // PWM ticks (modern, direct)
    MICROSECONDS    // Microsecond pulse width (legacy)
};

// PWM driver configuration
struct PWMDriverConfig {
    PWMDriverType type;         // Driver type (PCA9685, ESP32_LEDC, etc.)
    uint8_t i2cAddress;         // I2C address (0x40-0x7F) - only for I2C drivers
    uint16_t frequency;         // PWM frequency in Hz (50 for servos)
};

// Servo configuration
struct ServoConfig {
    const char* name;           // Human-readable name
    uint8_t pwmDriverIndex;     // Index into PWM_DRIVER_CONFIGS array
    uint8_t pwmChannel;         // PWM channel (0-15 on PCA9685)
    uint16_t deviceId;          // Device ID (100, 101, ...)
    CalibrationMode calMode;    // Calibration method

    // STEPS mode parameters
    uint16_t minSteps;          // Minimum PWM ticks
    uint16_t maxSteps;          // Maximum PWM ticks

    // MICROSECONDS mode parameters
    uint16_t minUs;             // Minimum pulse width (microseconds)
    uint16_t maxUs;             // Maximum pulse width (microseconds)
    uint16_t angleMin;          // Minimum angle (degrees)
    uint16_t angleMax;          // Maximum angle (degrees)
};

// Joystick configuration
struct JoystickConfig {
    const char* name;           // Human-readable name
    uint16_t deviceId;          // Device ID (200, 201, ...)
    uint8_t xPin;               // ESP32 ADC pin for X-axis
    uint8_t yPin;               // ESP32 ADC pin for Y-axis
    uint16_t xMin;              // X-axis minimum raw value
    uint16_t xCenter;           // X-axis center raw value
    uint16_t xMax;              // X-axis maximum raw value
    uint16_t yMin;              // Y-axis minimum raw value
    uint16_t yCenter;           // Y-axis center raw value
    uint16_t yMax;              // Y-axis maximum raw value
    uint16_t deadzone;          // Deadzone radius (raw ADC units)
};

// Distance sensor configuration
struct DistanceSensorConfig {
    const char* name;           // Human-readable name
    uint16_t deviceId;          // Device ID (300, 301, ...)
    uint8_t trigPin;            // GPIO pin for TRIG signal
    uint8_t echoPin;            // GPIO pin for ECHO signal (use voltage divider!)
    float filterStrength;       // Low-pass filter alpha (0.0-1.0)
    unsigned long measurementIntervalMs;  // Measurement interval
};

// ============================================================================
// Device Configuration Arrays - std::array for zero-size safety
// ============================================================================

// ============================================================================
// PWM driver configurations
// ============================================================================

static constexpr std::array<PWMDriverConfig, 1> PWM_DRIVER_CONFIGS = {{
    // type, i2cAddress, frequency
    {PWMDriverType::PCA9685, 0x40, 50}  // PCA9685 at 0x40, 50Hz for servos
}};

// ============================================================================
// Servo configurations
// ============================================================================

// To add a new servo: Increment size, add entry to initializer
// To remove all servos: Change size to 0, empty initializer
static constexpr std::array<ServoConfig, 2> SERVO_CONFIGS = {{
    // name, pwmDrvIdx, pwmCh, devID, calMode, minSteps, maxSteps, minUs, maxUs, angleMin, angleMax
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS,        110,  540, 0,    0,    0,   0},
    {"BaseServo",    0, 1, 101, CalibrationMode::MICROSECONDS,   0,    0, 500, 2500,  0, 180}
}};


// ============================================================================
// Joystick configurations
// ============================================================================

static constexpr std::array<JoystickConfig, 1> JOYSTICK_CONFIGS = {{
    // name, devID, xPin, yPin, xMin, xCenter, xMax, yMin, yCenter, yMax, deadzone
    {"MainJoystick", 200, 0, 1, 3, 1677, 3290, 3, 1677, 3290, 50}
}};

// ============================================================================
// Distance sensor configurations
// ============================================================================

static constexpr std::array<DistanceSensorConfig, 1> DISTANCE_SENSOR_CONFIGS = {{
    // name, devID, trigPin, echoPin, filterStrength, measurementIntervalMs
    {"ObstacleSensor", 300, 16, 17, 0.3f, 100}
}};

// ============================================================================
// Device Counts - Computed from std::array::size()
// ============================================================================

// CRITICAL: Counts computed from array.size(), NOT hardcoded
// Zero-device safe: std::array<T, 0> is valid C++, .size() returns 0
static constexpr uint8_t PWM_DRIVER_COUNT = PWM_DRIVER_CONFIGS.size();
static constexpr uint8_t SERVO_COUNT = SERVO_CONFIGS.size();
static constexpr uint8_t JOYSTICK_COUNT = JOYSTICK_CONFIGS.size();
static constexpr uint8_t DISTANCE_SENSOR_COUNT = DISTANCE_SENSOR_CONFIGS.size();

}  // namespace TwiST

// ============================================================================
// Notes on Future Features
// ============================================================================

/*
 * Configuration validation moved to TwiST_ConfigValidator.h/.cpp
 * See runSystemConfigSafetyCheck() for pre-flight checks.
 */

// ============================================================================
// Notes on Future Features
// ============================================================================

/*
 * REMOVED UNUSED DEFINES (were placeholders for future features):
 *
 * - EVENT_QUEUE_SIZE, EVENT_PRIORITY_LEVELS - No priority queue yet
 * - CONFIG_JSON_SIZE, CONFIG_FILE_PATH - No JSON persistence yet
 * - BRIDGE_* constants - No concrete Bridge implementation yet
 * - DEVICE_RESPONSE_TIMEOUT, NETWORK_TIMEOUT - No timeouts implemented
 * - ENABLE_PROFILING, ENABLE_DEBUG_LOGS - No profiling/logging system
 * - I2C_CLOCK_SPEED - Not configurable in current driver
 * - ADC_*, DEFAULT_* - Hardcoded in device classes, not from config
 * - ENABLE_* flags - No conditional compilation yet
 * - LOG_LEVEL_* - No logging system
 *
 * These will be added when features are implemented.
 * Config file reflects ACTUAL framework state, not wishlist.
 */

#endif // TWIST_CONFIG_H