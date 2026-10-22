// Turn a PID correction into EEEBot drive outputs.
//
// The EEEBot steers its front wheels with a servo (90 = straight) and has
// two rear DC motors, so a correction both turns the servo and slows the
// inside wheel, as in the logbook's "base speed +/- PID value".
//
// Plain C++ with no Arduino dependency, so it can be unit-tested on a PC.
#pragma once

struct DriveCommand {
  int servoAngle;  // degrees
  int leftSpeed;   // PWM 0..255
  int rightSpeed;  // PWM 0..255
};

struct SteeringConfig {
  int centreAngle = 90;   // servo angle for straight ahead
  int maxSteer = 30;      // servo degrees either side at full correction
  int baseSpeed = 180;    // PWM when the line is centred
  int speedDiff = 80;     // PWM taken off the inside wheel at full correction
};

// correction in [-1, 1]: negative steers left, positive steers right.
DriveCommand mixSteering(float correction, const SteeringConfig& config);
