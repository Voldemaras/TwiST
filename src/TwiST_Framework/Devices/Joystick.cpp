#include "Joystick.h"

namespace TwiST {
    namespace Devices {

        Joystick::Joystick(IADCDriver& xAxis, IADCDriver& yAxis, uint16_t deviceId, EventBus& eventBus)
            : _xAxis(xAxis), _yAxis(yAxis), _deviceId(deviceId), _eventBus(eventBus) {
            // All dependencies locked at construction - no half-initialized state possible
        }

        // ===== IDevice Lifecycle =====

        bool Joystick::initialize() {
            _state = STATE_INITIALIZING;
            // Default calibration for 12-bit ADC
            _minX = 0;
            _centerX = 2048;
            _maxX = 4095;
            _minY = 0;
            _centerY = 2048;
            _maxY = 4095;
            _state = STATE_READY;
            return true;
        }

        void Joystick::shutdown() {
            _state = STATE_DISABLED;
            _enabled = false;
        }

        void Joystick::update() {
            if (!_enabled || _state != STATE_READY) return;
            // Future: event generation on significant movement
        }

        // ===== IDevice Identity & Capabilities =====

        DeviceInfo Joystick::getInfo() const {
            DeviceInfo info;
            info.type = "Joystick";
            info.name = "Analog Joystick";
            info.id = _deviceId;
            info.capabilities = CAP_INPUT | CAP_ANALOG | CAP_CALIBRATABLE | CAP_CONFIGURABLE;
            info.channelCount = 2;  // X and Y axes
            return info;
        }

        uint16_t Joystick::getCapabilities() const {
            return CAP_INPUT | CAP_ANALOG | CAP_CALIBRATABLE | CAP_CONFIGURABLE;
        }

        bool Joystick::hasCapability(DeviceCapability cap) const {
            return (getCapabilities() & cap) != 0;
        }

        // ===== IDevice State Management =====

        DeviceState Joystick::getState() const {
            return _state;
        }

        void Joystick::enable() {
            _enabled = true;
            if (_state == STATE_DISABLED) {
                _state = STATE_READY;
            }
        }

        void Joystick::disable() {
            _enabled = false;
            _state = STATE_DISABLED;
        }

        bool Joystick::isEnabled() const {
            return _enabled;
        }

        // ===== IDevice Configuration =====

        bool Joystick::configure(const JsonDocument& config) {
            if (config.containsKey("deadzone")) _deadzone = config["deadzone"];
            if (config.containsKey("minX")) _minX = config["minX"];
            if (config.containsKey("centerX")) _centerX = config["centerX"];
            if (config.containsKey("maxX")) _maxX = config["maxX"];
            if (config.containsKey("minY")) _minY = config["minY"];
            if (config.containsKey("centerY")) _centerY = config["centerY"];
            if (config.containsKey("maxY")) _maxY = config["maxY"];
            return true;
        }

        void Joystick::getConfiguration(JsonDocument& config) const {
            config["deadzone"] = _deadzone;
            config["minX"] = _minX;
            config["centerX"] = _centerX;
            config["maxX"] = _maxX;
            config["minY"] = _minY;
            config["centerY"] = _centerY;
            config["maxY"] = _maxY;
        }

        // ===== IDevice Serialization =====

        void Joystick::toJson(JsonDocument& doc) const {
            doc["id"] = _deviceId;
            doc["type"] = "Joystick";
            // Note: Can't call readAnalog from const method, so we just serialize config
            doc["enabled"] = _enabled;
            doc["state"] = _state;
            doc["deadzone"] = _deadzone;
        }

        bool Joystick::fromJson(const JsonDocument& doc) {
            if (doc.containsKey("enabled")) {
                _enabled = doc["enabled"];
            }
            return true;
        }

        // ===== IInputDevice Implementation =====

        float Joystick::readAnalog(uint8_t axis) {
            if (axis == 0) {
                return getX();
            } else if (axis == 1) {
                return getY();
            }
            return 0.5f;  // Center position for unknown axes
        }

        bool Joystick::readDigital(uint8_t button) {
            // Joystick doesn't have digital inputs
            // This is technical debt - will split interface in Phase 4
            return false;
        }

        // ===== Joystick-Specific Clean API =====

        float Joystick::getX() {
            uint16_t raw = _xAxis.readRaw();
            return mapAxisValue(raw, _minX, _centerX, _maxX);
        }

        float Joystick::getY() {
            uint16_t raw = _yAxis.readRaw();
            return mapAxisValue(raw, _minY, _centerY, _maxY);
        }

        void Joystick::calibrate(uint16_t minX, uint16_t centerX, uint16_t maxX,
                                uint16_t minY, uint16_t centerY, uint16_t maxY) {
            _minX = minX;
            _centerX = centerX;
            _maxX = maxX;
            _minY = minY;
            _centerY = centerY;
            _maxY = maxY;
        }

        float Joystick::mapAxisValue(uint16_t raw, uint16_t min, uint16_t center, uint16_t max) {
            // Clamp to calibrated range
            if (raw < min) raw = min;
            if (raw > max) raw = max;

            // Apply deadzone around center
            int16_t offset = raw - center;
            if (abs(offset) < _deadzone) {
                return 0.5f;  // Center position
            }

            // Map to 0.0 - 1.0
            float result;
            if (raw < center) {
                // Lower half (0.0 - 0.5)
                result = 0.5f * ((float)(raw - min) / (float)(center - min));
            } else {
                // Upper half (0.5 - 1.0)
                result = 0.5f + 0.5f * ((float)(raw - center) / (float)(max - center));
            }

            // Final safety clamp to 0.0-1.0
            if (result < 0.0f) result = 0.0f;
            if (result > 1.0f) result = 1.0f;

            return result;
        }
    }
}  // namespace TwiST::Devices