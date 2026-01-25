/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IDevice.h
 * @brief     Base interface for all devices - defines lifecycle, identity, and configuration
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   None (IS the base interface)
 *
 * PRINCIPLES:
 * - All devices inherit from this interface
 * - Defines common lifecycle (initialize, update, shutdown)
 * - Provides capability discovery mechanism
 * - Supports JSON-based configuration
 * - Hardware-independent abstraction
 *
 * CAPABILITIES:
 * - Lifecycle management (init, update, shutdown)
 * - Device identity and metadata
 * - Capability flags (input, output, analog, digital, etc.)
 * - State management (enabled/disabled)
 * - JSON serialization and configuration
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef I_DEVICE_H
#define I_DEVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>

namespace TwiST {

    // Device capabilities flags (bitmask)
    enum DeviceCapability {
        CAP_INPUT         = 0x01,  // Can provide input values
        CAP_OUTPUT        = 0x02,  // Can receive output commands
        CAP_ANALOG        = 0x04,  // Analog values (0.0-1.0)
        CAP_DIGITAL       = 0x08,  // Digital values (on/off)
        CAP_POSITION      = 0x10,  // Positional control (angles, steps)
        CAP_VELOCITY      = 0x20,  // Velocity/speed control
        CAP_CALIBRATABLE  = 0x40,  // Supports calibration
        CAP_CONFIGURABLE  = 0x80   // Has runtime configuration
    };

    // Device states
    enum DeviceState {
        STATE_UNINITIALIZED,
        STATE_INITIALIZING,
        STATE_READY,
        STATE_ACTIVE,
        STATE_ERROR,
        STATE_DISABLED
    };

    // Device metadata
    struct DeviceInfo {
        const char* type;        // "Servo", "Joystick", "Stepper", etc.
        const char* name;        // User-friendly name
        uint16_t id;            // Unique device ID
        uint16_t capabilities;  // Bitfield of DeviceCapability
        uint8_t channelCount;   // Number of channels/axes
    };

    /**
     * @brief Base interface for all devices in the framework
     *
     * This is the fundamental abstraction that allows the framework to work with
     * any type of hardware device (servos, steppers, sensors, etc.) in a uniform way.
     *
     * All device implementations must inherit from this interface.
     */
    class IDevice {
    public:
        virtual ~IDevice() {}

        // ===== Lifecycle Management =====

        /**
         * @brief Initialize the device hardware
         * @return true if initialization successful, false otherwise
         */
        virtual bool initialize() = 0;

        /**
         * @brief Shutdown the device and release resources
         */
        virtual void shutdown() = 0;

        /**
         * @brief Update device state (called in main loop)
         *
         * This method handles animations, state machines, and other
         * time-dependent operations. Must be called regularly.
         */
        virtual void update() = 0;

        // ===== Identity & Capabilities =====

        /**
         * @brief Get device information
         * @return DeviceInfo structure with type, name, ID, capabilities, channels
         */
        virtual DeviceInfo getInfo() const = 0;

        /**
         * @brief Get device capabilities bitmask
         * @return Bitfield of DeviceCapability flags
         */
        virtual uint16_t getCapabilities() const = 0;

        /**
         * @brief Check if device has specific capability
         * @param cap Capability to check
         * @return true if device has the capability
         */
        virtual bool hasCapability(DeviceCapability cap) const = 0;

        // ===== State Management =====

        /**
         * @brief Get current device state
         * @return Current DeviceState
         */
        virtual DeviceState getState() const = 0;

        /**
         * @brief Enable the device
         */
        virtual void enable() = 0;

        /**
         * @brief Disable the device
         */
        virtual void disable() = 0;

        /**
         * @brief Check if device is enabled
         * @return true if device is enabled
         */
        virtual bool isEnabled() const = 0;

        // ===== Configuration =====

        /**
         * @brief Configure device from JSON
         * @param config JSON configuration object
         * @return true if configuration successful
         */
        virtual bool configure(const JsonDocument& config) = 0;

        /**
         * @brief Get current configuration as JSON
         * @param config JSON object to populate with configuration
         */
        virtual void getConfiguration(JsonDocument& config) const = 0;

        // ===== Serialization =====

        /**
         * @brief Serialize device state to JSON (for API/storage)
         * @param doc JSON document to populate
         */
        virtual void toJson(JsonDocument& doc) const = 0;

        /**
         * @brief Deserialize device state from JSON
         * @param doc JSON document to read from
         * @return true if deserialization successful
         */
        virtual bool fromJson(const JsonDocument& doc) = 0;
    };

}  // namespace TwiST

#endif // I_DEVICE_H