/* ============================================================================
 * TwiST Framework | Native Driver
 * ============================================================================
 * @file      ESP32ADC.h
 * @brief     ESP32 analog input driver implementing IADCDriver abstraction.
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Drivers
 * - Type:         Concrete Hardware Driver
 * - Hardware:     ESP32 ADC
 * - Dependency:   None (direct MCU access)
 *
 * PRINCIPLES:
 * - Hardware-specific implementation only
 * - Devices layer NEVER includes this file
 * - Constructor locks pin assignment
 * - Resolution configurable at runtime
 * - Provides raw ADC reading
 *
 * CAPABILITIES:
 * - Raw analog read
 * - Resolution adjustment
 *
 * AUTHOR:    Voldemaras Birskys
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#include "ESP32ADC.h"

namespace TwiST {
    namespace Drivers {

        ESP32ADC::ESP32ADC(uint8_t pin) : _pin(pin), _resolution(12), _maxValue(4095) {
            pinMode(_pin, INPUT);
        }

        bool ESP32ADC::begin() {
            analogReadResolution(_resolution);
            return true;
        }

        void ESP32ADC::setResolution(uint8_t bits) {
            _resolution = bits;
            _maxValue = (1 << bits) - 1;  // 2^bits - 1
            analogReadResolution(_resolution);
        }

        uint16_t ESP32ADC::readRaw() {
            return analogRead(_pin);
        }
    }
}  // namespace TwiST::Drivers