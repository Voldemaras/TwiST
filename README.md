# TwiST Framework

**Twin System Technology** - Clean architecture framework for ESP32 robotics.

Simple API. Professional architecture. No Arduino chaos.

---

## What is TwiST?

TwiST separates hardware drivers from application logic. Write clean code that works with any hardware.

**Traditional Arduino:**
```cpp
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
pwm.setPWM(0, 0, map(angle, 0, 180, 150, 600)); // Magic numbers everywhere
```

**With TwiST:**
```cpp
Servo servo(pwmDriver, 0, 100, eventBus);
servo.calibrateBySteps(110, 540);
servo.setAngle(90);  // Clean and simple
```

---

## Quick Start

### 1. Install Libraries

Arduino Library Manager:
- Adafruit PWM Servo Driver Library
- ArduinoJson

### 2. Open Example

```
File → Open → PROJEKTAS/examples/basic_servo/basic_servo.ino
```

Upload to ESP32-C6. Servo sweeps 0° → 180° with smooth easing.

### 3. Calibrate Your Servo

See [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) for step-by-step instructions.

---

## Supported Modules

### Output Devices
- **Servo** - Precise servo motor control with easing functions
  - Step-based or pulse-based calibration
  - Smooth animations (EASE_IN_OUT_QUAD, EASE_CUBIC, etc.)
  - Speed-based movement
  - Pause/resume support

### Input Devices
- **Joystick** - 2-axis analog joystick with deadzone
  - Automatic calibration
  - Center deadzone configuration
  - Normalized output (0.0-1.0)

- **DistanceSensor** - Ultrasonic distance measurement
  - HC-SR04 driver (2-400cm range)
  - Low-pass filter for noise reduction
  - Event-driven distance updates
  - Integer cm readings for stable control

### Supported Hardware

**PWM Drivers:**
- PCA9685 (16-channel, 12-bit)

**ADC:**
- ESP32 built-in ADC

**Distance Sensors:**
- HC-SR04 ultrasonic sensor

**Coming Soon:**
- Buttons
- LEDs
- Stepper motors
- Additional distance sensors (VL53L0X, etc.)

---

## Hardware Setup

### Minimum Setup (Servo Only)

**Components:**
- ESP32-C6 (XIAO or equivalent)
- PCA9685 PWM driver
- Servo motor (SG90 or similar)
- External 5V power supply (2A minimum)

**Wiring:**
```
ESP32    →  PCA9685
GPIO 22  →  SDA
GPIO 23  →  SCL
GND      →  GND
3.3V     →  VCC

PCA9685 Channel 0 → Servo signal wire
External 5V → Servo power (red wire)
GND → Servo ground (brown wire)
```

**CRITICAL:** Never power servos from ESP32 pins. Use external 5V supply.

### With Joystick

**Additional wiring:**
```
ESP32    →  Joystick
GPIO 0   →  VRx (X-axis)
GPIO 1   →  VRy (Y-axis)
3.3V     →  VCC
GND      →  GND
```

### With Distance Sensor

**Additional wiring:**
```
ESP32    →  HC-SR04
5V       →  VCC
GPIO 16  →  TRIG
GPIO 17  →  ECHO
GND      →  GND
```

Note: HC-SR04 3.3V version recommended for ESP32 compatibility.

---

## Examples

### Basic Servo

Simplest example - one servo sweeping with easing:

```cpp
#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/Drivers/PWM/PCA9685.h"

using namespace TwiST;
using namespace TwiST::Devices;
using namespace TwiST::Drivers;

TwiSTFramework framework;
PCA9685 pca9685(0x40);
IPWMDriver& pwm = pca9685;
Servo servo(pwm, 0, 100, framework.eventBus());

void setup() {
    framework.initialize();
    pca9685.begin(22, 23);
    pca9685.setFrequency(50);
    servo.initialize();
    servo.calibrateBySteps(110, 540);
    framework.registry()->registerDevice(&servo);
}

void loop() {
    servo.moveToWithEasing(180, 3000, Servo::EASE_IN_OUT_QUAD);
    while (servo.isMoving()) {
        framework.update();
        delay(20);
    }

    servo.moveToWithEasing(0, 3000, Servo::EASE_IN_OUT_QUAD);
    while (servo.isMoving()) {
        framework.update();
        delay(20);
    }
}
```

### Joystick Control

Control servo with joystick X-axis:

```cpp
// Add after servo setup:
ESP32ADC adcX(0);
ESP32ADC adcY(1);
IADCDriver& xAxis = adcX;
IADCDriver& yAxis = adcY;
Joystick joystick(xAxis, yAxis, 200, framework.eventBus());

void setup() {
    // ... servo setup ...

    adcX.begin();
    adcY.begin();
    joystick.initialize();
    joystick.calibrate(3, 1677, 3290, 3, 1677, 3290);
    joystick.setDeadzone(50);
    framework.registry()->registerDevice(&joystick);
}

void loop() {
    float x = joystick.getX();
    servo.setAngle(x * 180.0);
    framework.update();
    delay(20);
}
```

### Distance-Controlled Servo

See `examples/distance_servo_control/` for hand distance controlling servo position.

---

## Key Concepts

### Clean Architecture

```
Application (your code)
    ↓
Devices (Servo, Joystick - hardware-independent)
    ↓
Interfaces (IPWMDriver, IADCDriver - abstractions)
    ↓
Drivers (PCA9685, ESP32ADC - hardware-specific)
```

Devices never know about specific hardware. Swap PCA9685 for another PWM driver? Change only the driver instantiation.

### Calibration

Every servo is different. Find your values:

```cpp
servo.calibrateBySteps(110, 540);  // Recommended
// or
servo.calibrate(500, 2500, 0, 180);  // Traditional microseconds
```

See [CALIBRATION_GUIDE.md](CALIBRATION_GUIDE.md) for detailed instructions.

### Easing Functions

Smooth, professional motion:

```cpp
servo.moveToWithEasing(180, 3000, Servo::EASE_IN_OUT_QUAD);
```

Available easing types:
- `EASE_LINEAR` - Constant speed
- `EASE_IN_QUAD` / `EASE_OUT_QUAD` - Acceleration/deceleration
- `EASE_IN_OUT_QUAD` - S-curve (smooth start and end)
- `EASE_IN_CUBIC` / `EASE_OUT_CUBIC` - Stronger curves

---

## Project Structure

```
PROJEKTAS/
├── examples/
│   ├── basic_servo/           # Start here
│   ├── distance_sensor/        # HC-SR04 example
│   ├── distance_servo_control/ # Hand distance control
│   └── advanced/               # Production examples with filtering
├── main/
│   └── main.ino                # Full example (joystick + servos)
├── README.md                   # This file
├── CALIBRATION_GUIDE.md        # How to calibrate servos
├── CHANGELOG.md                # Version history
└── LICENSE                     # MIT License
```

---

## Troubleshooting

**Servo buzzing:**
- Wrong calibration values
- See CALIBRATION_GUIDE.md

**Servo not moving:**
- Forgot `framework.update()` in loop()
- Device not registered

**Joystick always center:**
- Wrong calibration
- Deadzone too large

**Distance sensor jittery:**
- Use low-pass filter: `sensor.setFilterStrength(0.3)`
- Use integer cm: `sensor.getDistanceCm()`

---

## License

MIT License

---

## Author

Voldemaras Birškys

TwiST Framework v1.0
