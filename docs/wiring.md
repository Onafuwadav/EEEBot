# Wiring

Pins taken from the firmware in this repo.

## ESP32: maze menu

| Signal | ESP32 GPIO | Notes |
|---|---|---|
| Keypad row 1 (`1 2 3`) | 2 | strapping pin |
| Keypad row 2 (`4 5 6`) | 0 | strapping pin: don't hold a key in this row while booting |
| Keypad row 3 (`7 8 9`) | 4 | |
| Keypad row 4 (`* 0 #`) | 17 | |
| Keypad col 1 | 5 | |
| Keypad col 2 | 18 | |
| Keypad col 3 | 19 | |
| LCD RS | 27 | `LiquidCrystal lcd(27, 14, 32, 33, 25, 26)` |
| LCD E | 14 | |
| LCD D4 | 32 | |
| LCD D5 | 33 | |
| LCD D6 | 25 | |
| LCD D7 | 26 | |
| I2C SDA | 21 | ESP32 default, to the level shifter's 3.3 V side |
| I2C SCL | 22 | ESP32 default, to the level shifter's 3.3 V side |

## Arduino Nano: drive

| Signal | Nano pin | Notes |
|---|---|---|
| Steering servo | D4 | 90 = straight, 120 = right/clockwise, 60 = left |
| Motor A enable (PWM) | D5 | `enA` |
| Motor B enable (PWM) | D6 | `enB` |
| Motor A direction | A0, A1 | `INa`, `INb` |
| Motor B direction | A2, A3 | `INc`, `INd` |
| I2C SDA | A4 | to the level shifter's 5 V side |
| I2C SCL | A5 | to the level shifter's 5 V side |
| Left encoder | D2 | interrupt pin, only used with `USE_ENCODERS 1` |
| Right encoder | D3 | interrupt pin, only used with `USE_ENCODERS 1` |

Direction truth table (from `nano_drive.ino`):

| Move | INa | INb | INc | INd |
|---|---|---|---|---|
| Forwards | H | L | H | L |
| Backwards | L | H | L | H |
| Clockwise | H | L | L | H |
| Anticlockwise | L | H | H | L |
| Stop | L | L | L | L |

## Line follower example (`control/`)

| Signal | Nano pin | Notes |
|---|---|---|
| IR sensors, left to right | A4, A5, A6, A7 | the Nano's only free analog inputs once A0-A3 drive the motors. A4/A5 are the I2C pins, so this sketch runs without the ESP32 link |
| Servo, motors | as above | |

## Level shifter

| 3.3 V side | 5 V side |
|---|---|
| ESP32 3V3 (LV) | Nano 5V (HV) |
| ESP32 GND | Nano GND |
| ESP32 GPIO 21 (SDA) | Nano A4 (SDA) |
| ESP32 GPIO 22 (SCL) | Nano A5 (SCL) |

The grounds must be common.

## Raspberry Pi 3A+

| Item | Connection |
|---|---|
| Camera | CSI ribbon, mounted at the front facing forwards |
| MQTT broker | Mosquitto on the Pi (`sudo apt-get install mosquitto`) |
| Dashboard | Node-RED on `http://<pi>:1880` |
| Vision output | MQTT topic `eeebot/vision` ([`pi_camera_mqtt.py`](../vision/scripts/pi_camera_mqtt.py)) |
