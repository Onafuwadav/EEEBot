"""Live loop for the Raspberry Pi 3A+ camera: recognise symbols, track the
current line colour and publish the result to Mosquitto over MQTT.

    pip install -e ".[pi]"
    python scripts/pi_camera_mqtt.py --broker localhost

Publishes JSON to ``eeebot/vision`` about 10 times a second, e.g.
    {"symbol": "star", "score": 0.93, "colour": "green", "error": -0.21}
Node-RED (or the ESP32) subscribes to that topic and feeds ``error`` into
the PID controller in ``control/``. ``error`` is null when the line is lost.

Not tested on the robot hardware; the pipeline it calls is covered by the
tests in ``tests/``.
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from pathlib import Path

import cv2

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from eeebot_vision import line_position, load_references, recognise_symbol  # noqa: E402


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--broker", default="localhost")
    parser.add_argument("--topic", default="eeebot/vision")
    parser.add_argument("--camera", type=int, default=0)
    parser.add_argument("--start-colour", default="black", help="line to follow before any symbol is seen")
    parser.add_argument("--period", type=float, default=0.1, help="seconds between frames")
    args = parser.parse_args()

    try:
        import paho.mqtt.client as mqtt
    except ImportError:
        sys.exit('paho-mqtt is not installed: pip install -e ".[pi]"')

    client = mqtt.Client()
    client.connect(args.broker)
    client.loop_start()

    camera = cv2.VideoCapture(args.camera)
    references = load_references()
    colour = args.start_colour

    try:
        while True:
            ok, frame = camera.read()
            if not ok:
                print("camera read failed", file=sys.stderr)
                time.sleep(1)
                continue
            match = recognise_symbol(frame, references)
            if match:
                colour = match.line_colour  # a symbol picks the next line to follow
            payload = {
                "symbol": match.symbol if match else None,
                "score": round(match.score, 3) if match else None,
                "colour": colour,
                "error": line_position(frame, colour),
            }
            client.publish(args.topic, json.dumps(payload))
            time.sleep(args.period)
    except KeyboardInterrupt:
        pass
    finally:
        camera.release()
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
