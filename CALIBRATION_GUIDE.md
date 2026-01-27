# Servo Calibration Guide

Servos from different manufacturers need different calibration values. Using default values causes buzzing, reduced range, or physical damage over time.

This guide shows how to find the correct calibration values for your specific servo.

---

## Why Calibrate?

Each servo has physical limits. Sending PWM signals beyond those limits causes:

- Servo buzzing or humming
- Overheating
- Shortened servo lifespan
- Inaccurate positioning
- Reduced movement range

Calibration ensures servo operates within safe range.

---

## Two Calibration Methods

TwiST supports two calibration modes configured in [TwiST_Config.h](src/TwiST_Framework/TwiST_Config.h).

### Method 1: PWM Steps (Recommended)

Direct control using PCA9685 internal step values (0-4095).

**Config Example:**
```cpp
{"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
//                                                    ^^^^^^^^
//                                                    min=110, max=540
```

**Advantages:**
- No conversion overhead
- Most precise
- Direct hardware control
- What the chip actually uses

**Typical Values:**
- SG90 (blue plastic micro servo): 110 to 540 steps
- MG996R (metal gear): 102 to 512 steps
- MG90S: 102 to 512 steps

### Method 2: Microseconds (Traditional)

Pulse width in microseconds (servo datasheet values).

**Config Example:**
```cpp
{"BaseServo", 0, 1, 101, CalibrationMode::MICROSECONDS, 0, 0, 500, 2500, 0, 180}
//                                                              ^^^^^^^^^^^^^^^^^
//                                                              500us-2500us, 0-180 degrees
```

**Advantages:**
- Matches servo datasheet specs
- Familiar from Arduino Servo library
- Good for porting existing projects

**Typical Values:**
- Most servos: 500us to 2500us
- High-end servos: 600us to 2400us

---

## Finding Calibration Values

### Step 1: Create Test Sketch

Create a simple test program to move servo between limits.

```cpp
#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/ApplicationConfig.h"

TwiSTFramework framework;

void setup() {
    Serial.begin(115200);
    framework.initialize();
    App::initializeSystem(framework);

    Logger::info("CALIBRATE", "Starting calibration test");
    Logger::info("CALIBRATE", "Watch for buzzing at 0 and 180 degrees");
}

void loop() {
    Logger::info("CALIBRATE", "Testing 0 degrees (minimum)");
    App::servo("GripperServo").setAngle(0);
    delay(3000);

    Logger::info("CALIBRATE", "Testing 90 degrees (center)");
    App::servo("GripperServo").setAngle(90);
    delay(3000);

    Logger::info("CALIBRATE", "Testing 180 degrees (maximum)");
    App::servo("GripperServo").setAngle(180);
    delay(3000);

    framework.update();
}
```

### Step 2: Start with Safe Values

Edit `TwiST_Config.h`, use conservative starting values:

```cpp
static constexpr std::array<ServoConfig, 1> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 150, 500, 0, 0, 0, 0}
    //                                                   ^^^^^^^^
    //                                                   Safe middle values
}};
```

Upload and observe servo movement.

### Step 3: Listen to Your Servo

**At 0 degrees (minimum position):**
- Silent and stopped = Good
- Buzzing or vibrating = minSteps too low, increase it

**At 180 degrees (maximum position):**
- Silent and stopped = Good
- Buzzing or vibrating = maxSteps too high, decrease it

**At 90 degrees (center):**
- Should always be silent (safe position)

### Step 4: Adjust Values

**If buzzing at 0 degrees:**
```cpp
{"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 160, 500, 0, 0, 0, 0}
//                                                   ^^^
//                                                   Increased from 150 to 160
```

**If buzzing at 180 degrees:**
```cpp
{"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 150, 490, 0, 0, 0, 0}
//                                                       ^^^
//                                                       Decreased from 500 to 490
```

### Step 5: Iterate Until Perfect

Keep adjusting by 10-20 steps until:
- Servo silent at both limits
- Full range of motion achieved
- No buzzing, no overheating

### Step 6: Record Final Values

Document your calibration for future reference:

```
Servo Model: SG90 blue from AliExpress
Calibration: CalibrationMode::STEPS, min=110, max=540
Notes: Buzzing started at 105 (min) and 545 (max)
Date: 2026-01-27
```

Update `TwiST_Config.h` with final values:

```cpp
{"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
```

---

## Conversion Between Methods

### Steps to Microseconds

If you found good step values and need microsecond equivalent:

```
pulse_us = (step × 20000) / 4096
```

**Example:**
- 110 steps = (110 × 20000) / 4096 = 537us
- 540 steps = (540 × 20000) / 4096 = 2636us

### Microseconds to Steps

If you have pulse values from datasheet:

```
step = (pulse_us × 4096) / 20000
```

**Example:**
- 500us = (500 × 4096) / 20000 = 102 steps
- 2500us = (2500 × 4096) / 20000 = 512 steps

---

## Common Servo Values

These are typical values. Always test your specific servo.

### SG90 (Blue Plastic Micro Servo, 9g)

```cpp
{"Servo1", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0}
// Or using microseconds:
{"Servo1", 0, 0, 100, CalibrationMode::MICROSECONDS, 0, 0, 500, 2500, 0, 180}
```

### MG90S (Metal Gear Micro Servo, 9g)

```cpp
{"Servo2", 0, 1, 101, CalibrationMode::STEPS, 102, 512, 0, 0, 0, 0}
```

### MG996R (High Torque Servo, 55g)

```cpp
{"Servo3", 0, 2, 102, CalibrationMode::STEPS, 102, 512, 0, 0, 0, 0}
```

### DS3218 (Digital High Torque Servo)

```cpp
{"Servo4", 0, 3, 103, CalibrationMode::STEPS, 105, 520, 0, 0, 0, 0}
```

---

## Multiple Servos Configuration

Each servo can have different calibration in `TwiST_Config.h`:

```cpp
static constexpr std::array<ServoConfig, 3> SERVO_CONFIGS = {{
    {"GripperServo", 0, 0, 100, CalibrationMode::STEPS, 110, 540, 0, 0, 0, 0},  // SG90
    {"ArmServo", 0, 1, 101, CalibrationMode::STEPS, 102, 512, 0, 0, 0, 0},      // MG996R
    {"BaseServo", 0, 2, 102, CalibrationMode::MICROSECONDS, 0, 0, 600, 2400, 0, 180}  // DS3218
}};
```

Framework applies calibration automatically during initialization.

---

## Troubleshooting

### Servo buzzes at one limit

**At 0 degrees:**
- Increase minSteps by 10-20
- Example: 110 → 120 → 130 until buzzing stops

**At 180 degrees:**
- Decrease maxSteps by 10-20
- Example: 540 → 530 → 520 until buzzing stops

### Servo doesn't reach full 180 degrees

Some servos have limited physical range. Two options:

**Option 1: Accept reduced range**
```cpp
{"Servo", 0, 0, 100, CalibrationMode::STEPS, 110, 480, 0, 0, 0, 0}
// Servo physically moves only 150 degrees
// Use: servo.setAngle(0) to servo.setAngle(150)
```

**Option 2: Map to smaller angle range**
```cpp
{"Servo", 0, 0, 100, CalibrationMode::STEPS, 110, 480, 0, 150, 0, 0}
//                                                       ^^^
//                                                       Angle range 0-150
// Now: servo.setAngle(150) reaches physical limit
```

### Servo jitters in middle position

Not a calibration issue. Check:

1. **Power supply** - Servos need stable 5V with adequate current (2A minimum for multiple servos)
2. **Wiring** - Loose connections cause jitter
3. **Load** - Servo may be struggling against weight or friction
4. **Interference** - Keep servo wires away from power lines

### Servo gets hot

**Hot at rest (not moving):**
- Servo is buzzing at limit (fix calibration)
- Servo fighting external force

**Hot after prolonged use:**
- Normal for high-load servos
- Ensure adequate cooling
- Give servos rest time between heavy operations

---

## Advanced: Custom Angle Ranges

TwiST allows mapping PWM range to any angle range.

### 270-Degree Servo

Some servos have extended 270-degree range:

```cpp
{"WideServo", 0, 0, 100, CalibrationMode::STEPS, 102, 614, 0, 270, 0, 0}
//                                                           ^^^
//                                                           0-270 degrees
```

Usage:
```cpp
App::servo("WideServo").setAngle(0);    // Full left
App::servo("WideServo").setAngle(135);  // Center
App::servo("WideServo").setAngle(270);  // Full right
```

### Reverse Direction

Swap min/max steps to reverse servo direction:

```cpp
{"ReverseServo", 0, 0, 100, CalibrationMode::STEPS, 540, 110, 0, 0, 0, 0}
//                                                   ^^^^^^^^
//                                                   max=540, min=110 (swapped)
```

Result:
```cpp
App::servo("ReverseServo").setAngle(0);    // Physically at 180 degrees
App::servo("ReverseServo").setAngle(180);  // Physically at 0 degrees
```

### Limited Range

Use only portion of servo's physical range:

```cpp
{"LimitedServo", 0, 0, 100, CalibrationMode::STEPS, 325, 540, 0, 0, 0, 0}
//                                                   ^^^^^^^^
//                                                   Uses 90-180 degrees only
```

Result:
```cpp
App::servo("LimitedServo").setAngle(0);    // Physically at 90 degrees
App::servo("LimitedServo").setAngle(180);  // Physically at 180 degrees
```

---

## Best Practices

1. **Test every servo** - Even identical models from same batch vary
2. **Start conservative** - Use narrow range (150-500), expand gradually
3. **Listen for buzzing** - Audible warning of unsafe limits
4. **Document values** - Write them down for future projects
5. **Retest after changes** - Adding servo horn or load changes characteristics
6. **Use external power** - Never power servos from ESP32 pins
7. **Prefer STEPS mode** - More direct and precise than microseconds

---

## Quick Reference

**Configuration Format:**

```cpp
// STEPS mode (recommended):
{"Name", pwmDrvIdx, pwmCh, deviceId, CalibrationMode::STEPS, minSteps, maxSteps, 0, 0, 0, 0}

// MICROSECONDS mode:
{"Name", pwmDrvIdx, pwmCh, deviceId, CalibrationMode::MICROSECONDS, 0, 0, minUs, maxUs, angleMin, angleMax}
```

**Common Starting Values:**

```cpp
// Safe conservative start:
CalibrationMode::STEPS, 150, 500

// SG90 typical:
CalibrationMode::STEPS, 110, 540

// MG996R typical:
CalibrationMode::STEPS, 102, 512
```

---

## Summary

1. Edit `TwiST_Config.h` with safe starting values (150-500)
2. Upload test sketch
3. Observe servo at 0, 90, and 180 degrees
4. Listen for buzzing
5. Adjust minSteps if buzzing at 0 degrees
6. Adjust maxSteps if buzzing at 180 degrees
7. Iterate until silent at both limits
8. Record final values in config
9. Use calibrated servo in your application

Calibrate once, use forever. Framework applies calibration automatically.

---

Written for TwiST Framework v1.2.0
Author: Voldemaras Birskys
Email: voldemaras@gmail.com
