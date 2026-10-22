# vision

Symbol recognition and coloured-line detection for the EEEBot's Raspberry Pi
camera, in Python and OpenCV. No machine learning.

This is a rebuild: the logbook kept the method, the Figure 39 flowchart and
some pseudo-code, but not the original program. The reference PNGs and sample
frames here are drawn by [`synthetic.py`](eeebot_vision/synthetic.py), not
photographed. If you have the original reference PNGs, drop them into
`references/` with the same names.

```bash
pip install -e ".[test]"
pytest
python -m eeebot_vision symbol samples/symbol_*.png -v
python -m eeebot_vision line samples/line_blue_right.png --colour blue
python tools/make_assets.py          # regenerate references/ and samples/
```

| File | What it does |
|---|---|
| `eeebot_vision/symbols.py` | Pink HSV mask, find the border's 4 corners, perspective warp to 400x400, IoU against each reference (all four rotations), threshold 0.70 |
| `eeebot_vision/colour_line.py` | HSV ranges for red/yellow/green/blue/black, line error in [-1, 1] from the bottom third of the frame |
| `scripts/pi_camera_mqtt.py` | Camera loop on the Pi that publishes `{symbol, colour, error}` to Mosquitto. Not tested on hardware |

On the synthetic samples, correct matches score about 0.99 and the best wrong
symbol scores at most about 0.56, so 0.70 leaves room either side. Tune
`DEFAULT_THRESHOLD` and the HSV ranges against real camera frames.
