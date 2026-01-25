/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IBridge.h
 * @brief     Bridge interface for connecting input devices to output devices
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   None (IS the bridge interface)
 *
 * PRINCIPLES:
 * - Maps input device channels to output device channels
 * - Supports transformations (scaling, inversion, deadzone, custom)
 * - One bridge manages multiple mappings
 * - Hardware-independent abstraction
 * - JSON-based configuration
 *
 * CAPABILITIES:
 * - Add/remove input-output mappings
 * - Apply transformations (direct, scaled, inverted, custom, deadband)
 * - Update interval control
 * - Enable/disable bridge processing
 * - JSON configuration and serialization
 * - Mapping statistics
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef I_BRIDGE_H
#define I_BRIDGE_H

#include "IInputDevice.h"
#include "IOutputDevice.h"
#include <ArduinoJson.h>

namespace TwiST {

    // Mapping types
    enum MappingType {
        MAP_DIRECT,        // Direct 1:1 mapping
        MAP_SCALED,        // Scaled with ratio
        MAP_INVERTED,      // Inverted direction
        MAP_CUSTOM,        // Custom transform function
        MAP_DEADBAND       // With deadzone filter
    };

    // Transform function pointer type
    typedef float (*TransformFunction)(float input);

    // Bridge mapping configuration
    struct BridgeMapping {
        uint16_t inputDeviceId;
        uint8_t inputChannel;
        uint16_t outputDeviceId;
        uint8_t outputChannel;
        MappingType type;
        float scale;           // Scaling factor (for MAP_SCALED)
        float offset;          // Offset value
        float deadzone;        // Deadzone radius (for MAP_DEADBAND)
        bool inverted;         // Invert direction (for MAP_INVERTED)
        unsigned long updateInterval;  // Min time between updates (ms)
        TransformFunction customTransform; // Function pointer for MAP_CUSTOM
    };

    /**
     * @brief Interface for bridges that connect input devices to output devices
     *
     * Bridges map input device channels to output device channels with optional
     * transformations (scaling, inversion, deadzone filtering, custom functions).
     *
     * Examples:
     * - Joystick X axis → Servo channel 0
     * - LoRa receiver channel 1 → Stepper motor
     * - Ultrasonic distance sensor → Servo (obstacle avoidance)
     *
     * A single bridge can manage multiple mappings and automatically update
     * outputs based on input changes.
     */
    class IBridge {
    public:
        virtual ~IBridge() {}

        // ===== Lifecycle =====

        /**
         * @brief Initialize the bridge
         * @return true if initialization successful
         */
        virtual bool initialize() = 0;

        /**
         * @brief Shutdown the bridge and clear all mappings
         */
        virtual void shutdown() = 0;

        /**
         * @brief Update all mappings (read inputs, apply transforms, write outputs)
         *
         * This must be called regularly in the main loop. It processes all active
         * mappings and updates output devices based on input device values.
         */
        virtual void update() = 0;

        // ===== Mapping Management =====

        /**
         * @brief Add a new input→output mapping
         * @param mapping BridgeMapping configuration
         * @return true if mapping added successfully
         */
        virtual bool addMapping(const BridgeMapping& mapping) = 0;

        /**
         * @brief Remove mapping by input device ID and channel
         * @param inputDeviceId Input device ID
         * @param inputChannel Input channel index
         * @return true if mapping removed
         */
        virtual bool removeMapping(uint16_t inputDeviceId, uint8_t inputChannel) = 0;

        /**
         * @brief Remove all mappings
         * @return true if all mappings cleared
         */
        virtual bool removeAllMappings() = 0;

        /**
         * @brief Get number of active mappings
         * @return Mapping count
         */
        virtual uint8_t getMappingCount() const = 0;

        /**
         * @brief Get mapping by index
         * @param index Mapping index (0-based)
         * @param mapping Output parameter to store mapping
         * @return true if mapping exists at index
         */
        virtual bool getMapping(uint8_t index, BridgeMapping& mapping) const = 0;

        // ===== Enable/Disable =====

        /**
         * @brief Enable the bridge (start processing mappings)
         */
        virtual void enable() = 0;

        /**
         * @brief Disable the bridge (stop processing mappings)
         */
        virtual void disable() = 0;

        /**
         * @brief Check if bridge is enabled
         * @return true if bridge is enabled
         */
        virtual bool isEnabled() const = 0;

        // ===== Configuration =====

        /**
         * @brief Configure bridge from JSON
         * @param config JSON configuration object
         * @return true if configuration successful
         *
         * Example JSON:
         * {
         *   "mappings": [
         *     {
         *       "input": { "deviceId": 200, "channel": 0 },
         *       "output": { "deviceId": 100, "channel": 0 },
         *       "transform": {
         *         "type": "scaled",
         *         "scale": 1.0,
         *         "inverted": false,
         *         "deadzone": 0.05
         *       },
         *       "updateInterval": 20
         *     }
         *   ]
         * }
         */
        virtual bool configure(const JsonDocument& config) = 0;

        /**
         * @brief Get current configuration as JSON
         * @param config JSON object to populate
         */
        virtual void getConfiguration(JsonDocument& config) const = 0;

        // ===== Statistics =====

        /**
         * @brief Get total number of updates performed
         * @return Update count since bridge started
         */
        virtual unsigned long getUpdateCount() const = 0;

        /**
         * @brief Get timestamp of last update
         * @return Millis timestamp of last update() call
         */
        virtual unsigned long getLastUpdateTime() const = 0;
    };

}  // namespace TwiST

#endif // I_BRIDGE_H