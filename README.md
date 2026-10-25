<h1 align="center">Triangles</h1>

<p align="center">
  Finding intersecting triangles in 3D using exact arithmetic, and visualizing them in OpenGL
</p>

<p align="center">
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white">
  <img alt="CMake" src="https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white">
  <img alt="OpenGL 3.3" src="https://img.shields.io/badge/OpenGL-3.3%20core-5586A4?logo=opengl&logoColor=white">
</p>

<p align="center">
  <img alt="Camera flying through a scene with intersecting triangles highlighted" src="assets/scene.gif" width="800">
</p>

---

Given a set of triangles in 3D space, the program finds every triangle that intersects at least one other. The visualizer renders the scene in an OpenGL window with intersecting triangles highlighted, and lets you fly around it freely. The command-line version prints the indices of the intersecting triangles.

- Touching at a vertex or along an edge counts as an intersection.
- Degenerate triangles — points and segments — are handled like any other.
- Every decision is based on the signs of **exact** geometric predicates, so the answer never depends on rounding errors.

## Contents

- [Getting started](#getting-started)
- [Usage](#usage)
- [Geometry](#geometry)
  - [Problem statement](#problem-statement)
  - [Exact predicates](#exact-predicates)
  - [Intersecting two triangles](#intersecting-two-triangles)
  - [Finding all intersecting triangles](#finding-all-intersecting-triangles)
- [Visualizer](#visualizer)
  - [Controls](#controls)
  - [Color themes](#color-themes)
  - [How rendering works](#how-rendering-works)
- [Testing](#testing)
- [Project layout](#project-layout)

## Getting started

> [!NOTE]
> The build and tests are verified on Windows ([MSYS2](https://www.msys2.org/) UCRT64) and Linux (Ubuntu 24.04).

Requirements:

| Tool | Version | Used for |
|---|---|---|
| GCC | C++20 support, tested with GCC 13 | building |
| [CMake](https://cmake.org/) | 3.16+ | building, fetching dependencies |
| [Ninja](https://ninja-build.org/) | any | build generator |
| Python | 3.10+ | e2e tests and the scene generator |
| Jinja2 | any | generating the OpenGL loader ([glad](https://github.com/Dav1dde/glad)) |
| GPU | OpenGL 3.3 core | the visualizer window |

The libraries (GLFW, glad, GLM, GoogleTest) are fetched automatically on the first configure.

```bash
git clone https://github.com/andreyphm/triangles.git
cd triangles
```

### Windows (MSYS2 UCRT64)

Install the toolchain from an MSYS2 UCRT64 terminal:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-python-jinja2
```

Build:

```bash
cmake -S . -B build
cmake --build build
```

### Linux (Ubuntu 24.04)

Install the toolchain. The X11 and Wayland packages are needed to build GLFW:

```bash
sudo apt install -y build-essential cmake ninja-build git pkg-config python3 python3-jinja2 libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev libwayland-dev libwayland-bin wayland-protocols libgl1-mesa-dev
```

Build:

```bash
cmake -S . -B build
cmake --build build
```

## Usage

### Windows

Command-line version:

```powershell
cmd /c ".\build\cli\triangles_cli.exe < examples\cloud_5k.in"
```

Visualizer (optionally with a [color theme](#color-themes)):

```powershell
cmd /c ".\build\triangles.out.exe [--theme ...] < examples\cloud_5k.in"
```

> [!TIP]
> PowerShell has no `<` input redirection, and piping with `Get-Content file | program` corrupts the input, so the programs are launched through `cmd /c`. In an MSYS2 terminal the usual syntax works: `./build/triangles.out < examples/cloud_5k.in`.

### Linux

Command-line version:

```bash
./build/cli/triangles_cli < examples/cloud_5k.in
```

Visualizer:

```bash
./build/triangles.out [--theme ...] < examples/cloud_5k.in
```

> [!NOTE]
> Under WSL the window runs through WSLg: the mouse turns the camera, but the cursor stays visible. This is a WSLg limitation, not a bug in the program.

### Input format

The first number is the triangle count `N`, `0 < N < 1'000'000`. It is followed by `N` triangles of 9 coordinates each: three vertices as `x y z`. Line breaks can go anywhere.

```text
3
0 0 0  1 0 0  0 1 0
1 0 0  2 0 0  1 1 1
5 5 5  6 5 5  5 6 5
```

### Output format

The **command-line version** prints the indices of the intersecting triangles — zero-based, in ascending order, one per line. In the example above the first two triangles touch at the vertex `(1, 0, 0)` and the third one stands apart:

```text
0
1
```

The **visualizer** prints the OpenGL version and opens a window with the scene, where intersecting triangles are highlighted.

On invalid input — a non-numeric value, `N` out of range, fewer triangles than announced — both programs print a message to stderr and exit with code `1`. The visualizer also exits with a usage hint when given an unknown argument or theme name.

### Example scenes

[`examples/`](examples) contains random scenes:

| File | Triangles | Contents |
|---|---:|---|
| `cloud_5k.in` | 5,000 | scene ~40 across, triangles of similar size ~1 |
| `mixed_sizes_10k.in` | 10,000 | scene ~75 across, triangles from ~0.1 to ~4 |
| `tiny_cloud_20k.in` | 20,000 | very small coordinates: scene ~0.1 across |
| `huge_cloud_50k.in` | 50,000 | very large coordinates: scene ~10⁶ across |

You can generate your own scenes with [`tools/generate_scene.py`](tools/generate_scene.py). Triangle centers are scattered randomly over the cube `[0, SIZE]³`, and each vertex lies within the triangle size of its center along every axis:

```bash
python tools/generate_scene.py --count 5000 --scene-size 100 --triangle-size 1 3 --seed 1 --output examples/my_scene.in
```

| Argument | Meaning |
|---|---|
| `--count` | number of triangles, from 1 to 999,999 |
| `--scene-size` | edge of the cube that contains the triangle centers |
| `--triangle-size` | triangle size: a single value or a `MIN MAX` range |
| `--seed` | random generator seed, `1` by default |
| `--output` | file to write the scene to |

## Geometry

### Problem statement

A triangle is a closed shape: its boundary together with its interior. Two triangles intersect if they share at least one point.

Three vertices do not have to form a proper triangle:

- if all three vertices coincide, the triangle is a **point**;
- if they lie on one line, it is a **segment**;
- otherwise it is a **non-degenerate triangle** — a triangle in the usual sense.

### Exact predicates

Implementation: [`geometry/source/robust_predicates.cpp`](geometry/source/robust_predicates.cpp).

All intersection checks come down to a single question: *which side is the point on?*

- **`orient3d(a, b, c, d)`** — which side of the plane through `a`, `b`, `c` is the point `d` on?
- **`orient2d(a, b, c)`** — the same in the plane: which side of the line `ab` is the point `c` on?

The answer is the sign of a determinant: `+`, `-` or `0`. For `orient3d`:

$$
\mathrm{orient3d}(a, b, c, d) = \mathrm{sign}
\begin{vmatrix}
a_x - d_x & a_y - d_y & a_z - d_z \\
b_x - d_x & b_y - d_y & b_z - d_z \\
c_x - d_x & c_y - d_y & c_z - d_z
\end{vmatrix}
$$

Zero means the four points are coplanar.

A determinant computed in `double` carries rounding error, and near zero that error can turn `0` into `+` or `-` — while zero is exactly the touching case. An early version of the project used plain `double` and reported "no intersection" in about 9% of touching cases with large coordinates; those inputs are now pinned down by regression tests.

#### Computing the sign

The approach follows J. Shewchuk, [*Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates*](https://www.cs.cmu.edu/~quake/robust.html).

1. **Fast path.** The determinant is computed in `double` and compared against a guaranteed upper bound on its rounding error. The bound is proportional to the sum of the absolute values of the determinant's terms. If the absolute value of the result is larger than the bound, rounding cannot have changed its sign. On real data this covers almost every call.
2. **Exact path.** Otherwise the determinant is recomputed without any rounding. A number is represented as an exact sum of several `double`s whose bits do not overlap. The sum or product of two `double`s can be written exactly as a pair "rounded result + its error", and the whole determinant is assembled from such pairs. The sign of the sum is the sign of its component with the largest absolute value.

Infinity and `NaN` arguments are treated as errors.

> [!IMPORTANT]
> The exact path relies on every operation being rounded exactly as written. That is why the predicates file is compiled with strict floating point (`-ffp-contract=off`, `-fno-fast-math`): the compiler may not fuse a multiply with an add or reorder operations.

### Intersecting two triangles

Implementation: [`geometry/source/intersection.cpp`](geometry/source/intersection.cpp).

#### Classification

First, each triangle is classified as a point, a segment, or a non-degenerate triangle.

A pair of shapes then falls into one of six cases:

|  | Point | Segment | Triangle |
|---|:---:|:---:|:---:|
| **Point** | coincidence | point on segment | point in triangle |
| **Segment** | | segment vs segment | segment vs triangle |
| **Triangle** | | | triangle vs triangle |

#### Point

- **Point and point** intersect if all their coordinates are equal.
- **Point on segment:** the point lies on the line through the segment's endpoints (checked in all three projections) and lies between the endpoints along every coordinate.
- **Point in triangle:** the point lies in the triangle's plane (`orient3d = 0`). The triangle and the point are then projected onto a coordinate plane where the triangle's projection is not degenerate. The point is inside or on the boundary unless it is to the left of one edge and to the right of another.

#### Segment and segment

1. The four endpoints must be coplanar; otherwise the segments are skew and share no points.
2. The segments are projected onto all three coordinate planes, and they must intersect in **every** projection.

Two segments in the plane intersect if:

- the endpoints of each one lie strictly on opposite sides of the other's line — the segments cross;
- or else, if some endpoint of one segment lies on the other.

#### Segment and triangle

What to check depends on where the segment's endpoints are relative to the triangle's plane:

| Endpoints | What is checked |
|---|---|
| strictly on the same side | no intersection |
| strictly on opposite sides | the segment crosses the plane at a single point. That point is inside the triangle if the segment's line passes "inside" relative to each of the three edges, i.e. the three `orient3d` calls give no two opposite non-zero signs |
| one endpoint on the plane | that endpoint is the only point shared with the plane; check whether it lies in the triangle |
| both endpoints on the plane | the segment lies entirely inside the triangle or crosses one of its edges |

#### Triangle and triangle

1. For each vertex of the second triangle, determine which side of the first triangle's plane it is on. If all three are strictly on the same side, the plane separates the triangles and they do not intersect. The vertices of the first triangle are checked against the second triangle's plane the same way.
2. Otherwise the triangles intersect if and only if **some edge** of one triangle intersects the other triangle. Each edge is checked as a segment-vs-triangle case.

### Finding all intersecting triangles

Implementation: [`geometry/source/intersection_search.cpp`](geometry/source/intersection_search.cpp), [`geometry/source/uniform_grid.cpp`](geometry/source/uniform_grid.cpp).

Checking every pair means `N(N−1)/2` exact checks — about five billion for `N = 100,000`. But only triangles that are close to each other can intersect. So the search runs in two stages: first, nearby candidate pairs are selected cheaply, and only those go through the exact check.

#### Bounding boxes

Each triangle gets its smallest axis-aligned bounding box, computed once. If the boxes of two triangles do not overlap, neither do the triangles. The check is six number comparisons — far cheaper than the exact check.

#### Uniform grid

Space is divided into equal cubic cells. Each triangle is registered in every cell its bounding box touches.

If two triangles intersect, their boxes overlap too, so they share at least one cell. Therefore, it is enough to compare only triangles that share a cell.

The cell size is chosen from the scene:

- the initial size is the average bounding box size, so a typical triangle covers a few cells;
- along each axis the scene is split into at most `2²¹ − 1` cells, so that the three cell indices fit into one 64-bit number — the cell key;
- while there are more than 16 cell entries per triangle, the cell size is doubled. Otherwise a single huge triangle could take up millions of cells.

**Reference cell.** A pair may share several cells, but it must be checked exactly only once. So the pair is checked in just one of its shared cells — the one containing the minimum corner of the overlap of the two bounding boxes.

#### Traversal

1. All "cell — triangle" entries are sorted by cell key, so the triangles of each cell end up next to each other.
2. Within each cell, all pairs of its triangles are visited, with checks ordered from cheapest to most expensive:
   - both triangles are already marked as intersecting — skip, the pair adds no new information;
   - the bounding boxes do not overlap — skip;
   - this is not the pair's reference cell — skip;
   - otherwise run the exact check.
3. Once every triangle is marked, the search stops early.

> [!NOTE]
> When triangles are small and spread across the scene, each cell holds only a few of them, and the search time grows almost linearly with `N`. The worst case is still quadratic: many large triangles that lie close together without intersecting end up in the same cells.

## Visualizer

The window shows the whole scene: regular and intersecting triangles are drawn in two different colors of the current theme. The camera flies freely through the scene.

### Controls

| Key | Action |
|---|---|
| <kbd>W</kbd> / <kbd>S</kbd> | move forward / backward along the view direction |
| <kbd>A</kbd> / <kbd>D</kbd> | move left / right |
| <kbd>Space</kbd> / <kbd>Shift</kbd> | move up / down |
| <kbd>Ctrl</kbd> | ×2 speed while held |
| Mouse wheel | flight speed: up is faster, down is slower |
| Mouse | look around |
| <kbd>Alt</kbd> | show the cursor while held |
| <kbd>←</kbd> / <kbd>→</kbd> | previous / next color theme |
| <kbd>Esc</kbd> | close the window |

### Color themes

Implementation: [`graphics/headers/theme.hpp`](graphics/headers/theme.hpp).

A theme defines three colors: the background, regular triangles, and intersecting triangles. The starting theme is chosen with `--theme`, `ocean` by default. In the window, the arrow keys cycle through the themes.

Click a theme name to see a screenshot:

| Theme | Background | Triangles | Intersecting |
|---|---|---|---|
| [`ocean`](assets/ocean_theme.png) | `#0C2D48` | `#5AAAC8` | `#FF7F50` |
| [`pine`](assets/pine_theme.png) | `#1E4637` | `#96C3A0` | `#FF6E5A` |
| [`ultraviolet`](assets/ultraviolet_theme.png) | `#230A46` | `#BE6EFF` | `#5FEC5F` |
| [`desert`](assets/desert_theme.png) | `#EEE2C8` | `#C8AA78` | `#285AC8` |

### How rendering works

#### Scene preparation

Implementation: [`scene/source/vertices.cpp`](scene/source/vertices.cpp).

The GPU works with `float`, while input coordinates are `double` and can be of any scale — from `0.1` to `10⁶` and beyond. So the scene is first centered at the origin and scaled into the cube `[-0.5, 0.5]` in `double`, and only then converted to `float`. This way small details of a scene far from the origin are not lost to `float` precision.

#### Data on the GPU

Implementation: [`graphics/source/gpu_triangles.cpp`](graphics/source/gpu_triangles.cpp).

Vertices are uploaded to the GPU once. Each vertex stores only its coordinates and a flag telling whether its triangle intersects another. Colors are passed to the shader separately. The whole frame is drawn with a single call.

#### Lighting

Implementation: [`graphics/shaders/triangles_fragment.glsl`](graphics/shaders/triangles_fragment.glsl).

The shader computes the normal at each point from how the point's position changes between neighboring pixels. Brightness depends on the angle between the normal and the direction to the camera:

$$
\text{brightness} = 0.65 + 0.35 \cdot |\cos \alpha|
$$

Taking the absolute value of the cosine lights both sides of a triangle the same way.

#### Camera

Implementation: [`graphics/source/camera.cpp`](graphics/source/camera.cpp).

A perspective camera with a 60° field of view. It starts outside the scene, looking at its center. Pitch is limited to ±89° so the camera never flips over the vertical.

#### Window

Implementation: [`graphics/source/window.cpp`](graphics/source/window.cpp).

The window and OpenGL context are created by [GLFW](https://www.glfw.org/), and OpenGL functions are loaded by [glad](https://github.com/Dav1dde/glad). The scene keeps redrawing while the window is being resized. Shader sources are embedded into the program at build time, so the executable does not depend on the directory it is launched from.

## Testing

All tests run through [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) after building:

```bash
ctest --test-dir build --output-on-failure
```

### Unit tests

Written with [GoogleTest](https://github.com/google/googletest). Labels: `geometry`, `scene`, `graphics`:

```bash
ctest --test-dir build -L geometry
```

### End-to-end tests

Implementation: [`e2e_tests/`](e2e_tests).

Python scripts test the built programs as a whole. A script builds the input itself, runs the program, and checks the exit code, stdout, and stderr.

Each group is a separate CTest test labeled `e2e`:

| Test | What it checks |
|---|---|
| `e2e_geometry` | intersections of regular triangles |
| `e2e_degenerate` | points and segments |
| `e2e_scale` | the same scenarios scaled by `2^±20`, `2^±300` and shifted by `2^20` |
| `e2e_search` | large scenes with a known answer |
| `e2e_input` | input format and input errors |
| `e2e_visualizer_arguments` | invalid arguments to `triangles.out` |
| `e2e_generator` | the scene generator |

All e2e tests:

```bash
ctest --test-dir build -L e2e
```

A single group:

```bash
ctest --test-dir build -R e2e_scale --output-on-failure
```

## Project layout

```text
.
├── CMakeLists.txt                      # root build, GoogleTest
├── main.cpp                            # visualizer
├── cmake/
│   └── warnings.cmake                  # shared warning flags
├── geometry/
│   ├── CMakeLists.txt
│   ├── headers/
│   │   ├── vector2.hpp                 # 2D vector
│   │   ├── vector3.hpp                 # 3D vector and point
│   │   ├── triangle.hpp                # triangle
│   │   ├── bounding_box.hpp            # axis-aligned bounding box
│   │   ├── robust_predicates.hpp       # exact orient2d, orient3d predicates
│   │   ├── intersection.hpp            # intersection of two triangles
│   │   ├── uniform_grid.hpp            # uniform grid for candidate selection
│   │   ├── intersection_search.hpp     # search for all intersecting triangles
│   │   └── input_reader.hpp            # input parsing
│   ├── source/
│   │   ├── triangle.cpp
│   │   ├── bounding_box.cpp
│   │   ├── robust_predicates.cpp
│   │   ├── intersection.cpp
│   │   ├── uniform_grid.cpp
│   │   ├── intersection_search.cpp
│   │   └── input_reader.cpp
│   └── tests/
│       ├── CMakeLists.txt
│       ├── vector3_tests.cpp
│       ├── triangle_tests.cpp
│       ├── bounding_box_tests.cpp
│       ├── predicates_tests.cpp
│       ├── intersection_tests.cpp
│       ├── uniform_grid_tests.cpp
│       ├── intersection_search_tests.cpp
│       └── input_reader_tests.cpp
├── cli/
│   ├── CMakeLists.txt
│   └── main.cpp                        # command-line version
├── scene/
│   ├── CMakeLists.txt
│   ├── headers/
│   │   └── vertices.hpp                # preparing scene vertices for rendering
│   ├── source/
│   │   └── vertices.cpp
│   └── tests/
│       ├── CMakeLists.txt
│       └── vertices_tests.cpp
├── graphics/
│   ├── CMakeLists.txt                  # GLFW, glad, GLM
│   ├── headers/
│   │   ├── window.hpp                  # window, keyboard and mouse input
│   │   ├── shader_program.hpp          # shader program
│   │   ├── shaders.hpp                 # shader sources
│   │   ├── vertex.hpp                  # vertex: coordinates and intersection flag
│   │   ├── gpu_triangles.hpp           # vertex buffers on the GPU
│   │   ├── camera.hpp                  # camera
│   │   └── theme.hpp                   # color themes
│   ├── source/
│   │   ├── window.cpp
│   │   ├── shader_program.cpp
│   │   ├── shaders.cpp
│   │   ├── gpu_triangles.cpp
│   │   ├── camera.cpp
│   │   └── theme.cpp
│   ├── shaders/
│   │   ├── triangles_vertex.glsl       # vertex shader
│   │   ├── triangles_fragment.glsl     # fragment shader
│   │   └── shader_sources.hpp.in       # template for the header with shader sources
│   └── tests/
│       ├── CMakeLists.txt
│       ├── camera_tests.cpp
│       └── theme_tests.cpp
├── e2e_tests/
│   ├── CMakeLists.txt                  # registers e2e tests with CTest
│   ├── common.py                       # building blocks, input encoding, running and checking
│   ├── cli_e2e.py                      # scenarios for triangles_cli
│   ├── visualizer_e2e.py               # invalid arguments to triangles.out
│   └── generator_e2e.py                # checks for generate_scene.py
├── tools/
│   └── generate_scene.py               # random scene generator
├── assets/                             # screenshots for the README
└── examples/                           # scene examples
    ├── cloud_5k.in
    ├── mixed_sizes_10k.in
    ├── tiny_cloud_20k.in
    └── huge_cloud_50k.in
```
