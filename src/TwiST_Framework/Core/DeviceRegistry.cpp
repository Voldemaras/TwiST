#include "DeviceRegistry.h"
#include "Logger.h"  // For centralized logging (v1.2.0)
#include <string.h>

using TwiST::Logger;  // Use Logger from TwiST namespace

DeviceRegistry::DeviceRegistry() : _deviceCount(0) {
    // Initialize device array to NULL
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        _devices[i] = NULL;
    }
}

DeviceRegistry::~DeviceRegistry() {
    // Note: We don't delete devices - ownership remains with caller
    unregisterAll();
}

// ===== Registration =====

bool DeviceRegistry::registerDevice(IDevice* device) {
    if (device == NULL) {
        Logger::error("REGISTRY", "Cannot register NULL device");
        return false;
    }

    if (_deviceCount >= MAX_DEVICES) {
        Logger::error("REGISTRY", "Registry full, cannot register more devices");
        return false;
    }

    // Check for duplicate ID
    DeviceInfo info = device->getInfo();
    if (findDevice(info.id) != NULL) {
        Logger::logf(Logger::Level::ERROR, "REGISTRY", "Device ID %d already registered", info.id);
        return false;
    }

    // Add device to registry
    _devices[_deviceCount] = device;
    _deviceCount++;

    Logger::logf(Logger::Level::INFO, "REGISTRY", "Registered device: %s (ID: %d, Type: %s)",
                info.name, info.id, info.type);

    return true;
}

bool DeviceRegistry::unregisterDevice(uint16_t deviceId) {
    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i] && _devices[i]->getInfo().id == deviceId) {
            Logger::logf(Logger::Level::INFO, "REGISTRY", "Unregistering device ID: %d", deviceId);

            // Shift remaining devices down
            for (uint8_t j = i; j < _deviceCount - 1; j++) {
                _devices[j] = _devices[j + 1];
            }
            _devices[_deviceCount - 1] = NULL;
            _deviceCount--;
            return true;
        }
    }
    return false;
}

void DeviceRegistry::unregisterAll() {
    Logger::info("REGISTRY", "Unregistering all devices");
    for (uint8_t i = 0; i < MAX_DEVICES; i++) {
        _devices[i] = NULL;
    }
    _deviceCount = 0;
}

// ===== Discovery =====

IDevice* DeviceRegistry::findDevice(uint16_t deviceId) {
    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i] && _devices[i]->getInfo().id == deviceId) {
            return _devices[i];
        }
    }
    return NULL;
}

IDevice* DeviceRegistry::findDeviceByName(const char* name) {
    if (name == NULL) {
        return NULL;
    }

    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i]) {
            DeviceInfo info = _devices[i]->getInfo();
            if (strcmp(info.name, name) == 0) {
                return _devices[i];
            }
        }
    }
    return NULL;
}

uint8_t DeviceRegistry::findDevices(DeviceFilter filter, IDevice** results, uint8_t maxResults) {
    uint8_t count = 0;

    for (uint8_t i = 0; i < _deviceCount && count < maxResults; i++) {
        if (_devices[i] && matchesFilter(_devices[i], filter)) {
            results[count] = _devices[i];
            count++;
        }
    }

    return count;
}

// ===== Iteration =====

void DeviceRegistry::forEach(DeviceCallback callback) {
    if (callback == NULL) {
        return;
    }

    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i]) {
            callback(_devices[i]);
        }
    }
}

void DeviceRegistry::forEach(DeviceFilter filter, DeviceCallback callback) {
    if (callback == NULL) {
        return;
    }

    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i] && matchesFilter(_devices[i], filter)) {
            callback(_devices[i]);
        }
    }
}

// ===== Capability Queries =====

uint8_t DeviceRegistry::getDeviceCount() const {
    return _deviceCount;
}

uint8_t DeviceRegistry::getInputDeviceCount() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i] && _devices[i]->hasCapability(CAP_INPUT)) {
            count++;
        }
    }
    return count;
}

uint8_t DeviceRegistry::getOutputDeviceCount() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i] && _devices[i]->hasCapability(CAP_OUTPUT)) {
            count++;
        }
    }
    return count;
}

// ===== Type-Safe Access =====

IInputDevice* DeviceRegistry::getInputDevice(uint16_t deviceId) {
    IDevice* device = findDevice(deviceId);
    if (device && device->hasCapability(CAP_INPUT)) {
        return static_cast<IInputDevice*>(device);
    }
    return NULL;
}

IOutputDevice* DeviceRegistry::getOutputDevice(uint16_t deviceId) {
    IDevice* device = findDevice(deviceId);
    if (device && device->hasCapability(CAP_OUTPUT)) {
        return static_cast<IOutputDevice*>(device);
    }
    return NULL;
}

// ===== Bulk Operations =====

bool DeviceRegistry::initializeAll() {
    Logger::info("REGISTRY", "Initializing all devices...");
    bool allSuccess = true;

    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i]) {
            DeviceInfo info = _devices[i]->getInfo();

            if (_devices[i]->initialize()) {
                Logger::logf(Logger::Level::INFO, "REGISTRY", "Initializing %s... OK", info.name);
            } else {
                Logger::logf(Logger::Level::ERROR, "REGISTRY", "Initializing %s... FAILED", info.name);
                allSuccess = false;
            }
        }
    }

    return allSuccess;
}

void DeviceRegistry::updateAll() {
    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i] && _devices[i]->isEnabled()) {
            _devices[i]->update();
        }
    }
}

void DeviceRegistry::shutdownAll() {
    Logger::info("REGISTRY", "Shutting down all devices...");
    for (uint8_t i = 0; i < _deviceCount; i++) {
        if (_devices[i]) {
            _devices[i]->shutdown();
        }
    }
}

// ===== Private Helpers =====

bool DeviceRegistry::matchesFilter(IDevice* device, const DeviceFilter& filter) {
    if (device == NULL) {
        return false;
    }

    DeviceInfo info = device->getInfo();

    // Check type filter
    if (filter.type != NULL && strcmp(info.type, filter.type) != 0) {
        return false;
    }

    // Check capabilities filter
    if (filter.capabilities != 0 && (info.capabilities & filter.capabilities) == 0) {
        return false;
    }

    // Check state filter
    if (filter.state != STATE_UNINITIALIZED && device->getState() != filter.state) {
        return false;
    }

    return true;
}