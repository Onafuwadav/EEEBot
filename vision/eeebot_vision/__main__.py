"""Command line: run the vision pipeline on image files.

    python -m eeebot_vision symbol samples/symbol_star.png
    python -m eeebot_vision line samples/line_red_left.png --colour red
    python -m eeebot_vision symbol samples/*.png --annotate out/
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import cv2
import numpy as np

from .colour_line import LINE_COLOURS, line_position
from .symbols import DEFAULT_THRESHOLD, load_references, recognise_symbol, score_symbols


def _read(path: str) -> np.ndarray:
    image = cv2.imread(path)
    if image is None:
        sys.exit(f"could not read image: {path}")
    return image


def cmd_symbol(args: argparse.Namespace) -> None:
    references = load_references()
    for path in args.images:
        image = _read(path)
        scores, _ = score_symbols(image, references)
        match = recognise_symbol(image, references, args.threshold)
        if match:
            print(f"{path}: {match.symbol} (score {match.score:.2f}) -> follow {match.line_colour} line")
        else:
            print(f"{path}: no symbol detected")
        if args.verbose and scores:
            print("    " + "  ".join(f"{k}={v:.2f}" for k, v in sorted(scores.items())))
        if args.annotate and match:
            out_dir = Path(args.annotate)
            out_dir.mkdir(parents=True, exist_ok=True)
            cv2.polylines(image, [match.corners.astype(np.int32)], True, (0, 255, 0), 3)
            cv2.putText(image, match.symbol, (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 3)
            cv2.imwrite(str(out_dir / Path(path).name), image)


def cmd_line(args: argparse.Namespace) -> None:
    for path in args.images:
        error = line_position(_read(path), args.colour)
        if error is None:
            print(f"{path}: no {args.colour} line found")
        else:
            side = "centre" if abs(error) < 0.05 else ("left" if error < 0 else "right")
            print(f"{path}: {args.colour} line error {error:+.2f} ({side})")


def main(argv: list[str] | None = None) -> None:
    parser = argparse.ArgumentParser(prog="eeebot_vision", description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)

    p_symbol = sub.add_parser("symbol", help="recognise the pink symbol in each image")
    p_symbol.add_argument("images", nargs="+")
    p_symbol.add_argument("--threshold", type=float, default=DEFAULT_THRESHOLD)
    p_symbol.add_argument("--annotate", metavar="DIR", help="save images with the detected border drawn")
    p_symbol.add_argument("-v", "--verbose", action="store_true", help="print every symbol's score")
    p_symbol.set_defaults(func=cmd_symbol)

    p_line = sub.add_parser("line", help="locate a coloured line in each image")
    p_line.add_argument("images", nargs="+")
    p_line.add_argument("--colour", choices=sorted(LINE_COLOURS), required=True)
    p_line.set_defaults(func=cmd_line)

    args = parser.parse_args(argv)
    args.func(args)


if __name__ == "__main__":
    main()
