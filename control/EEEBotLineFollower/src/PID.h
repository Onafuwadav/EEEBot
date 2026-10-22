// PID controller for line following (logbook section 7, Figure 38).
//
// The logbook version was:
//   integral   = integral + error;
//   derivative = error - prev_error;
//   output     = Kp*error + Ki*integral + Kd*derivative;
// This keeps that form, scales the I and D terms by the loop time dt so the
// gains don't change if the loop speed does (dt = 1 gives the logbook
// maths exactly), and adds the two things a real robot needs: a clamp on the
// integral (anti-windup) and a clamp on the output.
//
// Plain C++ with no Arduino dependency, so it can be unit-tested on a PC.
#pragma once

class PID {
 public:
  PID(float kp, float ki, float kd, float outputLimit = 1.0f, float integralLimit = 10.0f);

  // error = measured - desired (for line following, the line position).
  // dt = seconds since the last update. Returns the correction, clamped to
  // +/- outputLimit.
  float update(float error, float dt);

  void reset();
  void setTunings(float kp, float ki, float kd);

  float kp() const { return kp_; }
  float ki() const { return ki_; }
  float kd() const { return kd_; }
  float integral() const { return integral_; }

 private:
  float kp_, ki_, kd_;
  float outputLimit_, integralLimit_;
  float integral_ = 0.0f;
  float prevError_ = 0.0f;
  bool first_ = true;
};
