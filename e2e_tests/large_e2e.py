import argparse
import math
import random
import sys
from collections.abc import Callable, Sequence
from dataclasses import dataclass
from pathlib import Path

from common import Case, Point, Triangle, encode_marked, encode_triangles, run_cases

RUN_TIMEOUT_SECONDS: int = 240
CELL_LOW: float = 0.05
CELL_HIGH: float = 0.95
MIN_SHAPE_QUALITY: float = 0.1
EXACT_STEP: float = 2.0**-9
FINE_STEP: float = 2.0**-30
TILTED_STEP: float = 2.0**-31
TILT_SLOPES: tuple[float, ...] = (-0.5, -0.25, 0.25, 0.5)
OFFSET_LIMIT: int = 2**16
HUGE_PLANE_HEIGHT: float = 0.5
PIERCER_CELL_COUNT: int = 300

Flat = tuple[float, float]
FlatShape = tuple[Flat, Flat, Flat]
Item = tuple[Triangle, bool]
FlatItem = tuple[FlatShape, bool]
Motif = Callable[[random.Random], list[Item]]
FlatMotif = Callable[[random.Random], list[FlatItem]]


@dataclass(frozen=True)
class SceneSpec:
    count: int
    seed: int
    motifs: tuple[str, ...]
    scale_exponents: tuple[int, int]
    huge_plane: bool = False


@dataclass(frozen=True)
class Placement:
    scale: float
    offset: tuple[int, int, int]
    axes: tuple[int, int, int]


def add(first: Point, second: Point) -> Point:
    return (first[0] + second[0], first[1] + second[1], first[2] + second[2])


def subtract(first: Point, second: Point) -> Point:
    return (first[0] - second[0], first[1] - second[1], first[2] - second[2])


def multiply(vector: Point, factor: float) -> Point:
    return (vector[0] * factor, vector[1] * factor, vector[2] * factor)


def cross(first: Point, second: Point) -> Point:
    return (
        first[1] * second[2] - first[2] * second[1],
        first[2] * second[0] - first[0] * second[2],
        first[0] * second[1] - first[1] * second[0],
    )


def length(vector: Point) -> float:
    return math.sqrt(vector[0] ** 2 + vector[1] ** 2 + vector[2] ** 2)


def normalized(vector: Point) -> Point:
    return multiply(vector, 1.0 / length(vector))


def quantized(value: float, step: float) -> float:
    return round(value / step) * step


def random_point(rng: random.Random, low: float, high: float) -> Point:
    return (rng.uniform(low, high), rng.uniform(low, high), rng.uniform(low, high))


def random_unit_vector(rng: random.Random) -> Point:
    while True:
        vector = random_point(rng, -1.0, 1.0)
        if 0.1 < length(vector) <= 1.0:
            return normalized(vector)


def random_tangent(rng: random.Random, normal: Point) -> Point:
    while True:
        tangent = cross(normal, random_unit_vector(rng))
        if length(tangent) > 0.1:
            return normalized(tangent)


def shape_quality(shape: Triangle) -> float:
    edges = (
        subtract(shape[1], shape[0]),
        subtract(shape[2], shape[1]),
        subtract(shape[0], shape[2]),
    )
    longest = max(length(edge) for edge in edges)
    return length(cross(edges[0], edges[1])) / (longest * longest)


def well_shaped_triangle(
    rng: random.Random, low: float, high: float, step: float = 0.0
) -> Triangle:
    while True:
        vertices = [random_point(rng, low, high) for _ in range(3)]
        if step > 0.0:
            vertices = [
                (quantized(x, step), quantized(y, step), quantized(z, step))
                for x, y, z in vertices
            ]
        shape: Triangle = (vertices[0], vertices[1], vertices[2])
        if shape_quality(shape) >= MIN_SHAPE_QUALITY:
            return shape


def unit_normal(shape: Triangle) -> Point:
    return normalized(cross(subtract(shape[1], shape[0]), subtract(shape[2], shape[0])))


def interior_point(rng: random.Random, shape: Triangle) -> Point:
    weights = [rng.uniform(1.0, 2.0) for _ in range(3)]
    total = sum(weights)
    result: Point = (0.0, 0.0, 0.0)
    for weight, vertex in zip(weights, shape):
        result = add(result, multiply(vertex, weight / total))
    return result


def point_shape(vertex: Point) -> Triangle:
    return (vertex, vertex, vertex)


def segment_shape(first: Point, second: Point) -> Triangle:
    return (first, second, second)


def offset_vector(
    rng: random.Random, shape: Triangle, low: float, high: float
) -> Point:
    side = rng.choice((-1.0, 1.0))
    return multiply(unit_normal(shape), side * rng.uniform(low, high))


def crossing_vector(rng: random.Random, shape: Triangle) -> Point:
    normal = unit_normal(shape)
    along_normal = multiply(normal, rng.uniform(0.08, 0.2))
    along_plane = multiply(random_tangent(rng, normal), rng.uniform(0.0, 0.08))
    return add(along_normal, along_plane)


def single(rng: random.Random) -> list[Item]:
    return [(well_shaped_triangle(rng, 0.1, 0.9), False)]


def crossing_triangles(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    center = interior_point(rng, base)
    direction = crossing_vector(rng, base)
    spread = multiply(random_tangent(rng, unit_normal(base)), rng.uniform(0.03, 0.1))
    top = add(center, direction)
    other = (top, subtract(center, direction), add(top, spread))
    return [(base, True), (other, True)]


def crossing_segment(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    center = interior_point(rng, base)
    direction = crossing_vector(rng, base)
    other = segment_shape(add(center, direction), subtract(center, direction))
    return [(base, True), (other, True)]


def fan_from(rng: random.Random, corner: Point) -> Triangle:
    return (
        corner,
        add(corner, multiply(random_unit_vector(rng), 0.2)),
        add(corner, multiply(random_unit_vector(rng), 0.2)),
    )


def shared_vertex(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    return [(base, True), (fan_from(rng, base[rng.randrange(3)]), True)]


def shared_edge(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    first = rng.randrange(3)
    second = (first + 1) % 3
    apex = add(base[first], multiply(random_unit_vector(rng), 0.2))
    return [(base, True), ((base[first], base[second], apex), True)]


def vertex_on_edge(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7, EXACT_STEP)
    first = rng.randrange(3)
    middle = multiply(add(base[first], base[(first + 1) % 3]), 0.5)
    return [(base, True), (fan_from(rng, middle), True)]


def point_at_vertex(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    return [(base, True), (point_shape(base[rng.randrange(3)]), True)]


def point_off_plane(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    above = add(interior_point(rng, base), offset_vector(rng, base, 0.03, 0.2))
    return [(base, False), (point_shape(above), False)]


def segment_off_plane(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    normal = multiply(unit_normal(base), rng.choice((-1.0, 1.0)))
    first = add(interior_point(rng, base), multiply(normal, rng.uniform(0.03, 0.2)))
    second = add(interior_point(rng, base), multiply(normal, rng.uniform(0.03, 0.2)))
    return [(base, False), (segment_shape(first, second), False)]


def parallel_offset(rng: random.Random) -> list[Item]:
    base = well_shaped_triangle(rng, 0.3, 0.7)
    sliding = multiply(random_tangent(rng, unit_normal(base)), rng.uniform(0.0, 0.1))
    shift = add(offset_vector(rng, base, 0.03, 0.15), sliding)
    other = (add(base[0], shift), add(base[1], shift), add(base[2], shift))
    return [(base, False), (other, False)]


def flat_quality(shape: FlatShape) -> float:
    return shape_quality(tuple((x, y, 0.0) for x, y in shape))


def well_shaped_flat(rng: random.Random, low: float, high: float) -> FlatShape:
    while True:
        shape = (
            (rng.uniform(low, high), rng.uniform(low, high)),
            (rng.uniform(low, high), rng.uniform(low, high)),
            (rng.uniform(low, high), rng.uniform(low, high)),
        )
        if flat_quality(shape) >= MIN_SHAPE_QUALITY:
            return shape


def flat_interior_point(rng: random.Random, shape: FlatShape) -> Flat:
    weights = [rng.uniform(1.0, 2.0) for _ in range(3)]
    total = sum(weights)
    x = sum(weight * vertex[0] for weight, vertex in zip(weights, shape)) / total
    y = sum(weight * vertex[1] for weight, vertex in zip(weights, shape)) / total
    return (x, y)


def rotated_about_center(rng: random.Random, points: Sequence[Flat]) -> list[Flat]:
    angle = rng.uniform(0.0, 2.0 * math.pi)
    factor = rng.uniform(0.6, 1.0)
    cosine = math.cos(angle) * factor
    sine = math.sin(angle) * factor
    return [
        (
            0.5 + (x - 0.5) * cosine - (y - 0.5) * sine,
            0.5 + (x - 0.5) * sine + (y - 0.5) * cosine,
        )
        for x, y in points
    ]


def flat_segment(first: Flat, second: Flat) -> FlatShape:
    return (first, second, second)


def random_flat_step(rng: random.Random, limit: float) -> Flat:
    while True:
        step = (
            quantized(rng.uniform(-limit, limit), FINE_STEP),
            quantized(rng.uniform(-limit, limit), FINE_STEP),
        )
        if math.hypot(*step) > limit / 3.0:
            return step


def flat_overlap(rng: random.Random) -> list[FlatItem]:
    base = well_shaped_flat(rng, 0.25, 0.75)
    center_x, center_y = flat_interior_point(rng, base)
    radius = rng.uniform(0.03, 0.1)
    start = rng.uniform(0.0, 2.0 * math.pi)
    angles = [start + 2.0 * math.pi * k / 3.0 for k in range(3)]
    around = tuple(
        (center_x + radius * math.cos(angle), center_y + radius * math.sin(angle))
        for angle in angles
    )
    return [(base, True), ((around[0], around[1], around[2]), True)]


def flat_near_miss(rng: random.Random) -> list[FlatItem]:
    base_corners = [(0.2, 0.2), (0.8, 0.2), (0.2, 0.8)]
    other_corners = [(0.8, 0.8), (0.85, 0.25), (0.25, 0.85)]
    points = rotated_about_center(rng, base_corners + other_corners)
    base = (points[0], points[1], points[2])
    other = (points[3], points[4], points[5])
    return [(base, False), (other, False)]


def flat_segment_through(rng: random.Random) -> list[FlatItem]:
    base = well_shaped_flat(rng, 0.25, 0.75)
    center_x, center_y = flat_interior_point(rng, base)
    angle = rng.uniform(0.0, 2.0 * math.pi)
    reach = rng.uniform(0.05, 0.25)
    first = (center_x + reach * math.cos(angle), center_y + reach * math.sin(angle))
    second = (center_x - reach * math.cos(angle), center_y - reach * math.sin(angle))
    return [(base, True), (flat_segment(first, second), True)]


def flat_segment_outside(rng: random.Random) -> list[FlatItem]:
    points = rotated_about_center(
        rng, [(0.2, 0.2), (0.8, 0.2), (0.2, 0.8), (0.85, 0.3), (0.3, 0.85)]
    )
    base = (points[0], points[1], points[2])
    return [(base, False), (flat_segment(points[3], points[4]), False)]


def flat_collinear_segments(rng: random.Random) -> list[FlatItem]:
    start = (
        quantized(rng.uniform(0.3, 0.7), FINE_STEP),
        quantized(rng.uniform(0.3, 0.7), FINE_STEP),
    )
    step = random_flat_step(rng, 0.12)

    def along(k: int) -> Flat:
        return (start[0] + k * step[0], start[1] + k * step[1])

    layouts: dict[str, tuple[int, int, int, int, bool]] = {
        "touching": (-1, 1, 1, 2, True),
        "overlapping": (-1, 1, 0, 2, True),
        "apart": (-2, 0, 1, 2, False),
    }
    begin, end, other_begin, other_end, flag = layouts[rng.choice(list(layouts))]
    first = flat_segment(along(begin), along(end))
    second = flat_segment(along(other_begin), along(other_end))
    return [(first, flag), (second, flag)]


def flat_segment_t_contact(rng: random.Random) -> list[FlatItem]:
    while True:
        first = (
            quantized(rng.uniform(0.3, 0.5), FINE_STEP),
            quantized(rng.uniform(0.3, 0.5), FINE_STEP),
        )
        direction = random_flat_step(rng, 0.25)
        stem = random_flat_step(rng, 0.2)
        crossing = direction[0] * stem[1] - direction[1] * stem[0]
        if abs(crossing) > 0.3 * math.hypot(*direction) * math.hypot(*stem):
            break
    second = (first[0] + direction[0], first[1] + direction[1])
    middle = ((first[0] + second[0]) * 0.5, (first[1] + second[1]) * 0.5)
    touching = rng.random() < 0.5
    gap = 0.0 if touching else rng.uniform(0.1, 0.4)
    root = (middle[0] + gap * stem[0], middle[1] + gap * stem[1])
    tip = (root[0] + stem[0], root[1] + stem[1])
    return [
        (flat_segment(first, second), touching),
        (flat_segment(root, tip), touching),
    ]


def axis_plane(rng: random.Random) -> Callable[[Flat], Point]:
    level = rng.uniform(0.2, 0.8)
    axis = rng.randrange(3)

    def lift(point: Flat) -> Point:
        vertex = [point[0], point[1]]
        vertex.insert(axis, level)
        return (vertex[0], vertex[1], vertex[2])

    return lift


def tilted_plane(rng: random.Random) -> Callable[[Flat], Point]:
    slope_first = rng.choice(TILT_SLOPES)
    slope_second = rng.choice(TILT_SLOPES)
    constant = 0.5 - 0.5 * slope_first - 0.5 * slope_second
    axis = rng.randrange(3)

    def lift(point: Flat) -> Point:
        first = quantized(point[0], TILTED_STEP)
        second = quantized(point[1], TILTED_STEP)
        vertex = [first, second]
        vertex.insert(axis, slope_first * first + slope_second * second + constant)
        return (vertex[0], vertex[1], vertex[2])

    return lift


def in_random_plane(flat_motif: FlatMotif) -> Motif:
    def motif(rng: random.Random) -> list[Item]:
        lift = rng.choice((axis_plane, tilted_plane))(rng)
        return [
            ((lift(shape[0]), lift(shape[1]), lift(shape[2])), flag)
            for shape, flag in flat_motif(rng)
        ]

    return motif


def piercing_direction(rng: random.Random) -> Point:
    return (rng.uniform(-0.1, 0.1), rng.uniform(-0.1, 0.1), rng.uniform(0.15, 0.3))


def piercing_center(rng: random.Random) -> Point:
    return (rng.uniform(0.3, 0.7), rng.uniform(0.3, 0.7), HUGE_PLANE_HEIGHT)


def piercing_triangle(rng: random.Random) -> list[Item]:
    center = piercing_center(rng)
    direction = piercing_direction(rng)
    spread = (rng.uniform(-0.1, 0.1), rng.uniform(-0.1, 0.1), 0.0)
    top = add(center, direction)
    return [((top, subtract(center, direction), add(top, spread)), True)]


def piercing_segment(rng: random.Random) -> list[Item]:
    center = piercing_center(rng)
    direction = piercing_direction(rng)
    return [(segment_shape(add(center, direction), subtract(center, direction)), True)]


def hovering_triangle(rng: random.Random) -> list[Item]:
    return [(well_shaped_triangle(rng, 0.6, 0.9), False)]


MOTIFS: dict[str, Motif] = {
    "single": single,
    "crossing_triangles": crossing_triangles,
    "crossing_segment": crossing_segment,
    "shared_vertex": shared_vertex,
    "shared_edge": shared_edge,
    "vertex_on_edge": vertex_on_edge,
    "point_at_vertex": point_at_vertex,
    "point_off_plane": point_off_plane,
    "segment_off_plane": segment_off_plane,
    "parallel_offset": parallel_offset,
    "coplanar_overlap": in_random_plane(flat_overlap),
    "coplanar_near_miss": in_random_plane(flat_near_miss),
    "coplanar_segment_through": in_random_plane(flat_segment_through),
    "coplanar_segment_outside": in_random_plane(flat_segment_outside),
    "collinear_segments": in_random_plane(flat_collinear_segments),
    "segment_t_contact": in_random_plane(flat_segment_t_contact),
}

PIERCER_MOTIFS: tuple[Motif, ...] = (
    piercing_triangle,
    piercing_segment,
    hovering_triangle,
)

ALL_MOTIFS: tuple[str, ...] = tuple(MOTIFS)
CROSSING_MOTIFS: tuple[str, ...] = (
    "crossing_triangles",
    "crossing_segment",
    "coplanar_overlap",
    "coplanar_segment_through",
)
CONTACT_MOTIFS: tuple[str, ...] = (
    "shared_vertex",
    "shared_edge",
    "vertex_on_edge",
    "point_at_vertex",
    "collinear_segments",
    "segment_t_contact",
)
NEAR_MISS_MOTIFS: tuple[str, ...] = (
    "point_off_plane",
    "segment_off_plane",
    "parallel_offset",
    "coplanar_near_miss",
    "coplanar_segment_outside",
)
DEGENERATE_MOTIFS: tuple[str, ...] = (
    "crossing_segment",
    "point_at_vertex",
    "point_off_plane",
    "segment_off_plane",
    "coplanar_segment_through",
    "coplanar_segment_outside",
    "collinear_segments",
    "segment_t_contact",
)
COPLANAR_MOTIFS: tuple[str, ...] = (
    "coplanar_overlap",
    "coplanar_near_miss",
    "coplanar_segment_through",
    "coplanar_segment_outside",
    "collinear_segments",
    "segment_t_contact",
)

SCENES: dict[str, SceneSpec] = {
    "mixed_100k": SceneSpec(100_000, 1, ALL_MOTIFS, (-4, 4)),
    "mixed_250k": SceneSpec(250_000, 2, ALL_MOTIFS, (-30, 30)),
    "sparse_300k": SceneSpec(300_000, 3, ("single",) * 16 + ALL_MOTIFS, (-8, 8)),
    "crossings_200k": SceneSpec(200_000, 4, CROSSING_MOTIFS + CONTACT_MOTIFS, (-8, 8)),
    "near_misses_200k": SceneSpec(
        200_000, 5, NEAR_MISS_MOTIFS + ("crossing_triangles",), (-8, 8)
    ),
    "contacts_150k": SceneSpec(150_000, 6, CONTACT_MOTIFS + ("single",), (-20, 20)),
    "degenerate_150k": SceneSpec(150_000, 7, DEGENERATE_MOTIFS, (-20, 20)),
    "coplanar_150k": SceneSpec(150_000, 8, COPLANAR_MOTIFS, (-20, 20)),
    "huge_plane_200k": SceneSpec(200_000, 9, ALL_MOTIFS, (-4, 4), huge_plane=True),
    "tiny_scale_150k": SceneSpec(150_000, 10, ALL_MOTIFS, (-100, -90)),
    "large_scale_150k": SceneSpec(150_000, 11, ALL_MOTIFS, (90, 100)),
    "limit_999999": SceneSpec(999_999, 12, ALL_MOTIFS, (-4, 4), huge_plane=True),
}


def random_placement(rng: random.Random, spec: SceneSpec) -> Placement:
    axes = [0, 1, 2]
    rng.shuffle(axes)
    offset = (
        rng.randint(-OFFSET_LIMIT, OFFSET_LIMIT),
        rng.randint(-OFFSET_LIMIT, OFFSET_LIMIT),
        rng.randint(-OFFSET_LIMIT, OFFSET_LIMIT),
    )
    scale = 2.0 ** rng.randint(*spec.scale_exponents)
    return Placement(scale, offset, (axes[0], axes[1], axes[2]))


Cell = tuple[int, int, int]


def placed_point(placement: Placement, cell: Cell, local: Point) -> Point:
    unit = [
        (placement.offset[axis] + cell[axis] + local[axis]) * placement.scale
        for axis in range(3)
    ]
    return (unit[placement.axes[0]], unit[placement.axes[1]], unit[placement.axes[2]])


def placed_shape(
    rng: random.Random, placement: Placement, cell: Cell, shape: Triangle
) -> Triangle:
    vertices = [placed_point(placement, cell, vertex) for vertex in shape]
    rng.shuffle(vertices)
    return (vertices[0], vertices[1], vertices[2])


def inside_cell(items: Sequence[Item]) -> bool:
    return all(
        CELL_LOW <= coordinate <= CELL_HIGH
        for shape, _ in items
        for vertex in shape
        for coordinate in vertex
    )


def motif_items(rng: random.Random, motif: Motif) -> list[Item]:
    while True:
        items = motif(rng)
        if inside_cell(items):
            return items


def build_scene(spec: SceneSpec) -> tuple[list[Triangle], list[bool]]:
    rng = random.Random(spec.seed)
    placement = random_placement(rng, spec)
    side = math.ceil(spec.count ** (1.0 / 3.0)) + 2
    plane_layer = side // 2
    cells = [
        (x, y, z)
        for x in range(side)
        for y in range(side)
        for z in range(side)
        if not (spec.huge_plane and z == plane_layer)
    ]
    rng.shuffle(cells)

    shapes: list[Triangle] = []
    expected: list[bool] = []
    remaining = spec.count

    def put(cell: Cell, items: Sequence[Item]) -> None:
        nonlocal remaining
        for shape, flag in items:
            shapes.append(placed_shape(rng, placement, cell, shape))
            expected.append(flag)
        remaining -= len(items)

    if spec.huge_plane:
        layer_cells = [(x, y, plane_layer) for x in range(side) for y in range(side)]
        rng.shuffle(layer_cells)
        pierced = False
        for cell in layer_cells[: min(PIERCER_CELL_COUNT, remaining - 1)]:
            items = motif_items(rng, rng.choice(PIERCER_MOTIFS))
            pierced = pierced or any(flag for _, flag in items)
            put(cell, items)
        far = float(side)
        huge = (
            (-far, -far, HUGE_PLANE_HEIGHT),
            (4.0 * far, -far, HUGE_PLANE_HEIGHT),
            (-far, 4.0 * far, HUGE_PLANE_HEIGHT),
        )
        put((0, 0, plane_layer), [(huge, pierced)])

    for cell in cells:
        if remaining == 0:
            break
        items = motif_items(rng, MOTIFS[rng.choice(spec.motifs)])
        if len(items) > remaining:
            items = motif_items(rng, single)
        put(cell, items)

    if remaining != 0:
        raise RuntimeError(f"not enough cells for {spec.count} triangles")

    order = list(range(len(shapes)))
    rng.shuffle(order)
    return [shapes[index] for index in order], [expected[index] for index in order]


def scene_case(name: str, spec: SceneSpec) -> Case:
    shapes, expected = build_scene(spec)
    return Case(name, [], encode_triangles(shapes), 0, encode_marked(expected))


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Large random scenes with answers known by construction."
    )
    parser.add_argument(
        "--binary", type=Path, required=True, help="Path to triangles_cli"
    )
    parser.add_argument(
        "--scene", choices=sorted(SCENES), required=True, help="Scene to check"
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    case = scene_case(args.scene, SCENES[args.scene])
    return run_cases(args.binary, f"large {args.scene}", [case], RUN_TIMEOUT_SECONDS)


if __name__ == "__main__":
    sys.exit(main())
