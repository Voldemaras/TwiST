# TwiST Framework - Architecture Documentation

**Version:** 1.2.0.0 - STABLE
**Author:** Voldemaras Birskys
**Email:** voldemaras@gmail.com
**Last Updated:** 2026-01-27

---

## Table of Contents

- [Overview](#overview)
- [Architectural Philosophy](#architectural-philosophy)
- [System Layers](#system-layers)
- [Core Components](#core-components)
- [Configuration System](#configuration-system)
- [Data Flow](#data-flow)
- [Design Patterns](#design-patterns)
- [Memory Management](#memory-management)
- [Safety and Validation](#safety-and-validation)
- [Extensibility](#extensibility)

---

## Overview

TwiST Framework is a config-driven, event-based robotics framework for ESP32 microcontrollers. The architecture emphasizes:

- **Separation of Concerns** - Configuration, logic, and hardware abstraction cleanly separated
- **Dependency Injection** - Explicit, constructor-based dependency management
- **Fail-Fast Validation** - Pre-flight configuration checks prevent runtime errors
- **Zero Hardcoding** - Device topology defined in data structures, not code
- **Modularity** - Components interact through interfaces, enabling testing and hardware swapping

---

## Architectural Philosophy

### Core Principles

**1. Configuration as Data**

Device topology is defined in compile-time constexpr structs (TwiST_Config.h), not scattered across implementation files. This enables:
- Single source of truth for system topology
- Change device count without modifying application code
- Automatic device instantiation via for-loops

**2. Interfaces Over Implementations**

Devices depend on abstract driver interfaces (IPWMDriver, IADCDriver), not concrete implementations (PCA9685, ESP32ADC). This enables:
- Hardware swapping without changing device code
- Mock drivers for unit testing
- Multiple driver implementations coexisting

**3. Event-Driven Decoupling**

Components communicate via EventBus, not direct references. This enables:
- Adding subscribers without modifying publishers
- Runtime behavior changes through event listeners
- Loose coupling between system components

**4. Fail-Fast Safety**

Configuration validation runs BEFORE hardware initialization. Invalid config halts MCU immediately. This prevents:
- Hardware damage from misconfiguration
- Undefined behavior from duplicate device IDs
- I2C bus conflicts from address collisions

---

## System Layers

TwiST Framework consists of 8 architectural layers, each with clear responsibilities:

```
┌─────────────────────────────────────────────────┐
│  Layer 1: Application (main.ino)                │  User application logic
├─────────────────────────────────────────────────┤
│  Layer 2: TwiST_Config.h                        │  Device topology (data)
├─────────────────────────────────────────────────┤
│  Layer 3: TwiST_ConfigValidator                 │  Pre-flight safety checks
├─────────────────────────────────────────────────┤
│  Layer 4: ApplicationConfig                     │  Device/driver initialization
├─────────────────────────────────────────────────┤
│  Layer 5: TwiST Framework Core                  │  EventBus, DeviceRegistry
├─────────────────────────────────────────────────┤
│  Layer 6: Devices (Servo, Joystick, etc.)       │  High-level device logic
├─────────────────────────────────────────────────┤
│  Layer 7: Driver Interfaces                     │  Abstract hardware contracts
├─────────────────────────────────────────────────┤
│  Layer 8: Concrete Drivers (PCA9685, etc.)      │  Hardware implementation
└─────────────────────────────────────────────────┘
```

### Layer 1: Application

**File:** `main.ino`

**Purpose:** User application logic using high-level device API

**Responsibilities:**
- Initialize framework: `framework.initialize()`
- Initialize devices: `App::initializeSystem(framework)` (v1.1.0+)
- Control devices: `App::servo("GripperServo").setAngle(90)`
- Handle application-specific logic

**Dependencies:** TwiST Framework Core, ApplicationConfig

**Key Principle:** Application never creates devices directly. All device management delegated to ApplicationConfig.

### Layer 2: Configuration

**File:** `TwiST_Config.h`

**Purpose:** Single source of truth for device topology

**Responsibilities:**
- Define device configurations (ServoConfig, JoystickConfig, PWMDriverConfig)
- Specify GPIO pins, I2C addresses, calibration parameters
- Compute device counts from std::array::size()

**Dependencies:** None (pure data)

**Key Principle:** Configuration is compile-time constexpr data, zero runtime overhead.

**Example:**
```cpp
static constexpr std::array<ServoConfig, 2> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"BaseServo",    0, 1, 101, CalibrationMode::MICROSECONDS, 0, 0, 500, 2500, 0, 180}
}};

static constexpr uint8_t SERVO_COUNT = SERVO_CONFIGS.size();  // Auto-computed
```

### Layer 3: Configuration Validator

**Files:** `TwiST_ConfigValidator.h`, `TwiST_ConfigValidator.cpp`

**Purpose:** Pre-flight safety checks on device configuration

**Responsibilities:**
- Validate I2C address uniqueness (PWM drivers)
- Validate device ID/name uniqueness
- Validate GPIO pin conflicts
- Validate PWM channel overlaps

**Dependencies:** TwiST_Config.h

**Key Principle:** Fail-fast - halt MCU on validation failure, prevent hardware damage.

**Validation Flow:**
```cpp
bool runSystemConfigSafetyCheck() {
    // Check I2C addresses
    // Check device IDs
    // Check device names
    // Check GPIO pins

    if (!valid) {
        Serial.println("[VALIDATOR] FATAL: Config validation failed!");
        while(true) { delay(1000); }  // Halt MCU
    }
    return true;
}
```

### Layer 4: Application Configuration

**Files:** `ApplicationConfig.h`, `ApplicationConfig.cpp`

**Purpose:** Device and driver initialization from config data

**Responsibilities:**
- Run config validation (TwiST_ConfigValidator)
- Create drivers dynamically from PWM_DRIVER_CONFIGS
- Create devices dynamically from SERVO_CONFIGS, JOYSTICK_CONFIGS, etc.
- Apply calibration from config
- Register devices to framework

**Dependencies:** TwiST_Config.h, TwiST_ConfigValidator, Framework Core, Devices, Drivers

**Key Principle:** For-loop driven instantiation - no hardcoded device creation.

**v1.1.0 Single Entry Point:**
```cpp
void initializeSystem(TwiSTFramework& framework) {
    initializeDevices(framework.eventBus());     // Create drivers + devices
    calibrateDevices();                          // Apply config-based calibration
    registerAllDevices(framework.registry());    // Register to framework
}
```

**Initialization Pattern:**
```cpp
// Dynamic driver creation (Phase 1)
for (uint8_t i = 0; i < PWM_DRIVER_COUNT; i++) {
    const PWMDriverConfig& cfg = PWM_DRIVER_CONFIGS[i];
    pwmDrivers[i] = new Drivers::PCA9685(cfg.i2cAddress);
    pwmDrivers[i]->begin(XIAO_SDA_PIN, XIAO_SCL_PIN);
    pwmDrivers[i]->setFrequency(cfg.frequency);
}

// Dynamic device creation
for (uint8_t i = 0; i < SERVO_COUNT; i++) {
    const ServoConfig& cfg = SERVO_CONFIGS[i];
    servos[i] = new Devices::Servo(
        *pwmDrivers[cfg.pwmDriverIndex],
        cfg.pwmChannel,
        cfg.deviceId,
        cfg.name,
        eventBus
    );
    servos[i]->initialize();
}
```

### Layer 5: Framework Core

**Files:** `TwiST.h`, `TwiST.cpp`, `Core/EventBus.*`, `Core/DeviceRegistry.*`

**Purpose:** Central coordination and event management

**Components:**

**TwiSTFramework:**
- Lifecycle management (initialize, update)
- Component coordination (EventBus, DeviceRegistry)
- Provides access to core services

**EventBus:**
- Publish-subscribe event system
- Decouples event producers from consumers
- Fixed-size listener array (MAX_EVENT_LISTENERS)

**DeviceRegistry:**
- Centralized device collection
- Update loop coordination
- Device lookup by ID

**Key Principle:** Framework coordinates, doesn't control. Devices are autonomous.

### Layer 6: Devices

**Files:** `Devices/Servo.*`, `Devices/Joystick.*`, `Devices/DistanceSensor.*`

**Purpose:** High-level device behavior and logic

**Characteristics:**
- Depend on driver interfaces, not concrete drivers
- Implement IDevice interface (getId, getName, initialize, update)
- Publish events to EventBus
- Stateful (track current position, calibration, etc.)

**Example: Servo Device**
```cpp
class Servo : public IDevice {
private:
    IPWMDriver& driver;  // Interface, not PCA9685
    uint16_t deviceId;
    const char* name;
    EventBus& eventBus;

public:
    Servo(IPWMDriver& drv, uint8_t channel, uint16_t id, const char* name, EventBus& bus)
        : driver(drv), deviceId(id), name(name), eventBus(bus) {}

    void setAngle(float angle) {
        // Logic independent of driver implementation
        uint16_t pwmValue = mapAngleToPWM(angle);
        driver.setPWM(channel, 0, pwmValue);
        eventBus.publish("servo.moved", deviceId);
    }
};
```

**Key Principle:** Devices contain domain logic, drivers provide hardware abstraction.

### Layer 7: Driver Interfaces

**Files:** `Drivers/PWM/IPWMDriver.h`, `Drivers/ADC/IADCDriver.h`, `Drivers/Distance/IDistanceDriver.h`

**Purpose:** Abstract contracts for hardware communication

**Characteristics:**
- Pure virtual interfaces (no implementation)
- Minimal, focused API
- Enable dependency injection and testing

**Example: IPWMDriver**
```cpp
class IPWMDriver {
public:
    virtual ~IPWMDriver() = default;

    virtual void begin(uint8_t sda, uint8_t scl) = 0;
    virtual void setFrequency(uint16_t freq) = 0;
    virtual void setPWM(uint8_t channel, uint16_t on, uint16_t off) = 0;
};
```

**Key Principle:** Interfaces define contracts, not implementations.

### Layer 8: Concrete Drivers

**Files:** `Drivers/PWM/PCA9685.*`, `Drivers/ADC/ESP32ADC.*`, `Drivers/Distance/HCSR04.*`

**Purpose:** Hardware-specific implementations

**Characteristics:**
- Implement driver interfaces
- Encapsulate hardware communication details
- No device-specific logic (just hardware primitives)

**Example: PCA9685 Driver**
```cpp
class PCA9685 : public IPWMDriver {
private:
    Adafruit_PWMServoDriver pwm;
    uint8_t i2cAddress;

public:
    PCA9685(uint8_t addr) : i2cAddress(addr), pwm(addr) {}

    void begin(uint8_t sda, uint8_t scl) override {
        Wire.begin(sda, scl);
        pwm.begin();
    }

    void setPWM(uint8_t channel, uint16_t on, uint16_t off) override {
        pwm.setPWM(channel, on, off);
    }
};
```

**Key Principle:** Drivers are thin wrappers over hardware libraries.

---

## Core Components

### TwiSTFramework

**Purpose:** Central framework coordinator

**Lifecycle:**
1. Construction: Allocate core services (EventBus, DeviceRegistry)
2. Initialize: Prepare framework services
3. Update loop: Coordinate device updates

**API:**
```cpp
class TwiSTFramework {
public:
    void initialize();              // Initialize framework
    void update();                  // Update all devices
    EventBus& eventBus();          // Get EventBus reference
    DeviceRegistry* registry();    // Get DeviceRegistry pointer
};
```

**Usage:**
```cpp
TwiSTFramework framework;

void setup() {
    framework.initialize();
    App::initializeSystem(framework);  // v1.1.0 single entry point
}

void loop() {
    framework.update();  // Delegates to DeviceRegistry
    delay(20);
}
```

### EventBus

**Purpose:** Publish-subscribe event routing

**Characteristics:**
- Fixed-size listener array (MAX_EVENT_LISTENERS = 32)
- Event format: "topic.subtopic", e.g., "servo.100.moved"
- Synchronous event delivery (listeners called immediately)

**API:**
```cpp
class EventBus {
public:
    void subscribe(const char* topic, EventListener listener);
    void publish(const char* topic, uint16_t deviceId = 0);
};
```

**Usage:**
```cpp
// Publish event
eventBus.publish("servo.moved", 100);

// Subscribe to event
eventBus.subscribe("servo.moved", [](const char* topic, uint16_t deviceId) {
    Serial.print("Servo ");
    Serial.print(deviceId);
    Serial.println(" moved");
});
```

### DeviceRegistry

**Purpose:** Centralized device collection and update coordination

**Characteristics:**
- Fixed-size device array (MAX_DEVICES = 32)
- Manages device lifecycle (via update loop)
- Provides device lookup (by ID, not yet implemented)

**API:**
```cpp
class DeviceRegistry {
public:
    void registerDevice(IDevice* device);
    void updateAll();  // Called by framework.update()
};
```

**Usage:**
```cpp
// Register device
registry->registerDevice(servo);

// Framework calls updateAll() in loop
registry->updateAll();  // Calls update() on all devices
```

---

## Configuration System

### v1.1.0 Architecture (Config-Driven)

**Configuration Flow:**
```
TwiST_Config.h (constexpr data)
    ↓
TwiST_ConfigValidator (fail-fast validation)
    ↓
ApplicationConfig.cpp (for-loop creation)
    ↓
Devices and Drivers (runtime objects)
```

**Key Characteristics:**

**1. Single Source of Truth**
- All device topology in TwiST_Config.h
- Device counts auto-computed from std::array::size()
- No hardcoded device instantiation in ApplicationConfig.cpp

**2. Struct-Based Configuration**
- PWMDriverConfig: I2C address, frequency
- ServoConfig: Name, PWM driver index, channel, device ID, calibration
- JoystickConfig: Name, device ID, GPIO pins, calibration
- DistanceSensorConfig: Name, device ID, GPIO pins, filter strength

**3. Compile-Time Safety**
- constexpr ensures all config resolved at compile time
- std::array provides bounds checking
- Zero-size arrays supported (std::array<T, 0> is valid C++)

**4. Dynamic Driver Support**
- Multiple PCA9685 boards: PWM_DRIVER_CONFIGS array
- Each servo specifies pwmDriverIndex (which board to use)
- Scales to 62 PCA9685 boards (992 servo channels)

### Configuration Example

**Adding a New Servo:**
```cpp
// Step 1: Edit TwiST_Config.h
static constexpr std::array<ServoConfig, 3> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},
    {"BaseServo",    0, 1, 101, CalibrationMode::MICROSECONDS, 0, 0, 500, 2500, 0, 180},
    {"NewServo",     0, 2, 102, CalibrationMode::STEPS, 120, 550, 0, 0, 0, 0}  // NEW
}};

// Step 2: Device count auto-computed
static constexpr uint8_t SERVO_COUNT = SERVO_CONFIGS.size();  // Now 3

// Step 3: ApplicationConfig.cpp for-loop automatically creates all 3 servos
for (uint8_t i = 0; i < SERVO_COUNT; i++) {
    const ServoConfig& cfg = SERVO_CONFIGS[i];
    servos[i] = new Devices::Servo(...);  // Automatic instantiation
}

// Step 4: Use in main.ino
App::servo("NewServo").setAngle(90);  // Works immediately
```

---

## Data Flow

### Initialization Sequence (v1.1.0)

```
main.ino: setup()
    │
    ├─> framework.initialize()
    │       │
    │       ├─> EventBus initialization
    │       ├─> DeviceRegistry initialization
    │       └─> Framework ready
    │
    └─> App::initializeSystem(framework)  [Single Entry Point API]
            │
            ├─> initializeDevices(framework.eventBus())
            │       │
            │       ├─> runSystemConfigSafetyCheck()  [Fail-fast validation]
            │       │       │
            │       │       ├─> Validate I2C addresses
            │       │       ├─> Validate device IDs/names
            │       │       ├─> Validate GPIO pins
            │       │       └─> Halt MCU if invalid
            │       │
            │       ├─> Create PWM drivers (for-loop from PWM_DRIVER_CONFIGS)
            │       ├─> Create ADC drivers (for-loop from JOYSTICK_CONFIGS)
            │       ├─> Create ultrasonic drivers (for-loop from DISTANCE_SENSOR_CONFIGS)
            │       ├─> Create servos (for-loop from SERVO_CONFIGS)
            │       ├─> Create joysticks (for-loop from JOYSTICK_CONFIGS)
            │       └─> Create distance sensors (for-loop from DISTANCE_SENSOR_CONFIGS)
            │
            ├─> calibrateDevices()
            │       │
            │       ├─> Calibrate servos (STEPS or MICROSECONDS mode)
            │       ├─> Calibrate joysticks (min/center/max, deadzone)
            │       └─> Calibrate distance sensors (filter strength)
            │
            └─> registerAllDevices(framework.registry())
                    │
                    ├─> Register servos to DeviceRegistry
                    ├─> Register joysticks to DeviceRegistry
                    └─> Register distance sensors to DeviceRegistry
```

### Runtime Loop

```
main.ino: loop()
    │
    ├─> User logic
    │   │
    │   ├─> App::joystick("MainJoystick").getX()
    │   │       │
    │   │       ├─> Joystick device queries ADC driver
    │   │       ├─> ADC driver reads ESP32 GPIO
    │   │       └─> Joystick applies calibration and returns normalized value
    │   │
    │   └─> App::servo("GripperServo").setAngle(90)
    │           │
    │           ├─> Servo device calculates PWM value
    │           ├─> Servo queries PWM driver interface
    │           ├─> PCA9685 driver writes to hardware
    │           └─> Servo publishes "servo.100.moved" event
    │
    └─> framework.update()
            │
            └─> DeviceRegistry::updateAll()
                    │
                    ├─> Call servo.update() (animation state machine)
                    ├─> Call joystick.update() (no-op, state read on-demand)
                    └─> Call distanceSensor.update() (timer-based measurement)
```

### Event Flow Example

```
User Code: App::servo("GripperServo").setAngle(90)
    │
    ├─> Servo device (Devices::Servo)
    │       │
    │       ├─> Calculate PWM value from angle
    │       ├─> Call driver.setPWM(channel, 0, pwmValue)
    │       │       │
    │       │       └─> PCA9685 driver writes to I2C hardware
    │       │
    │       └─> eventBus.publish("servo.moved", 100)
    │               │
    │               └─> EventBus routes to all subscribers
    │                       │
    │                       ├─> Subscriber 1 (e.g., logger)
    │                       ├─> Subscriber 2 (e.g., telemetry)
    │                       └─> Subscriber N
```

---

## Design Patterns

### 1. Dependency Injection

**Pattern:** Constructor-based dependency injection

**Benefits:**
- Explicit dependencies (no hidden globals)
- Testable (inject mock dependencies)
- Compile-time dependency validation

**Example:**
```cpp
// Device depends on driver interface, not concrete driver
class Servo : public IDevice {
private:
    IPWMDriver& driver;  // Injected dependency

public:
    // Constructor receives dependency
    Servo(IPWMDriver& drv, ...) : driver(drv) {}

    void setAngle(float angle) {
        driver.setPWM(...);  // Uses injected driver
    }
};

// Usage
PCA9685 pwmDriver(0x40);              // Concrete driver
Servo servo(pwmDriver, ...);          // Inject dependency
```

### 2. Abstract Factory (Config-Driven)

**Pattern:** For-loop driven object creation from configuration data

**Benefits:**
- Zero hardcoded instantiation
- Change device count without code changes
- Automatic scaling

**Example:**
```cpp
// Configuration data (factory parameters)
static constexpr std::array<ServoConfig, N> SERVO_CONFIGS = {{ ... }};

// Factory loop (ApplicationConfig.cpp)
for (uint8_t i = 0; i < SERVO_COUNT; i++) {
    const ServoConfig& cfg = SERVO_CONFIGS[i];
    servos[i] = new Devices::Servo(
        *pwmDrivers[cfg.pwmDriverIndex],  // Dynamic driver selection
        cfg.pwmChannel,
        cfg.deviceId,
        cfg.name,
        eventBus
    );
}
```

### 3. Observer (EventBus)

**Pattern:** Publish-subscribe event system

**Benefits:**
- Decoupled communication
- Runtime behavior changes
- One-to-many event propagation

**Example:**
```cpp
// Publisher (Servo device)
eventBus.publish("servo.moved", deviceId);

// Subscribers (runtime registration)
eventBus.subscribe("servo.moved", logger);
eventBus.subscribe("servo.moved", telemetry);
```

### 4. Registry (DeviceRegistry)

**Pattern:** Centralized collection of devices

**Benefits:**
- Single update loop coordination point
- Centralized device lookup
- Lifecycle management

**Example:**
```cpp
// Register devices
registry->registerDevice(servo1);
registry->registerDevice(servo2);

// Update all devices in one call
registry->updateAll();  // Calls update() on all devices
```

### 5. Strategy (Calibration Modes)

**Pattern:** Configurable servo calibration algorithms

**Benefits:**
- Switch calibration method at compile time
- Support legacy (MICROSECONDS) and modern (STEPS) modes
- Same device interface, different calibration logic

**Example:**
```cpp
// Config specifies calibration strategy
ServoConfig cfg = {
    "GripperServo", 0, 0, 100,
    CalibrationMode::STEPS,  // Strategy selection
    110, 540, 0, 0, 0, 0
};

// Calibration applied based on mode
if (cfg.calMode == CalibrationMode::STEPS) {
    servo->calibrateBySteps(cfg.minSteps, cfg.maxSteps);
} else {
    servo->calibrate(cfg.minUs, cfg.maxUs, cfg.angleMin, cfg.angleMax);
}
```

---

## Memory Management

### Current Approach (v1.1.0)

**Allocation Strategy:**
- Drivers and devices allocated via `new` in ApplicationConfig.cpp
- Single allocation at startup (initializeDevices)
- Never deallocated (embedded pattern - run forever)

**Storage:**
```cpp
// Static arrays of raw pointers
static std::array<Drivers::PCA9685*, PWM_DRIVER_COUNT> pwmDrivers = {};
static std::array<Devices::Servo*, SERVO_COUNT> servos = {};
```

**Pros:**
- Simple implementation
- Predictable memory usage
- Works for single-shot initialization

**Cons:**
- No automatic cleanup (memory leak if reinitialization needed)
- Unclear ownership semantics
- Risk of dangling pointers

### Future Improvement (v1.2.0 Roadmap)

**Smart Pointers:**
```cpp
// Use std::unique_ptr for automatic cleanup
static std::array<std::unique_ptr<Drivers::PCA9685>, PWM_DRIVER_COUNT> pwmDrivers;
static std::array<std::unique_ptr<Devices::Servo>, SERVO_COUNT> servos;

// Creation
pwmDrivers[i] = std::make_unique<Drivers::PCA9685>(cfg.i2cAddress);

// Automatic cleanup (no delete needed)
```

**Benefits:**
- Clear ownership semantics (unique_ptr = sole owner)
- Automatic cleanup (RAII)
- Prevents memory leaks
- No runtime overhead (zero-cost abstraction)

---

## Safety and Validation

### Pre-Flight Validation (TwiST_ConfigValidator)

**Purpose:** Detect configuration errors before hardware initialization

**Validation Categories:**

**1. I2C Address Conflicts**
```cpp
// Detect multiple PWM drivers at same address
if (address already used) {
    Serial.println("[VALIDATOR] FATAL: I2C address conflict!");
    halt();
}
```

**2. Device ID Uniqueness**
```cpp
// Ensure all device IDs are unique (across all device types)
if (device ID already used) {
    Serial.println("[VALIDATOR] FATAL: Duplicate device ID!");
    halt();
}
```

**3. Device Name Uniqueness**
```cpp
// Ensure all device names are unique
if (device name already used) {
    Serial.println("[VALIDATOR] FATAL: Duplicate device name!");
    halt();
}
```

**4. GPIO Pin Conflicts**
```cpp
// Detect same pin used for multiple purposes
if (pin already assigned) {
    Serial.println("[VALIDATOR] FATAL: GPIO pin conflict!");
    halt();
}
```

**Fail-Fast Behavior:**
```cpp
if (!runSystemConfigSafetyCheck()) {
    Serial.println("[APP] System halted - fix TwiST_Config.h and recompile");
    while (true) { delay(1000); }  // Infinite loop, MCU halted
}
```

**Philosophy:** Better to halt immediately than risk hardware damage or undefined behavior.

---

## Extensibility

### Adding a New Device Type

**Example: Adding a Motor Device**

**Step 1: Create Device Class**
```cpp
// File: Devices/Motor.h
class Motor : public IDevice {
private:
    IPWMDriver& driver;
    uint16_t deviceId;
    const char* name;

public:
    Motor(IPWMDriver& drv, uint8_t channel, uint16_t id, const char* name, EventBus& bus);

    void setSpeed(float speed);  // -1.0 to 1.0
    void stop();

    // IDevice interface
    uint16_t getId() const override;
    const char* getName() const override;
    void initialize() override;
    void update() override;
};
```

**Step 2: Add Config Struct**
```cpp
// File: TwiST_Config.h
struct MotorConfig {
    const char* name;
    uint8_t pwmDriverIndex;
    uint8_t pwmChannel;
    uint16_t deviceId;
    float maxSpeed;
};

static constexpr std::array<MotorConfig, 2> MOTOR_CONFIGS = {{
    {"LeftMotor", 0, 5, 400, 1.0f},
    {"RightMotor", 0, 6, 401, 1.0f}
}};

static constexpr uint8_t MOTOR_COUNT = MOTOR_CONFIGS.size();
```

**Step 3: Add to ApplicationConfig**
```cpp
// File: ApplicationConfig.cpp
static std::array<Devices::Motor*, MOTOR_COUNT> motors = {};

// In initializeDevices():
for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    const MotorConfig& cfg = MOTOR_CONFIGS[i];
    motors[i] = new Devices::Motor(
        *pwmDrivers[cfg.pwmDriverIndex],
        cfg.pwmChannel,
        cfg.deviceId,
        cfg.name,
        eventBus
    );
    motors[i]->initialize();
}

// In registerAllDevices():
for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    registry->registerDevice(motors[i]);
}

// Public accessor:
Devices::Motor& getMotor(uint8_t index) {
    return *motors[index];
}

Devices::Motor& motor(const char* name) {
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        if (strcmp(motors[i]->getName(), name) == 0) {
            return *motors[i];
        }
    }
    // Fallback (v1.2.0 will return nullptr)
    return *motors[0];
}
```

**Step 4: Use in Application**
```cpp
// main.ino
void loop() {
    float speed = App::joystick("MainJoystick").getY();
    App::motor("LeftMotor").setSpeed(speed);
    App::motor("RightMotor").setSpeed(speed);

    framework.update();
    delay(20);
}
```

### Adding a New Driver Type

**Example: Adding I2C LCD Driver**

**Step 1: Define Interface**
```cpp
// File: Drivers/Display/IDisplayDriver.h
class IDisplayDriver {
public:
    virtual ~IDisplayDriver() = default;

    virtual void begin(uint8_t sda, uint8_t scl) = 0;
    virtual void clear() = 0;
    virtual void print(const char* text) = 0;
    virtual void setCursor(uint8_t col, uint8_t row) = 0;
};
```

**Step 2: Implement Concrete Driver**
```cpp
// File: Drivers/Display/LCD_I2C.h
class LCD_I2C : public IDisplayDriver {
private:
    LiquidCrystal_I2C lcd;
    uint8_t i2cAddress;

public:
    LCD_I2C(uint8_t addr, uint8_t cols, uint8_t rows);

    void begin(uint8_t sda, uint8_t scl) override;
    void clear() override;
    void print(const char* text) override;
    void setCursor(uint8_t col, uint8_t row) override;
};
```

**Step 3: Create Device Using Driver**
```cpp
// File: Devices/Display.h
class Display : public IDevice {
private:
    IDisplayDriver& driver;  // Depends on interface

public:
    Display(IDisplayDriver& drv, uint16_t id, const char* name, EventBus& bus);

    void showText(const char* text);
    void showSensorValue(float value);
};
```

**Step 4: Configure and Use**
```cpp
// TwiST_Config.h
struct DisplayConfig {
    const char* name;
    uint16_t deviceId;
    uint8_t i2cAddress;
    uint8_t cols;
    uint8_t rows;
};

static constexpr std::array<DisplayConfig, 1> DISPLAY_CONFIGS = {{
    {"MainDisplay", 500, 0x27, 16, 2}
}};

// ApplicationConfig.cpp - create driver and device
lcdDriver = new Drivers::LCD_I2C(cfg.i2cAddress, cfg.cols, cfg.rows);
display = new Devices::Display(*lcdDriver, cfg.deviceId, cfg.name, eventBus);

// main.ino - use device
App::display("MainDisplay").showText("TwiST v1.1.0");
```

---

## Conclusion

TwiST Framework's architecture provides:

- **Clean Separation:** Configuration, validation, initialization, and runtime logic clearly separated
- **Config-Driven:** Change device topology without touching application code
- **Fail-Fast Safety:** Pre-flight validation prevents hardware damage
- **Hardware Abstraction:** Swap drivers without changing device code
- **Event-Driven:** Loose coupling via EventBus
- **Extensible:** Add new devices and drivers following established patterns

**v1.1.0 Achievements:**
- Single entry point API (2-line setup)
- Config-driven architecture (TwiST_Config.h single source of truth)
- Dynamic driver creation (supports multiple PCA9685 boards)
- For-loop driven device instantiation (zero hardcoding)

**Future Roadmap:**
- v1.2.0: Smart pointers (std::unique_ptr), memory safety
- v1.3.0: Centralized logging system
- v1.4.0: Safe device lookup (nullptr return on not found)
- v2.0.0: Bridge system (multi-MCU, remote control)

---

**TwiST Framework - Architecture Built for Reliability, Designed for Growth**

**Author:** Voldemaras Birskys
**Email:** voldemaras@gmail.com

