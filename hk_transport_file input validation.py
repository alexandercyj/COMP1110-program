"""
Scope:
1. Handle invalid or missing inputs.
2. Load the transport network from a simple text file.
"""

from dataclasses import dataclass
from enum import Enum
from typing import Dict, List, Tuple


class TransportMode(Enum):
    MTR = "MTR"
    BUS = "BUS"
    WALK = "WALK"


@dataclass
class Stop:
    id: str
    name_cn: str
    name_en: str
    x: float
    y: float
    is_accessible: bool = True


@dataclass
class Segment:
    from_stop: str
    to_stop: str
    line: str
    duration: int
    cost: float
    mode: TransportMode


@dataclass
class Line:
    id: str
    name: str
    color: str
    mode: TransportMode
    stops: List[str]


class TransitNetwork:
    def __init__(self):
        self.stops: Dict[str, Stop] = {}
        self.lines: Dict[str, Line] = {}
        self.segments: List[Segment] = []
        self.adjacency: Dict[str, List[Segment]] = {}

    def add_stop(self, stop: Stop) -> None:
        self.stops[stop.id] = stop
        self.adjacency.setdefault(stop.id, [])

    def add_line(self, line: Line) -> None:
        self.lines[line.id] = line

    def add_segment(self, segment: Segment) -> None:
        if segment.from_stop not in self.stops or segment.to_stop not in self.stops:
            raise ValueError(
                f"Segment uses unknown stop: {segment.from_stop} -> {segment.to_stop}"
            )
        self.segments.append(segment)
        self.adjacency.setdefault(segment.from_stop, []).append(segment)


class NetworkLoadError(Exception):
    pass


class InputValidationError(Exception):
    pass


def parse_mode(mode_text: str) -> TransportMode:
    cleaned = mode_text.strip().upper()
    if cleaned == "MTR":
        return TransportMode.MTR
    if cleaned == "BUS":
        return TransportMode.BUS
    if cleaned == "WALK":
        return TransportMode.WALK
    raise NetworkLoadError(f"Unsupported transport mode: {mode_text}")


def load_network_from_text(filename: str) -> TransitNetwork:
    """
    Supported format:

    STOP,<id>,<name_cn>,<name_en>,<x>,<y>,<is_accessible>
    LINE,<id>,<name>,<color>,<mode>
    SEGMENT,<from_stop>,<to_stop>,<line>,<duration>,<cost>,<mode>

    Rules:
    - Empty lines are ignored.
    - Lines starting with # are treated as comments.
    - Missing file and empty file are handled gracefully by raising NetworkLoadError.
    """
    try:
        with open(filename, "r", encoding="utf-8") as f:
            raw_lines = f.readlines()
    except FileNotFoundError as e:
        raise NetworkLoadError(f"Network file not found: {filename}") from e

    if not raw_lines:
        raise NetworkLoadError(f"Network file is empty: {filename}")

    lines = []
    for raw in raw_lines:
        stripped = raw.strip()
        if stripped and not stripped.startswith("#"):
            lines.append(stripped)

    if not lines:
        raise NetworkLoadError(f"Network file is empty: {filename}")

    network = TransitNetwork()

    for line_no, line in enumerate(lines, start=1):
        parts = [part.strip() for part in line.split(",")]
        record_type = parts[0].upper()

        try:
            if record_type == "STOP":
                if len(parts) != 7:
                    raise NetworkLoadError(
                        f"Line {line_no}: STOP must have 7 fields."
                    )
                stop = Stop(
                    id=parts[1],
                    name_cn=parts[2],
                    name_en=parts[3],
                    x=float(parts[4]),
                    y=float(parts[5]),
                    is_accessible=parts[6].lower() in {"true", "1", "yes", "y"},
                )
                network.add_stop(stop)

            elif record_type == "LINE":
                if len(parts) != 5:
                    raise NetworkLoadError(
                        f"Line {line_no}: LINE must have 5 fields."
                    )
                line_obj = Line(
                    id=parts[1],
                    name=parts[2],
                    color=parts[3],
                    mode=parse_mode(parts[4]),
                    stops=[],
                )
                network.add_line(line_obj)

            elif record_type == "SEGMENT":
                if len(parts) != 8:
                    raise NetworkLoadError(
                        f"Line {line_no}: SEGMENT must have 8 fields."
                    )
                segment = Segment(
                    from_stop=parts[1],
                    to_stop=parts[2],
                    line=parts[3],
                    duration=int(parts[4]),
                    cost=float(parts[5]),
                    mode=parse_mode(parts[6]),
                )
                # allow either 7 or 8 fields if teammate later adds an extra note column
                # but current format uses 8 fields including record_type.
                if parts[1] not in network.stops or parts[2] not in network.stops:
                    raise NetworkLoadError(
                        f"Line {line_no}: unknown stop in segment {parts[1]} -> {parts[2]}."
                    )
                network.add_segment(segment)

            else:
                raise NetworkLoadError(
                    f"Line {line_no}: unknown record type '{parts[0]}'."
                )

        except ValueError as e:
            raise NetworkLoadError(f"Line {line_no}: invalid numeric value.") from e

    if not network.stops:
        raise NetworkLoadError("No stops were loaded from the file.")
    if not network.segments:
        raise NetworkLoadError("No segments were loaded from the file.")

    return network


VALID_PREFERENCES = {"fastest", "cheapest", "fewest_segments"}


def normalize_stop_input(network: TransitNetwork, user_input: str) -> str:
    text = user_input.strip()
    if not text:
        raise InputValidationError("Stop input is missing.")

    text_upper = text.upper()
    if text_upper in network.stops:
        return text_upper

    for stop in network.stops.values():
        if text == stop.name_cn or text.lower() == stop.name_en.lower():
            return stop.id

    raise InputValidationError(f"Unknown stop: {user_input}")


def validate_preference_mode(preference: str) -> str:
    cleaned = preference.strip().lower()
    if not cleaned:
        raise InputValidationError("Preference mode is missing.")
    if cleaned not in VALID_PREFERENCES:
        raise InputValidationError(
            "Invalid preference mode. Use: fastest, cheapest, or fewest_segments."
        )
    return cleaned


def validate_journey_query(
    network: TransitNetwork,
    origin_input: str,
    destination_input: str,
    preference_input: str,
) -> Tuple[str, str, str]:
    origin_id = normalize_stop_input(network, origin_input)
    destination_id = normalize_stop_input(network, destination_input)

    if origin_id == destination_id:
        raise InputValidationError(
            "Origin and destination cannot be the same stop."
        )

    preference = validate_preference_mode(preference_input)
    return origin_id, destination_id, preference


if __name__ == "__main__":
    print("Available functions:")
    print("- load_network_from_text(filename)")
    print("- validate_journey_query(network, origin, destination, preference)")
