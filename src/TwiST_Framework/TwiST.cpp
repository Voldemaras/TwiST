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
    Serial.println("");
    Serial.println("========================================");
    Serial.println("   TwiST Framework v1.0");
    Serial.println("========================================");
    Serial.println("");

    // Initialize ConfigManager first
    if (!_configManager.initialize()) {
        Serial.println("[TwiST ERROR] ConfigManager init failed");
        return false;
    }

    // Auto-load configuration if requested
    if (autoLoadConfig) {
        Serial.println("[TwiST] Auto-loading configuration...");
        loadConfigFrom(SOURCE_LITTLEFS);
    }

    _startTime = millis();
    _initialized = true;

    Serial.println("[TwiST] Initialization complete!");
    Serial.println("");

    return true;
}

void TwiSTFramework::shutdown() {
    if (!_initialized) {
        return;
    }

    Serial.println("[TwiST] Shutting down...");

    // Shutdown all bridges
    for (uint8_t i = 0; i < _bridgeCount; i++) {
        if (_bridges[i]) {
            _bridges[i]->shutdown();
        }
    }

    // Shutdown all devices
    _registry.shutdownAll();

    _initialized = false;
    Serial.println("[TwiST] Shutdown complete");
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
    Serial.print("[TwiST] Loading config from: ");
    Serial.println(filename);

    // For Phase 1, delegate to ConfigManager
    // In later phases, this will also create devices from config

    StaticJsonDocument<2048> doc;
    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.println("[TwiST ERROR] Cannot open config file");
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("[TwiST ERROR] JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Merge config
    _configManager.mergeConfig(doc);

    Serial.println("[TwiST] Configuration loaded");
    return true;
}

bool TwiSTFramework::saveConfig(const char* filename) {
    Serial.print("[TwiST] Saving config to: ");
    Serial.println(filename);

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
        Serial.println("[TwiST ERROR] Cannot add NULL bridge");
        return false;
    }

    if (_bridgeCount >= MAX_BRIDGES) {
        Serial.println("[TwiST ERROR] Bridge limit reached");
        return false;
    }

    _bridges[_bridgeCount] = bridge;
    _bridgeCount++;

    Serial.print("[TwiST] Added bridge (total: ");
    Serial.print(_bridgeCount);
    Serial.println(")");

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

            Serial.println("[TwiST] Removed bridge");
            return true;
        }
    }

    return false;
}

// ===== Statistics & Diagnostics =====

void TwiSTFramework::printStatus() {
    Serial.println("");
    Serial.println("========== Framework Status ==========");
    Serial.print("Uptime: ");
    Serial.print(getUptime() / 1000);
    Serial.println(" seconds");

    Serial.print("Updates: ");
    Serial.println(_updateCount);

    Serial.println("");
    Serial.println("--- Device Registry ---");
    Serial.print("Total devices: ");
    Serial.println(_registry.getDeviceCount());
    Serial.print("Input devices: ");
    Serial.println(_registry.getInputDeviceCount());
    Serial.print("Output devices: ");
    Serial.println(_registry.getOutputDeviceCount());

    Serial.println("");
    Serial.println("--- Event Bus ---");
    Serial.print("Active listeners: ");
    Serial.println(_eventBus.getListenerCount());
    Serial.print("Pending events: ");
    Serial.println(_eventBus.getPendingEventCount());
    Serial.print("Total events: ");
    Serial.println(_eventBus.getEventCount());

    Serial.println("");
    Serial.println("--- Bridges ---");
    Serial.print("Active bridges: ");
    Serial.println(_bridgeCount);

    Serial.println("======================================");
    Serial.println("");
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
    Serial.println("[TwiST] Device auto-creation not yet implemented (Phase 4 feature)");
    return true;
}

bool TwiSTFramework::initializeBridgesFromConfig() {
    // Phase 1: Not implemented yet
    // Phase 2: Will create GenericBridge from JSON config
    Serial.println("[TwiST] Bridge auto-creation not yet implemented (Phase 2 feature)");
    return true;
}