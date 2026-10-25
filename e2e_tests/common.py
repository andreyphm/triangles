import subprocess
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from pathlib import Path

RUN_TIMEOUT_SECONDS: int = 60

Point = tuple[float, float, float]
Triangle = tuple[Point, Point, Point]


@dataclass
class Case:
    name: str
    arguments: Sequence[str]
    input_text: str
    expected_code: int
    expected_stdout: str
    expected_stderr: str | Callable[[str], bool] = ""


@dataclass
class Failure:
    case: Case
    code: int
    stdout: str
    stderr: str


def triangle(*coordinates: float) -> Triangle:
    if len(coordinates) != 9:
        raise ValueError("a triangle needs 9 coordinates")
    return (
        (coordinates[0], coordinates[1], coordinates[2]),
        (coordinates[3], coordinates[4], coordinates[5]),
        (coordinates[6], coordinates[7], coordinates[8]),
    )


def point(x: float, y: float, z: float) -> Triangle:
    return ((x, y, z), (x, y, z), (x, y, z))


def segment(first: Point, second: Point) -> Triangle:
    return (first, second, second)


def shifted_point(vertex: Point, x: float, y: float, z: float) -> Point:
    return (vertex[0] + x, vertex[1] + y, vertex[2] + z)


def scaled_point(vertex: Point, factor: float) -> Point:
    return (vertex[0] * factor, vertex[1] * factor, vertex[2] * factor)


def translated(shape: Triangle, x: float, y: float, z: float) -> Triangle:
    first, second, third = shape
    return (
        shifted_point(first, x, y, z),
        shifted_point(second, x, y, z),
        shifted_point(third, x, y, z),
    )


def scaled(shape: Triangle, factor: float) -> Triangle:
    first, second, third = shape
    return (
        scaled_point(first, factor),
        scaled_point(second, factor),
        scaled_point(third, factor),
    )


def reversed_winding(shape: Triangle) -> Triangle:
    return (shape[0], shape[2], shape[1])


def encode_triangles(triangles: Sequence[Triangle]) -> str:
    lines = [str(len(triangles))]
    for shape in triangles:
        lines.append(
            "  ".join(
                " ".join(repr(coordinate) for coordinate in vertex) for vertex in shape
            )
        )
    return "\n".join(lines) + "\n"


def encode_marked(flags: Sequence[bool]) -> str:
    return "".join(f"{index}\n" for index, flag in enumerate(flags) if flag)


def run_case(binary: Path, case: Case) -> Failure | None:
    completed = subprocess.run(
        [str(binary), *case.arguments],
        input=case.input_text,
        text=True,
        capture_output=True,
        timeout=RUN_TIMEOUT_SECONDS,
        check=False,
    )
    if isinstance(case.expected_stderr, str):
        stderr_matches = completed.stderr == case.expected_stderr
    else:
        stderr_matches = case.expected_stderr(completed.stderr)

    if (
        completed.returncode == case.expected_code
        and completed.stdout == case.expected_stdout
        and stderr_matches
    ):
        return None
    return Failure(case, completed.returncode, completed.stdout, completed.stderr)


def shorten(text: str, limit: int = 400) -> str:
    return (
        text if len(text) <= limit else text[:limit] + f"... ({len(text)} characters)"
    )


def run_cases(binary: Path, group: str, cases: Sequence[Case]) -> int:
    failures = [
        failure for case in cases if (failure := run_case(binary, case)) is not None
    ]
    for failure in failures:
        print(f"FAILED: {failure.case.name}")
        print(f"  arguments: {list(failure.case.arguments)}")
        print(f"  expected code {failure.case.expected_code}, got {failure.code}")
        print(f"  expected stdout: {shorten(failure.case.expected_stdout)!r}")
        print(f"  actual stdout:   {shorten(failure.stdout)!r}")
        print(f"  actual stderr:   {shorten(failure.stderr)!r}")
        print(f"  input: {shorten(failure.case.input_text)!r}")

    print(f"{group}: {len(cases) - len(failures)} of {len(cases)} scenarios passed")
    return 1 if failures else 0
