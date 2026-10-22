#include "PID.h"

namespace {
float clamp(float value, float limit) {
  if (value > limit) return limit;
  if (value < -limit) return -limit;
  return value;
}
}  // namespace

PID::PID(float kp, float ki, float kd, float outputLimit, float integralLimit)
    : kp_(kp), ki_(ki), kd_(kd), outputLimit_(outputLimit), integralLimit_(integralLimit) {}

float PID::update(float error, float dt) {
  if (dt <= 0.0f) dt = 1e-3f;  // guard against a zero or backwards clock

  integral_ = clamp(integral_ + error * dt, integralLimit_);

  // No derivative on the first sample, otherwise the jump from 0 to the
  // first error gives the steering a kick.
  float derivative = first_ ? 0.0f : (error - prevError_) / dt;
  first_ = false;
  prevError_ = error;

  return clamp(kp_ * error + ki_ * integral_ + kd_ * derivative, outputLimit_);
}

void PID::reset() {
  integral_ = 0.0f;
  prevError_ = 0.0f;
  first_ = true;
}

void PID::setTunings(float kp, float ki, float kd) {
  kp_ = kp;
  ki_ = ki;
  kd_ = kd;
}
