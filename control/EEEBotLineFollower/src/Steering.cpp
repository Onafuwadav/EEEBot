#include "Steering.h"

namespace {
int clampInt(int value, int low, int high) {
  if (value < low) return low;
  if (value > high) return high;
  return value;
}
}  // namespace

DriveCommand mixSteering(float correction, const SteeringConfig& config) {
  if (correction > 1.0f) correction = 1.0f;
  if (correction < -1.0f) correction = -1.0f;

  DriveCommand cmd;
  cmd.servoAngle = clampInt(config.centreAngle + (int)(correction * config.maxSteer), 0, 180);

  // Slow only the wheel on the inside of the turn.
  int slowdown = (int)(correction * config.speedDiff);
  cmd.leftSpeed = clampInt(config.baseSpeed - (slowdown < 0 ? -slowdown : 0), 0, 255);
  cmd.rightSpeed = clampInt(config.baseSpeed - (slowdown > 0 ? slowdown : 0), 0, 255);
  return cmd;
}
