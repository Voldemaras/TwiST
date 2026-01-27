#include "ConfigManager.h"
#include "Logger.h"  // For centralized logging (v1.2.0)

using TwiST::Logger;  // Use Logger from TwiST namespace

ConfigManager::ConfigManager() : _initialized(false) {
}

ConfigManager::~ConfigManager() {
    if (_initialized) {
        _prefs.end();
    }
}

// ===== Initialization =====

bool ConfigManager::initialize() {
    Logger::info("CONFIG", "Initializing...");

    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Logger::error("CONFIG", "LittleFS init failed");
        return false;
    }
    Logger::info("CONFIG", "LittleFS mounted");

    // Initialize Preferences
    if (!_prefs.begin("robot_cfg", false)) {
        Logger::error("CONFIG", "Preferences init failed");
        return false;
    }
    Logger::info("CONFIG", "Preferences ready");

    _initialized = true;
    return true;
}

// ===== Load/Save Entire Config =====

bool ConfigManager::load(ConfigSource source) {
    switch (source) {
        case SOURCE_LITTLEFS:
            Logger::info("CONFIG", "Loading from LittleFS...");
            // Load device configs
            loadFromLittleFS("/config/devices.json", _deviceConfigs);
            // Load bridge config
            loadFromLittleFS("/config/bridges.json", _bridgeConfig);
            // Load system config
            loadFromLittleFS("/config/system.json", _systemConfig);
            return true;

        case SOURCE_EEPROM:
            Logger::info("CONFIG", "Loading from EEPROM...");
            return true;

        case SOURCE_DEFAULT:
            Logger::info("CONFIG", "Loading from defaults...");
            resetToDefaults();
            return true;

        default:
            Logger::error("CONFIG", "Unknown source");
            return false;
    }
}

bool ConfigManager::save(ConfigSource source) {
    switch (source) {
        case SOURCE_LITTLEFS:
            Logger::info("CONFIG", "Saving to LittleFS...");
            saveToLittleFS("/config/devices.json", _deviceConfigs);
            saveToLittleFS("/config/bridges.json", _bridgeConfig);
            saveToLittleFS("/config/system.json", _systemConfig);
            return true;

        case SOURCE_EEPROM:
            Logger::info("CONFIG", "Saving to EEPROM...");
            // For Phase 1, EEPROM save is simple
            return true;

        default:
            Logger::error("CONFIG", "Unknown source");
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

    Logger::logf(Logger::Level::INFO, "CONFIG", "Set device config for ID: %d", deviceId);
    return true;
}

bool ConfigManager::setBridgeConfig(const JsonDocument& config) {
    _bridgeConfig.set(config);
    Logger::info("CONFIG", "Set bridge config");
    return true;
}

bool ConfigManager::setSystemConfig(const JsonDocument& config) {
    _systemConfig.set(config);
    Logger::info("CONFIG", "Set system config");
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

    Logger::info("CONFIG", "Merged runtime config");
}

// ===== Reset =====

void ConfigManager::resetToDefaults() {
    Logger::info("CONFIG", "Resetting to defaults");

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
        Logger::error("CONFIG", "Validation failed: missing 'version'");
        return false;
    }

    return true;
}

// ===== Private Helpers =====

bool ConfigManager::loadFromLittleFS(const char* filename, JsonDocument& doc) {
    if (!LittleFS.exists(filename)) {
        Logger::logf(Logger::Level::INFO, "CONFIG", "File not found: %s", filename);
        return false;
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        Logger::logf(Logger::Level::ERROR, "CONFIG", "Cannot open file: %s", filename);
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Logger::logf(Logger::Level::ERROR, "CONFIG", "JSON parse failed for %s: %s",
                    filename, error.c_str());
        return false;
    }

    Logger::logf(Logger::Level::INFO, "CONFIG", "Loaded %s", filename);
    return true;
}

bool ConfigManager::saveToLittleFS(const char* filename, const JsonDocument& doc) {
    // Create directory if it doesn't exist
    // LittleFS doesn't have mkdir, but it creates dirs automatically

    File file = LittleFS.open(filename, "w");
    if (!file) {
        Logger::logf(Logger::Level::ERROR, "CONFIG", "Cannot write file: %s", filename);
        return false;
    }

    serializeJsonPretty(doc, file);
    file.close();

    Logger::logf(Logger::Level::INFO, "CONFIG", "Saved %s", filename);
    return true;
}

bool ConfigManager::loadFromEEPROM(const char* namespace_name, JsonDocument& doc) {
    // Phase 1: Basic EEPROM load
    // Actual implementation would serialize/deserialize JSON to/from EEPROM
    Logger::logf(Logger::Level::INFO, "CONFIG", "Loading from EEPROM namespace: %s", namespace_name);
    return true;
}

bool ConfigManager::saveToEEPROM(const char* namespace_name, const JsonDocument& doc) {
    // Phase 1: Basic EEPROM save
    Logger::logf(Logger::Level::INFO, "CONFIG", "Saving to EEPROM namespace: %s", namespace_name);
    return true;
}