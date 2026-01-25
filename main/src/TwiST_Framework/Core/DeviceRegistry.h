/* ============================================================================
 * TwiST Framework | Core Framework
 * ============================================================================
 * @file      DeviceRegistry.h
 * @brief     Central repository for device registration, discovery, and lifecycle management
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Core
 * - Type:         Core Service
 * - Hardware:     None
 * - Implements:   None (core infrastructure)
 *
 * PRINCIPLES:
 * - Single source of truth for all devices
 * - Type-safe device access (IInputDevice, IOutputDevice)
 * - Capability-based device queries
 * - Bulk lifecycle operations (init, update, shutdown)
 * - Zero dependency on concrete device types
 *
 * CAPABILITIES:
 * - Register/unregister devices by reference
 * - Find devices by ID, name, or capabilities
 * - Type-safe casting to input/output devices
 * - Iterate over devices with filters
 * - Bulk initialization and shutdown
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef DEVICE_REGISTRY_H
#define DEVICE_REGISTRY_H

#include "../Interfaces/IDevice.h"
#include "../Interfaces/IInputDevice.h"
#include "../Interfaces/IOutputDevice.h"

using namespace TwiST;  // Use TwiST namespace for interfaces

// Maximum number of devices (configurable, can be increased)
#ifndef MAX_DEVICES
#define MAX_DEVICES 32
#endif

// Device filter for queries
struct DeviceFilter {
    const char* type;          // NULL = any type
    uint16_t capabilities;     // 0 = any capabilities
    DeviceState state;         // STATE_UNINITIALIZED = any state
};

// Device iterator callback
typedef void (*DeviceCallback)(IDevice* device);

/**
 * @brief Device registry for managing all devices in the system
 *
 * This is the central repository for all devices. It provides:
 * - Device registration and lookup
 * - Capability-based queries
 * - Type-safe access to input/output devices
 * - Iteration over devices with filters
 *
 * Example usage:
 * ```cpp
 * DeviceRegistry registry;
 * ServoDevice servo(100, 0);
 * registry.registerDevice(&servo);
 *
 * IDevice* device = registry.findDevice(100);
 * IOutputDevice* output = registry.getOutputDevice(100);
 * ```
 */
class DeviceRegistry {
public:
    DeviceRegistry();
    ~DeviceRegistry();

    // ===== Registration =====

    /**
     * @brief Register a device
     * @param device Pointer to device (must remain valid!)
     * @return true if registration successful, false if ID already exists
     */
    bool registerDevice(IDevice* device);

    /**
     * @brief Unregister a device by ID
     * @param deviceId Device ID to unregister
     * @return true if device was found and unregistered
     */
    bool unregisterDevice(uint16_t deviceId);

    /**
     * @brief Unregister all devices
     */
    void unregisterAll();

    // ===== Discovery =====

    /**
     * @brief Find device by ID
     * @param deviceId Device ID to find
     * @return Pointer to device or NULL if not found
     */
    IDevice* findDevice(uint16_t deviceId);

    /**
     * @brief Find device by name
     * @param name Device name to search for
     * @return Pointer to device or NULL if not found
     */
    IDevice* findDeviceByName(const char* name);

    /**
     * @brief Find devices matching filter criteria
     * @param filter DeviceFilter with search criteria
     * @param results Array to store found devices
     * @param maxResults Maximum number of results
     * @return Number of devices found
     */
    uint8_t findDevices(DeviceFilter filter, IDevice** results, uint8_t maxResults);

    // ===== Iteration =====

    /**
     * @brief Iterate over all devices
     * @param callback Function to call for each device
     */
    void forEach(DeviceCallback callback);

    /**
     * @brief Iterate over devices matching filter
     * @param filter DeviceFilter with search criteria
     * @param callback Function to call for each matching device
     */
    void forEach(DeviceFilter filter, DeviceCallback callback);

    // ===== Capability Queries =====

    /**
     * @brief Get total number of registered devices
     * @return Device count
     */
    uint8_t getDeviceCount() const;

    /**
     * @brief Get number of input devices
     * @return Input device count
     */
    uint8_t getInputDeviceCount() const;

    /**
     * @brief Get number of output devices
     * @return Output device count
     */
    uint8_t getOutputDeviceCount() const;

    // ===== Type-Safe Access =====

    /**
     * @brief Get input device by ID (type-safe cast)
     * @param deviceId Device ID
     * @return Pointer to IInputDevice or NULL if not found or wrong type
     */
    IInputDevice* getInputDevice(uint16_t deviceId);

    /**
     * @brief Get output device by ID (type-safe cast)
     * @param deviceId Device ID
     * @return Pointer to IOutputDevice or NULL if not found or wrong type
     */
    IOutputDevice* getOutputDevice(uint16_t deviceId);

    // ===== Bulk Operations =====

    /**
     * @brief Initialize all registered devices
     * @return true if all devices initialized successfully
     */
    bool initializeAll();

    /**
     * @brief Update all registered devices
     */
    void updateAll();

    /**
     * @brief Shutdown all registered devices
     */
    void shutdownAll();

private:
    IDevice* _devices[MAX_DEVICES];
    uint8_t _deviceCount;

    // Helper to check if filter matches device
    bool matchesFilter(IDevice* device, const DeviceFilter& filter);
};

#endif // DEVICE_REGISTRY_H