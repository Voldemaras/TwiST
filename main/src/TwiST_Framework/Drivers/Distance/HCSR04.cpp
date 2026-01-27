#include "HCSR04.h"

namespace TwiST {
    namespace Drivers {

        HCSR04::HCSR04(uint8_t trigPin, uint8_t echoPin)
            : _trigPin(trigPin)
            , _echoPin(echoPin)
            , _measurementReady(false)
            , _lastDistance(0.0f)
        {
        }

        bool HCSR04::begin() {
            pinMode(_trigPin, OUTPUT);
            pinMode(_echoPin, INPUT);

            // Ensure TRIG is LOW initially
            digitalWrite(_trigPin, LOW);
            delayMicroseconds(2);

            return true;
        }

        void HCSR04::triggerMeasurement() {
            // Send 10μs pulse on TRIG pin
            digitalWrite(_trigPin, LOW);
            delayMicroseconds(2);
            digitalWrite(_trigPin, HIGH);
            delayMicroseconds(TRIGGER_PULSE_US);
            digitalWrite(_trigPin, LOW);

            _measurementReady = false;
        }

        float HCSR04::readDistanceCm() {
            // Read ECHO pulse duration (timeout after 30ms)
            unsigned long duration = pulseIn(_echoPin, HIGH, TIMEOUT_US);

            // Check for timeout (out of range)
            if (duration == 0) {
                _lastDistance = 0.0f;
                _measurementReady = false;
                return 0.0f;
            }

            // Calculate distance: duration * speed of sound / 2 (round trip)
            // duration [μs] * 0.034 [cm/μs] / 2 = distance [cm]
            _lastDistance = (duration * SOUND_SPEED_CM_US) / 2.0f;
            _measurementReady = true;

            return _lastDistance;
        }

        bool HCSR04::isMeasurementReady() const {
            return _measurementReady;
        }
    }
}  // namespace TwiST::Drivers