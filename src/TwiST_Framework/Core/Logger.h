/**
 * @file Logger.h
 * @brief Centralized logging system with severity levels and structured output
 *
 * TwiST Framework - Twin System Technology Framework
 * Version: 1.2.0 (Centralized Logger System)
 *
 * Features:
 * - Severity levels: DEBUG, INFO, WARNING, ERROR, FATAL
 * - Module-based logging for component identification
 * - Runtime log level filtering
 * - Structured output format: [timestamp] [level] [module] message
 * - Configurable output stream (Serial, SD card, etc.)
 * - Zero overhead when disabled (compile-time optimization)
 *
 * Architecture Benefits:
 * - Single point of control for all logging
 * - Production mode: ERROR/FATAL only (minimal overhead)
 * - Debug mode: DEBUG+ (full visibility)
 * - Future: SD card logging, remote logging via WiFi
 *
 * Author: Voldemaras Birskys
 * Email: voldemaras@gmail.com
 */

#ifndef TWIST_LOGGER_H
#define TWIST_LOGGER_H

#include <Arduino.h>

namespace TwiST {

/**
 * @class Logger
 * @brief Centralized logging system with severity-based filtering
 *
 * Static class - no instantiation required. All methods are static.
 *
 * Usage:
 * ```cpp
 * // Set minimum log level (typically in setup())
 * Logger::setLevel(Logger::Level::INFO);
 *
 * // Log messages
 * Logger::debug("APP", "Debug information");        // Not printed (below INFO)
 * Logger::info("APP", "System initialized");        // Printed
 * Logger::warning("SERVO", "Servo near limit");     // Printed
 * Logger::error("I2C", "I2C communication failed"); // Printed
 * Logger::fatal("CONFIG", "Invalid configuration"); // Printed + halts MCU
 * ```
 *
 * Output format:
 * ```
 * [12345] [INFO] [APP] System initialized
 * [12678] [ERROR] [I2C] I2C communication failed
 * ```
 */
class Logger {
public:
    /**
     * @enum Level
     * @brief Log severity levels (ascending severity)
     */
    enum class Level : uint8_t {
        DEBUG = 0,   ///< Detailed diagnostic information (development only)
        INFO = 1,    ///< General informational messages (production)
        WARNING = 2, ///< Warning conditions (potential issues)
        ERROR = 3,   ///< Error conditions (recoverable failures)
        FATAL = 4    ///< Fatal errors (unrecoverable, system halt)
    };

    /**
     * @brief Initialize logger with output stream and minimum level
     * @param stream Output stream (Serial, SerialUSB, etc.)
     * @param level Minimum severity level to log (default: INFO)
     *
     * Call once in setup():
     * ```cpp
     * Logger::begin(Serial, Logger::Level::INFO);
     * ```
     */
    static void begin(Stream& stream, Level level = Level::INFO);

    /**
     * @brief Set minimum log level (runtime filtering)
     * @param level Minimum severity to log
     *
     * Messages below this level are silently discarded.
     * Use to switch between debug and production modes at runtime.
     */
    static void setLevel(Level level);

    /**
     * @brief Get current log level
     * @return Current minimum log level
     */
    static Level getLevel();

    /**
     * @brief Set output stream for log messages
     * @param stream Output stream (Serial, SD card, etc.)
     */
    static void setOutput(Stream& stream);

    /**
     * @brief Log DEBUG level message
     * @param module Module name (e.g., "APP", "SERVO", "I2C")
     * @param message Log message
     *
     * Use for detailed diagnostic information during development.
     * Typically disabled in production (level >= INFO).
     */
    static void debug(const char* module, const char* message);

    /**
     * @brief Log INFO level message
     * @param module Module name
     * @param message Log message
     *
     * Use for general informational messages (system state changes).
     */
    static void info(const char* module, const char* message);

    /**
     * @brief Log WARNING level message
     * @param module Module name
     * @param message Log message
     *
     * Use for warning conditions that don't prevent operation.
     */
    static void warning(const char* module, const char* message);

    /**
     * @brief Log ERROR level message
     * @param module Module name
     * @param message Log message
     *
     * Use for error conditions (recoverable failures).
     */
    static void error(const char* module, const char* message);

    /**
     * @brief Log FATAL level message and halt MCU
     * @param module Module name
     * @param message Log message
     *
     * Use for unrecoverable errors. Prints message and halts MCU.
     * **DOES NOT RETURN** - calls while(true) delay loop.
     */
    static void fatal(const char* module, const char* message);

    /**
     * @brief Log message with format string (printf-style)
     * @param level Log level
     * @param module Module name
     * @param format Format string (printf syntax)
     * @param ... Variable arguments
     *
     * Example:
     * ```cpp
     * Logger::logf(Logger::Level::INFO, "SERVO", "Angle: %d degrees", angle);
     * ```
     */
    static void logf(Level level, const char* module, const char* format, ...);

private:
    /**
     * @brief Internal logging function
     * @param level Log level
     * @param module Module name
     * @param message Log message
     */
    static void log(Level level, const char* module, const char* message);

    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return String representation (e.g., "INFO", "ERROR")
     */
    static const char* levelToString(Level level);

    // State variables
    static Stream* outputStream;     ///< Output stream (Serial, SD, etc.)
    static Level minLevel;           ///< Minimum log level (filter threshold)
    static bool initialized;         ///< Initialization flag
};

}  // namespace TwiST

#endif  // TWIST_LOGGER_H
