"""EEEBot computer vision: symbol recognition and coloured-line detection.

Rebuilt in Python/OpenCV from the 2023 EEEE1002 logbook (sections 8.1 and 8.3,
Figure 39). No machine learning: HSV thresholding, contours, a perspective
warp and template comparison.
"""

from .colour_line import LINE_COLOURS, colour_mask, line_position
from .symbols import (
    SYMBOL_TO_COLOUR,
    Match,
    find_symbol_corners,
    load_references,
    recognise_symbol,
)

__all__ = [
    "LINE_COLOURS",
    "SYMBOL_TO_COLOUR",
    "Match",
    "colour_mask",
    "find_symbol_corners",
    "line_position",
    "load_references",
    "recognise_symbol",
]
