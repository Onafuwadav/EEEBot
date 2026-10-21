"""Draw the reference symbols and fake camera frames.

The original reference PNGs and test photos aren't in the logbook, so these
helpers recreate them: the four symbols in pink inside a 25 px pink border,
and scenes that place a symbol on a floor at an angle, or draw a coloured
line, with noise and uneven lighting. Used by ``tools/make_assets.py`` and
the tests.
"""

from __future__ import annotations

import cv2
import numpy as np

from .symbols import SYMBOL_BORDER, WARPED_SIZE

PINK_BGR = (180, 105, 255)  # hot pink
WHITE_BGR = (255, 255, 255)
FLOOR_BGR = (150, 160, 155)  # grey lab floor

LINE_BGR = {
    "red": (40, 40, 220),
    "green": (60, 170, 60),
    "blue": (200, 90, 30),
    "yellow": (40, 220, 235),
    "black": (25, 25, 25),
}


def draw_reference(symbol: str) -> np.ndarray:
    """A WARPED_SIZE square reference image: pink border, pink symbol, white inside."""
    size = WARPED_SIZE
    img = np.full((size, size, 3), PINK_BGR, dtype=np.uint8)
    inner = SYMBOL_BORDER
    img[inner:-inner, inner:-inner] = WHITE_BGR
    c = size // 2

    if symbol == "circle":
        cv2.circle(img, (c, c), 120, PINK_BGR, -1, cv2.LINE_AA)
    elif symbol == "triangle":
        pts = np.array([[c, 70], [330, 315], [70, 315]], np.int32)
        cv2.fillPoly(img, [pts], PINK_BGR, cv2.LINE_AA)
    elif symbol == "star":
        outer, inner_r = 140, 56
        pts = []
        for i in range(10):
            r = outer if i % 2 == 0 else inner_r
            a = -np.pi / 2 + i * np.pi / 5
            pts.append([c + r * np.cos(a), c + 15 + r * np.sin(a)])
        cv2.fillPoly(img, [np.array(pts, np.int32)], PINK_BGR, cv2.LINE_AA)
    elif symbol == "umbrella":
        # canopy: upper half of an ellipse with a scalloped bottom edge
        cv2.ellipse(img, (c, 190), (140, 115), 0, 180, 360, PINK_BGR, -1, cv2.LINE_AA)
        for x in (c - 105, c - 35, c + 35, c + 105):
            cv2.circle(img, (x, 190), 35, WHITE_BGR, -1, cv2.LINE_AA)
        cv2.rectangle(img, (c - 140, 170), (c + 140, 190), PINK_BGR, -1)
        # shaft and hooked handle
        cv2.rectangle(img, (c - 9, 185), (c + 9, 300), PINK_BGR, -1)
        cv2.ellipse(img, (c - 30, 300), (39, 39), 0, 0, 180, PINK_BGR, 18, cv2.LINE_AA)
    else:
        raise ValueError(f"unknown symbol {symbol!r}")
    return img


def _finish(scene: np.ndarray, rng: np.random.Generator, noise: float, gradient: float) -> np.ndarray:
    """Add sensor noise and a left-to-right lighting gradient."""
    h, w = scene.shape[:2]
    light = np.linspace(1.0 - gradient, 1.0, w, dtype=np.float32)[None, :, None]
    out = scene.astype(np.float32) * light
    out += rng.normal(0, noise, out.shape)
    return np.clip(out, 0, 255).astype(np.uint8)


def symbol_scene(
    symbol: str,
    *,
    size: tuple[int, int] = (640, 480),
    corners: np.ndarray | None = None,
    quarter_turns: int = 0,
    noise: float = 6.0,
    gradient: float = 0.25,
    seed: int = 0,
) -> np.ndarray:
    """A camera-like frame with ``symbol`` lying on the floor in perspective."""
    rng = np.random.default_rng(seed)
    width, height = size
    if corners is None:
        # a trapezoid, as if the camera is looking down at a sheet on the floor
        corners = np.array([[200, 110], [450, 120], [500, 400], [150, 385]], np.float32)
    ref = np.rot90(draw_reference(symbol), quarter_turns).copy()
    src = np.array(
        [[0, 0], [WARPED_SIZE - 1, 0], [WARPED_SIZE - 1, WARPED_SIZE - 1], [0, WARPED_SIZE - 1]],
        np.float32,
    )
    matrix = cv2.getPerspectiveTransform(src, corners.astype(np.float32))
    floor = np.full((height, width, 3), FLOOR_BGR, np.uint8)
    warped = cv2.warpPerspective(ref, matrix, (width, height))
    covered = cv2.warpPerspective(np.full(ref.shape[:2], 255, np.uint8), matrix, (width, height)) > 0
    floor[covered] = warped[covered]
    return _finish(floor, rng, noise, gradient)


def empty_scene(*, size: tuple[int, int] = (640, 480), noise: float = 6.0, gradient: float = 0.25, seed: int = 0) -> np.ndarray:
    """Bare floor: no symbol, no line."""
    width, height = size
    floor = np.full((height, width, 3), FLOOR_BGR, np.uint8)
    return _finish(floor, np.random.default_rng(seed), noise, gradient)


def line_scene(
    colour: str,
    centre_x: float,
    *,
    size: tuple[int, int] = (640, 480),
    thickness: int = 40,
    slant: float = 0.0,
    noise: float = 6.0,
    gradient: float = 0.25,
    seed: int = 0,
) -> np.ndarray:
    """A frame with a ``colour`` tape line whose bottom end is at ``centre_x`` pixels.

    ``slant`` moves the top end sideways by that many pixels, like a bend ahead.
    """
    rng = np.random.default_rng(seed)
    width, height = size
    floor = np.full((height, width, 3), FLOOR_BGR, np.uint8)
    bottom = (int(centre_x), height)
    top = (int(centre_x + slant), 0)
    cv2.line(floor, bottom, top, LINE_BGR[colour], thickness)
    return _finish(floor, rng, noise, gradient)
