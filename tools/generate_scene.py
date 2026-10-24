import argparse
import math
import random
from dataclasses import dataclass
from pathlib import Path

MAX_TRIANGLE_COUNT: int = 999_999
AXIS_COUNT: int = 3
VERTICES_PER_TRIANGLE: int = 3

Vertex = list[float]
Triangle = list[Vertex]


@dataclass
class SceneParameters:
    count: int
    scene_size: float
    min_triangle_size: float
    max_triangle_size: float
    seed: int
    output: Path


def is_positive_number(value: float) -> bool:
    return math.isfinite(value) and value > 0


def validate_arguments(
    parser: argparse.ArgumentParser, args: argparse.Namespace
) -> None:
    if not 0 < args.count <= MAX_TRIANGLE_COUNT:
        parser.error(f"--count must be from 1 to {MAX_TRIANGLE_COUNT}")
    if not is_positive_number(args.scene_size):
        parser.error("--scene-size must be a positive number")
    if len(args.triangle_size) > 2:
        parser.error("--triangle-size takes one or two values")
    if not all(is_positive_number(size) for size in args.triangle_size):
        parser.error("--triangle-size must be positive numbers")
    if args.triangle_size[0] > args.triangle_size[-1]:
        parser.error("--triangle-size range must be given as MIN MAX")


def parse_arguments() -> SceneParameters:
    parser = argparse.ArgumentParser(
        description="Generate a scene of randomly placed triangles for Triangles."
    )
    parser.add_argument(
        "--count",
        type=int,
        required=True,
        help=f"Number of triangles, from 1 to {MAX_TRIANGLE_COUNT}",
    )
    parser.add_argument(
        "--scene-size",
        type=float,
        required=True,
        help="Edge of the cube [0, SIZE]^3 that contains the triangle centers",
    )
    parser.add_argument(
        "--triangle-size",
        type=float,
        nargs="+",
        required=True,
        metavar="SIZE",
        help="One size or a MIN MAX range: every vertex lies within SIZE "
        "of the triangle center along each axis",
    )
    parser.add_argument(
        "--seed", type=int, default=1, help="Seed of the random generator (default: 1)"
    )
    parser.add_argument(
        "--output", type=Path, required=True, help="File to write the scene to"
    )

    args: argparse.Namespace = parser.parse_args()
    validate_arguments(parser, args)
    return SceneParameters(
        count=args.count,
        scene_size=args.scene_size,
        min_triangle_size=args.triangle_size[0],
        max_triangle_size=args.triangle_size[-1],
        seed=args.seed,
        output=args.output,
    )


def random_triangle(generator: random.Random, parameters: SceneParameters) -> Triangle:
    center: Vertex = [
        generator.uniform(0.0, parameters.scene_size) for _ in range(AXIS_COUNT)
    ]
    size: float = generator.uniform(
        parameters.min_triangle_size, parameters.max_triangle_size
    )
    return [
        [coordinate + generator.uniform(-size, size) for coordinate in center]
        for _ in range(VERTICES_PER_TRIANGLE)
    ]


def format_triangle(triangle: Triangle) -> str:
    return "  ".join(
        " ".join(repr(coordinate) for coordinate in vertex) for vertex in triangle
    )


def write_scene(parameters: SceneParameters) -> None:
    generator = random.Random(parameters.seed)

    with open(parameters.output, "w", encoding="ascii", newline="\n") as file:
        file.write(f"{parameters.count}\n")
        for _ in range(parameters.count):
            file.write(format_triangle(random_triangle(generator, parameters)) + "\n")


def main() -> None:
    write_scene(parse_arguments())


if __name__ == "__main__":
    main()
