/*
 * TwiST_ConfigValidator.h
 * System configuration safety validation for TwiST Framework
 *
 * PURPOSE:
 * Pre-flight check for system configuration integrity.
 * This is the last line of defense before hardware initialization.
 *
 * CRITICAL: System MUST NOT proceed if validation fails.
 *
 * Author: Voldemaras Birskys
 * Version: 1.0.1
 */

#ifndef TWIST_CONFIG_VALIDATOR_H
#define TWIST_CONFIG_VALIDATOR_H

namespace TwiST {

/**
 * @brief Runs comprehensive safety checks on system configuration
 *
 * This is the pre-flight check. System MUST halt if this fails.
 *
 * Validation checks:
 * 1. PWM driver I2C address uniqueness (no hardware collisions)
 * 2. Device ID uniqueness across ALL device types (EventBus routing)
 * 3. Device name uniqueness across ALL device types (API lookup)
 * 4. PWM frequency consistency (50Hz for servos)
 * 5. GPIO pin collision detection (no pin conflicts)
 * 6. Servo pwmDriverIndex range validation (no out-of-bounds)
 *
 * @return true if all safety checks pass, false otherwise
 * @note Prints detailed error messages to Serial on failure
 * @note MUST be called after Serial.begin() - requires Arduino runtime
 * @note On failure, system should halt immediately (fail-fast)
 */
bool runSystemConfigSafetyCheck();

}  // namespace TwiST

#endif // TWIST_CONFIG_VALIDATOR_H
