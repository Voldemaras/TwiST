# TwiST Framework - Changelog

All notable changes to the TwiST Framework project are documented here.

Version format: MAJOR.MINOR.PATCH.BUILD
- MAJOR: Breaking API changes
- MINOR: New features, backward compatible
- PATCH: Bug fixes, backward compatible
- BUILD: Internal improvements, no API changes

---

## [1.2.0.0] - 2026-01-27 - STABLE

### Added - Centralized Logging System

**Logger Infrastructure:**
- Added `Core/Logger.h` and `Core/Logger.cpp` - Centralized logging system
- Structured logging with severity levels (DEBUG, INFO, WARNING, ERROR, FATAL)
- Module-based categorization for component identification
- Timestamp-based log entries for debugging
- Consistent output format: `[timestamp] [level] [module] message`

**Logger API:**
```cpp
Logger::info("MODULE", "Message");
Logger::error("MODULE", "Error message");
Logger::logf(Logger::Level::INFO, "MODULE", "Value: %d", value);
Logger::fatal("MODULE", "Critical error");  // Halts system
```

**Logging Modules:**
- FRAMEWORK - Core framework operations
- REGISTRY - Device registration and management
- EVENTBUS - Event system operations
- CONFIG - Configuration management
- APP - Application-level operations
- PWM, SERVO, JOYSTICK, DISTANCE - Device-specific logging

**Framework-Wide Logger Integration:**
- All framework components migrated to Logger (DeviceRegistry, EventBus, ConfigManager, TwiSTFramework)
- ApplicationConfig uses Logger for device initialization and calibration
- TwiST_ConfigValidator uses Logger for safety check results
- main.ino updated with Logger usage examples
- Zero Serial.print calls in framework code (except TwiST.cpp startup banner)

### Changed - Memory Safety

**std::unique_ptr for Automatic Memory Management:**
- ApplicationConfig now uses std::unique_ptr for all device ownership
- Driver arrays (PWM, ADC, Ultrasonic) use std::unique_ptr
- Device arrays (Servos, Joysticks, DistanceSensors) use std::unique_ptr
- std::make_unique for RAII-compliant device creation
- Automatic cleanup on scope exit (no manual delete needed)
- Zero memory leaks, production-grade reliability

**Before v1.2.0:**
```cpp
Servo* servos[SERVO_COUNT];  // Raw pointers, manual cleanup needed
```

**After v1.2.0:**
```cpp
std::array<std::unique_ptr<Devices::Servo>, SERVO_COUNT> servos;  // RAII, automatic cleanup
```

### Changed - Single IO Channel

**Unified Output:**
- Logger is the single output mechanism for all framework operations
- No mixed Serial.print and Logger output
- Consistent message formatting across all components
- Clean, professional console output

**Exception:**
- TwiST.cpp startup banner remains as Serial output for visibility

### Changed - Configuration Documentation

**TwiST_Config.h Header:**
- Added comprehensive architecture documentation to TwiST_Config.h
- Documented two-system configuration philosophy:
  - TwiST_Config.h = Hardware topology (compile-time)
  - ConfigManager = User preferences (runtime)
- Clarified ConfigManager status: "Infrastructure - not active in v1.2.x"
- Documented role separation: Hardware vs Behavior

### Architecture Impact

**Before v1.2.0 (Multiple Output Channels):**
```cpp
Serial.println("[Registry] Registering device...");
Serial.print("Device: "); Serial.println(name);
```

**After v1.2.0 (Single Logger Channel):**
```cpp
Logger::info("REGISTRY", "Registering device...");
Logger::logf(Logger::Level::INFO, "REGISTRY", "Device: %s", name);
```

**Benefits:**
- Centralized control of output verbosity
- Structured log format for parsing
- Module filtering capability
- Production-ready logging infrastructure

### Files Modified

**New Files:**
- `src/TwiST_Framework/Core/Logger.h` - Logger interface
- `src/TwiST_Framework/Core/Logger.cpp` - Logger implementation

**Modified Files:**
- `src/TwiST_Framework/ApplicationConfig.cpp` - Logger + std::unique_ptr
- `src/TwiST_Framework/TwiST_ConfigValidator.cpp` - Logger integration
- `src/TwiST_Framework/Core/DeviceRegistry.cpp` - Logger integration
- `src/TwiST_Framework/Core/EventBus.cpp` - Logger integration
- `src/TwiST_Framework/Core/EventBus.h` - Updated documentation example
- `src/TwiST_Framework/Core/ConfigManager.cpp` - Logger integration
- `src/TwiST_Framework/TwiST.cpp` - Logger integration (kept Serial banner)
- `src/TwiST_Framework/TwiST_Config.h` - Architecture documentation
- `main/main.ino` - Logger usage examples

### Technical Details

**Logger Severity Levels:**
```cpp
enum class Level {
    DEBUG,    // Detailed diagnostic information
    INFO,     // General information messages
    WARNING,  // Potential issues
    ERROR,    // Errors (recoverable)
    FATAL     // Critical errors (system halts)
};
```

**Logger Usage Patterns:**
```cpp
// Simple message:
Logger::info("MODULE", "Operation complete");

// Formatted message:
Logger::logf(Logger::Level::INFO, "MODULE", "Device %d initialized", id);

// Fatal error (halts system):
Logger::fatal("MODULE", "Critical error - system halted");
```

**Output Format:**
```
[12345] [INFO] [REGISTRY] Registered device: GripperServo (ID: 100, Type: Servo)
[12360] [ERROR] [EVENTBUS] Listener limit reached
[12375] [FATAL] [APP] Device not found: InvalidServo
```

### Memory Safety Details

**Device Ownership:**
- ApplicationConfig owns all devices via std::unique_ptr
- DeviceRegistry stores non-owning raw pointers (registry pattern)
- TwiSTFramework stores non-owning raw pointers to bridges (registry pattern)
- Clear ownership semantics: ApplicationConfig creates, framework references

**RAII Benefits:**
- Automatic cleanup on scope exit
- Exception-safe resource management
- No manual delete calls needed
- Compile-time ownership enforcement

---

## [1.1.0.0] - 2026-01-26 - STABLE

### Added - Phase 2: Single Entry Point API

**Single Entry Point for Device Initialization:**
- Added `App::initializeSystem(framework)` convenience function
- Combines all 3 initialization steps into one call:
  - `initializeDevices()` - Create drivers and devices
  - `calibrateDevices()` - Apply config-based calibration
  - `registerAllDevices()` - Register to framework
- Reduces setup code from 4 lines to 2 lines in main.ino
- Backward compatible - individual functions remain available for advanced use

**Benefits:**
- One-line device setup (impossible to forget calibration or registration)
- Future-proof API (can add validation steps without breaking user code)
- Cleaner main.ino - minimal, portable across projects

**Example Usage:**
```cpp
void setup() {
    framework.initialize();
    App::initializeSystem(framework);  // All device init in one call
}
```

### Changed - Phase 1: Configuration Architecture

**Config-Driven Device Management:**
- Device topology moved to `TwiST_Config.h` (single source of truth)
- Struct-based configuration: `ServoConfig`, `JoystickConfig`, `DistanceSensorConfig`, `PWMDriverConfig`
- Device counts computed from `std::array::size()` - no manual tracking
- For-loop driven initialization - no hardcoded device instantiation
- Dynamic driver creation from config (supports multiple PCA9685 boards)

**Hardware Abstraction Cleanup:**
- Removed `HardwareConfig.h` and `HardwareConfig.cpp` (legacy)
- `ApplicationConfig.cpp` now sole owner of drivers and devices
- GPIO pins (xPin, yPin, trigPin, echoPin) moved to config structs
- ESP32 pin configuration centralized in `TwiST_Config.h`

**Safety and Validation:**
- Pre-flight safety check via `TwiST_ConfigValidator`
- Validates I2C addresses, device IDs/names, GPIO pin conflicts
- Fail-fast behavior - MCU halts on config errors (prevents hardware damage)
- Config validation runs automatically before hardware initialization

### Removed

**Legacy Defines:**
- `PCA9685_ADDRESS`, `PWM_FREQUENCY` - Now in `PWMDriverConfig` struct
- `JOYSTICK_X_PIN`, `JOYSTICK_Y_PIN` - Now in `JoystickConfig.xPin/yPin`
- `DISTANCE_SENSOR_TRIG_PIN`, `DISTANCE_SENSOR_ECHO_PIN` - Now in `DistanceSensorConfig.trigPin/echoPin`

**Note:** I2C pins (`XIAO_SDA_PIN`, `XIAO_SCL_PIN`) remain in config for ESP32 variant flexibility

### Architecture Impact

**Before v1.1.0:**
```cpp
// main.ino - 4 lines, manual steps
framework.initialize();
App::initializeDevices(framework.eventBus());
App::calibrateDevices();
App::registerAllDevices(framework.registry());
```

**After v1.1.0:**
```cpp
// main.ino - 2 lines, single entry point
framework.initialize();
App::initializeSystem(framework);
```

**Configuration Flow:**
```
TwiST_Config.h (device topology)
    ↓
TwiST_ConfigValidator (pre-flight checks)
    ↓
ApplicationConfig.cpp (dynamic driver/device creation)
    ↓
main.ino (minimal application logic)
```

---

## [1.0.0.0] - 2026-01-20 - STABLE

### Added - Initial Release

**Core Framework:**
- Event-driven architecture with `EventBus`
- Device registry for centralized device management
- Abstract driver interfaces (`IPWMDriver`, `IADCDriver`, `IDistanceDriver`)

**Devices:**
- `Servo` - Advanced servo control with easing animations
  - 6 easing functions (LINEAR, EASE_IN/OUT variants for QUAD, CUBIC, QUART)
  - Speed-based movement (degrees per second)
  - Incremental stepping (moveBySteps)
  - Dual calibration modes (STEPS and MICROSECONDS)
- `Joystick` - Analog joystick with calibration and deadzone
  - Axis calibration (min/center/max)
  - Configurable deadzone
  - Normalized output (-1.0 to 1.0)
- `DistanceSensor` - Ultrasonic distance measurement
  - Non-blocking measurement (timer-based)
  - Configurable measurement interval
  - Low-pass filter for noise reduction

**Drivers:**
- `PCA9685` - 16-channel PWM driver (I2C, up to 62 boards)
- `ESP32ADC` - ESP32 native ADC (analog input)
- `HCSR04` - HC-SR04 ultrasonic distance sensor

**Hardware Support:**
- ESP32 (all variants: C6, S3, DevKit, etc.)
- I2C pin configuration for different ESP32 boards
- PCA9685 PWM driver support (50Hz for servos)

**Name-Based Device Access:**
- `App::servo("GripperServo")` - Access devices by name
- `App::joystick("MainJoystick")` - Stable API across config changes
- `App::distanceSensor("ObstacleSensor")` - Production-ready access pattern

**Examples:**
- Basic servo control
- Joystick-controlled servo
- Distance sensor integration
- Multi-servo coordination
- Easing animation demos

### Documentation

- README.md - Quick start and feature overview
- ARCHITECTURE.md - System design and component relationships
- CONFIG_GUIDE.md - Configuration and customization
- CALIBRATION_GUIDE.md - Device calibration procedures

---

## Version History Summary

- **v1.2.0** (2026-01-27) - Logging system, memory safety, single IO channel
- **v1.1.0** (2026-01-26) - Single entry point API, config-driven architecture
- **v1.0.0** (2026-01-20) - Initial stable release with event-driven framework

---

## Upgrade Path

### From v1.1.0 to v1.2.0

**No breaking changes. Backward compatible.**

**Optional changes for Logger integration:**

1. Update main.ino to use Logger:
```cpp
// Before:
Serial.println("System started");

// After:
Logger::info("APP", "System started");
```

2. Framework automatically uses Logger (no changes needed)

### From v1.0.0 to v1.1.0

**Breaking changes: Configuration structure changed**

1. Update `TwiST_Config.h` to use struct-based configuration:
   - Convert device defines to struct arrays
   - See CONFIG_GUIDE.md for examples

2. Update main.ino:
```cpp
// Before v1.1.0:
framework.initialize();
App::initializeDevices(framework.eventBus());
App::calibrateDevices();
App::registerAllDevices(framework.registry());

// After v1.1.0:
framework.initialize();
App::initializeSystem(framework);
```

---

## Future Roadmap

**v1.3.0 - Error Handling:**
- Optional error handling with std::optional
- tryServo(), tryJoystick() safe lookup functions
- Error codes for validation failures

**v1.4.0 - EventBus Enhancements:**
- Event queue with priority system
- Listener execution time monitoring
- Async-only event mode

**v2.0.0 - Bridge System:**
- Concrete Bridge implementations (GenericBridge, ServoJoystickBridge)
- Automatic device mapping
- Remote device control (WiFi, Bluetooth)

---

## Contact

**Author:** Voldemaras Birskys
**Email:** voldemaras@gmail.com
**Project:** TwiST Framework (Twin System Technology)

---

**TwiST Framework - Production-Ready ESP32 Robotics**
