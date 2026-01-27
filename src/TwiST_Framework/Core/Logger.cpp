/**
 * @file Logger.cpp
 * @brief Implementation of centralized logging system
 *
 * TwiST Framework - Twin System Technology Framework
 * Version: 1.2.0 (Centralized Logger System)
 *
 * Author: Voldemaras Birskys
 * Email: voldemaras@gmail.com
 */

#include "Logger.h"
#include <stdarg.h>  // For va_list, va_start, va_end

namespace TwiST {

// Static member initialization
Stream* Logger::outputStream = nullptr;
Logger::Level Logger::minLevel = Logger::Level::INFO;
bool Logger::initialized = false;

// ============================================================================
// Public Interface
// ============================================================================

void Logger::begin(Stream& stream, Level level) {
    outputStream = &stream;
    minLevel = level;
    initialized = true;

    // Log initialization message
    log(Level::INFO, "LOGGER", "Logger initialized");
}

void Logger::setLevel(Level level) {
    minLevel = level;
}

Logger::Level Logger::getLevel() {
    return minLevel;
}

void Logger::setOutput(Stream& stream) {
    outputStream = &stream;
}

void Logger::debug(const char* module, const char* message) {
    log(Level::DEBUG, module, message);
}

void Logger::info(const char* module, const char* message) {
    log(Level::INFO, module, message);
}

void Logger::warning(const char* module, const char* message) {
    log(Level::WARNING, module, message);
}

void Logger::error(const char* module, const char* message) {
    log(Level::ERROR, module, message);
}

void Logger::fatal(const char* module, const char* message) {
    log(Level::FATAL, module, message);

    // Fatal error - halt MCU
    if (outputStream) {
        outputStream->println("[LOGGER] System halted due to fatal error");
        outputStream->flush();  // Ensure message is sent before halting
    }

    // Halt MCU - fail-fast
    while (true) {
        delay(1000);
    }
}

void Logger::logf(Level level, const char* module, const char* format, ...) {
    // Check if message should be logged
    if (!initialized || !outputStream || level < minLevel) {
        return;
    }

    // Format message using vsnprintf
    char buffer[256];  // Message buffer (adjust size as needed)
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Log formatted message
    log(level, module, buffer);
}

// ============================================================================
// Internal Implementation
// ============================================================================

void Logger::log(Level level, const char* module, const char* message) {
    // Filter by severity level
    if (!initialized || !outputStream || level < minLevel) {
        return;
    }

    // Structured output format: [timestamp] [level] [module] message
    // Example: [12345] [INFO] [APP] System initialized

    // Timestamp (milliseconds since boot)
    outputStream->print("[");
    outputStream->print(millis());
    outputStream->print("] ");

    // Log level
    outputStream->print("[");
    outputStream->print(levelToString(level));
    outputStream->print("] ");

    // Module name
    outputStream->print("[");
    outputStream->print(module);
    outputStream->print("] ");

    // Message
    outputStream->println(message);
}

const char* Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

}  // namespace TwiST
