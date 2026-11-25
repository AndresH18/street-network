from __future__ import annotations

import re
import sys
from dataclasses import dataclass
from enum import Enum, auto
from fractions import Fraction

# Regular expressions
ADD_OR_MOD_PATTERN = re.compile(
    rf'^(add|mod)\s+"([^"]+)"\s+((?:\([^()]*\)\s*)+)$'
)

RM_PATTERN = re.compile(
    r'^rm\s+"([^"]+)"$'
)

GG_PATTERN = re.compile(
    r'^gg$'
)

COORDINATES_PATTERN = re.compile(
    r'\(\s*([-+]?\d+\s*)\s*,\s*([-+]?\d+)\s*\)'
)


class CommandOperation(Enum):
    ADD = auto()
    MOD = auto()
    RM = auto()
    GG = auto()


@dataclass
class Command:
    operation: CommandOperation
    name: str | None
    coordinates: list[tuple[int, int]] | None

    def street(self) -> Street:
        if not self.name or not self.coordinates:
            raise ValueError("Street name and coordinates must be provided")
        assert self.name

        return Street(self.name, self.coordinates)


@dataclass
class Segment:
    start: tuple[int, int]
    end: tuple[int, int]
    street: Street


@dataclass
class Street:
    name: str
    coordinates: list[tuple[int, int]]

    def segments(self) -> list[Segment]:
        return [
            Segment(self.coordinates[i], self.coordinates[i + 1], self)
            for i in range(len(self.coordinates) - 1)
        ]


@dataclass
class Intersection:
    point: tuple[int | float | Fraction, int | float | Fraction]
    segment_1: Segment
    segment_2: Segment


class StreetData:
    def __init__(self):
        self.streets = {}

    def add(self, street: Street):
        if street.name in self.streets:
            raise ValueError(f"street '{street.name}' already exists. Use `mod` to modify it")

        self.streets[street.name] = street

    def mod(self, street: Street):
        if street.name not in self.streets:
            raise ValueError(f"street '{street.name}' does not exist. Use `add` to add a new street first")

        self.streets[street.name] = street

    def rm(self, street_name: str):
        """Removes a street from the street network"""
        if street_name not in self.streets:
            raise ValueError(f"street '{street_name}' does not exists.")

        del self.streets[street_name]

    def gen(self) -> Graph:
        """Generates the graph of the street network"""
        return build_graph(list(self.streets.values()))


class Graph:
    def __init__(self):
        self.vertices: dict[int, tuple[int | float, int | float]] = {}
        self.edges: set[tuple[int, int]] = set()
        self._point_to_id: dict[tuple[int | float, int | float], int] = {}
        self._next_id = 1

    def add_vertex(self, point: tuple[int | float, int | float]) -> int:
        if point in self._point_to_id:
            return self._point_to_id[point]

        vertex_id = self._next_id
        self._next_id += 1

        self.vertices[vertex_id] = point
        self._point_to_id[point] = vertex_id

        return vertex_id

    def get_vertex_id(self, point: tuple[int | float, int | float]) -> int:
        return self._point_to_id[point]

    def add_edge(self, a: int, b: int):
        if a == b:
            return
        # Store edges in one consistent direction
        self.edges.add((min(a, b), max(a, b)))


def parse_input(line) -> Command:
    match = ADD_OR_MOD_PATTERN.fullmatch(line)

    if match:
        operation = match.group(1)
        name = match.group(2).strip().casefold()
        coordinates_text = match.group(3)

        coordinates = COORDINATES_PATTERN.findall(coordinates_text)
        if not coordinates:
            raise ValueError("Coordinates are badly formatted")

        coordinates = [(int(x), int(y)) for x, y in coordinates]
        if len(coordinates) < 2:
            raise ValueError("Coordinates must have at least two points")
        return Command(
            CommandOperation.ADD if operation == 'add' else CommandOperation.MOD,
            name,
            coordinates
        )

    match = RM_PATTERN.fullmatch(line)

    if match:
        return Command(CommandOperation.RM, match.group(1).strip().casefold(), None)  # group(1) is street name

    if GG_PATTERN.fullmatch(line):
        return Command(CommandOperation.GG, None, None)

    raise ValueError("Invalid input")


def apply_command(data: StreetData, command: Command):
    if command.operation is CommandOperation.GG:
        graph = data.gen()
        # print(json.dumps(graph.vertices, indent=2))
        # print(str(graph.edges))
        print_graph(graph)
        return

    if command.operation is CommandOperation.RM:
        data.rm(command.name)
        return

    street = command.street()
    if command.operation is CommandOperation.ADD:
        data.add(street)
        return

    if command.operation is CommandOperation.MOD:
        data.mod(street)
        return


def segment_intersection(s1: Segment, s2: Segment) -> tuple[Fraction, Fraction] | None:
    """
    Find the intersection point of two-line segments
    :param s1: First Segment
    :param s2: Second Segment
    :return: returns tuple representing the intersection or None if the segments don't intercept
    """

    x1, y1 = s1.start
    x2, y2 = s1.end
    x3, y3 = s2.start
    x4, y4 = s2.end

    den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)

    # Lines are parallel or collinear
    if den == 0:
        return None

    u = Fraction((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4), den)
    v = Fraction((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2), den)
    # u = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den
    # v = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / den

    # If t and u are between 0 and 1, the intersection falls within both segments
    if 0 <= u <= 1 and 0 <= v <= 1:
        intercept_x = x1 + u * (x2 - x1)
        intercept_y = y1 + u * (y2 - y1)
        return intercept_x, intercept_y

    return None  # Intersection point lies beyond the segment bounds


def street_intersections(street_1: Street, street_2: Street) -> list[Intersection]:
    segments_1 = street_1.segments()
    segments_2 = street_2.segments()

    intersections: list[Intersection] = []

    if segments_1 is None:
        raise ValueError(f"Street {street_1.name} does not contain any segments")

    if segments_2 is None:
        raise ValueError(f"Street {street_1.name} does not contain any segments")

    for s1 in segments_1:
        for s2 in segments_2:
            point = segment_intersection(s1, s2)
            if point is not None:
                intersections.append(Intersection(point, s1, s2))

    return intersections


def find_all_intersections(streets: list[Street]) -> list[Intersection]:
    intersections = []

    for i in range(len(streets)):
        for j in range(i + 1, len(streets)):
            intersections.extend(
                street_intersections(streets[i], streets[j])
            )

    return intersections


def collect_vertex_points(intersections: list[Intersection]) -> tuple[
    set[tuple[int | float, int | float]],
    set[tuple[int | float, int | float]]
]:
    vertex_points = set()
    intersection_points = set()

    for intersection in intersections:
        intersection_points.add(intersection.point)

        # The actual intersection is a vertex
        vertex_points.add(intersection.point)

        # Endpoints of the intersecting segments are also vertices
        vertex_points.add(intersection.segment_1.start)
        vertex_points.add(intersection.segment_1.end)

        vertex_points.add(intersection.segment_2.start)
        vertex_points.add(intersection.segment_2.end)

    return vertex_points, intersection_points


def intersections_for_segment(
        segment: Segment,
        intersections: list[Intersection]
) -> set[tuple[int | float, int | float]]:
    """
    Returns the points that intersect a segment
    """
    points = set()
    for intersection in intersections:
        if intersection.segment_1 == segment or intersection.segment_2 == segment:
            points.add(intersection.point)

    return points


def distance_squared(p1: tuple[int | float, int | float], p2: tuple[int | float, int | float]) -> float:
    dx = p1[0] - p2[0]
    dy = p1[1] - p2[1]
    return (dx ** 2) + (dy ** 2)


def segment_points(segment: Segment, intersections: list[Intersection]) -> list[tuple[int | float, int | float]]:
    points = [
        segment.start,
        *intersections_for_segment(segment, intersections),
        segment.end
    ]

    points.sort(
        key=lambda point: distance_squared(segment.start, point)
    )

    return points


def build_vertices(graph: Graph, intersections: list[Intersection]):
    vertex_points, intersection_points = collect_vertex_points(intersections)

    for point in vertex_points:
        graph.add_vertex(point)

    return intersection_points


def build_edges(
        graph: Graph,
        streets: list[Street],
        intersections: list[Intersection],
        intersection_points: set[tuple[int | float, int | float]]
):
    for street in streets:
        for segment in street.segments():

            points = segment_points(
                segment,
                intersections
            )

            for left, right in zip(points, points[1:]):
                if (
                        left in intersection_points
                        or right in intersection_points
                ):
                    graph.add_edge(
                        graph.get_vertex_id(left),
                        graph.get_vertex_id(right)
                    )


def build_graph(streets: list[Street]) -> Graph:
    graph = Graph()

    intersections = find_all_intersections(streets)

    intersection_points = build_vertices(graph, intersections)

    build_edges(graph, streets, intersections, intersection_points)

    return graph


def print_graph(graph: Graph):
    sys.stdout.write(f"V {len(graph.vertices)}\n")

    edges = [f"<{a},{b}>" for a, b in graph.edges]
    edges = ["E {", ','.join(edges), "}\n"]

    sys.stdout.writelines(edges)
    sys.stdout.flush() # flush stdout

