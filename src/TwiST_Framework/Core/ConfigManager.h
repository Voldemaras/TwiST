/* ============================================================================
 * TwiST Framework | Core Framework
 * ============================================================================
 * @file      ConfigManager.h
 * @brief     Unified configuration management for EEPROM, filesystem, and runtime settings
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Core
 * - Type:         Core Service
 * - Hardware:     None (uses ESP32 Preferences and LittleFS)
 * - Implements:   None (core infrastructure)
 *
 * PRINCIPLES:
 * - Single interface for multiple config sources (EEPROM, LittleFS, runtime)
 * - JSON-based configuration format
 * - Separation of device, bridge, and system configs
 * - Runtime config merging without persistence
 * - Validation before save
 *
 * CAPABILITIES:
 * - Load/save from EEPROM (Preferences API)
 * - Load/save from LittleFS (JSON files)
 * - Runtime configuration cache
 * - Per-device configuration management
 * - Bridge mapping configuration
 * - System-wide settings (WiFi, I2C, logging)
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <LittleFS.h>

// Configuration sources
enum ConfigSource {
    SOURCE_EEPROM,    // ESP32 Preferences (EEPROM emulation)
    SOURCE_LITTLEFS,  // File system (JSON files)
    SOURCE_RUNTIME,   // Runtime configuration (RAM only)
    SOURCE_DEFAULT    // Hardcoded defaults
};

/**
 * @brief Configuration manager for unified config handling
 *
 * The ConfigManager provides a single interface for loading and saving
 * configuration from multiple sources (EEPROM, filesystem, runtime).
 *
 * It manages three types of configuration:
 * - Device configurations (per-device settings)
 * - Bridge configurations (input-output mappings)
 * - System configuration (WiFi, I2C, logging, etc.)
 *
 * Example usage:
 * ```cpp
 * ConfigManager configMgr;
 * configMgr.initialize();
 *
 * // Load from filesystem
 * configMgr.load(SOURCE_LITTLEFS);
 *
 * // Get device config
 * StaticJsonDocument<512> deviceConfig;
 * configMgr.getDeviceConfig(100, deviceConfig);
 *
 * // Save to EEPROM
 * configMgr.save(SOURCE_EEPROM);
 * ```
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // ===== Initialization =====

    /**
     * @brief Initialize the config manager
     * @return true if initialization successful
     *
     * Initializes LittleFS and Preferences (EEPROM).
     */
    bool initialize();

    // ===== Load/Save Entire Config =====

    /**
     * @brief Load configuration from source
     * @param source Configuration source (EEPROM, LittleFS, etc.)
     * @return true if load successful
     */
    bool load(ConfigSource source = SOURCE_LITTLEFS);

    /**
     * @brief Save configuration to source
     * @param source Configuration source
     * @return true if save successful
     */
    bool save(ConfigSource source = SOURCE_LITTLEFS);

    // ===== Get Config Sections =====

    /**
     * @brief Get device configuration by ID
     * @param deviceId Device ID
     * @param config JSON document to populate with configuration
     * @return true if device config found
     */
    bool getDeviceConfig(uint16_t deviceId, JsonDocument& config);

    /**
     * @brief Get bridge configuration
     * @param config JSON document to populate with all bridge mappings
     * @return true if bridge config found
     */
    bool getBridgeConfig(JsonDocument& config);

    /**
     * @brief Get system configuration
     * @param config JSON document to populate with system settings
     * @return true if system config found
     */
    bool getSystemConfig(JsonDocument& config);

    // ===== Set Config Sections =====

    /**
     * @brief Set device configuration
     * @param deviceId Device ID
     * @param config JSON configuration to store
     * @return true if save successful
     */
    bool setDeviceConfig(uint16_t deviceId, const JsonDocument& config);

    /**
     * @brief Set bridge configuration
     * @param config JSON configuration with all bridge mappings
     * @return true if save successful
     */
    bool setBridgeConfig(const JsonDocument& config);

    /**
     * @brief Set system configuration
     * @param config JSON system configuration
     * @return true if save successful
     */
    bool setSystemConfig(const JsonDocument& config);

    // ===== Merge Configs (Runtime Override) =====

    /**
     * @brief Merge runtime config on top of existing config
     * @param config JSON configuration to merge
     *
     * Useful for temporary runtime overrides without persisting.
     */
    void mergeConfig(const JsonDocument& config);

    // ===== Reset =====

    /**
     * @brief Reset all configuration to defaults
     */
    void resetToDefaults();

    // ===== Validation =====

    /**
     * @brief Validate configuration structure
     * @param config JSON configuration to validate
     * @return true if configuration is valid
     */
    bool validate(const JsonDocument& config) const;

private:
    Preferences _prefs;
    bool _initialized;

    // In-memory config cache (runtime config)
    StaticJsonDocument<2048> _deviceConfigs;
    StaticJsonDocument<1024> _bridgeConfig;
    StaticJsonDocument<1024> _systemConfig;

    // Helper methods
    bool loadFromLittleFS(const char* filename, JsonDocument& doc);
    bool saveToLittleFS(const char* filename, const JsonDocument& doc);
    bool loadFromEEPROM(const char* namespace_name, JsonDocument& doc);
    bool saveToEEPROM(const char* namespace_name, const JsonDocument& doc);
};

#endif // CONFIG_MANAGER_H