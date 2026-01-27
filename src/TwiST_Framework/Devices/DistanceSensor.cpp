/* ============================================================================
 * TwiST Framework | Device Implementation
 * ============================================================================
 * @file      DistanceSensor.cpp
 * @brief     Distance sensor device implementation
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0.1
 * ============================================================================
 */

#include "DistanceSensor.h"
#include <Arduino.h>

namespace TwiST {
namespace Devices {

DistanceSensor::DistanceSensor(IDistanceDriver& driver, uint16_t deviceId,
                               const char* name, EventBus& eventBus,
                               unsigned long measurementIntervalMs)
    : _driver(driver)
    , _deviceId(deviceId)
    , _name(name)
    , _eventBus(eventBus)
    , _measurementInterval(measurementIntervalMs)
    , _lastMeasurementTime(0)
    , _currentDistance(0.0f)
    , _lastReportedDistance(0.0f)
    , _filterAlpha(DEFAULT_FILTER_ALPHA)
    , _state(STATE_UNINITIALIZED)
    , _enabled(false)
{
}

// ===== IDevice Lifecycle =====

bool DistanceSensor::initialize() {
    _state = STATE_INITIALIZING;
    _lastMeasurementTime = millis();
    _currentDistance = 0.0f;
    _lastReportedDistance = 0.0f;
    _enabled = true;
    _state = STATE_READY;
    return true;
}

void DistanceSensor::shutdown() {
    _state = STATE_DISABLED;
    _enabled = false;
}

void DistanceSensor::update() {
    if (!_enabled || _state != STATE_READY) return;

    unsigned long now = millis();

    // Check if it's time for next measurement
    if (now - _lastMeasurementTime >= _measurementInterval) {
        _lastMeasurementTime = now;

        // Trigger measurement
        _driver.triggerMeasurement();

        // Read raw distance from driver
        float rawDistance = _driver.readDistanceCm();

        // Apply low-pass filter (exponential moving average)
        // Formula: filtered = alpha * raw + (1 - alpha) * previous
        // alpha = 0.0 → no change (full smoothing)
        // alpha = 1.0 → raw value (no smoothing)
        if (_currentDistance == 0.0f) {
            // First measurement - initialize filter
            _currentDistance = rawDistance;
        } else {
            // Apply exponential moving average filter
            _currentDistance = _filterAlpha * rawDistance + (1.0f - _filterAlpha) * _currentDistance;
        }

        // Emit event if distance changed significantly
        float change = abs(_currentDistance - _lastReportedDistance);
        if (change >= DISTANCE_CHANGE_THRESHOLD) {
            Event evt = {
                .name = "distance.changed",
                .sourceDeviceId = _deviceId,
                .data = NULL,
                .priority = PRIORITY_NORMAL,
                .timestamp = millis()
            };
            _eventBus.publish(evt);

            _lastReportedDistance = _currentDistance;
        }
    }
}

// ===== IDevice Identity & Capabilities =====

DeviceInfo DistanceSensor::getInfo() const {
    DeviceInfo info;
    info.type = "DistanceSensor";
    info.name = _name;  // Use human-readable name from constructor
    info.id = _deviceId;
    info.capabilities = CAP_INPUT | CAP_ANALOG | CAP_CONFIGURABLE;
    info.channelCount = 1;  // One distance sensor = one channel
    return info;
}

uint16_t DistanceSensor::getCapabilities() const {
    return CAP_INPUT | CAP_ANALOG | CAP_CONFIGURABLE;
}

bool DistanceSensor::hasCapability(DeviceCapability cap) const {
    return (getCapabilities() & cap) != 0;
}

// ===== IDevice State Management =====

DeviceState DistanceSensor::getState() const {
    return _state;
}

void DistanceSensor::enable() {
    _enabled = true;
    if (_state == STATE_DISABLED) {
        _state = STATE_READY;
    }
}

void DistanceSensor::disable() {
    _enabled = false;
    _state = STATE_DISABLED;
}

bool DistanceSensor::isEnabled() const {
    return _enabled;
}

// ===== IDevice Configuration =====

bool DistanceSensor::configure(const JsonDocument& config) {
    if (config.containsKey("measurementInterval")) {
        _measurementInterval = config["measurementInterval"];
    }
    return true;
}

void DistanceSensor::getConfiguration(JsonDocument& config) const {
    config["measurementInterval"] = _measurementInterval;
}

// ===== IDevice Serialization =====

void DistanceSensor::toJson(JsonDocument& doc) const {
    doc["id"] = _deviceId;
    doc["type"] = "DistanceSensor";
    doc["distance"] = _currentDistance;
    doc["maxRange"] = _driver.getMaxRange();
    doc["enabled"] = _enabled;
    doc["state"] = _state;
}

bool DistanceSensor::fromJson(const JsonDocument& doc) {
    if (doc.containsKey("enabled")) {
        _enabled = doc["enabled"];
    }
    if (doc.containsKey("measurementInterval")) {
        _measurementInterval = doc["measurementInterval"];
    }
    return true;
}

// ===== IInputDevice Implementation =====

float DistanceSensor::readAnalog(uint8_t axis) {
    // Normalize distance: 0.0 (at sensor) to 1.0 (max range)
    if (axis == 0) {
        float maxRange = _driver.getMaxRange();
        if (_currentDistance <= 0.0f || maxRange <= 0.0f) {
            return 0.0f;
        }
        return _currentDistance / maxRange;
    }
    return 0.0f;
}

bool DistanceSensor::isInputReady() {
    return _driver.isMeasurementReady();
}

// ===== DistanceSensor-Specific Methods =====

void DistanceSensor::setFilterStrength(float alpha) {
    // Clamp to valid range [0.0, 1.0]
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    _filterAlpha = alpha;
}

void DistanceSensor::triggerManualMeasurement() {
    _driver.triggerMeasurement();
    float rawDistance = _driver.readDistanceCm();

    // Apply filter to manual measurements too
    if (_currentDistance == 0.0f) {
        _currentDistance = rawDistance;
    } else {
        _currentDistance = _filterAlpha * rawDistance + (1.0f - _filterAlpha) * _currentDistance;
    }
}

}}  // namespace TwiST::Devices
