/* ============================================================================
 * TwiST Framework | Hardware Driver
 * ============================================================================
 * @file      PCA9685.h
 * @brief     PCA9685 16-channel PWM driver implementing IPWMDriver abstraction
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Drivers
 * - Type:         Concrete Hardware Driver
 * - Hardware:     PCA9685 (NXP, I2C, 12-bit PWM)
 * - Implements:   IPWMDriver
 *
 * PRINCIPLES:
 * - Hardware-specific implementation only
 * - Devices layer NEVER includes this file directly
 * - Uses IPWMDriver abstraction for device communication
 * - Constructor locks I2C address
 * - Supports frequency control (50Hz for servos)
 *
 * CAPABILITIES:
 * - 16 independent PWM channels
 * - 12-bit resolution (0-4095 steps)
 * - Adjustable PWM frequency (24Hz-1526Hz)
 * - I2C communication (400kHz)
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef TWIST_DRIVER_PCA9685_H
#define TWIST_DRIVER_PCA9685_H

#include "../../Interfaces/IPWMDriver.h"
#include <Adafruit_PWMServoDriver.h>

namespace TwiST {
    namespace Drivers {

        /**
         * @brief PCA9685 16-channel PWM driver - implements IPWMDriver
         *
         * Hardware-specific implementation for PCA9685 chip.
         * Devices NEVER include this directly - they use IPWMDriver.
         */
        class PCA9685 : public IPWMDriver {  // Implements abstraction
        public:
            PCA9685(uint8_t i2cAddress = 0x40);

            bool begin(uint8_t sda = 22, uint8_t scl = 23);

            // IPWMDriver interface implementation
            void setPWM(uint8_t channel, uint16_t value) override;
            uint16_t getMaxPWM() const override { return 4095; }
            bool supportsFrequency() const override { return true; }  // PCA9685 supports frequency control
            void setFrequency(float freq) override;

        private:
            Adafruit_PWMServoDriver _pwm;
            uint8_t _address;
        };

    }
}  // namespace TwiST::Drivers

#endif