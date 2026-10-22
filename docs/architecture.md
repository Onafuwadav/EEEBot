# System architecture

```mermaid
flowchart LR
  subgraph MENU["Maze menu"]
    KP["4x3 keypad"] --> ESP["ESP32<br/>UI + logic"]
    ESP --> LCD["16x2 LCD"]
  end

  ESP -- "I2C master<br/>3.3 V" --> LS["Bi-directional<br/>3.3 V / 5 V<br/>level shifter"]
  LS -- "I2C slave 0x08<br/>5 V" --> NANO["Arduino Nano<br/>drive"]

  subgraph DRIVE["Custom PCB"]
    NANO -- "enA D5 / enB D6 (PWM)<br/>INa-INd A0-A3" --> HB["H-bridge"]
    HB --> M["2x DC motors"]
    NANO -- "D4" --> SERVO["Steering servo"]
    ENC["Wheel encoders"] -.-> NANO
  end

  US["HC-SR04<br/>ultrasonic"] -.-> ESP
  IMU["MPU-9250 IMU"] -. "I2C" .-> ESP

  subgraph PI["Raspberry Pi 3A+"]
    CAM["Pi camera"] --> CV["vision/<br/>OpenCV"]
    CV -- "eeebot/vision" --> MQ[("Mosquitto<br/>MQTT broker")]
    MQ <--> NR["Node-RED"]
  end
  MQ -. "Wi-Fi (V2X)" .- ESP
```

Solid lines are the maze-navigation path that won the prize. Dashed lines
are the sensor and V2X lab tasks.

## Who does what

| Board | Firmware | Job |
|---|---|---|
| ESP32 | [`firmware/esp32_menu`](../firmware/esp32_menu/esp32_menu.ino) | Reads the keypad, shows the command on the LCD, checks it and sends it over I2C |
| Arduino Nano | [`firmware/nano_drive`](../firmware/nano_drive/nano_drive.ino) | I2C slave at `0x08`: parses the command, drives the motors and steering servo |
| Raspberry Pi 3A+ | [`vision/`](../vision) | Symbol recognition and coloured-line position from the camera, published over MQTT |
| Nano (line task) | [`control/`](../control) | IR weighted-average position, PID, servo and motor mixing |

The ESP32 runs at 3.3 V and the Nano at 5 V, so the I2C lines go through the
bi-directional level shifter. The ESP32 uses its default I2C pins
(SDA 21, SCL 22); the Nano's are fixed at A4 (SDA) and A5 (SCL).

## Maze command flow

```mermaid
sequenceDiagram
  actor User
  participant ESP as ESP32 (menu)
  participant LCD
  participant Nano as Nano (0x08)
  User->>ESP: keys 1 2
  ESP->>LCD: "12"
  User->>ESP: *
  ESP->>ESP: valid_command("12")
  ESP->>Nano: I2C write "12"
  ESP->>LCD: "Sent 12"
  Note over Nano: receiveEvent stores the bytes<br/>and sets the move flag
  Nano->>Nano: loop(): order 1, distance 1200 mm
  Nano->>Nano: goForwards(), moveFor(...), stopMotors()
```

## Symbol recognition
[`vision/eeebot_vision/symbols.py`](../vision/eeebot_vision/symbols.py):

```mermaid
flowchart TD
  A([Start]) --> B[/Capture an image/]
  B --> C[Convert to HSV to isolate the pink pixels]
  C --> D[Find the corners of the symbol's border]
  D --> E[Transform the perspective of the image]
  E --> F[Compare the transformed image with the four symbol images]
  F --> G{Match value greater<br/>than the threshold?}
  G -- Yes --> H[/Output which symbol has been detected/]
  G -- No --> I[/Output no symbol has been detected/]
  H --> B
  I --> B
```

<img src="images/symbol-recognition-flowchart.png" width="320" alt="Flowchart">

| Symbol | Line to follow |
|---|---|
| Circle | Red |
| Star | Green |
| Triangle | Blue |
| Umbrella | Yellow |

## Line following

The IR board has evenly spaced emitter/receiver pairs. Each reading is
proportional to how much line is under it, and the line's position is the
weighted average of the readings:

<img src="images/ir-weighted-average.png" width="480" alt="Eight sensors reading 0 0 0 4 9 4 0 0">

That position is the error fed into a PID controller, whose output turns the
steering servo and slows the inside wheel:

<img src="images/pid-block-diagram.png" width="400" alt="PID block diagram">

*PID diagram by Arturo Urquizo, [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:PID_en.svg), CC BY-SA 3.0*

The camera-based coloured-line follower reuses the same controller: the
vision code turns the HSV mask into the same [-1, 1] position.