/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IPWMDriver.h
 * @brief     PWM driver abstraction for hardware-independent PWM control
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   None (IS the driver interface)
 *
 * PRINCIPLES:
 * - Pure C++ interface (NO Arduino.h dependency)
 * - All PWM hardware must implement this interface
 * - Devices use this abstraction, never concrete drivers
 * - Supports variable PWM resolution
 * - Optional frequency control
 *
 * CAPABILITIES:
 * - Set PWM value per channel
 * - Query maximum PWM value (resolution)
 * - Optional frequency control (if supported)
 * - Hardware-independent abstraction
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef TWIST_IPWMDRIVER_H
#define TWIST_IPWMDRIVER_H

#include <stdint.h>  // NO Arduino.h - Pure C++ contract!

namespace TwiST {

    /**
     * @brief PWM Driver abstraction interface
     *
     * CRITICAL: Pure C++ interface - NO Arduino dependencies.
     * ANY PWM chip must implement this interface.
     * Devices NEVER know about specific hardware (PCA9685, TLC5940, etc.)
     * They ONLY know this abstraction.
     */
    class IPWMDriver {
    public:
        virtual ~IPWMDriver() = default;

        /**
         * @brief Set PWM value for channel
         * @param channel Channel number
         * @param value PWM value (0 to getMaxPWM())
         */
        virtual void setPWM(uint8_t channel, uint16_t value) = 0;

        /**
         * @brief Get maximum PWM value
         * @return Max PWM (e.g., 4095 for 12-bit)
         */
        virtual uint16_t getMaxPWM() const = 0;

        /**
         * @brief Check if driver supports frequency control
         * @return true if setFrequency() is functional
         */
        virtual bool supportsFrequency() const { return false; }

        /**
         * @brief Set PWM frequency (only if supportsFrequency() returns true)
         * @param freq Frequency in Hz
         */
        virtual void setFrequency(float freq) {}
    };

}  // namespace TwiST

#endif