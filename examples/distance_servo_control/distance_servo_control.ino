/* ============================================================================
 * TwiST Framework | Distance-Controlled Servo
 * ============================================================================
 *
 * Move your hand closer = servo moves to 0°
 * Move your hand away = servo moves to 180°
 * Remove your hand = servo returns to center (90°)
 *
 * Hardware:
 *   - ESP32-C6 + HC-SR04 sensor + PCA9685 + Servo
 *
 * Author: Voldemaras Birskys
 * License: MIT
 * ============================================================================ */

#include "src/TwiST_Framework/TwiST.h"
#include "src/TwiST_Framework/Drivers/Distance/HCSR04.h"
#include "src/TwiST_Framework/Drivers/PWM/PCA9685.h"

using namespace TwiST;
using namespace TwiST::Devices;
using namespace TwiST::Drivers;

TwiSTFramework framework;

HCSR04 hcsr04(16, 17);
PCA9685 pca9685(0x40);

IDistanceDriver& distanceDriver = hcsr04;
IPWMDriver& pwm = pca9685;

DistanceSensor sensor(distanceDriver, 300, framework.eventBus(), 50);
Servo servo(pwm, 0, 100, framework.eventBus());

void setup() {
    Serial.begin(115200);

    framework.initialize();

    hcsr04.begin();
    pca9685.begin(22, 23);
    pca9685.setFrequency(50);

    sensor.initialize();
    servo.initialize();

    sensor.setFilterStrength(0.3f);
    servo.calibrateBySteps(110, 540);

    framework.registry()->registerDevice(&sensor);
    framework.registry()->registerDevice(&servo);

    Serial.println("Distance-Controlled Servo Ready!");
    Serial.println("Move hand 0-10 cm from sensor\n");
}

void loop() {
    framework.update();

    uint16_t distance = sensor.getDistanceCm();

    if (distance <= 10) {
        float angle = (distance / 10.0f) * 180.0f;
        servo.setAngle(angle);
    } else {
        servo.setAngle(90.0f);
    }

    delay(30);
}
