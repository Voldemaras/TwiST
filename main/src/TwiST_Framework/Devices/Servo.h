/* ============================================================================
 * TwiST Framework | Native Device
 * ============================================================================
 * @file      Servo.h
 * @brief     Servo motor output device based on PWM abstraction.
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Devices
 * - Type:         Native Output Device
 * - Hardware:     PWM via IPWMDriver abstraction
 * - Dependency:   EventBus, IPWMDriver
 *
 * PRINCIPLES:
 * - No hardware knowledge (NO Drivers includes)
 * - Constructor Injection only
 * - One Servo instance = one physical servo motor
 * - PWM channel is locked at construction time
 * - No runtime channel passing
 * - Clean Architecture compliant
 *
 * CAPABILITIES:
 * - Angle-based control (degrees)
 * - Normalized control (0.0 → 1.0)
 * - Time-based movement (animation)
 * - Calibration (pulse width and angle range)
 * - JSON configuration & serialization
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================*/

#ifndef TWIST_DEVICE_SERVO_H
#define TWIST_DEVICE_SERVO_H

#include "../Interfaces/IOutputDevice.h"
#include "../Interfaces/IPWMDriver.h"  // ONLY ABSTRACTION!
#include "../Core/EventBus.h"

namespace TwiST {
    namespace Devices {

        /**
         * @brief Native servo device - implements IOutputDevice
         *
         * CRITICAL DESIGN: One Servo object = ONE physical servo motor.
         * Channel is known at construction time, NOT passed to methods.
         *
         * Uses IPWMDriver abstraction - NEVER knows about PCA9685 or any specific hardware.
         * Hardware independence achieved through dependency injection.
         */

        class Servo : public IOutputDevice {
        public:
            /**
             * @param pwm Reference to IPWMDriver (NOT PCA9685!)
             * @param channel PWM channel (LOCKED at construction)
             * @param deviceId Unique device ID
             * @param eventBus Reference to EventBus (ALL dependencies in constructor!)
             */
            Servo(IPWMDriver& pwm, uint8_t channel, uint16_t deviceId, EventBus& eventBus);
            // ABSTRACTION, not concrete hardware!
            // Constructor injection!

            // IDevice interface - Lifecycle
            bool initialize() override;
            void shutdown() override;
            void update() override;

            // IDevice interface - Identity & Capabilities
            DeviceInfo getInfo() const override;
            uint16_t getCapabilities() const override;
            bool hasCapability(DeviceCapability cap) const override;

            // IDevice interface - State Management
            DeviceState getState() const override;
            void enable() override;
            void disable() override;
            bool isEnabled() const override;

            // IDevice interface - Configuration
            bool configure(const JsonDocument& config) override;
            void getConfiguration(JsonDocument& config) const override;

            // IDevice interface - Serialization
            void toJson(JsonDocument& doc) const override;
            bool fromJson(const JsonDocument& doc) override;

            // IOutputDevice interface (CLEAN - no channel parameter!)
            void setValue(float angle) override;           // Set angle directly
            void setNormalized(float value) override;      // Set normalized (0-1)
            void moveTo(float target, unsigned long duration) override;
            float getValue() const override;               // Get current angle
            bool isMoving() const override;                // Check if animating

            // Servo-specific API - Basic Control
            void setAngle(float angle);                    // Same as setValue() for clarity
            void calibrate(uint16_t minPulse, uint16_t maxPulse, float minAngle, float maxAngle);
            void calibrateBySteps(uint16_t minStep, uint16_t maxStep,
                                  float minAngle = 0.0f, float maxAngle = 180.0f);

            // Advanced Motion Control - Robotic Arm API
            enum EasingType {
                EASE_LINEAR,          // Constant speed
                EASE_IN_QUAD,         // Slow start, fast end
                EASE_OUT_QUAD,        // Fast start, slow end
                EASE_IN_OUT_QUAD,     // Slow start, fast middle, slow end
                EASE_IN_CUBIC,        // Stronger acceleration
                EASE_OUT_CUBIC        // Stronger deceleration
            };

            void moveToWithEasing(float target, unsigned long duration, EasingType easing);
            void moveBySteps(float deltaAngle, unsigned long stepDuration);  // Incremental move
            void setSpeed(float degreesPerSecond);  // Constant speed mode
            void moveWithSpeed(float target);       // Move to target at set speed
            void stop();                            // Stop current movement immediately
            void pause();                           // Pause movement (can resume)
            void resume();                          // Resume paused movement

            // Training Mode Support - Position Recording
            float getCurrentAngle() const { return _currentAngle; }
            float getTargetAngle() const { return _targetAngle; }
            unsigned long getRemainingTime() const;
            float getProgress() const;  // 0.0-1.0 animation progress

        private:
            IPWMDriver& _pwm;  // Abstract interface, not concrete driver
            uint8_t _channel;  // CONST - known at construction, never changes
            uint16_t _deviceId;
            EventBus& _eventBus;  // Reference, not pointer - ALWAYS valid

            // State
            DeviceState _state = STATE_UNINITIALIZED;
            bool _enabled = true;

            // Calibration - Microseconds mode
            uint16_t _minPulse = 500;
            uint16_t _maxPulse = 2500;
            float _minAngle = 0;
            float _maxAngle = 180;
            float _currentAngle = 90;  // Track current position

            // Calibration - Step mode (direct PWM ticks)
            bool _useStepMode = false;  // false = microseconds, true = steps
            uint16_t _stepMin = 0;
            uint16_t _stepMax = 4095;
            float _stepRange = 4095;
            float _angleRange = 180;

            // Animation state
            float _startAngle = 90;    // Start angle for animation
            float _targetAngle = 90;
            unsigned long _animationStart = 0;
            unsigned long _animationDuration = 0;
            EasingType _easingType = EASE_LINEAR;
            bool _isPaused = false;
            unsigned long _pausedAt = 0;
            unsigned long _pausedDuration = 0;

            // Speed control
            float _degreesPerSecond = 0;  // 0 = time-based, >0 = speed-based

            // Helper methods
            uint16_t mapAngleToPWM(float angle);
            float applyEasing(float t, EasingType type);
        };
    }
}  // namespace TwiST::Devices

#endif