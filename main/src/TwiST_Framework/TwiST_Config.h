/*
 * TwiST_Config.h
 * Central configuration for TwiST Framework
 *
 * Twin System Technology - Configuration Settings
 * All TwiST framework constants centralized here
 *
 * Author: TwiST Framework
 * Version: 1.0
 */

#ifndef TWIST_CONFIG_H
#define TWIST_CONFIG_H

// ============================================================================
// Framework Version
// ============================================================================
#define TWIST_VERSION_MAJOR   1
#define TWIST_VERSION_MINOR   0
#define TWIST_VERSION_PATCH   0
#define TWIST_VERSION_STRING  "1.0.0"

// ============================================================================
// Device Registry Configuration
// ============================================================================
#ifndef MAX_DEVICES
#define MAX_DEVICES           32        // Maximum devices in registry
#endif

#ifndef MAX_BRIDGES
#define MAX_BRIDGES           16        // Maximum bridges
#endif

// ============================================================================
// Event Bus Configuration
// ============================================================================
#define MAX_EVENT_LISTENERS   32        // Maximum event subscribers
#define EVENT_QUEUE_SIZE      64        // Event queue size
#define EVENT_PRIORITY_LEVELS 3         // High, Normal, Low

// ============================================================================
// Config Manager Settings
// ============================================================================
#define CONFIG_JSON_SIZE      8192      // JSON document size
#define CONFIG_FILE_PATH      "/config/twist.json"
#define CONFIG_BACKUP_PATH    "/config/twist.backup.json"
#define CONFIG_AUTO_SAVE      true      // Auto-save on changes
#define CONFIG_AUTO_SAVE_DELAY 5000     // Delay before auto-save (ms)

// LittleFS Configuration
#define LITTLEFS_FORMAT_ON_FAIL true    // Format if mount fails
#define LITTLEFS_MAX_FILES      10      // Max open files

// Preferences (EEPROM) Configuration
#define PREFERENCES_NAMESPACE "twist"   // Namespace for Preferences
#define PREFERENCES_READONLY  false     // Allow writes

// ============================================================================
// Bridge Configuration
// ============================================================================
#define BRIDGE_UPDATE_INTERVAL 10       // Bridge update interval (ms)
#define BRIDGE_SMOOTHING       0.0f     // Default input smoothing (0.0-1.0)
#define BRIDGE_DEADZONE        0.05f    // Default deadzone (normalized)

// ============================================================================
// Communication Timeouts
// ============================================================================
#define DEVICE_RESPONSE_TIMEOUT   1000  // Device response timeout (ms)
#define NETWORK_TIMEOUT           5000  // Network operation timeout (ms)
#define HEARTBEAT_INTERVAL        1000  // Device heartbeat (ms)
#define WATCHDOG_TIMEOUT          10000 // Watchdog timeout (ms)

// ============================================================================
// Performance Settings
// ============================================================================
#define TWIST_UPDATE_RATE     100       // Framework update rate (Hz)
#define TWIST_LOOP_DELAY      10        // Main loop delay (ms)
#define ENABLE_PROFILING      false     // Enable performance profiling
#define ENABLE_DEBUG_LOGS     false     // Enable debug logging

// ============================================================================
// Memory Management
// ============================================================================
#define ENABLE_HEAP_MONITORING false    // Monitor heap usage
#define HEAP_WARNING_THRESHOLD 4096     // Warn if free heap < this (bytes)

// ============================================================================
// Hardware Pin Definitions (XIAO ESP32-C6)
// ============================================================================
// Phase 3: Native devices - hardware config integrated directly
#define XIAO_SDA_PIN          22        // GPIO22 (D4) - I2C Data
#define XIAO_SCL_PIN          23        // GPIO23 (D5) - I2C Clock
#define I2C_CLOCK_SPEED       400000    // 400kHz I2C speed

// PCA9685 Configuration
#define PCA9685_ADDRESS       0x40      // Default I2C address
#define PWM_FREQUENCY         50        // 50Hz standard servo frequency
#define PCA9685_PWM_STEPS     4096      // 12-bit resolution

// ADC Configuration (ESP32-C6)
#define ADC_RESOLUTION_BITS   12        // ESP32-C6 ADC is 12-bit
#define ADC_MAX_VALUE         4095      // 2^12 - 1
#define ADC_MIN_VALUE         0
#define ADC_CENTER_VALUE      2048      // Center position

// Servo Configuration
#define MAX_SERVOS            16        // PCA9685 has 16 channels
#define DEFAULT_MIN_PULSE     500       // Default minimum pulse width (μs)
#define DEFAULT_MAX_PULSE     2500      // Default maximum pulse width (μs)
#define DEFAULT_MIN_ANGLE     0         // Default minimum angle (degrees)
#define DEFAULT_MAX_ANGLE     180       // Default maximum angle (degrees)

// Joystick Configuration
#define MAX_JOYSTICKS         4         // Maximum number of joysticks supported
#define DEFAULT_DEADZONE      50        // Default deadzone radius (ADC units)

// Additional TwiST-specific pins
#define STATUS_LED_PIN        -1        // Status LED (-1 = disabled)
#define EMERGENCY_STOP_PIN    -1        // E-stop pin (-1 = disabled)

// ============================================================================
// Feature Flags
// ============================================================================
#define ENABLE_DEVICE_REGISTRY  true    // Enable device registry
#define ENABLE_EVENT_BUS        true    // Enable event system
#define ENABLE_CONFIG_MANAGER   true    // Enable config management
#define ENABLE_REMOTE_DEVICES   false   // Enable remote device support
#define ENABLE_OTA_UPDATES      false   // Enable OTA firmware updates

// ============================================================================
// Safety Settings
// ============================================================================
#define ENABLE_WATCHDOG         true    // Enable watchdog timer
#define ENABLE_SAFE_MODE        true    // Enable safe mode on errors
#define SAFE_MODE_TIMEOUT       5000    // Safe mode timeout (ms)

// ============================================================================
// Logging Configuration
// ============================================================================
#define LOG_LEVEL_NONE        0
#define LOG_LEVEL_ERROR       1
#define LOG_LEVEL_WARN        2
#define LOG_LEVEL_INFO        3
#define LOG_LEVEL_DEBUG       4
#define LOG_LEVEL_VERBOSE     5

#ifndef LOG_LEVEL
#define LOG_LEVEL             LOG_LEVEL_INFO  // Default log level
#endif

// ============================================================================
// Development/Debug Settings
// ============================================================================
#ifdef DEBUG
  #undef ENABLE_DEBUG_LOGS
  #define ENABLE_DEBUG_LOGS   true
  #undef LOG_LEVEL
  #define LOG_LEVEL           LOG_LEVEL_DEBUG
#endif

#endif // TWIST_CONFIG_H