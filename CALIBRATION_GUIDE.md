# Servo Calibration Guide

Servo motors vary by manufacturer. **Default values DO NOT work reliably.** Wrong calibration causes:
- Servo buzzing/humming at limits
- Reduced movement range
- Physical damage over time
- Inaccurate positioning

This guide shows how to calibrate servos properly with TwiST.

---

## Two Calibration Methods

### Method 1: Step-Based (Recommended)

Direct PWM step values (0-4095) that PCA9685 chip uses internally.

```cpp
servo.calibrateBySteps(110, 540);
```

**Advantages:**
- No conversion overhead
- Direct hardware control
- Most precise
- What the chip actually uses

**Typical values:**
- SG90: `110` to `540` steps
- MG996R: `102` to `512` steps

### Method 2: Pulse-Based (Traditional)

Microsecond pulse widths from servo datasheets.

```cpp
servo.calibrate(500, 2500, 0, 180);
```

**Advantages:**
- Familiar if coming from Arduino servo library
- Matches servo datasheet specs
- Good for porting existing code

**Typical values:**
- Most servos: `500μs` to `2500μs`
- High-end servos: `600μs` to `2400μs`

---

## Finding Your Calibration Values

### Manual Calibration (Step-by-Step)

Start with safe middle values, then expand until you find limits.

**1. Create test sketch:**

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
    Serial.begin(115200);

    framework.initialize();
    pca9685.begin(22, 23);
    pca9685.setFrequency(50);
    servo.initialize();

    // Start conservative
    servo.calibrateBySteps(150, 500);

    framework.registry()->registerDevice(&servo);

    Serial.println("Calibration Test - Watch for buzzing!");
}

void loop() {
    // Test minimum limit
    Serial.println("Testing 0° (minimum)...");
    servo.setAngle(0);
    delay(3000);

    // Test center
    Serial.println("Testing 90° (center)...");
    servo.setAngle(90);
    delay(3000);

    // Test maximum limit
    Serial.println("Testing 180° (maximum)...");
    servo.setAngle(180);
    delay(3000);

    framework.update();
}
```

**2. Listen to your servo:**

At **0° (minimum)**:
- ✅ Silent and stopped → Good
- ❌ Buzzing/vibrating → `minStep` is too low, increase it

At **180° (maximum)**:
- ✅ Silent and stopped → Good
- ❌ Buzzing/vibrating → `maxStep` is too high, decrease it

**3. Adjust values:**

If buzzing at 0°:
```cpp
servo.calibrateBySteps(160, 500);  // Increased minStep from 150 to 160
```

If buzzing at 180°:
```cpp
servo.calibrateBySteps(150, 490);  // Decreased maxStep from 500 to 490
```

**4. Iterate until perfect:**

Keep adjusting by 10-20 steps until servo is silent at both limits.

**5. Record your values:**

```
My Servo: SG90 blue from AliExpress
Final calibration: calibrateBySteps(110, 540)
Notes: Buzzing started at 105 (min) and 545 (max)
Date: 2026-01-22
```

---

## Conversion Between Methods

### Steps to Microseconds

If you found good step values and want microseconds:

```
pulse_μs = (step × 20000) / 4096
```

**Example:**
- 110 steps → `(110 × 20000) / 4096` = **537μs**
- 540 steps → `(540 × 20000) / 4096` = **2636μs**

### Microseconds to Steps

If you have pulse values from datasheet:

```
step = (pulse_μs × 4096) / 20000
```

**Example:**
- 500μs → `(500 × 4096) / 20000` = **102 steps**
- 2500μs → `(2500 × 4096) / 20000` = **512 steps**

---

## Common Servo Values

### SG90 (Blue plastic, 9g micro servo)
```cpp
servo.calibrateBySteps(110, 540);
// Or pulse-based:
servo.calibrate(500, 2500, 0, 180);
```

### MG90S (Metal gear, 9g servo)
```cpp
servo.calibrateBySteps(102, 512);
// Or:
servo.calibrate(500, 2500, 0, 180);
```

### MG996R (High torque, 55g servo)
```cpp
servo.calibrateBySteps(102, 512);
// Or:
servo.calibrate(500, 2500, 0, 180);
```

### DS3218 (Digital, high torque)
```cpp
servo.calibrateBySteps(105, 520);
// Or:
servo.calibrate(512, 2500, 0, 180);
```

**⚠️ Important:** These are typical values. **Always test YOUR specific servo!**

---

## Multiple Servos on Same PCA9685

Each servo can have different calibration:

```cpp
Servo servo1(pwm, 0, 100, framework.eventBus());
Servo servo2(pwm, 1, 101, framework.eventBus());
Servo servo3(pwm, 2, 102, framework.eventBus());

void setup() {
    // ...initialization...

    // Different calibration for each
    servo1.calibrateBySteps(110, 540);  // SG90 on channel 0
    servo2.calibrateBySteps(102, 512);  // MG996R on channel 1
    servo3.calibrateBySteps(105, 520);  // DS3218 on channel 2
}
```

---

## Troubleshooting

### Servo buzzes at one limit

**At 0° (minimum):**
- Increase `minStep` by 10-20
- Example: `110` → `120` → `130` until buzzing stops

**At 180° (maximum):**
- Decrease `maxStep` by 10-20
- Example: `540` → `530` → `520` until buzzing stops

### Servo doesn't reach full 180° range

Your servo might have smaller physical range.

**Option 1:** Accept reduced range
```cpp
servo.calibrateBySteps(110, 480);  // Only 150° physical range
// Just use: servo.setAngle(0) to servo.setAngle(150)
```

**Option 2:** Map to smaller angle range
```cpp
servo.calibrateBySteps(110, 480, 0, 150);  // Map 110-480 to 0-150°
// Now: servo.setAngle(150) reaches physical limit
```

### Servo jitters in middle position

Not a calibration issue. Check:
1. **Power supply** - Servos need stable 5V/2A minimum
2. **Wiring** - Loose connections cause jitter
3. **Load** - Servo struggling against weight/friction

### Servo gets hot

**If hot at rest (not moving):**
- Buzzing at limit (fix calibration)
- Servo fighting external force

**If hot after use:**
- Normal for high-load servos
- Ensure adequate cooling/rest time

---

## Advanced: Custom Angle Ranges

You can map PWM range to any angle range:

### Example: 270° Servo

```cpp
// Servo physically moves 0-270°
servo.calibrateBySteps(102, 614, 0, 270);

servo.setAngle(0);    // Full left
servo.setAngle(135);  // Center
servo.setAngle(270);  // Full right
```

### Example: Reverse Direction

```cpp
// Swap min/max to reverse servo direction
servo.calibrateBySteps(540, 110, 0, 180);

servo.setAngle(0);    // Physically at 180°
servo.setAngle(180);  // Physically at 0°
```

### Example: Limited Range

```cpp
// Use only 90-180° of servo's physical range
servo.calibrateBySteps(325, 540, 0, 180);

servo.setAngle(0);    // Physically at 90°
servo.setAngle(180);  // Physically at 180°
```

---

## Best Practices

1. **Test every servo** - Even identical models from same batch vary
2. **Start conservative** - Use narrow range, expand gradually
3. **Listen for buzzing** - Audible warning of over-travel
4. **Document values** - Write them down for future reference
5. **Retest after changes** - Adding servo horn changes load characteristics
6. **Use external power** - Never power servos from ESP32 pins
7. **Prefer step-based** - More direct, more precise than pulse-based

---

## Quick Reference Card

```cpp
// STEP-BASED (Recommended)
servo.calibrateBySteps(minStep, maxStep);
servo.calibrateBySteps(minStep, maxStep, minAngle, maxAngle);

// PULSE-BASED (Traditional)
servo.calibrate(minPulse, maxPulse, minAngle, maxAngle);

// Common values:
// SG90:    calibrateBySteps(110, 540)
// MG996R:  calibrateBySteps(102, 512)
// Custom:  calibrateBySteps(???, ???)  ← Test yours!
```

---

## Summary

1. Start with `calibrateBySteps(150, 500)` (safe middle values)
2. Test servo at 0° and 180°
3. Listen for buzzing
4. Adjust `minStep`/`maxStep` until silent
5. Record your final values
6. Use them in your application

**That's it.** Calibrate once, enjoy precise servo control.

---

Written for TwiST Framework v1.0
Author: Voldemaras Birškys
