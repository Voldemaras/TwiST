/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IOutputDevice.h
 * @brief     Output device interface for actuators and output devices
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   IDevice (extends base interface)
 *
 * PRINCIPLES:
 * - One IOutputDevice instance = ONE physical actuator
 * - NO channel parameter - channel locked at construction
 * - Supports semantic values (angles, speeds) and normalized (0.0-1.0)
 * - Hardware-independent abstraction
 * - Time-based motion control
 *
 * CAPABILITIES:
 * - Set semantic values (device-specific: angles, speeds, brightness)
 * - Set normalized values (0.0-1.0)
 * - Animated movement with duration
 * - Motion state tracking
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef TWIST_IOUTPUTDEVICE_H
#define TWIST_IOUTPUTDEVICE_H

#include "IDevice.h"

namespace TwiST {

    /**
     * @brief Interface for single output device
     *
     * CRITICAL ARCHITECTURE: One IOutputDevice object = ONE physical actuator.
     * NO channel parameter - channel is locked at construction in implementation.
     *
     * Examples:
     * - Servo object controls ONE servo motor on ONE PWM channel
     * - Motor object controls ONE motor on ONE driver channel
     * - LED object controls ONE LED on ONE GPIO pin
     *
     * This is NOT a multi-channel controller interface!
     */
    class IOutputDevice : public IDevice {
    public:
        virtual ~IOutputDevice() = default;

        /**
         * @brief Set output value (semantic depends on device type)
         * @param value Device-specific value:
         *   - Servo: angle in degrees (0-180)
         *   - Motor: speed (-100 to +100)
         *   - LED: brightness (0.0-1.0)
         */
        virtual void setValue(float value) = 0;

        /**
         * @brief Set normalized output (0.0 - 1.0)
         * @param value Normalized value
         */
        virtual void setNormalized(float value) = 0;

        /**
         * @brief Move to target with animation
         * @param target Target value
         * @param duration Animation duration in ms
         */
        virtual void moveTo(float target, unsigned long duration) = 0;

        /**
         * @brief Get current output value
         * @return Current value
         */
        virtual float getValue() const = 0;

        /**
         * @brief Check if device is currently moving/animating
         * @return true if in motion
         */
        virtual bool isMoving() const = 0;
    };

}  // namespace TwiST

#endif