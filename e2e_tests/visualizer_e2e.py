import argparse
import sys
from collections.abc import Sequence
from pathlib import Path

from common import Case, run_cases


def is_usage(stderr: str) -> bool:
    lines = stderr.splitlines()
    return (
        len(lines) == 2
        and lines[0].startswith("usage: ")
        and lines[0].endswith(" [--theme NAME] < input")
        and lines[1].startswith("themes: ocean")
    )


def usage_case(name: str, arguments: Sequence[str]) -> Case:
    return Case(name, arguments, "", 1, "", is_usage)


CASES: tuple[Case, ...] = (
    usage_case("unknown theme", ("--theme", "no-such-theme")),
    usage_case("theme option without value", ("--theme",)),
    usage_case("extra argument after theme", ("--theme", "ocean", "extra")),
    usage_case("unknown option", ("--bogus",)),
    usage_case("theme name without option", ("ocean",)),
)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Argument checks of triangles.out before a window opens."
    )
    parser.add_argument(
        "--binary", type=Path, required=True, help="Path to triangles.out"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    return run_cases(args.binary, "visualizer arguments", CASES)


if __name__ == "__main__":
    sys.exit(main())
