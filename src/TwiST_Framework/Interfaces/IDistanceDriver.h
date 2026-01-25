/* ============================================================================
 * TwiST Framework | Interface
 * ============================================================================
 * @file      IDistanceDriver.h
 * @brief     Distance measurement driver abstraction
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Interfaces
 * - Type:         Pure Abstract Interface
 * - Hardware:     None (pure abstraction)
 * - Implements:   None (IS the interface)
 *
 * PRINCIPLES:
 * - Pure C++ interface (no Arduino.h)
 * - Hardware-independent distance measurement contract
 * - ANY distance sensor must implement this interface
 * - Devices NEVER know about specific sensor hardware
 *
 * CAPABILITIES:
 * - Trigger distance measurement
 * - Read distance in centimeters
 * - Check measurement status
 * - Query maximum range
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0.1
 * ============================================================================
 */

#ifndef TWIST_IDISTANCEDRIVER_H
#define TWIST_IDISTANCEDRIVER_H

#include <stdint.h>

namespace TwiST {

/**
 * @brief Distance measurement driver abstraction interface
 *
 * CRITICAL: Pure C++ interface - NO Arduino dependencies.
 * ANY distance sensor (ultrasonic, IR, laser) must implement this.
 * Devices NEVER know about specific hardware (HC-SR04, VL53L0X, etc.)
 */
class IDistanceDriver {
public:
    virtual ~IDistanceDriver() = default;

    /**
     * @brief Trigger a new distance measurement
     *
     * For ultrasonic sensors: sends trigger pulse
     * For continuous sensors: may be no-op
     */
    virtual void triggerMeasurement() = 0;

    /**
     * @brief Read distance measurement in centimeters
     * @return Distance in cm (0 if invalid or out of range)
     */
    virtual float readDistanceCm() = 0;

    /**
     * @brief Check if measurement is ready
     * @return true if valid measurement available
     */
    virtual bool isMeasurementReady() const = 0;

    /**
     * @brief Get maximum measurement range
     * @return Maximum range in centimeters
     */
    virtual float getMaxRange() const = 0;
};

}  // namespace TwiST

#endif
