#include "TwiST.h"

TwiSTFramework::TwiSTFramework()
    : _bridgeCount(0),
      _initialized(false),
      _startTime(0),
      _updateCount(0) {

    // Initialize bridge array
    for (uint8_t i = 0; i < MAX_BRIDGES; i++) {
        _bridges[i] = NULL;
    }
}

TwiSTFramework::~TwiSTFramework() {
    shutdown();
}

// ===== Initialization =====

bool TwiSTFramework::initialize(bool autoLoadConfig) {
    // Initialize Logger first (before any other output)
    Logger::begin(Serial, Logger::Level::INFO);

    Serial.println("");
    Serial.println("========================================");
    Serial.println("   TwiST Framework v1.2.0");
    Serial.println("========================================");
    Serial.println("");

    Logger::info("FRAMEWORK", "Initializing TwiST Framework...");

    // Initialize ConfigManager
    if (!_configManager.initialize()) {
        Logger::error("FRAMEWORK", "ConfigManager initialization failed");
        return false;
    }
    Logger::info("FRAMEWORK", "ConfigManager initialized");

    // Auto-load configuration if requested
    if (autoLoadConfig) {
        Logger::info("FRAMEWORK", "Auto-loading configuration...");
        loadConfigFrom(SOURCE_LITTLEFS);
    }

    _startTime = millis();
    _initialized = true;

    Logger::info("FRAMEWORK", "Initialization complete");
    Serial.println("");

    return true;
}

void TwiSTFramework::shutdown() {
    if (!_initialized) {
        return;
    }

    Logger::info("FRAMEWORK", "Shutting down...");

    // Shutdown all bridges
    for (uint8_t i = 0; i < _bridgeCount; i++) {
        if (_bridges[i]) {
            _bridges[i]->shutdown();
        }
    }

    // Shutdown all devices
    _registry.shutdownAll();

    _initialized = false;
    Logger::info("FRAMEWORK", "Shutdown complete");
}

// ===== Main Update Loop =====

void TwiSTFramework::update() {
    if (!_initialized) {
        return;
    }

    _updateCount++;

    // Process event queue
    _eventBus.processEvents();

    // Update all registered devices
    _registry.updateAll();

    // Update all bridges
    for (uint8_t i = 0; i < _bridgeCount; i++) {
        if (_bridges[i] && _bridges[i]->isEnabled()) {
            _bridges[i]->update();
        }
    }
}

// ===== Configuration =====

bool TwiSTFramework::loadConfig(const char* filename) {
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Loading config from: %s", filename);

    // For Phase 1, delegate to ConfigManager
    // In later phases, this will also create devices from config

    StaticJsonDocument<2048> doc;
    File file = LittleFS.open(filename, "r");
    if (!file) {
        Logger::error("FRAMEWORK", "Cannot open config file");
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Logger::logf(Logger::Level::ERROR, "FRAMEWORK", "JSON parse failed: %s", error.c_str());
        return false;
    }

    // Merge config
    _configManager.mergeConfig(doc);

    Logger::info("FRAMEWORK", "Configuration loaded");
    return true;
}

bool TwiSTFramework::saveConfig(const char* filename) {
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Saving config to: %s", filename);

    // For Phase 1, simple save
    return _configManager.save(SOURCE_LITTLEFS);
}

bool TwiSTFramework::loadConfigFrom(ConfigSource source) {
    return _configManager.load(source);
}

bool TwiSTFramework::saveConfigTo(ConfigSource source) {
    return _configManager.save(source);
}

// ===== Bridge Management =====

bool TwiSTFramework::addBridge(IBridge* bridge) {
    if (bridge == NULL) {
        Logger::error("FRAMEWORK", "Cannot add NULL bridge");
        return false;
    }

    if (_bridgeCount >= MAX_BRIDGES) {
        Logger::error("FRAMEWORK", "Bridge limit reached");
        return false;
    }

    _bridges[_bridgeCount] = bridge;
    _bridgeCount++;

    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Added bridge (total: %d)", _bridgeCount);

    return true;
}

bool TwiSTFramework::removeBridge(IBridge* bridge) {
    if (bridge == NULL) {
        return false;
    }

    for (uint8_t i = 0; i < _bridgeCount; i++) {
        if (_bridges[i] == bridge) {
            // Shift remaining bridges down
            for (uint8_t j = i; j < _bridgeCount - 1; j++) {
                _bridges[j] = _bridges[j + 1];
            }
            _bridges[_bridgeCount - 1] = NULL;
            _bridgeCount--;

            Logger::info("FRAMEWORK", "Removed bridge");
            return true;
        }
    }

    return false;
}

// ===== Statistics & Diagnostics =====

void TwiSTFramework::printStatus() {
    Logger::info("FRAMEWORK", "");
    Logger::info("FRAMEWORK", "========== Framework Status ==========");
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Uptime: %lu seconds", getUptime() / 1000);
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Updates: %lu", _updateCount);

    Logger::info("FRAMEWORK", "");
    Logger::info("FRAMEWORK", "--- Device Registry ---");
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Total devices: %d", _registry.getDeviceCount());
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Input devices: %d", _registry.getInputDeviceCount());
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Output devices: %d", _registry.getOutputDeviceCount());

    Logger::info("FRAMEWORK", "");
    Logger::info("FRAMEWORK", "--- Event Bus ---");
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Active listeners: %lu", _eventBus.getListenerCount());
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Pending events: %d", _eventBus.getPendingEventCount());
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Total events: %lu", _eventBus.getEventCount());

    Logger::info("FRAMEWORK", "");
    Logger::info("FRAMEWORK", "--- Bridges ---");
    Logger::logf(Logger::Level::INFO, "FRAMEWORK", "Active bridges: %d", _bridgeCount);

    Logger::info("FRAMEWORK", "======================================");
    Logger::info("FRAMEWORK", "");
}

unsigned long TwiSTFramework::getUptime() const {
    if (!_initialized) {
        return 0;
    }
    return millis() - _startTime;
}

// ===== Private Helpers =====

bool TwiSTFramework::initializeDevicesFromConfig() {
    // Phase 1: Not implemented yet
    // Phase 4: Will use PluginManager to create devices from JSON
    Logger::info("FRAMEWORK", "Device auto-creation not yet implemented (Phase 4 feature)");
    return true;
}

bool TwiSTFramework::initializeBridgesFromConfig() {
    // Phase 1: Not implemented yet
    // Phase 2: Will create GenericBridge from JSON config
    Logger::info("FRAMEWORK", "Bridge auto-creation not yet implemented (Phase 2 feature)");
    return true;
}