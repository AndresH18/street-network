#!/usr/bin/env python3
import socket
import sys
from argparse import ArgumentParser, Namespace

import communication
from communication import CommandType, GraphUpdate
from segmenter import Command as StreetCommand, CommandOperation, StreetData

street_data = StreetData()


def vertex_correction(vertice: int) -> int:
    return vertice


def edge_correction(edge: tuple[int, int]) -> tuple[int, int]:
    return edge[0] - 1, edge[1] - 1


# noinspection bad-assignment
def adapt_command(command: communication.Command) -> StreetCommand | None:
    if command.type is CommandType.GenerateGraph:
        return StreetCommand(
            operation=CommandOperation.GG,
            name=None,
            coordinates=None
        )
    if command.type is CommandType.AddStreet:
        payload: communication.Street = command.payload
        return StreetCommand(
            operation=CommandOperation.ADD,
            name=payload.name.strip().casefold(),
            coordinates=payload.points
        )
    if command.type is CommandType.ModStreet:
        payload: communication.Street = command.payload
        return StreetCommand(
            operation=CommandOperation.MOD,
            name=payload.name.strip().casefold(),
            coordinates=payload.points
        )
    if command.type is CommandType.RmStreet:
        payload: communication.RmStreet = command.payload
        return StreetCommand(
            operation=CommandOperation.RM,
            name=payload.name.strip().casefold(),
            coordinates=None
        )
    return None


def apply_command(command: StreetCommand, data: StreetData) -> GraphUpdate | None:
    if command.operation is CommandOperation.GG:
        graph = data.gen()
        new = communication.GraphUpdate(
            vertex_count=vertex_correction(len(graph.vertices)),
            edges=[edge_correction(edge) for edge in graph.edges]
        )
        return new
    if command.operation is CommandOperation.RM:
        data.rm(command.name)
        return None

    street = command.street()
    if command.operation is CommandOperation.ADD:
        data.add(street)
        return None

    if command.operation is CommandOperation.MOD:
        data.mod(street)
        return None

    return None


def get_socket(args: Namespace) -> socket.socket:
    # initialize and return ready to use socket
    return socket.socket(fileno=args.socket)


def main():
    parser = ArgumentParser(
        description='What the program does',
        epilog='Text at the bottom of help'
    )

    parser.add_argument('-s', '--socket',
                        help='Socket file descriptor',
                        required=True,
                        type=int)
    args = parser.parse_args()

    sock = get_socket(args)

    while True:

        com_command = communication.recv(sock)
        print("[PY] com_command=", com_command, file=sys.stderr)
        if com_command is None:
            print("[py] No command received", file=sys.stderr)
            break

        if com_command.type is communication.CommandType.Shutdown:
            print("[py] Shutdown", flush=True, file=sys.stderr)
            break

        street_command = adapt_command(com_command)
        if street_command is None:
            print("[py] No street command", file=sys.stderr)
            continue

        update = apply_command(street_command, street_data)
        if update is None:
            continue

        res_command = communication.Command(
            version=com_command.version,
            requestId=com_command.requestId,
            type=CommandType.GraphUpdate,
            payload=update
        )

        print("[PY] res_command=", res_command, file=sys.stderr)

        communication.send(res_command, sock)


def main2():
    parser = ArgumentParser(
        description='What the program does',
        epilog='Text at the bottom of help'
    )
    #
    parser.add_argument('-s', '--socket', help='Socket file descriptor', type=int)
    #
    # parser.print_help()
    args = parser.parse_args()
    # print("socket:", args.socket)
    # if not args.socket:
    #     print("Socket is invalid")
    #     return
    #
    # print("Socket is valid")
    # sock = socket(fileno=args.socket)
    # # Optional: blocking mode.
    # # sock.setblocking(True)

    j = r"""
        {
          "version": 1,
          "request_id": 1,
          "type": "ADD_STREET",
          "payload": {
            "name": "Main Street",
            "points": [
              [
                0,
                0
              ],
              [
                10,
                10
              ],
              [
                0,
                10,
                100
              ],
              [
                10,
                0
              ]
            ]
          }
        }
        """
    command = communication.deserialize(j.encode("utf-8"))
    print(command)


if __name__ == "__main__":
    main()
