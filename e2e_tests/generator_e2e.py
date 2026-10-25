import argparse
import math
import subprocess
import sys
import tempfile
from collections.abc import Callable, Sequence
from pathlib import Path

from common import Case, run_cases

RUN_TIMEOUT_SECONDS: int = 60
VALID_ARGUMENTS: tuple[str, ...] = (
    "--count",
    "300",
    "--scene-size",
    "20",
    "--triangle-size",
    "0.5",
    "1.5",
)


def contains(text: str) -> Callable[[str], bool]:
    return lambda stderr: text in stderr


def generator_error(name: str, arguments: Sequence[str], message: str) -> Case:
    return Case(name, arguments, "", 2, "", contains(message))


def error_cases(generator: Path, output: Path) -> list[Case]:
    def with_script(*arguments: str) -> tuple[str, ...]:
        return (str(generator), *arguments)

    out = ("--output", str(output))
    return [
        generator_error(
            "missing output", with_script(*VALID_ARGUMENTS), "required: --output"
        ),
        generator_error(
            "zero count",
            with_script(
                "--count", "0", "--scene-size", "1", "--triangle-size", "1", *out
            ),
            "--count must be from 1 to 999999",
        ),
        generator_error(
            "count above limit",
            with_script(
                "--count", "1000000", "--scene-size", "1", "--triangle-size", "1", *out
            ),
            "--count must be from 1 to 999999",
        ),
        generator_error(
            "negative scene size",
            with_script(
                "--count", "1", "--scene-size", "-1", "--triangle-size", "1", *out
            ),
            "--scene-size must be a positive number",
        ),
        generator_error(
            "infinite scene size",
            with_script(
                "--count", "1", "--scene-size", "inf", "--triangle-size", "1", *out
            ),
            "--scene-size must be a positive number",
        ),
        generator_error(
            "three triangle sizes",
            with_script(
                "--count",
                "1",
                "--scene-size",
                "1",
                "--triangle-size",
                "1",
                "2",
                "3",
                *out,
            ),
            "--triangle-size takes one or two values",
        ),
        generator_error(
            "zero triangle size",
            with_script(
                "--count", "1", "--scene-size", "1", "--triangle-size", "0", *out
            ),
            "--triangle-size must be positive numbers",
        ),
        generator_error(
            "reversed triangle size range",
            with_script(
                "--count", "1", "--scene-size", "1", "--triangle-size", "2", "1", *out
            ),
            "--triangle-size range must be given as MIN MAX",
        ),
    ]


def generate(generator: Path, output: Path, *arguments: str) -> None:
    subprocess.run(
        [sys.executable, str(generator), *arguments, "--output", str(output)],
        timeout=RUN_TIMEOUT_SECONDS,
        check=True,
    )


def check_scene_format(scene: Path, count: int) -> list[str]:
    lines = scene.read_text(encoding="ascii").splitlines()
    problems: list[str] = []
    if lines[0] != str(count):
        problems.append(f"first line is {lines[0]!r}, expected {count}")
    if len(lines) != count + 1:
        problems.append(f"{len(lines)} lines, expected {count + 1}")
    for number, line in enumerate(lines[1:], start=1):
        values = [float(word) for word in line.split()]
        if len(values) != 9 or not all(math.isfinite(value) for value in values):
            problems.append(f"line {number} is not 9 finite numbers: {line!r}")
            break
    return problems


def check_generated_scenes(generator: Path, cli: Path, directory: Path) -> list[str]:
    first = directory / "first.in"
    second = directory / "second.in"
    other_seed = directory / "other_seed.in"
    generate(generator, first, *VALID_ARGUMENTS)
    generate(generator, second, *VALID_ARGUMENTS)
    generate(generator, other_seed, *VALID_ARGUMENTS, "--seed", "2")

    problems = check_scene_format(first, 300)
    if first.read_bytes() != second.read_bytes():
        problems.append("same arguments produced different scenes")
    if first.read_bytes() == other_seed.read_bytes():
        problems.append("different seeds produced the same scene")

    completed = subprocess.run(
        [str(cli)],
        input=first.read_text(encoding="ascii"),
        text=True,
        capture_output=True,
        timeout=RUN_TIMEOUT_SECONDS,
        check=False,
    )
    if completed.returncode != 0 or completed.stderr:
        problems.append(
            "triangles_cli rejected the scene: "
            f"{completed.returncode}, {completed.stderr!r}"
        )
    marked = [int(line) for line in completed.stdout.split()]
    if marked != sorted(set(marked)) or any(not 0 <= index < 300 for index in marked):
        problems.append(f"unexpected triangles_cli output: {completed.stdout!r}")
    return problems


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="End-to-end checks of generate_scene.py."
    )
    parser.add_argument(
        "--generator", type=Path, required=True, help="Path to generate_scene.py"
    )
    parser.add_argument("--cli", type=Path, required=True, help="Path to triangles_cli")
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    with tempfile.TemporaryDirectory() as directory_name:
        directory = Path(directory_name)
        result = run_cases(
            Path(sys.executable),
            "generator arguments",
            error_cases(args.generator, directory / "unused.in"),
        )
        problems = check_generated_scenes(args.generator, args.cli, directory)

    for problem in problems:
        print(f"FAILED: {problem}")
    print(f"generator scenes: {'failed' if problems else 'passed'}")
    return 1 if result or problems else 0


if __name__ == "__main__":
    sys.exit(main())
