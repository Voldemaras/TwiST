# TwiST Framework v1.2.0.0

Production-ready ESP32 robotics framework with centralized logging and memory safety.

---

## What's New

### 1. Centralized Logging System

Professional structured logging replaces all scattered `Serial.print` usage.

- **Severity levels:**
  - `DEBUG` - Detailed diagnostic information
  - `INFO` - General information messages
  - `WARNING` - Potential issues
  - `ERROR` - Recoverable errors
  - `FATAL` - Critical errors (automatically halts MCU – fail-fast)
- **Module categorization:**
  - `FRAMEWORK`, `REGISTRY`, `EVENTBUS`, `CONFIG`, `APP`, `SERVO`, `JOYSTICK`, `DISTANCE`, etc.
- **Structured output format:**
  ```
  [timestamp] [level] [module] message
  ```
- **Millisecond timestamps** on every log entry
- **Single unified output channel** for the entire framework
- **`Logger::logf()`** for printf-style formatted output
- **Runtime log level filtering**

Logger is now the single source of truth for all framework diagnostics.

**Example:**
```cpp
Logger::info("APP", "System started");
Logger::logf(Logger::Level::INFO, "SERVO", "Angle: %d degrees", angle);
Logger::fatal("CONFIG", "Invalid configuration - system halted");
```

---

### 2. Memory Safety

Deterministic memory management based on RAII.

- **All dynamically created devices and drivers use `std::unique_ptr`**
- **Clear ownership model** enforced by the compiler
- **Automatic cleanup** on scope exit
- **No manual `delete`**
- **No memory leaks**
- **Safe for long-running embedded systems**
- **Production-grade resource handling**

**Before v1.2.0:**
```cpp
Servo* servos[SERVO_COUNT];  // Raw pointers, manual cleanup needed
```

**After v1.2.0:**
```cpp
std::array<std::unique_ptr<Devices::Servo>, SERVO_COUNT> servos;  // Automatic cleanup
```

---

### 3. Clean Architecture

Strict separation of responsibilities.

- **Single system entry point:**
  ```cpp
  App::initializeSystem(framework);
  ```
  Replaces multiple fragmented initialization calls.

- **Configuration philosophy:**
  - `TwiST_Config.h` → Compile-time hardware topology (pins, devices, wiring, defaults)
  - `ConfigManager` → Runtime behavior tuning (infrastructure, inactive in v1.2.x)

- **Ownership model:**
  - `ApplicationConfig` owns all devices (`std::unique_ptr`)
  - `DeviceRegistry` and `TwiSTFramework` store non-owning references (raw pointers)

- **No mixed concerns:**
  - Logger handles diagnostics
  - Serial is used only for hardware communication

---

### 4. Single IO Channel

100% of framework output goes through Logger.

- **No `Serial.print` or `Serial.println`** inside framework logic
- **Consistent formatting**
- **Centralized verbosity control**
- **Improved traceability and debugging**

**Exception:**
- Startup banner remains on Serial for user visibility

---

### 5. Developer Experience

All examples updated to use Logger.

- **API stabilized** for v1.2.x
- **Documentation aligned with real code**
- **Copy-paste ready examples**
- **No internal framework paths** in documentation
- **Professional, deterministic output format**

---

## Quick Start

```cpp
#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/ApplicationConfig.h"

TwiSTFramework framework;

void setup() {
    Serial.begin(115200);
    framework.initialize();
    App::initializeSystem(framework);

    Logger::info("APP", "System ready!");
}

void loop() {
    float x = App::joystick("MainJoystick").getX();
    App::servo("GripperServo").setAngle(x * 180);

    framework.update();
    delay(20);
}
```

---

## Files Changed

**New:**
- `Core/Logger.h` / `Core/Logger.cpp` - Centralized logging system

**Updated:**
- `ApplicationConfig.cpp` - std::unique_ptr device ownership
- `DeviceRegistry.cpp` - Logger integration
- `EventBus.cpp` - Logger integration
- `ConfigManager.cpp` - Logger integration
- `TwiST.cpp` - Logger integration
- `TwiST_ConfigValidator.cpp` - Logger integration
- `examples/basic_servo/basic_servo.ino` - v1.2.0 API
- `examples/distance_sensor/distance_sensor.ino` - v1.2.0 API
- `examples/distance_servo_control/distance_servo_control.ino` - v1.2.0 API
- `examples/advanced/distance_servo_filtered.ino` - v1.2.0 API
- `README.md`, `CALIBRATION_GUIDE.md`, `CONFIG_GUIDE.md`, `CHANGELOG.md` - Complete rewrite

---

## Upgrade from v1.1.0

**Backward compatible** - your existing code works without changes.

**Optional: Modernize to v1.2.0 style:**

```cpp
// Old:
Serial.println("System started");
Serial.print("Angle: ");
Serial.println(angle);

// New:
Logger::info("APP", "System started");
Logger::logf(Logger::Level::INFO, "APP", "Angle: %d", angle);
```

---

## Documentation

- **[README.md](README.md)** - Quick start with working examples
- **[CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md)** - Step-by-step servo calibration
- **[CONFIG_GUIDE.md](CONFIG_GUIDE.md)** - Complete configuration reference
- **[CHANGELOG.md](CHANGELOG.md)** - Full version history
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System design and component structure

---

## Hardware Support

- **ESP32-C6** (XIAO Seed)
- **ESP32-S3**
- **ESP32 DevKit**
- All ESP32 variants with I2C and ADC

**Peripherals:**
- PCA9685 16-channel PWM driver (up to 62 boards = 992 servo channels)
- Analog joysticks (ESP32 ADC)
- HC-SR04 ultrasonic distance sensors
- Standard servos (SG90, MG996R, etc.)

---

**Author:** Voldemaras Birškys
**Email:** voldemaras@gmail.com
**Project:** TwiST Framework - Build Complex Robotics with Simple Code

**Release Date:** 2026-01-27
**Status:** STABLE
