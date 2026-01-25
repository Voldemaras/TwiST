/* ============================================================================
 * TwiST Framework | Hardware Driver
 * ============================================================================
 * @file      ESP32ADC.h
 * @brief     ESP32-C6 ADC driver implementing IADCDriver abstraction
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Drivers
 * - Type:         Concrete Hardware Driver
 * - Hardware:     ESP32-C6 ADC (12-bit)
 * - Implements:   IADCDriver
 *
 * PRINCIPLES:
 * - Hardware-specific implementation only
 * - Devices layer NEVER includes this file directly
 * - Uses IADCDriver abstraction for device communication
 * - Constructor locks GPIO pin
 * - Supports configurable resolution
 *
 * CAPABILITIES:
 * - 12-bit resolution (0-4095 values)
 * - Configurable ADC resolution
 * - Direct GPIO pin reading
 * - Normalized value conversion (0.0-1.0)
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef TWIST_DRIVER_ESP32ADC_H
#define TWIST_DRIVER_ESP32ADC_H

#include "../../Interfaces/IADCDriver.h"  // Implements interface
#include <Arduino.h>

namespace TwiST {
    namespace Drivers {

        /**
         * @brief ESP32 built-in ADC driver - implements IADCDriver
         *
         * Hardware-specific implementation for ESP32-C6 ADC.
         * Devices NEVER include this directly - they use IADCDriver.
         */
        class ESP32ADC : public IADCDriver {  // Implements abstraction
        public:
            ESP32ADC(uint8_t pin);

            bool begin();
            void setResolution(uint8_t bits = 12);

            // IADCDriver interface implementation
            uint16_t readRaw() override;  // Renamed from read()
            uint16_t getMaxValue() const override { return _maxValue; }
            // readNormalized() uses default implementation from interface

        private:
            uint8_t _pin;
            uint8_t _resolution;
            uint16_t _maxValue;
        };

    }
}  // namespace TwiST::Drivers

#endif