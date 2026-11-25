# https://docs.python.org/3/library/argparse.html
import json
import socket
from dataclasses import asdict, dataclass, is_dataclass
from enum import Enum
from typing import Any, Optional

MAX_PACKET_SIZE = 64 * 1024  # 64 KiB


class CommandType(Enum):
    AddStreet = "ADD_STREET"
    ModStreet = "MOD_STREET"
    RmStreet = "RM_STREET"
    GenerateGraph = "GENERATE_GRAPH"
    GraphUpdate = "GRAPH_UPDATE"
    Shutdown = "SHUTDOWN"


@dataclass
class Street:
    name: str
    points: list[tuple[int, int]]


@dataclass
class RmStreet:
    name: str


@dataclass
class GraphUpdate:
    vertex_count: int
    edges: list[tuple[int, int]]


@dataclass
class Command:
    version: int
    requestId: int
    type: CommandType
    payload: Optional[Street | RmStreet | GraphUpdate | None]


class DataclassEncoder(json.JSONEncoder):
    def default(self, o: Any):
        if isinstance(o, Enum):
            return o.value

        if is_dataclass(o) and not isinstance(o, type):
            return asdict(o)

        return super().default(o)


def serialize(command: Command) -> bytes:
    j = json.dumps(command, cls=DataclassEncoder)
    return j.encode("utf-8")


def deserialize(data: bytes) -> Command | None:
    j = json.loads(data.decode("utf-8"))
    command_type = j["type"]
    payload_data = j.get("payload")

    payload = None

    if command_type == CommandType.AddStreet.value:
        com_type = CommandType.AddStreet
        payload = Street(
            name=payload_data["name"],
            points=[(point[0], point[1]) for point in payload_data["points"]]
        )

    elif command_type == CommandType.ModStreet.value:
        com_type = CommandType.ModStreet
        payload = Street(
            name=payload_data["name"],
            points=[(point[0], point[1]) for point in payload_data["points"]]
        )

    elif command_type == CommandType.RmStreet.value:
        com_type = CommandType.RmStreet
        payload = RmStreet(
            name=payload_data["name"]
        )

    elif command_type == CommandType.GenerateGraph.value:
        com_type = CommandType.GenerateGraph

    elif command_type == CommandType.Shutdown.value:
        com_type = CommandType.GenerateGraph

    else:
        return None
        # raise ValueError(f"Unknown command type: {command_type}")

    return Command(
        version=j["version"],
        requestId=j["requestId"],
        type=com_type,
        payload=payload
    )


def send(command: Command, sock: socket.socket):
    data = serialize(command)
    sent = sock.send(data)
    if sent != len(data):
        raise RuntimeError("Incomplete SOCK_SEQPACKET send")


def recv(sock: socket.socket) -> Command | None:
    data = sock.recv(MAX_PACKET_SIZE)
    if not data:
        raise RuntimeError("Incomplete SOCK_SEQPACKET recv")
    command = deserialize(data)
    return command
