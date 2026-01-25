/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IADCDriver.h
 * @brief     ADC driver abstraction for hardware-independent analog input
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   None (IS the driver interface)
 *
 * PRINCIPLES:
 * - Pure C++ interface (NO Arduino.h dependency)
 * - All ADC hardware must implement this interface
 * - Devices use this abstraction, never concrete drivers
 * - Supports variable ADC resolution
 * - Provides both raw and normalized reading
 *
 * CAPABILITIES:
 * - Read raw ADC values
 * - Query maximum ADC value (resolution)
 * - Read normalized values (0.0-1.0)
 * - Hardware-independent abstraction
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef TWIST_IADCDRIVER_H
#define TWIST_IADCDRIVER_H

#include <stdint.h>  // NO Arduino.h - Pure C++ contract!

namespace TwiST {

    /**
     * @brief ADC Driver abstraction interface
     *
     * CRITICAL: Pure C++ interface - NO Arduino dependencies.
     * ANY ADC (built-in ESP32, external ADS1115, etc.) must implement this.
     * Devices NEVER know about specific ADC hardware.
     */
    class IADCDriver {
    public:
        virtual ~IADCDriver() = default;

        /**
         * @brief Read raw ADC value
         * @return Raw ADC reading (0 to getMaxValue())
         */
        virtual uint16_t readRaw() = 0;

        /**
         * @brief Get maximum ADC value
         * @return Max value (e.g., 4095 for 12-bit)
         */
        virtual uint16_t getMaxValue() const = 0;

        /**
         * @brief Read normalized value (0.0 - 1.0)
         * @return Normalized value
         *
         * Default implementation: raw / maxValue
         * Can be overridden for hardware-specific normalization
         */
        virtual float readNormalized() {
            return (float)readRaw() / (float)getMaxValue();
        }
    };

}  // namespace TwiST

#endif