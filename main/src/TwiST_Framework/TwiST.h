#ifndef TWIST_H
#define TWIST_H

#include "TwiST_Config.h"

// Interfaces (abstractions)
#include "Interfaces/IDevice.h"
#include "Interfaces/IInputDevice.h"
#include "Interfaces/IOutputDevice.h"
#include "Interfaces/IBridge.h"
#include "Interfaces/IPWMDriver.h"
#include "Interfaces/IADCDriver.h"
#include "Interfaces/IDistanceDriver.h"

// Core framework
#include "Core/DeviceRegistry.h"
#include "Core/EventBus.h"
#include "Core/ConfigManager.h"
#include "Core/Logger.h"

// Devices (hardware-independent)
#include "Devices/Servo.h"
#include "Devices/Joystick.h"
#include "Devices/DistanceSensor.h"

// Main sketch includes drivers, NOT framework core!

using namespace TwiST;  // Export TwiST namespace to users

/**
 * @brief Main framework facade - entry point for all framework functionality
 *
 * RobotFramework provides a unified interface to all framework components:
 * - Device registry for managing devices
 * - Event bus for pub/sub messaging
 * - Configuration manager for persistent settings
 * - Bridge management for input-output mappings
 *
 * This is the "Laravel for Robotics" - everything starts here!
 *
 * Example usage:
 * ```cpp
 * RobotFramework framework;
 * framework.initialize();
 *
 * // Register devices
 * ServoDevice servo(100, 0);
 * framework.registry()->registerDevice(&servo);
 *
 * // Subscribe to events
 * framework.eventBus()->subscribe("servo.move.complete", onServoComplete);
 *
 * // Load configuration
 * framework.loadConfig("/config/devices.json");
 *
 * // Main loop
 * void loop() {
 *   framework.update();
 * }
 * ```
 */
/**
 * @brief TwiST - Twin System Technology Framework
 * Main framework class for ESP32 robotics applications
 *
 * NOTE: Named TwiSTFramework to avoid conflict with namespace TwiST
 */
class TwiSTFramework {
public:
    TwiSTFramework();
    ~TwiSTFramework();

    // ===== Initialization =====

    /**
     * @brief Initialize the framework
     * @param autoLoadConfig If true, automatically load config from default source
     * @return true if initialization successful
     *
     * Initializes all framework components:
     * - DeviceRegistry
     * - EventBus
     * - ConfigManager
     */
    bool initialize(bool autoLoadConfig = false);

    /**
     * @brief Shutdown the framework
     *
     * Shuts down all devices and framework components.
     */
    void shutdown();

    // ===== Main Update Loop =====

    /**
     * @brief Update framework (call in main loop)
     *
     * Updates:
     * - All registered devices (animations, state machines)
     * - Event queue processing
     * - Active bridges
     */
    void update();

    // ===== Component Access =====

    /**
     * @brief Get device registry
     * @return Pointer to DeviceRegistry
     */
    DeviceRegistry* registry() { return &_registry; }

    /**
     * @brief Get event bus
     * @return Reference to EventBus
     */
    EventBus& eventBus() { return _eventBus; }

    /**
     * @brief Get configuration manager
     * @return Pointer to ConfigManager
     */
    ConfigManager* config() { return &_configManager; }

    // ===== Configuration =====

    /**
     * @brief Load configuration from file
     * @param filename Configuration file path (e.g., "/config/devices.json")
     * @return true if load successful
     */
    bool loadConfig(const char* filename);

    /**
     * @brief Save configuration to file
     * @param filename Configuration file path
     * @return true if save successful
     */
    bool saveConfig(const char* filename);

    /**
     * @brief Load configuration from specific source
     * @param source Configuration source (LITTLEFS, EEPROM, etc.)
     * @return true if load successful
     */
    bool loadConfigFrom(ConfigSource source);

    /**
     * @brief Save configuration to specific source
     * @param source Configuration source
     * @return true if save successful
     */
    bool saveConfigTo(ConfigSource source);

    // ===== Bridge Management =====

    /**
     * @brief Add a bridge to the framework
     * @param bridge Pointer to bridge (must remain valid!)
     * @return true if bridge added successfully
     */
    bool addBridge(IBridge* bridge);

    /**
     * @brief Remove a bridge
     * @param bridge Pointer to bridge to remove
     * @return true if bridge removed
     */
    bool removeBridge(IBridge* bridge);

    /**
     * @brief Get number of active bridges
     * @return Bridge count
     */
    uint8_t getBridgeCount() const { return _bridgeCount; }

    // ===== Statistics & Diagnostics =====

    /**
     * @brief Print framework status to Serial
     */
    void printStatus();

    /**
     * @brief Get framework uptime
     * @return Uptime in milliseconds
     */
    unsigned long getUptime() const;

    /**
     * @brief Get update count (number of update() calls)
     * @return Update count
     */
    unsigned long getUpdateCount() const { return _updateCount; }

private:
    DeviceRegistry _registry;
    EventBus _eventBus;
    ConfigManager _configManager;

    // Bridge management
    IBridge* _bridges[MAX_BRIDGES];
    uint8_t _bridgeCount;

    // Framework state
    bool _initialized;
    unsigned long _startTime;
    unsigned long _updateCount;

    // Private helpers
    bool initializeDevicesFromConfig();
    bool initializeBridgesFromConfig();
};

#endif // TWIST_H