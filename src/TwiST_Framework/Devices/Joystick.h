/* ============================================================================
 * TwiST Framework | Native Device
 * ============================================================================
 * @file      Joystick.h
 * @brief     Analog joystick input device (X/Y axes) based on ADC abstraction.
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Devices
 * - Type:         Native Input Device
 * - Hardware:     ADC via IADCDriver abstraction
 * - Dependency:   EventBus, IADCDriver
 *
 * PRINCIPLES:
 * - No hardware knowledge (NO Drivers includes)
 * - Constructor Injection only
 * - Works with any ADC implementation (ESP32 ADC, ADS1115, etc.)
 * - Clean Architecture compliant
 *
 * CAPABILITIES:
 * - Analog input
 * - Calibration
 * - Deadzone filtering
 * - JSON configuration & serialization
 *
 * AUTHOR:    Voldemaras Birskys
 *  * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================ */

#ifndef TWIST_DEVICE_JOYSTICK_H
#define TWIST_DEVICE_JOYSTICK_H

#include "../Interfaces/IInputDevice.h"
#include "../Interfaces/IADCDriver.h" 
#include "../Core/EventBus.h"

namespace TwiST {
    namespace Devices {

        /**
         * @brief Native joystick device - implements IInputDevice
         *
         * Uses IADCDriver abstraction - NEVER knows about ESP32ADC or any specific hardware.
         * Can work with built-in ESP32 ADC, external ADS1115, or any other ADC.
         */
        class Joystick : public IInputDevice {
        public:
            /**
             * @param xAxis Reference to IADCDriver for X-axis (NOT ESP32ADC!)
             * @param yAxis Reference to IADCDriver for Y-axis
             * @param deviceId Unique device ID
             * @param eventBus Reference to EventBus (ALL dependencies in constructor!)
             */
            Joystick(IADCDriver& xAxis, IADCDriver& yAxis, uint16_t deviceId, EventBus& eventBus);
            // ABSTRACTION, not concrete hardware!
            // Constructor injection!

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

            // IInputDevice interface (for framework compatibility)
            float readAnalog(uint8_t axis) override;       // Internal use - prefer getX()/getY()
            bool readDigital(uint8_t button) override;     // Button state
            bool isInputReady() override { return true; }

            // Joystick-specific - Clean API (NO magic numbers!)
            float getX();  // Read X-axis (normalized 0.0-1.0)
            float getY();  // Read Y-axis (normalized 0.0-1.0)

            void calibrate(uint16_t minX, uint16_t centerX, uint16_t maxX,
                        uint16_t minY, uint16_t centerY, uint16_t maxY);
            void setDeadzone(uint16_t deadzone) { _deadzone = deadzone; }

        private:
            IADCDriver& _xAxis;  // Abstract interface
            IADCDriver& _yAxis;  // Abstract interface
            uint16_t _deviceId;
            EventBus& _eventBus;  // Reference, not pointer - ALWAYS valid

            // State
            DeviceState _state = STATE_UNINITIALIZED;
            bool _enabled = true;

            // Calibration
            uint16_t _deadzone = 50;
            uint16_t _minX, _centerX, _maxX;
            uint16_t _minY, _centerY, _maxY;

            float mapAxisValue(uint16_t raw, uint16_t min, uint16_t center, uint16_t max);
        };
    }
}  // namespace TwiST::Devices
#endif