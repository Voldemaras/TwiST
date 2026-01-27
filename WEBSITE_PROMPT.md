# TwiST Framework Website Design Brief

## Project Overview

We are building a website for **TwiST Framework**, a production-ready embedded robotics framework for ESP32 microcontrollers. This is not a hobby project—it's industrial-grade software for serious embedded engineers and robotics developers who need deterministic, fail-safe systems.

---

## Target Audience

- **Embedded systems engineers** working on robotics and automation
- **Professional robotics developers** building commercial products
- **Automation engineers** who need reliable, debuggable systems
- **Advanced makers** who understand memory safety, RAII, and system architecture
- **Engineers transitioning from Arduino prototypes to production code**

---

## Brand Identity

### Core Message
**"TwiST Framework – Complex Robotics Made Simple."**

TwiST is about **deterministic behavior, structured diagnostics, and memory-safe embedded systems**. It replaces the chaos of scattered `Serial.print` calls and raw pointer spaghetti with a **compiler-enforced, fail-fast architecture**.

### Brand Keywords
- **Deterministic** – No hidden side effects, clear ownership
- **Safe** – RAII-based memory management, zero leaks
- **Structured** – Centralized logging, module-based diagnostics
- **Professional** – Production-ready, not prototype-grade
- **Embedded-grade** – Designed for real hardware constraints
- **Fail-fast** – Errors halt the system immediately (safety-critical design)
- **Production-ready** – Used in long-running embedded systems

### Tone of Voice
- **Professional** – This is engineering software, not a toy
- **Precise** – Every word has meaning, no marketing fluff
- **Technical** – Speak to engineers who understand pointers, RAII, and embedded constraints
- **Confident** – We know this architecture works
- **No emojis, no "fun" language, no childish design**

---

## Visual Style

### Design Philosophy
The site should feel like a **professional embedded systems control panel** mixed with **modern developer documentation**. Think: industrial software meets polished SDK landing page.

### Visual References
- **GitHub Docs** – Clean, technical, no nonsense
- **Linear** – Sharp typography, minimal animations
- **Vercel Developer Platform** – Professional dark/light theme
- **JetBrains Product Pages** – Focused on functionality, not marketing
- **Embedded Vendor Portals** (STMicroelectronics, Nordic Semiconductor, Texas Instruments) – Serious, industrial design

### Design Rules
- **Dark/light professional theme** (like GitHub or Linear)
- **Sharp, readable typography** (system fonts or IBM Plex Mono for code)
- **Grid-based layout** – Everything aligned, nothing floating randomly
- **Subtle animations only** – Fade-ins, micro-interactions, no bouncing or spinning
- **No gradient overload** – One or two accent gradients max
- **No childish colors** – Stick to grays, blues, industrial tones
- **Feels like a control panel** – Buttons look like switches, sections like modules

---

## Site Structure

### 1. Hero Section

**Headline (Large, Bold):**
```
TwiST Framework – Embedded Robotics, Done Right.
```

**Subtitle (1-2 lines):**
```
Production-ready ESP32 framework with centralized logging, memory safety, and deterministic architecture.
```

**CTA Buttons:**
- `[Get Started]` (primary button) → Links to https://github.com/Voldemaras/TwiST#quick-start
- `[View on GitHub]` (secondary button) → Links to https://github.com/Voldemaras/TwiST

**Visual:**
- Background: Dark grid or subtle circuit pattern
- Code snippet overlay showing minimal TwiST code (fade in animation)

---

### 2. Why TwiST? (Three Pillars)

Three blocks, side by side:

#### Block 1: Centralized Logging
**Icon:** Terminal/console symbol

**Text:**
```
One unified logging system.
Structured output. Fail-fast error handling.
No scattered Serial.print chaos.
```

**Code snippet:**
```cpp
Logger::info("SERVO", "Angle: 90°");
Logger::fatal("CONFIG", "Invalid - halted");
```

---

#### Block 2: Memory Safety
**Icon:** Lock or shield

**Text:**
```
RAII-based design using std::unique_ptr.
No leaks. No undefined ownership.
Compiler-enforced correctness.
```

**Code snippet:**
```cpp
std::unique_ptr<Devices::Servo> servo;
// Automatic cleanup, zero leaks
```

---

#### Block 3: Deterministic Architecture
**Icon:** Network/flow diagram

**Text:**
```
Clear ownership. Single entry point.
Configuration-driven hardware topology.
No hidden state. No magic.
```

**Code snippet:**
```cpp
App::initializeSystem(framework);
// One call, everything initialized
```

---

### 3. What Makes It Different?

**Comparison Table (Two-column layout):**

| **Typical Arduino Code**       | **TwiST Framework**              |
|--------------------------------|----------------------------------|
| `Serial.println` chaos         | Centralized Logger               |
| Raw pointers everywhere        | Explicit ownership               |
| Hidden side effects            | Deterministic flow               |
| Hard to debug                  | Structured diagnostics           |
| Prototype mindset              | Production mindset               |
| Manual device creation         | Config-driven topology           |
| No error handling              | Fail-fast safety                 |

**Caption below table:**
```
Stop prototyping. Start building production systems.
```

---

### 4. Code Example Section

**Headline:**
```
Write Control Logic, Not Boilerplate
```

**Code Example (Large, centered):**
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
    float x = App::joystick("MainJoystick").getX();
    App::servo("GripperServo").setAngle(x * 180);

    framework.update();
    delay(20);
}
```

**Caption:**
```
No pin numbers. No global state. No magic.
Just named devices and deterministic behavior.
```

---

### 5. Architecture Philosophy

**Headline:**
```
System Design That Makes Sense
```

**Bullet points (short, technical):**
- **Hardware topology is compile-time** (`TwiST_Config.h`)
- **Behavior tuning is runtime** (`ConfigManager`)
- **Devices are owned once, referenced safely**
- **Registry never owns memory**
- **Logger is the only output channel**
- **ApplicationConfig owns devices** (`std::unique_ptr`)
- **DeviceRegistry stores references** (raw pointers, non-owning)

**Diagram (optional):**
```
TwiST_Config.h (Compile-time)
        ↓
ApplicationConfig (Owns devices)
        ↓
DeviceRegistry (References devices)
        ↓
main.ino (Uses devices)
```

**Closing statement:**
```
This is not documentation. This is system architecture.
```

---

### 6. Status Badge Section

**Layout:** Horizontal cards with badges

**Card 1:**
```
Current Version: v1.2.0
Status: STABLE
Release Date: 2026-01-27
```

**Card 2:**
```
Logger Integration: 100%
Memory Safety: Enforced
Serial Usage: Eliminated
```

**Card 3:**
```
Supported Hardware:
ESP32-C6, ESP32-S3, ESP32 DevKit
Up to 992 servo channels (PCA9685)
```

---

### 7. Features Grid

**Four-column grid:**

#### 1. Centralized Logging
- Severity levels (DEBUG, INFO, WARNING, ERROR, FATAL)
- Module-based categorization
- Millisecond timestamps
- Printf-style formatting

#### 2. Memory Safety
- std::unique_ptr ownership
- RAII compliance
- Zero memory leaks
- Automatic cleanup

#### 3. Config-Driven
- Hardware topology in config file
- No hardcoded device counts
- Add devices without touching main code
- Compile-time validation

#### 4. Name-Based Access
- `App::servo("GripperServo")`
- No array indices
- Stable API
- Refactor-safe

---

### 8. Documentation Section

**Headline:**
```
Documentation That Doesn't Waste Your Time
```

**Cards (clickable, link to GitHub):**

1. **Getting Started**
   - Hardware setup
   - Installation
   - First project
   - Link: https://github.com/Voldemaras/TwiST/blob/main/README.md

2. **Configuration Guide**
   - TwiST_Config.h reference
   - Device topology
   - Pin mappings
   - Link: https://github.com/Voldemaras/TwiST/blob/main/CONFIG_GUIDE.md

3. **Calibration Guide**
   - Servo calibration
   - Step-by-step procedure
   - Common values
   - Link: https://github.com/Voldemaras/TwiST/blob/main/CALIBRATION_GUIDE.md

4. **Architecture Overview**
   - System design
   - Component structure
   - Ownership model
   - Link: https://github.com/Voldemaras/TwiST/blob/main/ARCHITECTURE.md

5. **API Reference**
   - Logger API
   - Device access
   - Framework methods
   - Link: https://github.com/Voldemaras/TwiST/tree/main/src/TwiST_Framework

6. **Changelog**
   - Version history
   - Release notes
   - Upgrade paths
   - Link: https://github.com/Voldemaras/TwiST/blob/main/CHANGELOG.md

---

### 9. Hardware Support

**Headline:**
```
Supported Hardware
```

**Two-column layout:**

**Microcontrollers:**
- ESP32-C6 (XIAO Seed)
- ESP32-S3
- ESP32 DevKit
- All ESP32 variants with I2C and ADC

**Peripherals:**
- PCA9685 16-channel PWM driver (up to 62 boards)
- Analog joysticks (ESP32 ADC)
- HC-SR04 ultrasonic distance sensors
- Standard servos (SG90, MG996R, etc.)

---

### 10. Quick Install

**Code block (copy-paste ready):**
```bash
git clone https://github.com/Voldemaras/TwiST.git
cd TwiST
# Open main/main.ino in Arduino IDE
# Select ESP32 board
# Upload
```

---

### 11. Footer

**Minimal, professional:**

**Left side:**
```
TwiST Framework v1.2.0
Production-ready embedded robotics
```

**Center:**
- [GitHub Repository](https://github.com/Voldemaras/TwiST)
- [Documentation](https://github.com/Voldemaras/TwiST#documentation)
- [Changelog](https://github.com/Voldemaras/TwiST/blob/main/CHANGELOG.md)

**Right side:**
```
Author: Voldemaras Birškys
Email: voldemaras@gmail.com
License: MIT
```

**Bottom:**
```
© 2026 TwiST Framework. Built for engineers who care about reliability.
```

---

## Key Design Elements

### Typography
- **Headlines:** Bold, sans-serif (e.g., Inter, SF Pro)
- **Body text:** Clean sans-serif
- **Code:** Monospace (JetBrains Mono, IBM Plex Mono, Fira Code)

### Colors
- **Primary:** Industrial blue (#2563EB or similar)
- **Accent:** Steel gray (#64748B)
- **Error/Fatal:** Red (#DC2626)
- **Success:** Green (#16A34A)
- **Background (dark mode):** #0F172A
- **Background (light mode):** #FFFFFF

### Animations
- **Fade-ins** for sections as user scrolls
- **Hover effects** on buttons (subtle glow)
- **Code syntax highlighting** with animations
- **No bouncing, spinning, or childish effects**

---

## What This Site Is NOT

- ❌ A startup landing page with gradient overload
- ❌ A hobby project with fun illustrations
- ❌ A marketing site with vague buzzwords
- ❌ A cluttered page with 10 CTAs

## What This Site IS

- ✅ Professional embedded systems documentation
- ✅ Industrial-grade product page
- ✅ Technical, precise, and confident
- ✅ Designed for engineers who understand memory safety and RAII

---

## One-Sentence Summary for Lovable/V0

**"Build a modern, minimal, professional website for an embedded robotics framework that feels like industrial-grade software documentation mixed with a polished product landing page—targeting serious embedded engineers who care about deterministic architecture, memory safety, and fail-fast design."**

---

## Final Note

This framework was built by engineers who got tired of Arduino chaos. TwiST is about **bringing software engineering discipline to embedded systems**. The website must reflect that philosophy: **clean, structured, deterministic, and professional**.

No fluff. No emojis. Just reliable systems.
