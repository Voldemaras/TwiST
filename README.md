# TwiST Framework v1.2.0

**Build Complex Robotics with Simple Code**

TwiST Framework makes ESP32 robotics easy. Control servos, joysticks, and sensors with just a few lines of code. No need to understand low-level hardware details - the framework handles everything for you.

**Author:** Voldemaras Birskys
**Contact:** voldemaras@gmail.com
**Version:** 1.2.0.0
**Release Date:** 2026-01-27

---

## Why TwiST?

**For Beginners:**
- Write your robot control in 10 lines of code
- No complex setup, no manual initialization
- Named devices: `servo("Gripper")` instead of `servo[0]`
- Built-in safety checks catch configuration errors

**For Advanced Users:**
- Config-driven architecture scales from 2 to 60+ devices
- Automatic memory management (no memory leaks)
- Structured logging for debugging
- Event system for complex automation

---

## Quick Start: Your First Robot

### Step 1: Hardware Setup

Connect to your ESP32:
- PCA9685 PWM board via I2C (servos)
- Analog joystick to GPIO pins
- HC-SR04 distance sensor (optional)

### Step 2: Write Your Code

```cpp
#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/ApplicationConfig.h"

TwiSTFramework framework;

void setup() {
    Serial.begin(115200);
    framework.initialize();
    App::initializeSystem(framework);
}

void loop() {
    // Control gripper with joystick
    float x = App::joystick("MainJoystick").getX();
    App::servo("GripperServo").setAngle(x * 180);

    framework.update();
    delay(20);
}
```

That's it. No driver initialization, no calibration code, no manual device creation.

### Step 3: Configure Your Devices

Edit `TwiST_Config.h` to match your hardware:

```cpp
static constexpr std::array<ServoConfig, 2> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"ArmServo", 0, 1, 101, CalibrationMode::STEPS, 120, 550, 0, 0, 0, 0}
}};
```

Framework creates all devices automatically from this config.

---

## What You Can Build

### Example 1: Joystick-Controlled Robot Arm

```cpp
void loop() {
    float x = App::joystick("MainJoystick").getX();
    float y = App::joystick("MainJoystick").getY();

    App::servo("BaseServo").setAngle(x * 180);
    App::servo("ArmServo").setAngle(y * 180);

    framework.update();
    delay(20);
}
```

### Example 2: Autonomous Smooth Motion

```cpp
void loop() {
    if (!App::servo("GripperServo").isMoving()) {
        // Smooth 3-second motion with easing
        App::servo("GripperServo").moveToWithEasing(160, 3000, Devices::Servo::EASE_OUT_CUBIC);
    }

    framework.update();
    delay(20);
}
```

### Example 3: Distance-Based Control

```cpp
void loop() {
    int distance = App::distanceSensor("ObstacleSensor").getDistanceCm();

    if (distance < 20) {
        App::servo("GripperServo").setAngle(0);   // Close gripper
    } else {
        App::servo("GripperServo").setAngle(90);  // Open gripper
    }

    framework.update();
    delay(20);
}
```

---

## Key Features

### Simple Device Access

**Named devices** - no arrays, no indices:
```cpp
App::servo("GripperServo").setAngle(90);
App::joystick("MainJoystick").getX();
App::distanceSensor("ObstacleSensor").getDistanceCm();
```

### Advanced Servo Control

**Smooth animations** - 6 easing modes:
```cpp
servo.moveToWithEasing(160, 3000, Devices::Servo::EASE_OUT_CUBIC);
```

**Speed-based movement**:
```cpp
servo.setSpeed(30.0);  // 30 degrees per second
servo.moveWithSpeed(180);
```

**Incremental steps**:
```cpp
servo.moveBySteps(10, 500);  // Move +10 degrees in 500ms
```

### Automatic Calibration

Configure once in `TwiST_Config.h`, framework applies automatically:
```cpp
{"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}

// These values are applied automatically
```

### Built-in Safety

Framework checks your configuration at startup:
- Duplicate device IDs or names
- I2C address conflicts
- GPIO pin collisions
- PWM channel overlaps

If configuration is wrong, system halts with clear error message.

---

## What's New in v1.2.0

**Centralized Logging System:**
```cpp
Logger::info("APP", "System started");
Logger::logf(Logger::Level::INFO, "APP", "Servo angle: %d", angle);
```
- Structured logs with timestamps and severity levels
- Module-based categorization
- Consistent output format across framework

**Memory Safety:**
- Automatic memory management with std::unique_ptr
- No memory leaks, no manual cleanup
- Production-grade reliability

**Single IO Channel:**
- All output goes through Logger
- No mixed Serial.print and framework messages
- Clean, professional console output

---

## Configuration Guide

### Adding a New Servo

Open `TwiST_Config.h`, add one line to `SERVO_CONFIGS`:

```cpp
static constexpr std::array<ServoConfig, 3> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"BaseServo", 0, 1, 101, CalibrationMode::STEPS, 120, 550, 0, 0, 0, 0},
    {"NewServo", 0, 2, 102, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}  // NEW
}};
```

Framework automatically:
- Creates the servo device
- Applies calibration
- Registers to registry
- Makes it available as `App::servo("NewServo")`

No code changes in main.ino.

### Adding a New Joystick

Add to `JOYSTICK_CONFIGS`:

```cpp
static constexpr std::array<JoystickConfig, 2> JOYSTICK_CONFIGS = {{
    {"MainJoystick", 200, 0, 1, 3, 1677, 3290, 3, 1677, 3290, 50},
    {"SecondJoystick", 201, 2, 3, 0, 2048, 4095, 0, 2048, 4095, 100}  // NEW
}};
```

Use it immediately:
```cpp
float x = App::joystick("SecondJoystick").getX();
```

### Changing ESP32 Board

Different ESP32 boards use different I2C pins. Update in `TwiST_Config.h`:

```cpp
// For ESP32-C6 (XIAO Seed):
#define XIAO_SDA_PIN  22
#define XIAO_SCL_PIN  23

// For ESP32 DevKit:
#define XIAO_SDA_PIN  21
#define XIAO_SCL_PIN  22
```

---

## Supported Hardware

**Microcontrollers:**
- ESP32-C6 (XIAO Seed)
- ESP32-S3
- ESP32 DevKit
- All ESP32 variants with I2C and ADC

**Servo Control:**
- PCA9685 16-channel PWM driver (I2C address 0x40)
- Up to 62 PCA9685 boards (992 servo channels)
- Any standard servo (SG90, MG996R, etc.)

**Input Devices:**
- Analog joysticks (2-axis, ESP32 ADC)
- HC-SR04 ultrasonic distance sensors

---

## Installation

1. Download or clone this repository
2. Open `main/main.ino` in Arduino IDE
3. Install required library:
   - Adafruit_PWMServoDriver
4. Select your ESP32 board in Arduino IDE
5. Upload to ESP32

---

## Documentation

**Getting Started:**
- [README.md](README.md) - This file (quick start and examples)
- [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) - Servo calibration step-by-step
- [CONFIG_GUIDE.md](CONFIG_GUIDE.md) - Complete configuration reference

**Advanced:**
- [ARCHITECTURE.md](ARCHITECTURE.md) - System design and component structure
- [CHANGELOG.md](CHANGELOG.md) - Version history and release notes

---

## How It Works

### Configuration-Driven

TwiST separates **what devices you have** (configuration) from **how you use them** (code).

**Configuration** (TwiST_Config.h):
```cpp
// Hardware topology - what devices exist
static constexpr std::array<ServoConfig, 2> SERVO_CONFIGS = {{...}};
```

**Application Code** (main.ino):
```cpp
// Device usage - what devices do
App::servo("GripperServo").setAngle(90);
```

Change device count, add sensors, swap pins - all in config file. Application code stays unchanged.

### Two-Line Initialization

```cpp
framework.initialize();          // 1. Core framework + Logger
App::initializeSystem(framework); // 2. Create devices from config
```

Framework handles:
- Driver creation (PCA9685, ESP32ADC, etc.)
- Device initialization (servos, joysticks, sensors)
- Calibration (from config)
- Registration (makes devices available)

### Automatic Updates

```cpp
framework.update();  // Call this in loop()
```

Handles:
- Servo animations (easing, speed-based movement)
- Distance sensor measurements
- Event processing
- Device state updates

---

## Device Calibration

Servos need calibration because manufacturers vary. Framework supports two methods:

**Method 1: PWM Steps (Recommended)**
```cpp
{"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
//                                                    ^^^^^^^^
//                                                    min=110, max=540
```

**Method 2: Microseconds (Traditional)**
```cpp
{"BaseServo", 0, 1, 101, CalibrationMode::MICROSECONDS, 0, 0, 500, 2500, 0, 180}
//                                                              ^^^^^^^^^^^^^^^^^
//                                                              500us-2500us, 0-180 degrees
```

See [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) for finding your servo's values.

---

## Logging and Debugging

Framework uses structured logging:

```cpp
Logger::info("APP", "System started");
Logger::error("APP", "Servo not found");
Logger::logf(Logger::Level::INFO, "APP", "Angle: %d", angle);
```

Output format:
```
[12345] [INFO] [APP] System started
[12360] [ERROR] [APP] Servo not found
[12375] [INFO] [APP] Angle: 90
```

**Severity Levels:**
- DEBUG - Detailed diagnostic information
- INFO - General information messages
- WARNING - Potential issues
- ERROR - Errors (recoverable)
- FATAL - Critical errors (system halts)

---

## Project Structure

```
├── main/
│   ├── main.ino                    # Your application code
│   └── src/TwiST_Framework/       # Framework (copied from src/)
├── src/TwiST_Framework/            # Framework source
│   ├── TwiST_Config.h              # EDIT THIS - Device configuration
│   ├── TwiST.h/cpp                 # Framework core
│   ├── ApplicationConfig.h/cpp     # Device initialization
│   ├── TwiST_ConfigValidator.h/cpp # Safety checks
│   ├── Core/
│   │   ├── Logger.h/cpp            # Logging system
│   │   ├── EventBus.h/cpp          # Event system
│   │   ├── DeviceRegistry.h/cpp    # Device management
│   │   └── ConfigManager.h/cpp     # Config loading (future)
│   ├── Devices/
│   │   ├── Servo.h/cpp             # Servo control
│   │   ├── Joystick.h/cpp          # Joystick input
│   │   └── DistanceSensor.h/cpp    # Distance sensing
│   └── Drivers/
│       ├── PWM/PCA9685.h/cpp       # PWM driver
│       ├── ADC/ESP32ADC.h/cpp      # ADC driver
│       └── Distance/HCSR04.h/cpp   # Ultrasonic driver
├── README.md                        # This file
├── CALIBRATION_GUIDE.md            # Calibration procedures
├── CONFIG_GUIDE.md                 # Configuration reference
└── CHANGELOG.md                    # Version history
```

---

## Common Questions

**Q: How do I add more servos?**
A: Edit `SERVO_CONFIGS` in `TwiST_Config.h`, increment array size, add entry. Framework creates it automatically.

**Q: How do I find calibration values for my servo?**
A: See [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) for step-by-step procedure.

**Q: Can I use different ESP32 board?**
A: Yes, just update I2C pins in `TwiST_Config.h`.

**Q: How many servos can I control?**
A: 16 per PCA9685 board, up to 62 boards = 992 servos total.

**Q: Do I need to call device initialization manually?**
A: No, `App::initializeSystem(framework)` handles everything.

**Q: What if I misspell a device name?**
A: System halts with error message listing available device names.

---

## Examples Included

**Simple Examples:**
- Basic servo control
- Joystick input reading
- Distance sensor reading

**Advanced Examples:**
- Joystick-controlled robot arm
- Autonomous servo sequences with easing
- Distance-based gripper control
- Multi-servo coordination
- Incremental movement patterns

See `main/main.ino` for working demo.

---

## Safety Features

**Pre-Flight Checks:**
Framework validates configuration before hardware initialization:
- Duplicate device IDs
- Duplicate device names
- I2C address conflicts
- GPIO pin collisions
- PWM channel overlaps

**Fail-Fast Behavior:**
If configuration is invalid, system halts immediately with error message. Prevents hardware damage from misconfiguration.

**Example Error Output:**
```
[1234] [FATAL] [VALIDATOR] I2C address conflict!
[1234] [FATAL] [VALIDATOR] Servo 'GripperServo' and 'BaseServo' both use address 0x40
[1234] [FATAL] [APP] System halted - fix TwiST_Config.h and recompile
```

---

## Version History

- **v1.2.0** (2026-01-27) - Logging system, memory safety, single IO channel
- **v1.1.0** (2026-01-26) - Single entry point API, config-driven architecture
- **v1.0.0** (2026-01-20) - Initial release with event-driven framework

See [CHANGELOG.md](CHANGELOG.md) for detailed release notes.

---

## License

Copyright (c) 2026 Voldemaras Birskys

This project is provided as-is for educational and personal use.

---

## Contact

**Author:** Voldemaras Birskys
**Email:** voldemaras@gmail.com
**Project:** TwiST Framework (Twin System Technology)

---

**TwiST Framework v1.2.0 - Complex Robotics Made Simple**