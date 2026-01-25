/* ============================================================================
 * TwiST Framework | Device
 * ============================================================================
 * @file      DistanceSensor.h
 * @brief     Distance sensor device implementing IInputDevice
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Devices
 * - Type:         Logical Device (hardware-independent)
 * - Hardware:     None (uses IDistanceDriver abstraction)
 * - Implements:   IInputDevice
 *
 * PRINCIPLES:
 * - Hardware-independent distance measurement
 * - Uses IDistanceDriver interface (NEVER includes concrete drivers)
 * - Periodic automatic measurements
 * - Event-driven distance updates
 * - Configurable measurement interval
 *
 * CAPABILITIES:
 * - Automatic periodic distance measurement
 * - Manual trigger support
 * - Distance change event emission
 * - Configurable update rate
 * - Out-of-range detection
 *
 * USAGE PATTERN:
 * - One DistanceSensor object = ONE physical distance sensor
 * - Driver reference locked at construction
 * - Measurement interval configurable (default: 100ms)
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0.1
 * ============================================================================
 */

#ifndef TWIST_DEVICE_DISTANCESENSOR_H
#define TWIST_DEVICE_DISTANCESENSOR_H

#include "../Interfaces/IInputDevice.h"
#include "../Interfaces/IDistanceDriver.h"
#include "../Core/EventBus.h"

namespace TwiST {
    namespace Devices {

    /**
     * @brief Distance sensor device - implements IInputDevice
     *
     * CRITICAL: Uses IDistanceDriver abstraction, NOT concrete hardware.
     * Works with ANY distance sensor (HC-SR04, VL53L0X, etc.)
     */
    class DistanceSensor : public IInputDevice {
        public:
            /**
             * @brief Construct distance sensor device
             * @param driver Reference to IDistanceDriver (NOT HCSR04!)
             * @param deviceId Unique device identifier
             * @param eventBus Reference to EventBus (ALL dependencies in constructor!)
             * @param measurementIntervalMs Automatic measurement interval (default: 100ms)
             */
            DistanceSensor(IDistanceDriver& driver, uint16_t deviceId, EventBus& eventBus,
                        unsigned long measurementIntervalMs = 100);

            // IDevice interface - Lifecycle
            bool initialize() override;
            void shutdown() override;
            void update() override;

            // IDevice interface - Identity & Capabilities
            DeviceInfo getInfo() const override;
            uint16_t getCapabilities() const override;
            bool hasCapability(DeviceCapability cap) const override;

            // IDevice interface - State Management
            DeviceState getState() const override;
            void enable() override;
            void disable() override;
            bool isEnabled() const override;

            // IDevice interface - Configuration
            bool configure(const JsonDocument& config) override;
            void getConfiguration(JsonDocument& config) const override;

            // IDevice interface - Serialization
            void toJson(JsonDocument& doc) const override;
            bool fromJson(const JsonDocument& doc) override;

            // IInputDevice interface
            float readAnalog(uint8_t axis) override;
            bool readDigital(uint8_t button) override { return false; }  // No digital inputs
            bool isInputReady() override;

            // DistanceSensor-specific API
            float getDistance() const { return _currentDistance; }
            uint16_t getDistanceCm() const { return static_cast<uint16_t>(_currentDistance); }  // Whole cm only
            float getMaxRange() const { return _driver.getMaxRange(); }
            bool isInRange() const { return _currentDistance > 0.0f; }
            void setMeasurementInterval(unsigned long intervalMs) { _measurementInterval = intervalMs; }
            void setFilterStrength(float alpha);  // 0.0 = no filter, 0.9 = heavy filter
            void triggerManualMeasurement();

        private:
            IDistanceDriver& _driver;
            uint16_t _deviceId;
            EventBus& _eventBus;

            unsigned long _measurementInterval;
            unsigned long _lastMeasurementTime;
            float _currentDistance;
            float _lastReportedDistance;
            float _filterAlpha;  // Low-pass filter coefficient (0.0-1.0)

            DeviceState _state;
            bool _enabled;

            static constexpr float DISTANCE_CHANGE_THRESHOLD = 1.0f;  // Report if change > 1cm
            static constexpr float DEFAULT_FILTER_ALPHA = 0.3f;       // Default filter strength
        };

    }
}  // namespace TwiST::Devices

#endif