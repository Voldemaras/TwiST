/* ============================================================================
 * TwiST Framework | Hardware Driver
 * ============================================================================
 * @file      HCSR04.h
 * @brief     HC-SR04 ultrasonic distance sensor driver implementing IDistanceDriver
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Drivers
 * - Type:         Concrete Hardware Driver
 * - Hardware:     HC-SR04 (Ultrasonic, TRIG/ECHO, 5V)
 * - Implements:   IDistanceDriver
 *
 * PRINCIPLES:
 * - Hardware-specific implementation only
 * - Devices layer NEVER includes this file directly
 * - Uses IDistanceDriver abstraction for device communication
 * - Constructor locks TRIG and ECHO pins
 * - Non-blocking measurement (trigger → wait → read)
 *
 * CAPABILITIES:
 * - Distance measurement: 2cm - 400cm
 * - Resolution: 0.3cm
 * - Measurement angle: 15° cone
 * - Trigger pulse: 10μs
 * - Echo timeout: 30ms (400cm max)
 *
 * WIRING WARNING:
 * - ECHO pin outputs 5V - use voltage divider for ESP32 (3.3V logic)!
 * - Recommended: 1kΩ + 2kΩ resistors (5V → 3.3V)
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0.1
 * ============================================================================
 */

#ifndef TWIST_DRIVER_HCSR04_H
#define TWIST_DRIVER_HCSR04_H

#include "../../Interfaces/IDistanceDriver.h"
#include <Arduino.h>

namespace TwiST {
    namespace Drivers {

    /**
     * @brief HC-SR04 ultrasonic distance sensor driver
     *
     * CRITICAL: ECHO pin outputs 5V - use voltage divider for ESP32!
     * Implements IDistanceDriver interface for hardware independence.
     */
    class HCSR04 : public IDistanceDriver {
        public:
            /**
             * @brief Construct HC-SR04 driver
             * @param trigPin GPIO pin for TRIG signal (locked at construction)
             * @param echoPin GPIO pin for ECHO signal (locked at construction)
             */
            HCSR04(uint8_t trigPin, uint8_t echoPin);

            /**
             * @brief Initialize hardware (pinMode setup)
             * @return true if initialization successful
             */
            bool begin();

            // IDistanceDriver interface implementation
            void triggerMeasurement() override;
            float readDistanceCm() override;
            bool isMeasurementReady() const override;
            float getMaxRange() const override { return 400.0f; }

        private:
            uint8_t _trigPin;
            uint8_t _echoPin;
            bool _measurementReady;
            float _lastDistance;

            // Constants
            static constexpr unsigned long TRIGGER_PULSE_US = 10;    // 10μs trigger pulse
            static constexpr unsigned long TIMEOUT_US = 30000;       // 30ms timeout (400cm max)
            static constexpr float SOUND_SPEED_CM_US = 0.034f;       // 340 m/s = 0.034 cm/μs
        };
    }
}  // namespace TwiST::Drivers
#endif