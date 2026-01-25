# Changelog

## [1.0.0] - 2026-01-22

Initial release of TwiST Framework.

### Framework Core
- Event-driven architecture with EventBus
- Device registry for centralized device management
- Configuration manager for JSON-based device configuration
- Clean dependency injection (constructor-based)

### Output Devices
- **Servo** - Precise servo motor control
  - Step-based calibration (direct PWM control)
  - Pulse-based calibration (traditional microseconds)
  - Easing functions (LINEAR, QUAD, CUBIC)
  - Speed-based movement
  - Pause/resume support
  - Progress tracking

### Input Devices
- **Joystick** - 2-axis analog joystick
  - Automatic calibration (min/center/max)
  - Center deadzone configuration
  - Normalized output (0.0-1.0)

- **DistanceSensor** - Ultrasonic distance measurement
  - HC-SR04 driver (2-400cm range)
  - Automatic periodic measurements
  - Low-pass filter (exponential moving average)
  - Event-driven distance change notifications
  - Integer cm readings for stable control

### Hardware Drivers
- **PCA9685** - 16-channel 12-bit PWM driver
  - I2C communication
  - Configurable frequency (50Hz for servos)
  - Full 4096-step resolution

- **ESP32ADC** - Built-in ADC support
  - Configurable resolution (12-bit default)
  - Normalized readings (0.0-1.0)

- **HCSR04** - HC-SR04 ultrasonic distance sensor
  - 2-400cm measurement range
  - Trigger/echo pin control
  - Timeout handling

### Examples
- `examples/basic_servo/` - Simple servo sweep with easing
- `examples/distance_sensor/` - Basic HC-SR04 distance reading
- `examples/distance_servo_control/` - Hand distance controls servo
- `examples/advanced/distance_servo_filtered.ino` - Production example with filtering + hysteresis
- `main/main.ino` - Full application (joystick + multi-servo control)

### Documentation
- README.md - Quick start guide with supported modules
- CALIBRATION_GUIDE.md - Step-by-step servo calibration
- Code examples in all headers
- Inline documentation

### Architecture
- Clean separation: Devices → Interfaces → Drivers
- Hardware-independent devices
- Interface-based abstraction (IPWMDriver, IADCDriver, IDistanceDriver)
- No device depends on concrete hardware

### Validated
- Step boundaries: 110-540 (SG90 servo)
- Single-step resolution verified
- Easing precision: < 0.5° error
- Physical hardware tested on ESP32-C6 + PCA9685
