from pathlib import Path

import cv2
import numpy as np
import pytest

from eeebot_vision.symbols import (
    DEFAULT_THRESHOLD,
    SYMBOL_TO_COLOUR,
    WARPED_SIZE,
    find_symbol_corners,
    load_references,
    order_corners,
    recognise_symbol,
    score_symbols,
)
from eeebot_vision.synthetic import PINK_BGR, empty_scene, line_scene, symbol_scene

SAMPLES = Path(__file__).resolve().parent.parent / "samples"
SYMBOLS = list(SYMBOL_TO_COLOUR)


@pytest.fixture(scope="module")
def references():
    return load_references()


def test_symbol_to_colour_matches_logbook():
    assert SYMBOL_TO_COLOUR == {
        "circle": "red",
        "star": "green",
        "triangle": "blue",
        "umbrella": "yellow",
    }


def test_references_load(references):
    assert set(references) == set(SYMBOLS)
    for mask in references.values():
        assert mask.dtype == bool
        assert mask.any()


@pytest.mark.parametrize("symbol", SYMBOLS)
def test_sample_images_are_recognised(symbol, references):
    image = cv2.imread(str(SAMPLES / f"symbol_{symbol}.png"))
    match = recognise_symbol(image, references)
    assert match is not None
    assert match.symbol == symbol
    assert match.line_colour == SYMBOL_TO_COLOUR[symbol]


@pytest.mark.parametrize("quarter_turns", [0, 1, 2, 3])
@pytest.mark.parametrize("symbol", SYMBOLS)
def test_recognised_from_any_side(symbol, quarter_turns, references):
    image = symbol_scene(symbol, quarter_turns=quarter_turns, seed=quarter_turns)
    match = recognise_symbol(image, references)
    assert match is not None and match.symbol == symbol


@pytest.mark.parametrize(
    "corners",
    [
        [[120, 60], [520, 60], [520, 420], [120, 420]],  # square on, close up
        [[260, 200], [380, 205], [395, 330], [250, 325]],  # far away, small
        [[150, 150], [430, 80], [520, 360], [190, 430]],  # rotated and skewed
    ],
    ids=["close", "far", "skewed"],
)
@pytest.mark.parametrize("symbol", SYMBOLS)
def test_recognised_under_perspective(symbol, corners, references):
    image = symbol_scene(symbol, corners=np.array(corners, np.float32), seed=1)
    match = recognise_symbol(image, references)
    assert match is not None and match.symbol == symbol


@pytest.mark.parametrize("symbol", SYMBOLS)
def test_recognised_in_poor_lighting(symbol, references):
    image = symbol_scene(symbol, noise=14, gradient=0.5, seed=3)
    match = recognise_symbol(image, references)
    assert match is not None and match.symbol == symbol


@pytest.mark.parametrize("symbol", SYMBOLS)
def test_other_symbols_stay_below_threshold(symbol, references):
    scores, _ = score_symbols(symbol_scene(symbol), references)
    others = [score for name, score in scores.items() if name != symbol]
    assert max(others) < DEFAULT_THRESHOLD
    assert scores[symbol] - max(others) > 0.2


def test_no_symbol_in_empty_frame(references):
    image = empty_scene(seed=4)
    assert find_symbol_corners(image) is None
    assert recognise_symbol(image, references) is None


def test_empty_border_is_not_a_symbol(references):
    blank = np.full((WARPED_SIZE, WARPED_SIZE, 3), PINK_BGR, np.uint8)
    blank[25:-25, 25:-25] = 255
    frame = np.full((480, 640, 3), 150, np.uint8)
    frame[40:440, 120:520] = blank
    assert find_symbol_corners(frame) is not None
    assert recognise_symbol(frame, references) is None


def test_red_line_is_not_mistaken_for_symbol_border():
    assert find_symbol_corners(line_scene("red", 320, thickness=80)) is None


def test_order_corners():
    shuffled = np.array([[400, 390], [10, 20], [20, 380], [390, 15]], np.float32)
    ordered = order_corners(shuffled)
    np.testing.assert_array_equal(ordered, [[10, 20], [390, 15], [400, 390], [20, 380]])
