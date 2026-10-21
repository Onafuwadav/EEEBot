"""Symbol recognition (logbook section 8.3 and Figure 39).

Pipeline, following the flowchart:

1. Capture an image.
2. Convert to HSV and keep only the pink pixels (every symbol and its
   25 px border are the same pink).
3. ``findContours`` on that mask, take the biggest outline and simplify it
   to the four corners of the border.
4. Perspective-transform the border to a flat, square image.
5. Compare it with each reference PNG. If the best match is above the
   threshold, that symbol is reported; otherwise "no symbol".

Each symbol tells the robot which coloured line to follow next
(``SYMBOL_TO_COLOUR``), mirroring ``followLine(RGBYK::...)`` in the logbook.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import cv2
import numpy as np

# Geometry from the logbook snippet: symbolWidth/Height = 350, symbolBorder = 25.
SYMBOL_SIZE = 350
SYMBOL_BORDER = 25
WARPED_SIZE = SYMBOL_SIZE + 2 * SYMBOL_BORDER  # 400 x 400 including the border

# Pixels trimmed inside the border before comparing, so a slightly
# imperfect warp doesn't count border pixels as part of the symbol.
COMPARE_MARGIN = 15

# Minimum match score (intersection over union of the pink pixels).
DEFAULT_THRESHOLD = 0.70

# Pink in OpenCV HSV. Hue stops at 169 so a red line (hue 170-179) isn't
# mistaken for a symbol.
PINK_LOWER = (140, 60, 80)
PINK_UPPER = (169, 255, 255)

SYMBOL_TO_COLOUR = {
    "circle": "red",
    "star": "green",
    "triangle": "blue",
    "umbrella": "yellow",
}

REFERENCE_DIR = Path(__file__).resolve().parent.parent / "references"


@dataclass(frozen=True)
class Match:
    symbol: str
    score: float
    corners: np.ndarray

    @property
    def line_colour(self) -> str:
        return SYMBOL_TO_COLOUR[self.symbol]


def pink_mask(image_bgr: np.ndarray) -> np.ndarray:
    """Binary mask (0/255) of the pink symbol colour."""
    hsv = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, np.array(PINK_LOWER), np.array(PINK_UPPER))
    kernel = np.ones((3, 3), np.uint8)
    return cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)


def order_corners(points: np.ndarray) -> np.ndarray:
    """Sort four points into top-left, top-right, bottom-right, bottom-left."""
    pts = points.reshape(4, 2).astype(np.float32)
    sums = pts.sum(axis=1)
    diffs = np.diff(pts, axis=1).ravel()  # y - x
    return np.array(
        [
            pts[np.argmin(sums)],  # top-left has the smallest x + y
            pts[np.argmin(diffs)],  # top-right has the smallest y - x
            pts[np.argmax(sums)],  # bottom-right has the largest x + y
            pts[np.argmax(diffs)],  # bottom-left has the largest y - x
        ],
        dtype=np.float32,
    )


def find_symbol_corners(image_bgr: np.ndarray, min_area_fraction: float = 0.01) -> np.ndarray | None:
    """Find the four corners of the pink border, or ``None`` if there isn't one.

    The contours "may not be perfect and may have more than four corners"
    (logbook), so the outline is simplified with increasing tolerance until
    exactly four corners remain.
    """
    mask = pink_mask(image_bgr)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None

    border = max(contours, key=cv2.contourArea)
    image_area = image_bgr.shape[0] * image_bgr.shape[1]
    if cv2.contourArea(border) < min_area_fraction * image_area:
        return None

    hull = cv2.convexHull(border)
    perimeter = cv2.arcLength(hull, True)
    for tolerance in (0.02, 0.03, 0.04, 0.06, 0.08, 0.10):
        approx = cv2.approxPolyDP(hull, tolerance * perimeter, True)
        if len(approx) == 4:
            return order_corners(approx)
    return None


def warp_symbol(image_bgr: np.ndarray, corners: np.ndarray) -> np.ndarray:
    """Perspective-transform the bordered symbol to a flat WARPED_SIZE square."""
    target = np.array(
        [[0, 0], [WARPED_SIZE - 1, 0], [WARPED_SIZE - 1, WARPED_SIZE - 1], [0, WARPED_SIZE - 1]],
        dtype=np.float32,
    )
    matrix = cv2.getPerspectiveTransform(corners.astype(np.float32), target)
    return cv2.warpPerspective(image_bgr, matrix, (WARPED_SIZE, WARPED_SIZE))


def symbol_interior(binary: np.ndarray) -> np.ndarray:
    """The symbol area inside the border, as a boolean array."""
    inset = SYMBOL_BORDER + COMPARE_MARGIN
    return binary[inset:-inset, inset:-inset] > 0


def compare_images(candidate: np.ndarray, reference: np.ndarray) -> float:
    """Intersection over union of two boolean masks, in [0, 1].

    The candidate is tried at all four 90 degree rotations, since the robot
    may approach a symbol from any side of the square border.
    """
    best = 0.0
    for quarter_turns in range(4):
        rotated = np.rot90(candidate, quarter_turns)
        union = np.logical_or(rotated, reference).sum()
        if union == 0:
            continue
        best = max(best, float(np.logical_and(rotated, reference).sum() / union))
    return best


def load_references(directory: Path | str = REFERENCE_DIR) -> dict[str, np.ndarray]:
    """Load ``<symbol>.png`` for each symbol and return their interiors."""
    directory = Path(directory)
    references = {}
    for name in SYMBOL_TO_COLOUR:
        image = cv2.imread(str(directory / f"{name}.png"))
        if image is None:
            raise FileNotFoundError(directory / f"{name}.png")
        if image.shape[:2] != (WARPED_SIZE, WARPED_SIZE):
            image = cv2.resize(image, (WARPED_SIZE, WARPED_SIZE), interpolation=cv2.INTER_NEAREST)
        references[name] = symbol_interior(pink_mask(image))
    return references


def score_symbols(image_bgr: np.ndarray, references: dict[str, np.ndarray]) -> tuple[dict[str, float], np.ndarray | None]:
    """Score the image against every reference. Empty scores if no border is found."""
    corners = find_symbol_corners(image_bgr)
    if corners is None:
        return {}, None
    candidate = symbol_interior(pink_mask(warp_symbol(image_bgr, corners)))
    return {name: compare_images(candidate, ref) for name, ref in references.items()}, corners


def recognise_symbol(
    image_bgr: np.ndarray,
    references: dict[str, np.ndarray] | None = None,
    threshold: float = DEFAULT_THRESHOLD,
) -> Match | None:
    """Return the best-matching symbol, or ``None`` if nothing beats ``threshold``."""
    if references is None:
        references = load_references()
    scores, corners = score_symbols(image_bgr, references)
    if not scores:
        return None
    best = max(scores, key=scores.get)
    if scores[best] < threshold:
        return None
    return Match(symbol=best, score=scores[best], corners=corners)
