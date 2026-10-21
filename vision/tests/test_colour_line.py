from pathlib import Path

import cv2
import numpy as np
import pytest

from eeebot_vision.colour_line import colour_mask, line_position
from eeebot_vision.synthetic import empty_scene, line_scene

SAMPLES = Path(__file__).resolve().parent.parent / "samples"
COLOURS = ["red", "green", "blue", "yellow", "black"]


@pytest.mark.parametrize("colour", COLOURS)
def test_centred_line_has_zero_error(colour):
    assert line_position(line_scene(colour, 320), colour) == pytest.approx(0, abs=0.03)


@pytest.mark.parametrize("colour", COLOURS)
def test_left_line_is_negative_and_right_is_positive(colour):
    assert line_position(line_scene(colour, 100), colour) < -0.5
    assert line_position(line_scene(colour, 540), colour) > 0.5


def test_error_increases_left_to_right():
    errors = [line_position(line_scene("blue", x), "blue") for x in range(60, 600, 60)]
    assert errors == sorted(errors)


def test_error_is_measured_near_the_robot_not_far_ahead():
    # bottom of the line is centred; the bend at the top shouldn't dominate
    error = line_position(line_scene("green", 320, slant=250), "green")
    assert abs(error) < 0.25


def test_lost_line_returns_none():
    assert line_position(empty_scene(seed=2), "red") is None


@pytest.mark.parametrize("wanted, drawn", [("red", "blue"), ("green", "yellow"), ("yellow", "red"), ("blue", "green")])
def test_other_colours_are_ignored(wanted, drawn):
    assert line_position(line_scene(drawn, 320), wanted) is None


def test_red_wraps_around_hue_zero():
    hsv = np.array([[[2, 200, 200], [176, 200, 200], [90, 200, 200]]], np.uint8)
    bgr = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
    big = cv2.resize(bgr, (30, 10), interpolation=cv2.INTER_NEAREST)  # survive the 5x5 opening
    mask = colour_mask(big, "red")
    assert mask[:, :10].all() and mask[:, 10:20].all()
    assert not mask[:, 20:].any()


def test_unknown_colour_raises():
    with pytest.raises(ValueError):
        colour_mask(empty_scene(), "purple")


@pytest.mark.parametrize(
    "name, colour, sign",
    [("line_red_left", "red", -1), ("line_green_centre", "green", 0), ("line_blue_right", "blue", 1)],
)
def test_sample_images(name, colour, sign):
    error = line_position(cv2.imread(str(SAMPLES / f"{name}.png")), colour)
    assert error is not None
    if sign == 0:
        assert abs(error) < 0.05
    else:
        assert np.sign(error) == sign
