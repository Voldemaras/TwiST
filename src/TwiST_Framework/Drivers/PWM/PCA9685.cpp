#include "PCA9685.h"
#include <Wire.h>

namespace TwiST {
    namespace Drivers {

        PCA9685::PCA9685(uint8_t i2cAddress) : _address(i2cAddress) {}

        bool PCA9685::begin(uint8_t sda, uint8_t scl) {
            Wire.begin(sda, scl);
            _pwm = Adafruit_PWMServoDriver(_address, Wire);
            _pwm.begin();
            return true;
        }

        void PCA9685::setPWM(uint8_t channel, uint16_t value) {
            if (channel < 16) {  // PCA9685 has 16 channels
                _pwm.setPWM(channel, 0, value);
            }
        }

        void PCA9685::setFrequency(float freq) {
            _pwm.setPWMFreq(freq);
        }

    }
}  // namespace TwiST::Drivers