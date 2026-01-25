#include "ConfigManager.h"

ConfigManager::ConfigManager() : _initialized(false) {
}

ConfigManager::~ConfigManager() {
    if (_initialized) {
        _prefs.end();
    }
}

// ===== Initialization =====

bool ConfigManager::initialize() {
    Serial.println("[ConfigManager] Initializing...");

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("[ConfigManager ERROR] LittleFS init failed");
        return false;
    }
    Serial.println("[ConfigManager] LittleFS mounted");

    // Initialize Preferences
    if (!_prefs.begin("robot_cfg", false)) {
        Serial.println("[ConfigManager ERROR] Preferences init failed");
        return false;
    }
    Serial.println("[ConfigManager] Preferences ready");

    _initialized = true;
    return true;
}

// ===== Load/Save Entire Config =====

bool ConfigManager::load(ConfigSource source) {
    Serial.print("[ConfigManager] Loading from ");

    switch (source) {
        case SOURCE_LITTLEFS:
            Serial.println("LittleFS...");
            // Load device configs
            loadFromLittleFS("/config/devices.json", _deviceConfigs);
            // Load bridge config
            loadFromLittleFS("/config/bridges.json", _bridgeConfig);
            // Load system config
            loadFromLittleFS("/config/system.json", _systemConfig);
            return true;

        case SOURCE_EEPROM:
            Serial.println("EEPROM...");
            return true;

        case SOURCE_DEFAULT:
            Serial.println("defaults...");
            resetToDefaults();
            return true;

        default:
            Serial.println("unknown source");
            return false;
    }
}

bool ConfigManager::save(ConfigSource source) {
    Serial.print("[ConfigManager] Saving to ");

    switch (source) {
        case SOURCE_LITTLEFS:
            Serial.println("LittleFS...");
            saveToLittleFS("/config/devices.json", _deviceConfigs);
            saveToLittleFS("/config/bridges.json", _bridgeConfig);
            saveToLittleFS("/config/system.json", _systemConfig);
            return true;

        case SOURCE_EEPROM:
            Serial.println("EEPROM...");
            // For Phase 1, EEPROM save is simple
            return true;

        default:
            Serial.println("unknown source");
            return false;
    }
}

// ===== Get Config Sections =====

bool ConfigManager::getDeviceConfig(uint16_t deviceId, JsonDocument& config) {
    // Check if device config exists in cache
    if (_deviceConfigs.containsKey("devices")) {
        JsonArray devices = _deviceConfigs["devices"].as<JsonArray>();
        for (JsonVariant device : devices) {
            if (device["id"].as<uint16_t>() == deviceId) {
                config.set(device);
                return true;
            }
        }
    }
    return false;
}

bool ConfigManager::getBridgeConfig(JsonDocument& config) {
    config.set(_bridgeConfig);
    return true;
}

bool ConfigManager::getSystemConfig(JsonDocument& config) {
    config.set(_systemConfig);
    return true;
}

// ===== Set Config Sections =====

bool ConfigManager::setDeviceConfig(uint16_t deviceId, const JsonDocument& config) {
    // Find and update or add device config
    bool found = false;

    if (!_deviceConfigs.containsKey("devices")) {
        _deviceConfigs.createNestedArray("devices");
    }

    JsonArray devices = _deviceConfigs["devices"].as<JsonArray>();
    for (JsonVariant device : devices) {
        if (device["id"].as<uint16_t>() == deviceId) {
            device.set(config);
            found = true;
            break;
        }
    }

    if (!found) {
        devices.add(config);
    }

    Serial.print("[ConfigManager] Set device config for ID: ");
    Serial.println(deviceId);
    return true;
}

bool ConfigManager::setBridgeConfig(const JsonDocument& config) {
    _bridgeConfig.set(config);
    Serial.println("[ConfigManager] Set bridge config");
    return true;
}

bool ConfigManager::setSystemConfig(const JsonDocument& config) {
    _systemConfig.set(config);
    Serial.println("[ConfigManager] Set system config");
    return true;
}

// ===== Merge Configs =====

void ConfigManager::mergeConfig(const JsonDocument& config) {
    // Simple merge - for Phase 1, just overwrite sections
    if (config.containsKey("devices")) {
        _deviceConfigs["devices"] = config["devices"];
    }
    if (config.containsKey("bridges")) {
        _bridgeConfig.set(config["bridges"]);
    }
    if (config.containsKey("system")) {
        _systemConfig.set(config["system"]);
    }

    Serial.println("[ConfigManager] Merged runtime config");
}

// ===== Reset =====

void ConfigManager::resetToDefaults() {
    Serial.println("[ConfigManager] Resetting to defaults");

    _deviceConfigs.clear();
    _bridgeConfig.clear();
    _systemConfig.clear();

    // Set minimal default system config
    _systemConfig["version"] = "1.0";
    _systemConfig["nodeId"] = 1;
    _systemConfig["nodeName"] = "ESP32-Robot";
}

// ===== Validation =====

bool ConfigManager::validate(const JsonDocument& config) const {
    // Basic validation for Phase 1
    // TODO: Implement JSON schema validation in later phases

    if (!config.containsKey("version")) {
        Serial.println("[ConfigManager] Validation failed: missing 'version'");
        return false;
    }

    return true;
}

// ===== Private Helpers =====

bool ConfigManager::loadFromLittleFS(const char* filename, JsonDocument& doc) {
    if (!LittleFS.exists(filename)) {
        Serial.print("[ConfigManager] File not found: ");
        Serial.println(filename);
        return false;
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.print("[ConfigManager ERROR] Cannot open file: ");
        Serial.println(filename);
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("[ConfigManager ERROR] JSON parse failed for ");
        Serial.print(filename);
        Serial.print(": ");
        Serial.println(error.c_str());
        return false;
    }

    Serial.print("[ConfigManager] Loaded ");
    Serial.println(filename);
    return true;
}

bool ConfigManager::saveToLittleFS(const char* filename, const JsonDocument& doc) {
    // Create directory if it doesn't exist
    // LittleFS doesn't have mkdir, but it creates dirs automatically

    File file = LittleFS.open(filename, "w");
    if (!file) {
        Serial.print("[ConfigManager ERROR] Cannot write file: ");
        Serial.println(filename);
        return false;
    }

    serializeJsonPretty(doc, file);
    file.close();

    Serial.print("[ConfigManager] Saved ");
    Serial.println(filename);
    return true;
}

bool ConfigManager::loadFromEEPROM(const char* namespace_name, JsonDocument& doc) {
    // Phase 1: Basic EEPROM load
    // Actual implementation would serialize/deserialize JSON to/from EEPROM
    Serial.print("[ConfigManager] Loading from EEPROM namespace: ");
    Serial.println(namespace_name);
    return true;
}

bool ConfigManager::saveToEEPROM(const char* namespace_name, const JsonDocument& doc) {
    // Phase 1: Basic EEPROM save
    Serial.print("[ConfigManager] Saving to EEPROM namespace: ");
    Serial.println(namespace_name);
    return true;
}