/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IInputDevice.h
 * @brief     Input device interface for sensors and input sources
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   IDevice (extends base interface)
 *
 * PRINCIPLES:
 * - One IInputDevice instance = ONE physical sensor
 * - Multi-axis devices use axis parameter (e.g., joystick X/Y)
 * - Normalized values (0.0-1.0) for consistency
 * - Hardware-independent abstraction
 * - Supports both analog and digital inputs
 *
 * CAPABILITIES:
 * - Read analog values (normalized 0.0-1.0)
 * - Read digital states (boolean)
 * - Multi-axis input support
 * - Input readiness checking
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef TWIST_IINPUTDEVICE_H
#define TWIST_IINPUTDEVICE_H

#include "IDevice.h"

namespace TwiST {

    /**
     * @brief Interface for single input device
     *
     * CRITICAL ARCHITECTURE: One IInputDevice object = ONE physical sensor.
     * For multi-axis devices like joysticks, axis parameter represents logical axes (0=X, 1=Y).
     *
     * Examples:
     * - Joystick object reads TWO axes (X, Y) from ONE joystick hardware
     * - Button object reads ONE digital input
     * - Potentiometer object reads ONE analog value
     */
    class IInputDevice : public IDevice {
    public:
        virtual ~IInputDevice() = default;

        /**
         * @brief Read analog value
         * @param axis Axis index (0=first, 1=second, etc.)
         * @return Normalized value (0.0 - 1.0)
         */
        virtual float readAnalog(uint8_t axis) = 0;

        /**
         * @brief Read digital state
         * @param button Button index
         * @return true if active/pressed
         */
        virtual bool readDigital(uint8_t button) = 0;

        /**
         * @brief Check if input is available
         * @return true if data ready
         */
        virtual bool isInputReady() = 0;
    };

}  // namespace TwiST

#endif