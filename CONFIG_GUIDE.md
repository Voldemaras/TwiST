# TwiST Configuration Guide

Complete reference for configuring TwiST Framework hardware and devices.

All device configuration is in: [src/TwiST_Framework/TwiST_Config.h](src/TwiST_Framework/TwiST_Config.h)

---

## Configuration Philosophy

TwiST uses two separate configuration systems:

**1. TwiST_Config.h (Compile-Time Hardware Topology)**
- What devices exist
- Hardware wiring (GPIO pins, I2C addresses)
- Device calibration defaults
- Immutable after compilation
- Role: "What hardware do I have?"

**2. ConfigManager (Runtime User Preferences)**
- How devices behave
- Speed limits, easing curves
- Calibration overrides
- Status: Infrastructure ready, not active in v1.2.0
- Role: "How should hardware behave?"

This guide focuses on TwiST_Config.h - the hardware topology configuration.

---

## File Location

```
src/TwiST_Framework/TwiST_Config.h
```

Edit this file to configure your hardware.

---

## Framework Configuration

### Version Information

```cpp
#define TWIST_VERSION_MAJOR   1
#define TWIST_VERSION_MINOR   2
#define TWIST_VERSION_PATCH   0
#define TWIST_VERSION_STRING  "1.2.0"
```

Framework version identification. Do not modify manually.

### Device Registry Limits

```cpp
#define MAX_DEVICES  32
```

Maximum number of devices that can be registered (servos, joysticks, sensors combined).

**Memory:** 4 bytes per device (pointer array)

**When to change:**
- Increase for large projects (more than 32 devices)
- Decrease to save RAM on memory-constrained boards

**Example:**
```cpp
#define MAX_DEVICES  10  // Small project, saves 88 bytes RAM
```

### Event Bus Limits

```cpp
#define MAX_EVENT_LISTENERS  32
```

Maximum number of event subscribers.

**Memory:** ~16 bytes per listener

**When to change:**
- Decrease for simple projects with few event handlers
- Increase for event-heavy applications

---

## Hardware Pin Configuration

### I2C Pins (ESP32 Variants)

```cpp
#define XIAO_SDA_PIN  22   // GPIO22 (D4) - I2C Data
#define XIAO_SCL_PIN  23   // GPIO23 (D5) - I2C Clock
```

I2C pins for PCA9685 PWM driver communication.

**ESP32-C6 (XIAO Seed):**
```cpp
#define XIAO_SDA_PIN  22
#define XIAO_SCL_PIN  23
```

**ESP32-S3:**
Check your board pinout, typically GPIO8 (SDA), GPIO9 (SCL)

**ESP32 DevKit:**
```cpp
#define XIAO_SDA_PIN  21
#define XIAO_SCL_PIN  22
```

---

## Device Configuration Structures

All devices are configured using C++ struct arrays. Framework creates devices automatically from these arrays.

### PWM Driver Configuration

```cpp
struct PWMDriverConfig {
    PWMDriverType type;     // Driver type (PCA9685, ESP32_LEDC)
    uint8_t i2cAddress;     // I2C address (0x40-0x7F)
    uint16_t frequency;     // PWM frequency in Hz (50 for servos)
};
```

**Example:**
```cpp
static constexpr std::array<PWMDriverConfig, 2> PWM_DRIVER_CONFIGS = {{
    {PWMDriverType::PCA9685, 0x40, 50},  // First PCA9685 board
    {PWMDriverType::PCA9685, 0x41, 50}   // Second PCA9685 board (different address)
}};
```

**Field Details:**

**type:**
- `PWMDriverType::PCA9685` - PCA9685 16-channel I2C PWM driver (implemented)
- `PWMDriverType::ESP32_LEDC` - ESP32 native LED PWM (future)

**i2cAddress:**
- Default: `0x40`
- Change by soldering jumpers on PCA9685 board
- Valid range: `0x40` to `0x7F`
- Each board on same I2C bus must have unique address

**frequency:**
- Servos: `50` Hz (standard)
- Never change for servos

### Servo Configuration

```cpp
struct ServoConfig {
    const char* name;           // Device name
    uint8_t pwmDriverIndex;     // Index into PWM_DRIVER_CONFIGS array
    uint8_t pwmChannel;         // PWM channel (0-15 on PCA9685)
    uint16_t deviceId;          // Device ID (100, 101, ...)
    CalibrationMode calMode;    // STEPS or MICROSECONDS
    uint16_t minSteps;          // Minimum PWM ticks (STEPS mode)
    uint16_t maxSteps;          // Maximum PWM ticks (STEPS mode)
    uint16_t minUs;             // Minimum pulse width (MICROSECONDS mode)
    uint16_t maxUs;             // Maximum pulse width (MICROSECONDS mode)
    uint16_t angleMin;          // Minimum angle (MICROSECONDS mode)
    uint16_t angleMax;          // Maximum angle (MICROSECONDS mode)
};
```

**Example:**
```cpp
static constexpr std::array<ServoConfig, 3> SERVO_CONFIGS = {{
    // STEPS mode (recommended):
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},

    // MICROSECONDS mode:
    {"BaseServo", 0, 1, 101, CalibrationMode::MICROSECONDS, 0, 0, 500, 2500, 0, 180},

    // Multiple PWM drivers:
    {"ArmServo", 1, 0, 102, CalibrationMode::STEPS, 102, 512, 0, 0, 0, 0}
    //           ^
    //           PWM driver index 1 (second PCA9685 board)
}};
```

**Field Details:**

**name:**
- Human-readable device name
- Used in code: `App::servo("GripperServo")`
- Must be unique across all devices

**pwmDriverIndex:**
- Index into `PWM_DRIVER_CONFIGS` array
- 0 = first PWM driver, 1 = second, etc.

**pwmChannel:**
- PWM channel on driver (0-15 on PCA9685)
- Each servo needs unique channel on its driver

**deviceId:**
- Unique numerical ID (100, 101, 102, ...)
- Convention: 100-199 for servos
- Must be unique across all devices

**calMode:**
- `CalibrationMode::STEPS` - Direct PWM steps (recommended)
- `CalibrationMode::MICROSECONDS` - Pulse width in microseconds

**STEPS mode (minSteps, maxSteps):**
- Direct PCA9685 step values (0-4095)
- Example: 110 to 540 steps
- See [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) for finding values

**MICROSECONDS mode (minUs, maxUs, angleMin, angleMax):**
- Pulse width in microseconds
- Example: 500us to 2500us, mapped to 0-180 degrees
- Traditional servo calibration method

### Joystick Configuration

```cpp
struct JoystickConfig {
    const char* name;       // Device name
    uint16_t deviceId;      // Device ID (200, 201, ...)
    uint8_t xPin;           // ESP32 ADC pin for X-axis
    uint8_t yPin;           // ESP32 ADC pin for Y-axis
    uint16_t xMin;          // X-axis minimum raw value
    uint16_t xCenter;       // X-axis center raw value
    uint16_t xMax;          // X-axis maximum raw value
    uint16_t yMin;          // Y-axis minimum raw value
    uint16_t yCenter;       // Y-axis center raw value
    uint16_t yMax;          // Y-axis maximum raw value
    uint16_t deadzone;      // Deadzone radius (raw ADC units)
};
```

**Example:**
```cpp
static constexpr std::array<JoystickConfig, 2> JOYSTICK_CONFIGS = {{
    // First joystick:
    {"MainJoystick", 200, 0, 1, 3, 1677, 3290, 3, 1677, 3290, 50},
    //               ^^^  ^  ^  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //               ID   X  Y  Calibration values

    // Second joystick:
    {"SecondJoystick", 201, 2, 3, 0, 2048, 4095, 0, 2048, 4095, 100}
}};
```

**Field Details:**

**name:**
- Used in code: `App::joystick("MainJoystick")`
- Must be unique

**deviceId:**
- Convention: 200-299 for joysticks
- Must be unique

**xPin, yPin:**
- ESP32 GPIO pin numbers
- Must be ADC-capable pins
- ESP32-C6: GPIO0-4 (ADC1)
- Important: Use ADC1 channels (GPIO0-4), ADC2 conflicts with WiFi

**xMin, xCenter, xMax:**
- Raw ADC values for X-axis calibration
- ESP32-C6: 12-bit ADC (0-4095)
- Find by reading raw values from joystick
- Center should be value when joystick at rest

**yMin, yCenter, yMax:**
- Raw ADC values for Y-axis calibration
- Same as X-axis

**deadzone:**
- Center deadzone radius in raw ADC units
- Prevents drift when joystick at rest
- Typical: 50-100 units
- Larger = less sensitive center, more stable

### Distance Sensor Configuration

```cpp
struct DistanceSensorConfig {
    const char* name;               // Device name
    uint16_t deviceId;              // Device ID (300, 301, ...)
    uint8_t trigPin;                // GPIO pin for TRIG signal
    uint8_t echoPin;                // GPIO pin for ECHO signal
    float filterStrength;           // Low-pass filter (0.0-1.0)
    unsigned long measurementIntervalMs;  // Measurement interval
};
```

**Example:**
```cpp
static constexpr std::array<DistanceSensorConfig, 2> DISTANCE_SENSOR_CONFIGS = {{
    // HC-SR04 sensor:
    {"ObstacleSensor", 300, 16, 17, 0.3f, 100},
    //                 ^^^  ^^  ^^  ^^^^  ^^^
    //                 ID   Trig Echo Flt  Interval

    // Second sensor:
    {"RangeSensor", 301, 18, 19, 0.5f, 200}
}};
```

**Field Details:**

**name:**
- Used in code: `App::distanceSensor("ObstacleSensor")`
- Must be unique

**deviceId:**
- Convention: 300-399 for distance sensors
- Must be unique

**trigPin:**
- ESP32 GPIO pin for TRIG signal (5V output to sensor)

**echoPin:**
- ESP32 GPIO pin for ECHO signal
- Important: HC-SR04 outputs 5V, ESP32 input is 3.3V max
- Use voltage divider on ECHO pin (2 resistors: 1K and 2K)

**filterStrength:**
- Low-pass filter coefficient (0.0 to 1.0)
- 0.0 = no filtering (raw values)
- 0.3 = light filtering (recommended)
- 0.5 = medium filtering
- 1.0 = maximum filtering (very smooth, slow response)

**measurementIntervalMs:**
- Milliseconds between measurements
- 100ms = 10 measurements per second (recommended)
- Lower = more frequent updates, higher CPU usage
- Higher = less frequent, lower CPU usage

---

## Device Counts

Device counts are computed automatically from array sizes:

```cpp
static constexpr uint8_t PWM_DRIVER_COUNT = PWM_DRIVER_CONFIGS.size();
static constexpr uint8_t SERVO_COUNT = SERVO_CONFIGS.size();
static constexpr uint8_t JOYSTICK_COUNT = JOYSTICK_CONFIGS.size();
static constexpr uint8_t DISTANCE_SENSOR_COUNT = DISTANCE_SENSOR_CONFIGS.size();
```

Do not modify these. Framework computes them automatically.

---

## Adding Devices

### Adding a Servo

1. Open `TwiST_Config.h`
2. Increment array size in `SERVO_CONFIGS` declaration
3. Add new entry to initializer

```cpp
// Before (2 servos):
static constexpr std::array<ServoConfig, 2> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"BaseServo", 0, 1, 101, CalibrationMode::STEPS, 120, 550, 0, 0, 0, 0}
}};

// After (3 servos):
static constexpr std::array<ServoConfig, 3> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"BaseServo", 0, 1, 101, CalibrationMode::STEPS, 120, 550, 0, 0, 0, 0},
    {"ArmServo", 0, 2, 102, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}  // NEW
}};
```

4. Recompile and upload
5. Use new servo: `App::servo("ArmServo").setAngle(90);`

### Adding a Joystick

```cpp
// Before (1 joystick):
static constexpr std::array<JoystickConfig, 1> JOYSTICK_CONFIGS = {{
    {"MainJoystick", 200, 0, 1, 3, 1677, 3290, 3, 1677, 3290, 50}
}};

// After (2 joysticks):
static constexpr std::array<JoystickConfig, 2> JOYSTICK_CONFIGS = {{
    {"MainJoystick", 200, 0, 1, 3, 1677, 3290, 3, 1677, 3290, 50},
    {"SecondJoystick", 201, 2, 3, 0, 2048, 4095, 0, 2048, 4095, 100}  // NEW
}};
```

### Adding a Distance Sensor

```cpp
// Before (1 sensor):
static constexpr std::array<DistanceSensorConfig, 1> DISTANCE_SENSOR_CONFIGS = {{
    {"ObstacleSensor", 300, 16, 17, 0.3f, 100}
}};

// After (2 sensors):
static constexpr std::array<DistanceSensorConfig, 2> DISTANCE_SENSOR_CONFIGS = {{
    {"ObstacleSensor", 300, 16, 17, 0.3f, 100},
    {"RangeSensor", 301, 18, 19, 0.5f, 200}  // NEW
}};
```

### Adding a PWM Driver Board

To control more than 16 servos, add second PCA9685 board:

```cpp
// Before (1 board = 16 servos):
static constexpr std::array<PWMDriverConfig, 1> PWM_DRIVER_CONFIGS = {{
    {PWMDriverType::PCA9685, 0x40, 50}
}};

// After (2 boards = 32 servos):
static constexpr std::array<PWMDriverConfig, 2> PWM_DRIVER_CONFIGS = {{
    {PWMDriverType::PCA9685, 0x40, 50},  // First board (address 0x40)
    {PWMDriverType::PCA9685, 0x41, 50}   // Second board (address 0x41)
}};
```

Then assign servos to different boards:

```cpp
static constexpr std::array<ServoConfig, 18> SERVO_CONFIGS = {{
    // First 16 servos on first board (driver index 0):
    {"Servo1", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    // ... servos 2-16 ...

    // Next 2 servos on second board (driver index 1):
    {"Servo17", 1, 0, 116, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"Servo18", 1, 1, 117, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
}};
```

---

## Removing Devices

### Removing All Servos

Set array size to 0, empty initializer:

```cpp
static constexpr std::array<ServoConfig, 0> SERVO_CONFIGS = {{}};
```

### Removing Specific Device

Remove entry from array, decrement size:

```cpp
// Before (3 servos):
static constexpr std::array<ServoConfig, 3> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"BaseServo", 0, 1, 101, CalibrationMode::STEPS, 120, 550, 0, 0, 0, 0},
    {"ArmServo", 0, 2, 102, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
}};

// After (2 servos, removed "BaseServo"):
static constexpr std::array<ServoConfig, 2> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"ArmServo", 0, 2, 102, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
}};
```

---

## Configuration Validation

Framework validates configuration at startup using `TwiST_ConfigValidator`.

**Checks performed:**
- Duplicate device IDs
- Duplicate device names
- I2C address conflicts
- GPIO pin collisions
- PWM channel overlaps

**If validation fails:**
- System halts with error message
- Lists conflicting devices
- Prevents hardware damage

**Example error output:**
```
[1234] [FATAL] [VALIDATOR] Device ID conflict!
[1234] [FATAL] [VALIDATOR]   Servo 'GripperServo' and joystick 'MainJoystick' both use ID 100
[1234] [FATAL] [APP] System halted - fix TwiST_Config.h and recompile
```

---

## ESP32 ADC Pin Reference

### ESP32-C6 (XIAO Seed)

**ADC1 channels (use these):**
- GPIO0 - ADC1_CH0
- GPIO1 - ADC1_CH1
- GPIO2 - ADC1_CH2
- GPIO3 - ADC1_CH3
- GPIO4 - ADC1_CH4

**ADC2 channels (avoid - conflicts with WiFi):**
- Not recommended for joystick use

### ESP32-S3

Check your board pinout. Typically GPIO1-10 for ADC1.

### ESP32 DevKit

**ADC1 channels:**
- GPIO32-39

---

## Common Configuration Scenarios

### Minimal Project (1 Servo, No Joystick)

```cpp
static constexpr std::array<PWMDriverConfig, 1> PWM_DRIVER_CONFIGS = {{
    {PWMDriverType::PCA9685, 0x40, 50}
}};

static constexpr std::array<ServoConfig, 1> SERVO_CONFIGS = {{
    {"Servo1", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
}};

static constexpr std::array<JoystickConfig, 0> JOYSTICK_CONFIGS = {{}};
static constexpr std::array<DistanceSensorConfig, 0> DISTANCE_SENSOR_CONFIGS = {{}};
```

### Robot Arm (4 Servos, 1 Joystick)

```cpp
static constexpr std::array<ServoConfig, 4> SERVO_CONFIGS = {{
    {"BaseRotation", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"ShoulderPitch", 0, 1, 101, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"ElbowPitch", 0, 2, 102, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"Gripper", 0, 3, 103, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
}};

static constexpr std::array<JoystickConfig, 1> JOYSTICK_CONFIGS = {{
    {"MainJoystick", 200, 0, 1, 3, 1677, 3290, 3, 1677, 3290, 50}
}};
```

### Large Project (32 Servos, 2 PCA9685 Boards)

```cpp
static constexpr std::array<PWMDriverConfig, 2> PWM_DRIVER_CONFIGS = {{
    {PWMDriverType::PCA9685, 0x40, 50},
    {PWMDriverType::PCA9685, 0x41, 50}
}};

static constexpr std::array<ServoConfig, 32> SERVO_CONFIGS = {{
    // Servos 0-15 on first board (driver 0):
    {"Servo1", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    // ... servos 2-16 ...

    // Servos 16-31 on second board (driver 1):
    {"Servo17", 1, 0, 116, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    // ... servos 18-32 ...
}};
```

---

## Best Practices

1. **Device IDs:** Use consistent ranges (100-199 servos, 200-299 joysticks, 300-399 sensors)
2. **Device Names:** Use descriptive names ("GripperServo" not "Servo1")
3. **Calibration:** Test each servo individually, document values
4. **GPIO Pins:** Use ADC1 channels for joysticks (ADC2 conflicts with WiFi)
5. **I2C Addresses:** Use unique address for each PCA9685 board
6. **Comments:** Add comments for complex configurations
7. **Validation:** Always test configuration with validation enabled

---

## Troubleshooting

**Problem:** System halts with "Device ID conflict"
**Solution:** Check all deviceId fields, ensure unique across all device types

**Problem:** System halts with "I2C address conflict"
**Solution:** Each PCA9685 board needs unique i2cAddress (0x40, 0x41, etc.)

**Problem:** Servo buzzes at limits
**Solution:** See [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) for calibration procedure

**Problem:** Joystick readings incorrect
**Solution:** Calibrate joystick by reading raw values and updating xMin/xCenter/xMax

**Problem:** Distance sensor returns 0
**Solution:** Check GPIO pin numbers, verify TRIG and ECHO connections

---

## Summary

1. All device configuration is in `TwiST_Config.h`
2. Devices defined using struct arrays
3. Framework creates devices automatically from config
4. Add devices by incrementing array size and adding entry
5. Device counts computed automatically
6. Configuration validated at startup
7. Change ESP32 variant by updating I2C pins

For calibration procedures, see [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md).

---

Written for TwiST Framework v1.2.0
Author: Voldemaras Birskys
Email: voldemaras@gmail.com
