"""Regenerate the reference PNGs and the sample images.

    python tools/make_assets.py

Writes references/<symbol>.png and samples/*.png. If you have the original
reference PNGs from the module, drop them into references/ instead: anything
square with the same 25 px border layout works.
"""

from __future__ import annotations

import sys
from pathlib import Path

import cv2
import numpy as np

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from eeebot_vision.symbols import SYMBOL_TO_COLOUR  # noqa: E402
from eeebot_vision.synthetic import draw_reference, empty_scene, line_scene, symbol_scene  # noqa: E402


def main() -> None:
    refs = ROOT / "references"
    samples = ROOT / "samples"
    refs.mkdir(exist_ok=True)
    samples.mkdir(exist_ok=True)

    for i, symbol in enumerate(SYMBOL_TO_COLOUR):
        cv2.imwrite(str(refs / f"{symbol}.png"), draw_reference(symbol))
        cv2.imwrite(str(samples / f"symbol_{symbol}.png"), symbol_scene(symbol, quarter_turns=i % 4, seed=i))

    # a frame with no symbol in it, just floor
    cv2.imwrite(str(samples / "symbol_none.png"), empty_scene(seed=9))

    for colour, x, slant, name in [
        ("red", 140, 60, "left"),
        ("green", 320, 0, "centre"),
        ("blue", 500, -80, "right"),
        ("yellow", 260, 120, "left_bend"),
    ]:
        cv2.imwrite(str(samples / f"line_{colour}_{name}.png"), line_scene(colour, x, slant=slant, seed=x))

    print(f"wrote {len(list(refs.glob('*.png')))} references and {len(list(samples.glob('*.png')))} samples")


if __name__ == "__main__":
    np.set_printoptions(precision=2)
    main()
