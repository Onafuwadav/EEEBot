"""Coloured-line detection (logbook section 8.1).

The camera image is converted from BGR to HSV and thresholded so the chosen
line colour becomes white and everything else black. The line's position is
then the weighted average of the white pixels' columns in a strip at the
bottom of the frame, the same weighted-average idea the IR line follower uses,
so the PID controller can be reused unchanged.
"""

from __future__ import annotations

import cv2
import numpy as np

# HSV ranges as (lower, upper) pairs, OpenCV scale: H 0-179, S and V 0-255.
# Red wraps around hue 0, so it needs two ranges.
LINE_COLOURS: dict[str, list[tuple[tuple[int, int, int], tuple[int, int, int]]]] = {
    "red": [((0, 100, 70), (10, 255, 255)), ((170, 100, 70), (179, 255, 255))],
    "yellow": [((20, 100, 100), (35, 255, 255))],
    "green": [((40, 70, 50), (85, 255, 255))],
    "blue": [((95, 100, 50), (130, 255, 255))],
    "black": [((0, 0, 0), (179, 255, 60))],
}


def colour_mask(image_bgr: np.ndarray, colour: str) -> np.ndarray:
    """Return a binary mask (0/255) of the pixels that match ``colour``."""
    if colour not in LINE_COLOURS:
        raise ValueError(f"unknown colour {colour!r}; expected one of {sorted(LINE_COLOURS)}")
    hsv = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2HSV)
    mask = np.zeros(hsv.shape[:2], dtype=np.uint8)
    for lower, upper in LINE_COLOURS[colour]:
        mask |= cv2.inRange(hsv, np.array(lower), np.array(upper))
    # Remove speckle noise so stray pixels don't pull the average around.
    kernel = np.ones((5, 5), np.uint8)
    return cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)


def line_position(
    image_bgr: np.ndarray,
    colour: str,
    roi_fraction: float = 1 / 3,
    min_pixels: int = 200,
) -> float | None:
    """Where the line is across the bottom of the frame.

    Returns an error in [-1, 1]: -1 is the left edge, 0 the centre and
    +1 the right edge. Returns ``None`` when there aren't enough line pixels
    in the region of interest, i.e. the line has been lost.
    """
    if not 0 < roi_fraction <= 1:
        raise ValueError("roi_fraction must be in (0, 1]")
    mask = colour_mask(image_bgr, colour)
    height, width = mask.shape
    roi = mask[int(height * (1 - roi_fraction)) :, :]

    column_weights = np.count_nonzero(roi, axis=0).astype(np.float64)
    total = column_weights.sum()
    if total < min_pixels:
        return None

    # Weighted average of column index, as in sum(value * weight) / sum(value).
    columns = np.arange(width, dtype=np.float64)
    centre_x = float((column_weights * columns).sum() / total)
    half_width = (width - 1) / 2
    return (centre_x - half_width) / half_width
