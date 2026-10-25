import argparse
import sys
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from pathlib import Path

from common import (
    Case,
    Triangle,
    encode_marked,
    encode_triangles,
    point,
    reversed_winding,
    run_cases,
    scaled,
    segment,
    translated,
    triangle,
)

BASE: Triangle = triangle(0, 0, 0, 2, 0, 0, 0, 2, 0)
INNER: Triangle = triangle(0.25, 0.25, 0, 0.75, 0.25, 0, 0.25, 0.75, 0)
COPLANAR_CROSSING: Triangle = triangle(1, -0.5, 0, 1, 2.5, 0, 1.5, 1, 0)
COPLANAR_OUTSIDE: Triangle = triangle(1.25, 1.25, 0, 3, 1.25, 0, 1.25, 3, 0)
SHARED_EDGE: Triangle = triangle(0, 0, 0, 2, 0, 0, 1, -1, 0)
SHARED_VERTEX: Triangle = triangle(2, 0, 0, 3, 0, 0, 2, 1, 0)
VERTEX_ON_EDGE: Triangle = triangle(1, 0, 0, 1, -1, 0, 2, -1, 0)
VERTEX_ON_FACE: Triangle = triangle(0.5, 0.5, 0, 0.5, 0.5, 1, 1, 0.5, 1)
VERTICAL: Triangle = triangle(0.5, 0.5, -1, 0.5, 0.5, 1, 0.5, 1.5, 0)
CROSSING_EDGE: Triangle = triangle(1, -1, -1, 1, 1, 1, 1, 1, -1)
ABOVE: Triangle = translated(BASE, 0, 0, 1)
FAR: Triangle = translated(BASE, 100, 100, 100)
SMALL_GAP: Triangle = triangle(2.0009765625, 0, 0, 3, 0, 0, 2.0009765625, 1, 0)


@dataclass
class GeometryCase:
    name: str
    triangles: Sequence[Triangle]
    expected: Sequence[bool]


def geometry_case(case: GeometryCase) -> Case:
    return Case(
        case.name,
        (),
        encode_triangles(case.triangles),
        0,
        encode_marked(case.expected),
    )


GEOMETRY_CASES: tuple[GeometryCase, ...] = (
    GeometryCase("single triangle", (BASE,), (False,)),
    GeometryCase("identical triangles", (BASE, BASE), (True, True)),
    GeometryCase("containment", (BASE, INNER), (True, True)),
    GeometryCase("coplanar edges crossing", (BASE, COPLANAR_CROSSING), (True, True)),
    GeometryCase(
        "coplanar bounds overlap without contact",
        (BASE, COPLANAR_OUTSIDE),
        (False, False),
    ),
    GeometryCase("shared edge", (BASE, SHARED_EDGE), (True, True)),
    GeometryCase("shared vertex", (BASE, SHARED_VERTEX), (True, True)),
    GeometryCase("vertex on edge interior", (BASE, VERTEX_ON_EDGE), (True, True)),
    GeometryCase("vertex on face of other plane", (BASE, VERTEX_ON_FACE), (True, True)),
    GeometryCase("non-coplanar piercing", (BASE, VERTICAL), (True, True)),
    GeometryCase("non-coplanar crossing an edge", (BASE, CROSSING_EDGE), (True, True)),
    GeometryCase("parallel planes", (BASE, ABOVE), (False, False)),
    GeometryCase("far apart", (BASE, FAR), (False, False)),
    GeometryCase("small positive gap", (BASE, SMALL_GAP), (False, False)),
    GeometryCase("reversed winding", (BASE, reversed_winding(BASE)), (True, True)),
    GeometryCase("mixed flags", (BASE, VERTICAL, FAR), (True, True, False)),
    GeometryCase(
        "chain of touching triangles",
        (BASE, SHARED_VERTEX, translated(SHARED_VERTEX, 1, 0, 0), FAR),
        (True, True, True, False),
    ),
    GeometryCase(
        "independent groups",
        (BASE, INNER, translated(BASE, 20, 0, 0), translated(INNER, 20, 0, 0), FAR),
        (True, True, True, True, False),
    ),
    GeometryCase(
        "sparse input order",
        (
            translated(INNER, 1000, 0, 0),
            translated(BASE, -1000, 0, 0),
            BASE,
            translated(BASE, 1000, 0, 0),
            INNER,
        ),
        (True, False, True, True, True),
    ),
)

DEGENERATE_CASES: tuple[GeometryCase, ...] = (
    GeometryCase("equal points", (point(1, 1, 1), point(1, 1, 1)), (True, True)),
    GeometryCase(
        "different points", (point(1, 1, 1), point(1, 1, 1.125)), (False, False)
    ),
    GeometryCase("point inside triangle", (BASE, point(0.5, 0.5, 0)), (True, True)),
    GeometryCase("point on triangle edge", (BASE, point(1, 1, 0)), (True, True)),
    GeometryCase("point at triangle vertex", (BASE, point(2, 0, 0)), (True, True)),
    GeometryCase(
        "point outside triangle in its plane", (BASE, point(2, 2, 0)), (False, False)
    ),
    GeometryCase("point above triangle", (BASE, point(0.5, 0.5, 0.25)), (False, False)),
    GeometryCase(
        "point inside segment",
        (segment((0, 0, 0), (2, 2, 2)), point(1, 1, 1)),
        (True, True),
    ),
    GeometryCase(
        "point on segment line beyond its end",
        (segment((0, 0, 0), (2, 2, 2)), point(3, 3, 3)),
        (False, False),
    ),
    GeometryCase(
        "segment pierces triangle",
        (BASE, segment((0.5, 0.5, -1), (0.5, 0.5, 1))),
        (True, True),
    ),
    GeometryCase(
        "segment ends on triangle face",
        (BASE, segment((0.5, 0.5, 0), (0.5, 0.5, 1))),
        (True, True),
    ),
    GeometryCase(
        "coplanar segment crosses triangle",
        (BASE, segment((-1, 1, 0), (2, 1, 0))),
        (True, True),
    ),
    GeometryCase(
        "coplanar segment inside triangle",
        (BASE, segment((0.25, 0.25, 0), (0.5, 0.25, 0))),
        (True, True),
    ),
    GeometryCase(
        "coplanar segment outside triangle",
        (BASE, segment((3, 0, 0), (3, 3, 0))),
        (False, False),
    ),
    GeometryCase(
        "segment misses triangle",
        (BASE, segment((3, 3, -1), (3, 3, 1))),
        (False, False),
    ),
    GeometryCase(
        "crossing segments",
        (segment((-1, 0, 0), (1, 0, 0)), segment((0, -1, 0), (0, 1, 0))),
        (True, True),
    ),
    GeometryCase(
        "collinear overlapping and separate segments",
        (
            segment((0, 0, 0), (2, 0, 0)),
            segment((1, 0, 0), (3, 0, 0)),
            segment((4, 0, 0), (5, 0, 0)),
        ),
        (True, True, False),
    ),
    GeometryCase(
        "parallel segments on different lines",
        (segment((0, 0, 0), (2, 0, 0)), segment((0, 1, 0), (2, 1, 0))),
        (False, False),
    ),
    GeometryCase(
        "skew segments",
        (segment((0, 0, 0), (1, 0, 0)), segment((0.5, -1, 1), (0.5, 1, 1))),
        (False, False),
    ),
    GeometryCase(
        "three distinct collinear vertices act as segment",
        (triangle(-1, 1, 0, 1, 1, 0, 3, 1, 0), BASE),
        (True, True),
    ),
)

SCALES: tuple[tuple[str, float], ...] = (
    ("scaled by 2^-300", 2.0**-300),
    ("scaled by 2^-20", 2.0**-20),
    ("scaled by 2^20", 2.0**20),
    ("scaled by 2^300", 2.0**300),
)
SHIFT: float = 2.0**20


def transformed_cases(cases: Sequence[GeometryCase]) -> list[GeometryCase]:
    result: list[GeometryCase] = []
    for case in cases:
        for suffix, factor in SCALES:
            shapes = tuple(scaled(shape, factor) for shape in case.triangles)
            result.append(GeometryCase(f"{case.name}, {suffix}", shapes, case.expected))
        shapes = tuple(
            translated(shape, SHIFT, -SHIFT, SHIFT) for shape in case.triangles
        )
        result.append(
            GeometryCase(f"{case.name}, shifted by 2^20", shapes, case.expected)
        )
    return result


def lattice_triangle(x: float, y: float, z: float) -> Triangle:
    return triangle(x, y, z, x + 0.5, y, z, x, y + 0.5, z)


def crossing_triangle(x: float, y: float, z: float) -> Triangle:
    return triangle(
        x + 0.125,
        y + 0.125,
        z - 0.25,
        x + 0.125,
        y + 0.125,
        z + 0.25,
        x + 0.25,
        y + 0.25,
        z + 0.25,
    )


def separate_lattice() -> GeometryCase:
    shapes = [
        lattice_triangle(x, y, z)
        for x in range(20)
        for y in range(20)
        for z in range(20)
    ]
    return GeometryCase(
        "lattice of 8000 separate triangles", shapes, [False] * len(shapes)
    )


def crossed_lattice() -> GeometryCase:
    shapes: list[Triangle] = []
    expected: list[bool] = []
    for x in range(12):
        for y in range(12):
            for z in range(12):
                crossed = (x + y + z) % 3 == 0
                shapes.append(lattice_triangle(x, y, z))
                expected.append(crossed)
                if crossed:
                    shapes.append(crossing_triangle(x, y, z))
                    expected.append(True)
    return GeometryCase("lattice with every third triangle crossed", shapes, expected)


def touching_chain() -> GeometryCase:
    shapes = [
        triangle(2 * index, 0, 0, 2 * index + 2, 0, 0, 2 * index, 1, 0)
        for index in range(1000)
    ]
    return GeometryCase(
        "chain of 1000 triangles touching by vertices", shapes, [True] * len(shapes)
    )


def huge_triangle_among_small() -> GeometryCase:
    column = 25
    shapes: list[Triangle] = []
    expected: list[bool] = []
    for x in range(50):
        for y in range(50):
            shapes.append(lattice_triangle(x, y, 0))
            expected.append(x == column)
    plane_x = column + 0.125
    shapes.append(triangle(plane_x, -10, -1, plane_x, 200, -1, plane_x, -10, 1))
    expected.append(True)
    return GeometryCase(
        "one huge triangle crossing a column of small ones", shapes, expected
    )


def copies_of_one_triangle() -> GeometryCase:
    shapes = [BASE] * 500
    return GeometryCase("500 copies of one triangle", shapes, [True] * len(shapes))


def distant_clusters() -> GeometryCase:
    far = 2.0**300
    size = 2.0**290
    shapes = (
        translated(scaled(BASE, size), far, far, far),
        translated(scaled(INNER, size), far, far, far),
        translated(scaled(BASE, size), -far, -far, -far),
        translated(scaled(FAR, size), -far, -far, -far),
    )
    return GeometryCase(
        "clusters at opposite ends of coordinate range",
        shapes,
        (True, True, False, False),
    )


def search_cases() -> list[GeometryCase]:
    return [
        separate_lattice(),
        crossed_lattice(),
        touching_chain(),
        huge_triangle_among_small(),
        copies_of_one_triangle(),
        distant_clusters(),
    ]


def error_case(name: str, input_text: str, message: str) -> Case:
    return Case(name, (), input_text, 1, "", f"error: {message}\n")


NO_INTERSECTIONS: str = ""
COUNT_READ_FAILED: str = "failed to read triangle count"
COUNT_OUT_OF_RANGE: str = "triangle count must satisfy 0 < N < 1000000"


def input_cases() -> list[Case]:
    return [
        Case(
            "windows line endings, tabs and arbitrary line breaks",
            (),
            "2\r\n0\t0\r\n0  2 0 0\r\n0 2 0\r\n\t0 0 0 1 0 0\r\n0 1 0\r\n",
            0,
            "0\n1\n",
        ),
        Case(
            "leading plus signs",
            (),
            "+1\n+0 0 0  +2 0 0  0 +2 0\n",
            0,
            NO_INTERSECTIONS,
        ),
        Case(
            "exponent notation in coordinates",
            (),
            "1\n0 0 0  2e0 0 0  0 0.2e1 0\n",
            0,
            NO_INTERSECTIONS,
        ),
        Case(
            "data after last triangle is ignored",
            (),
            "1\n0 0 0  2 0 0  0 2 0\nanything 42\n",
            0,
            NO_INTERSECTIONS,
        ),
        error_case("empty input", "", COUNT_READ_FAILED),
        error_case("only whitespace", " \n\t\r\n", COUNT_READ_FAILED),
        error_case("non-numeric count", "triangles\n", COUNT_READ_FAILED),
        error_case("fractional count", "1.5\n0 0 0 1 0 0 0 1 0\n", COUNT_READ_FAILED),
        error_case(
            "count in exponent notation", "1e0\n0 0 0 1 0 0 0 1 0\n", COUNT_READ_FAILED
        ),
        error_case(
            "count glued to letters", "1x\n0 0 0 1 0 0 0 1 0\n", COUNT_READ_FAILED
        ),
        error_case("zero count", "0\n", COUNT_OUT_OF_RANGE),
        error_case("negative count", "-1\n", COUNT_OUT_OF_RANGE),
        error_case("count equal to limit", "1000000\n", COUNT_OUT_OF_RANGE),
        error_case(
            "count above 64-bit range", "99999999999999999999\n", COUNT_READ_FAILED
        ),
        error_case("missing first triangle", "1\n", "failed to read triangle 1"),
        error_case(
            "truncated second triangle",
            "2\n0 0 0 1 0 0 0 1 0\n5 5 5 6 5\n",
            "failed to read triangle 2",
        ),
        error_case(
            "non-numeric coordinate",
            "1\n0 0 0 1 x 0 0 1 0\n",
            "failed to read triangle 1",
        ),
        error_case(
            "coordinate with trailing letters",
            "1\n0 0 0 1 0 0 0 1 0x\n",
            "failed to read triangle 1",
        ),
        error_case(
            "double sign", "1\n0 0 0 +-1 0 0 0 1 0\n", "failed to read triangle 1"
        ),
        error_case(
            "nan coordinate", "1\n0 0 0 1 0 0 0 1 nan\n", "failed to read triangle 1"
        ),
        error_case(
            "infinite coordinate",
            "1\n0 0 0 1 0 0 0 1 inf\n",
            "failed to read triangle 1",
        ),
        error_case(
            "coordinate overflowing double",
            "1\n0 0 0 1 0 0 0 1 1e400\n",
            "failed to read triangle 1",
        ),
    ]


GROUPS: dict[str, Callable[[], list[Case]]] = {
    "geometry": lambda: [geometry_case(case) for case in GEOMETRY_CASES],
    "degenerate": lambda: [geometry_case(case) for case in DEGENERATE_CASES],
    "scale": lambda: [
        geometry_case(case)
        for case in transformed_cases(GEOMETRY_CASES + DEGENERATE_CASES)
    ],
    "search": lambda: [geometry_case(case) for case in search_cases()],
    "input": input_cases,
}


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="End-to-end scenarios for triangles_cli."
    )
    parser.add_argument(
        "--binary", type=Path, required=True, help="Path to triangles_cli"
    )
    parser.add_argument(
        "--group",
        choices=sorted(GROUPS),
        required=True,
        help="Group of scenarios to run",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    return run_cases(args.binary, args.group, GROUPS[args.group]())


if __name__ == "__main__":
    sys.exit(main())
