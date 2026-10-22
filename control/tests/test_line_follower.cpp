// Host-side unit tests for the line-follower maths. No Arduino needed:
//   g++ -std=c++11 -Wall -Wextra -I control/EEEBotLineFollower/src \
//       control/tests/test_line_follower.cpp control/EEEBotLineFollower/src/*.cpp -o test_lf
//   ./test_lf
#include <cmath>
#include <cstdio>

#include "EEEBotLineFollower.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                   \
  do {                                                                \
    checks++;                                                         \
    if (!(cond)) {                                                    \
      failures++;                                                     \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);     \
    }                                                                 \
  } while (0)

#define CHECK_NEAR(a, b, tol) CHECK(std::fabs((a) - (b)) <= (tol))

static void test_pid_proportional_only() {
  PID pid(2.0f, 0.0f, 0.0f, 100.0f);
  CHECK_NEAR(pid.update(0.25f, 0.01f), 0.5f, 1e-6f);
  CHECK_NEAR(pid.update(-0.25f, 0.01f), -0.5f, 1e-6f);
}

static void test_pid_matches_logbook_form_with_dt_1() {
  // integral += error; derivative = error - prev; out = Kp e + Ki I + Kd D
  PID pid(1.0f, 0.1f, 0.01f, 100.0f, 100.0f);
  float e1 = 0.5f, e2 = 0.2f;
  CHECK_NEAR(pid.update(e1, 1.0f), 1.0f * e1 + 0.1f * e1 + 0.0f, 1e-6f);  // no D kick on first sample
  CHECK_NEAR(pid.update(e2, 1.0f), 1.0f * e2 + 0.1f * (e1 + e2) + 0.01f * (e2 - e1), 1e-6f);
}

static void test_pid_integral_accumulates_and_is_clamped() {
  PID pid(0.0f, 1.0f, 0.0f, 100.0f, 2.0f);
  for (int i = 0; i < 1000; i++) pid.update(1.0f, 0.1f);
  CHECK_NEAR(pid.integral(), 2.0f, 1e-6f);  // anti-windup limit
  pid.reset();
  CHECK_NEAR(pid.integral(), 0.0f, 1e-6f);
}

static void test_pid_output_is_clamped() {
  PID pid(10.0f, 0.0f, 0.0f, 1.0f);
  CHECK_NEAR(pid.update(5.0f, 0.01f), 1.0f, 1e-6f);
  CHECK_NEAR(pid.update(-5.0f, 0.01f), -1.0f, 1e-6f);
}

static void test_pid_derivative_opposes_fast_change() {
  PID pid(0.0f, 0.0f, 1.0f, 100.0f);
  pid.update(0.0f, 0.1f);
  CHECK(pid.update(0.5f, 0.1f) > 0.0f);   // error rising
  CHECK(pid.update(0.2f, 0.1f) < 0.0f);   // error falling
}

static void test_pid_zero_dt_is_safe() {
  PID pid(0.0f, 1.0f, 1.0f, 100.0f);
  float out = pid.update(1.0f, 0.0f);
  CHECK(!std::isnan(out) && !std::isinf(out));
}

static void test_pid_closed_loop_settles() {
  // Simple plant: position moves by the correction each step.
  PID pid(0.8f, 0.05f, 0.05f, 1.0f);
  float position = 1.0f;
  for (int i = 0; i < 200; i++) position -= 0.2f * pid.update(position, 0.05f);
  CHECK_NEAR(position, 0.0f, 0.02f);
}

static void test_line_position_centre_left_right() {
  float pos = 99.0f;
  int centre[] = {0, 0, 1000, 1000, 0, 0};
  CHECK(linePosition(centre, 6, pos));
  CHECK_NEAR(pos, 0.0f, 1e-6f);

  int left[] = {1000, 0, 0, 0, 0, 0};
  CHECK(linePosition(left, 6, pos));
  CHECK_NEAR(pos, -1.0f, 1e-6f);

  int right[] = {0, 0, 0, 0, 0, 1000};
  CHECK(linePosition(right, 6, pos));
  CHECK_NEAR(pos, 1.0f, 1e-6f);
}

static void test_line_position_weighted_between_sensors() {
  float pos = 0.0f;
  int between[] = {0, 0, 0, 500, 500, 0};  // between sensors 4 and 5 of 6
  CHECK(linePosition(between, 6, pos));
  CHECK_NEAR(pos, (0.2f + 0.6f) / 2, 1e-6f);
}

static void test_line_position_logbook_example() {
  // Figure on logbook p.28: 8 sensors reading 0 0 0 4 9 4 0 0 -> centred on sensor 5
  float pos = 0.0f;
  int reading[] = {0, 0, 0, 400, 900, 400, 0, 0};
  CHECK(linePosition(reading, 8, pos));
  float sensor5 = -1.0f + 2.0f * 4 / 7;
  CHECK_NEAR(pos, sensor5, 1e-5f);
}

static void test_line_lost() {
  float pos = 0.42f;
  int none[] = {10, 20, 0, 0, 30, 0};
  CHECK(!linePosition(none, 6, pos));
  CHECK_NEAR(pos, 0.42f, 1e-6f);  // unchanged
}

static void test_normalise_both_polarities() {
  CHECK(normaliseReading(200, 200, 800) == 0);
  CHECK(normaliseReading(800, 200, 800) == 1000);
  CHECK(normaliseReading(500, 200, 800) == 500);
  CHECK(normaliseReading(900, 200, 800) == 1000);  // clamped
  CHECK(normaliseReading(300, 900, 300) == 1000);  // line reads low
  CHECK(normaliseReading(5, 7, 7) == 0);           // uncalibrated
}

static void test_steering_mix() {
  SteeringConfig cfg;
  DriveCommand straight = mixSteering(0.0f, cfg);
  CHECK(straight.servoAngle == 90);
  CHECK(straight.leftSpeed == cfg.baseSpeed && straight.rightSpeed == cfg.baseSpeed);

  DriveCommand right = mixSteering(1.0f, cfg);
  CHECK(right.servoAngle == 120);  // same side as nano_drive's clockwise turn
  CHECK(right.leftSpeed == cfg.baseSpeed);
  CHECK(right.rightSpeed == cfg.baseSpeed - cfg.speedDiff);

  DriveCommand left = mixSteering(-0.5f, cfg);
  CHECK(left.servoAngle == 75);
  CHECK(left.leftSpeed == cfg.baseSpeed - cfg.speedDiff / 2);
  CHECK(left.rightSpeed == cfg.baseSpeed);

  DriveCommand over = mixSteering(5.0f, cfg);
  CHECK(over.servoAngle == right.servoAngle);
}

int main() {
  test_pid_proportional_only();
  test_pid_matches_logbook_form_with_dt_1();
  test_pid_integral_accumulates_and_is_clamped();
  test_pid_output_is_clamped();
  test_pid_derivative_opposes_fast_change();
  test_pid_zero_dt_is_safe();
  test_pid_closed_loop_settles();
  test_line_position_centre_left_right();
  test_line_position_weighted_between_sensors();
  test_line_position_logbook_example();
  test_line_lost();
  test_normalise_both_polarities();
  test_steering_mix();

  std::printf("%d/%d checks passed\n", checks - failures, checks);
  return failures == 0 ? 0 : 1;
}
