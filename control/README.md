# control

PID line following for the EEEBot as an Arduino library,
[`EEEBotLineFollower`](EEEBotLineFollower), rebuilt from the logbook.

```
IR readings -> normaliseReading() -> linePosition() -> PID::update() -> mixSteering() -> servo + motors
               0..1000 per sensor    weighted avg       Kp 1.0          servo 90 +/- 30
                                     in [-1, 1]          Ki 0.1          slow the inside wheel
                                                         Kd 0.01
```

The maths has no Arduino dependency, so it is unit-tested on a PC:

```bash
g++ -std=c++11 -Wall -Wextra -I control/EEEBotLineFollower/src \
    control/tests/test_line_follower.cpp control/EEEBotLineFollower/src/*.cpp -o test_lf && ./test_lf
```

Compile the robot sketch with the library on the path:

```bash
arduino-cli compile --fqbn arduino:avr:nano --library control/EEEBotLineFollower control/EEEBotLineFollower/examples/LineFollower
```

The sketch's sensor pins (A4-A7) are an assumption: the logbook doesn't record
them. Change `SENSOR_PINS` to match the IR board. The vision code's line
error uses the same [-1, 1] convention, so the same PID works for the camera.
