#include "Servo.h"

namespace TwiST {
    namespace Devices {

        Servo::Servo(IPWMDriver& pwm, uint8_t channel, uint16_t deviceId,
                     const char* name, EventBus& eventBus)
            : _pwm(pwm), _channel(channel), _deviceId(deviceId), _name(name), _eventBus(eventBus) {
            // All dependencies locked at construction - no half-initialized state possible
        }

        // ===== IDevice Lifecycle =====

        bool Servo::initialize() {
            _state = STATE_INITIALIZING;
            // Set to center position
            setValue(90);
            _state = STATE_READY;
            return true;
        }

        void Servo::shutdown() {
            _state = STATE_DISABLED;
            _enabled = false;
        }

        void Servo::update() {
            if (!_enabled || _state != STATE_READY) return;

            // Skip if paused
            if (_isPaused) return;

            // Handle animation if active
            if (_animationDuration > 0) {
                unsigned long now = millis();
                unsigned long elapsed = now - _animationStart - _pausedDuration;

                if (elapsed >= _animationDuration) {
                    // Animation complete
                    setValue(_targetAngle);
                    _animationDuration = 0;
                } else {
                    // Interpolate with easing
                    float t = (float)elapsed / (float)_animationDuration;
                    float easedT = applyEasing(t, _easingType);
                    float angle = _startAngle + easedT * (_targetAngle - _startAngle);
                    setValue(angle);
                }
            }
        }

        // ===== IDevice Identity & Capabilities =====

        DeviceInfo Servo::getInfo() const {
            DeviceInfo info;
            info.type = "Servo";
            info.name = _name;  // Use human-readable name from constructor
            info.id = _deviceId;
            info.capabilities = CAP_OUTPUT | CAP_POSITION | CAP_CONFIGURABLE;
            info.channelCount = 1;  // One servo = one channel
            return info;
        }

        uint16_t Servo::getCapabilities() const {
            return CAP_OUTPUT | CAP_POSITION | CAP_CONFIGURABLE;
        }

        bool Servo::hasCapability(DeviceCapability cap) const {
            return (getCapabilities() & cap) != 0;
        }

        // ===== IDevice State Management =====

        DeviceState Servo::getState() const {
            return _state;
        }

        void Servo::enable() {
            _enabled = true;
            if (_state == STATE_DISABLED) {
                _state = STATE_READY;
            }
        }

        void Servo::disable() {
            _enabled = false;
            _state = STATE_DISABLED;
        }

        bool Servo::isEnabled() const {
            return _enabled;
        }

        // ===== IDevice Configuration =====

        bool Servo::configure(const JsonDocument& config) {
            if (config.containsKey("minPulse")) _minPulse = config["minPulse"];
            if (config.containsKey("maxPulse")) _maxPulse = config["maxPulse"];
            if (config.containsKey("minAngle")) _minAngle = config["minAngle"];
            if (config.containsKey("maxAngle")) _maxAngle = config["maxAngle"];
            return true;
        }

        void Servo::getConfiguration(JsonDocument& config) const {
            config["minPulse"] = _minPulse;
            config["maxPulse"] = _maxPulse;
            config["minAngle"] = _minAngle;
            config["maxAngle"] = _maxAngle;
        }

        // ===== IDevice Serialization =====

        void Servo::toJson(JsonDocument& doc) const {
            doc["id"] = _deviceId;
            doc["type"] = "Servo";
            doc["channel"] = _channel;
            doc["angle"] = _currentAngle;
            doc["enabled"] = _enabled;
            doc["state"] = _state;
        }

        bool Servo::fromJson(const JsonDocument& doc) {
            if (doc.containsKey("angle")) {
                setValue(doc["angle"]);
            }
            if (doc.containsKey("enabled")) {
                _enabled = doc["enabled"];
            }
            return true;
        }

        // ===== IOutputDevice Implementation =====

        void Servo::setValue(float angle) {
            // Clamp to calibrated range
            if (angle < _minAngle) angle = _minAngle;
            if (angle > _maxAngle) angle = _maxAngle;

            _currentAngle = angle;
            uint16_t pwmValue = mapAngleToPWM(angle);
            _pwm.setPWM(_channel, pwmValue);  // Uses locked channel
        }

        void Servo::setNormalized(float value) {
            // Map 0.0-1.0 to angle range
            float angle = _minAngle + (value * (_maxAngle - _minAngle));
            setValue(angle);
        }

        void Servo::moveTo(float target, unsigned long duration) {
            _startAngle = _currentAngle;  // Save start position
            _targetAngle = target;
            _animationDuration = duration;
            _animationStart = millis();
            _easingType = EASE_LINEAR;  // Default to linear
            _pausedDuration = 0;
            _isPaused = false;

            if (duration == 0) {
                // Immediate movement
                setValue(target);
                _animationDuration = 0;
            }
        }

        float Servo::getValue() const {
            return _currentAngle;
        }

        bool Servo::isMoving() const {
            return _animationDuration > 0;
        }

        void Servo::setAngle(float angle) {
            setValue(angle);  // Delegate to setValue
        }

        void Servo::calibrate(uint16_t minPulse, uint16_t maxPulse, float minAngle, float maxAngle) {
            _minPulse = minPulse;
            _maxPulse = maxPulse;
            _minAngle = minAngle;
            _maxAngle = maxAngle;
            _useStepMode = false;  // Switch to microseconds mode
        }

        void Servo::calibrateBySteps(uint16_t minStep, uint16_t maxStep, float minAngle, float maxAngle) {
            _stepMin = minStep;
            _stepMax = maxStep;
            _stepRange = maxStep - minStep;
            _minAngle = minAngle;
            _maxAngle = maxAngle;
            _angleRange = maxAngle - minAngle;
            _useStepMode = true;  // Switch to step mode
        }

        // ===== Advanced Motion Control =====

        void Servo::moveToWithEasing(float target, unsigned long duration, EasingType easing) {
            _startAngle = _currentAngle;  // Save current position as start
            _targetAngle = target;
            _animationDuration = duration;
            _animationStart = millis();
            _easingType = easing;
            _pausedDuration = 0;
            _isPaused = false;
        }

        void Servo::moveBySteps(float deltaAngle, unsigned long stepDuration) {
            float target = _currentAngle + deltaAngle;
            moveTo(target, stepDuration);
        }

        void Servo::setSpeed(float degreesPerSecond) {
            _degreesPerSecond = degreesPerSecond;
        }

        void Servo::moveWithSpeed(float target) {
            if (_degreesPerSecond <= 0) {
                setValue(target);  // Immediate if no speed set
                return;
            }

            float distance = abs(target - _currentAngle);
            unsigned long duration = (unsigned long)((distance / _degreesPerSecond) * 1000.0);
            moveTo(target, duration);
        }

        void Servo::stop() {
            _animationDuration = 0;
            _isPaused = false;
            _pausedDuration = 0;
        }

        void Servo::pause() {
            if (_animationDuration > 0 && !_isPaused) {
                _isPaused = true;
                _pausedAt = millis();
            }
        }

        void Servo::resume() {
            if (_isPaused) {
                _pausedDuration += millis() - _pausedAt;
                _isPaused = false;
            }
        }

        unsigned long Servo::getRemainingTime() const {
            if (_animationDuration == 0) return 0;
            unsigned long elapsed = millis() - _animationStart - _pausedDuration;
            if (elapsed >= _animationDuration) return 0;
            return _animationDuration - elapsed;
        }

        float Servo::getProgress() const {
            if (_animationDuration == 0) return 1.0;
            unsigned long elapsed = millis() - _animationStart - _pausedDuration;
            if (elapsed >= _animationDuration) return 1.0;
            return (float)elapsed / (float)_animationDuration;
        }

        // ===== Helper Methods =====

        uint16_t Servo::mapAngleToPWM(float angle) {
            if (_useStepMode) {
                // Direct step-based conversion (no microsecond intermediate)
                // angle → step: step = stepMin + ((angle - angleMin) / angleRange) * stepRange
                float normalizedAngle = (angle - _minAngle) / _angleRange;  // 0.0 to 1.0
                uint16_t pwmTicks = _stepMin + (uint16_t)(normalizedAngle * _stepRange);
                return pwmTicks;
            } else {
                // Legacy microseconds-based conversion
                // Map angle to pulse width in microseconds
                float pulseUs = _minPulse + ((angle - _minAngle) / (_maxAngle - _minAngle)) * (_maxPulse - _minPulse);

                // Convert microseconds to PWM ticks
                // PCA9685: 50Hz = 20ms period, 12-bit = 4096 steps
                // Each tick = 20000us / 4096 = 4.88us
                uint16_t pwmTicks = (uint16_t)(pulseUs / (20000.0 / _pwm.getMaxPWM()));

                return pwmTicks;
            }
        }

        float Servo::applyEasing(float t, EasingType type) {
            // Clamp t to [0,1]
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            switch (type) {
                case EASE_LINEAR:
                    return t;

                case EASE_IN_QUAD:
                    return t * t;

                case EASE_OUT_QUAD:
                    return t * (2.0f - t);

                case EASE_IN_OUT_QUAD:
                    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;

                case EASE_IN_CUBIC:
                    return t * t * t;

                case EASE_OUT_CUBIC: {
                    float t1 = t - 1.0f;
                    return t1 * t1 * t1 + 1.0f;
                }

                default:
                    return t;
            }
        }
    }
}  // namespace TwiST::Devices