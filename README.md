\page a4 A4

# A4

## General Structure

The project is structured into the following programs:

- `driver`: The main program that initializes the simulation and manages the communication between the different
  processes.
- `sp`: The process responsible for handling graph updates and generating the shortest paths between 2 points.
- `py`: Responsible for handling streets data and generating graphs.
- `rgen`: Responsible for simulating streets generation.

## Classes

This project is structured under the following classes

```mermaid
classDiagram
    class Worker {
        -connection_: Connection&
        -dispatcher_: CommandDispatcher
        -running_: atomic_bool
        + run()
        + stop()
    }
    namespace transport {
        class Connection {
            Connection(fd_t)
            Connection(Connection&) = delete;
            Connection(Connection&&)
            fd() fd_t
            send(Command&)
            recv(Command&)
        }
    }

    namespace command {
        class CommandType {
            <<enumeration>>
        }
        class Command {
            version: int
            request_id: int
            type: CommandType
            payload: Payload
        }
        class Street {
            name: string
            points: vector~pair~int int~~
        }
        class RmStreet {
            name: string
        }
        class GraphUpdate {
            vertex_count: int
            edges: vector~pair~int int~~
        }
        class PathRequest {
            start: int
            end: int
        }
        class PathResult {
            success: bool
            vectices: vector~int~
        }
        class Error {
            message: string
        }

        class Payload {
            <<variant>>
        }

        class CommandDispatcher {
            Handler= std:: function~void(const Payload&)~
            register_handler(CommandType, Handler)
            dispatch(Command&)
        }
    }

    Payload <|-- Street
    Payload <|-- RmStreet
    Payload <|-- GraphUpdate
    Payload <|-- PathRequest
    Payload <|-- PathResult
    Payload <|-- Error
    Command *-- Payload
    Connection --> Command
```

**Py**

```mermaid
classDiagram
    namespace segmenter {
        class CommandOperation {
            <<enumeration>>
            ADD
            MOD
            RM
            GG
        }
        class Command {
            +CommandOperation operation
            +str name
            +list coordinates
            +street() Street
        }

        class Street {
            +str name
            +list coordinates
            +segments() list~Segment~
        }

        class Segment {
            +tuple start
            +tuple end
            +Street street
        }

        class Intersection {
            +tuple point
            +Segment segment_1
            +Segment segment_2
        }

        class StreetData {
            +dict streets
            +add(street) void
            +mod(street) void
            +rm(street_name) void
            +gen() Graph
        }

        class Graph {
            +dict vertices
            +set edges
            +add_vertex(point) int
            +get_vertex_id(point) int
            +add_edge(a, b) void
        }
    }

    Command --> CommandOperation
    Command ..> Street
    Street "1" *-- "0..*" Segment
    Intersection --> "1" Segment
    StreetData ..> Graph
StreetData o-- "0..*" Street
```

```mermaid
classDiagram
    namespace communication {
        class CommandType {
            <<enumeration>>
            AddStreet = "ADD_STREET"
            ModStreet = "MOD_STREET"
            RmStreet = "RM_STREET"
            GenerateGraph = "GENERATE_GRAPH"
            GraphUpdate = "GRAPH_UPDATE"
            Shutdown = "SHUTDOWN"
        }

        class Street {
            name: str
            points: list[tuple[int, int]]
        }

        class RmStreet {
            name: str
        }

        class GraphUpdate {
            vertex_count: int
            edges: list[tuple[int, int]]
        }

        class Command {
            version: int
            requestId: int
            type: CommandType
            payload: Optional[Street | RmStreet | GraphUpdate | None]
        }

        class Communication {
            <<utility>>
            MAX_PACKET_SIZE = 64 * 1024  # 64 KiB
            serialize(Command) bytes
            deserialize(bytes) Command | None
            send(Command, socket.Socket)
            recv(socket.Socket) Command | None
        }
    }

    Command *-- CommandType
    Command *-- Street
    Command *-- RmStreet
    Command *-- GraphUpdate
```

```mermaid
classDiagram
    class main {
        vertex_correction(int) int
        edge_correction(tuple[int, int]) tuple[int, int]
        adapt_command(communication.Command) segmenter.Command | None
        apply_command(segmenter.Command, segmenter.StreetData) communication.GraphUpdate | None
        get_socket(socket.Namespace) socket.socket
        main()
    }
```

**short-path**

```mermaid
classDiagram
    namespace sp {
class sp-main {
<<sp-main.cpp>>
handle_graph_update(command::GraphUpdate&, std:: optional~sp::Graph~ &)
handle_path_request(std:: optional~sp::Graph~ &, transport:: Connection&, command:: PathRequest&)
main(int, char*[])
}

class Graph {
+Graph(int, std:: vector~std::pair~int int~~ &)
+shortest_path(start: int, end: int) std:: optional~std::vector~int~~
+is_valid_vertex(v: int) bool
 }
}
```

**r-gen**

```mermaid
classDiagram
    namespace rgen {
        class Point {
            - x_: int
            - y_: int
            + Point(x: int, y: int)
            + operator==(const Point&, const Point&) bool
            + operator!=(const Point&, const Point&) bool
        }

        class Segment {
            + Segment(Point&, Point&)
            + is_zero_length() bool const
            + start() const Point&
            + end() const Point&
        }

        class Street {
            + Street(std:: string, std:: vector~Point~)
            + name() const std:: string
            + points() const std:: vector~Point~ &
        }

        class Orientation {
            <<enumeration>>
            Collinear = 0
            Clockwise = -1
            CounterClockwise = 1
        }

        class generator {
            <<Utility>>
            MAX_SEGMENT_RETRY_COUNT: int
            MAX_SPECIFICATION_RETRY_COUNT: int
            MIN_STREETS: int
            MIN_SEGMENTS: int
            MIN_COORDINATE_LIMIT: int
            MIN_WAIT: int
            DEFAULT_MAX_STREETS: int
            DEFAULT_MAX_SEGMENTS: int
            DEFAULT_MAX_COORDINATE_LIMIT: int
            DEFAULT_MAX_WAIT: int
            + generate_point(int) Point
            + generate_valid_segment(Point&, int, std:: vector~Segment~ &)
            + genereate_street(std::string&, int, int, int std::vector~Segment~&) std::optional~Street~
            + generate_specification(int, int, int) std:: optional~std::vector~Street~~
            + generate_valid_specification(GeneratorConfig&) std:: vector~Street~
        }

        class geometry {
            <<Utility>>
            + orientation(Point&, Point&, Point&) Orientation
            + point_on_segment(Point&, Segment&) bool
            + segments_overlap(Segment&, Segment&) bool
            + overalps_any(Segment&, std:: vector~Segment~ &) bool
        }
    }

    Segment o-- Point
    Street o-- Point
    geometry <-- Point
    geometry <-- Segment
    geometry <-- Orientation
```

## Communication

The different processes communicate via IPC sockets, by sending different commands.

![Socket pair before and after fork](https://image3.slideserve.com/7025142/ipc-using-socketpair11-n.jpg)

[//]: # (https://www.slideserve.com/ginger-olsen/sockets)

These commands are serialized into JSON and sent via sockets. These sockets have `AF_UNIX` domain and their type is
`SOCK_SEQPACKET`. Using `SOCK_SEQPACKET` ensures that messages are delivered in the order they are sent and that the
entire message is received, which is crucial for maintaining the integrity of the communication protocol in this
context.

The general C++ structure is:

```c++
enum class CommandType {
    None,
    AddStreet,
    ModStreet,
    RmStreet,
    GenerateGraph,
    GraphUpdate,
    PathRequest,
    PathResult,
    Error,
    Shutdown,
    Ack,
};

struct AddStreet {
    std::string name;
    std::vector<std::pair<int, int>> points;
};

struct ModStreet {
    std::string name;
    std::vector<std::pair<int, int>> points;
};

struct RmStreet {
    std::string name;
};

struct GraphUpdate {
    std::vector<int> vertices;
    std::vector<std::pair< int, int>> edges;
};

struct PathRequest {
    int start;
    int end;
};

struct PathResult {
    bool success = false;
    std::vector<int> vertices;
};

using Payload = std::
        variant<std::monostate, AddStreet, ModStreet, RmStreet, GraphUpdate, PathRequest, PathResult>;

struct Command {
    unsigned int version;
    unsigned int request_id;
    CommandType type;
    Payload payload;
};
```

The JSON format can be seen as

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "Command",
  "type": "object",
  "required": [
    "version",
    "request_id",
    "type",
    "payload"
  ],
  "properties": {
    "version": {
      "type": "integer",
      "minimum": 0
    },
    "request_id": {
      "type": "integer",
      "minimum": 0
    },
    "type": {
      "type": "string",
      "enum": [
        "NONE",
        "ADDSTREET",
        "MODSTREET",
        "RMSTREET",
        "GENERATEGRAPH",
        "GRAPHUPDATE",
        "PATHREQUEST",
        "PATHRESULT",
        "ERROR",
        "SHUTDOWN",
        "ACK"
      ]
    },
    "payload": {
      "oneOf": [
        {
          "type": "null"
        },
        {
          "type": "object",
          "required": [
            "name",
            "points"
          ],
          "properties": {
            "name": {
              "type": "string"
            },
            "points": {
              "type": "array",
              "items": {
                "type": "array",
                "items": {
                  "type": "integer"
                },
                "minItems": 2,
                "maxItems": 2
              }
            }
          }
        },
        {
          "type": "object",
          "required": [
            "name"
          ],
          "properties": {
            "name": {
              "type": "string"
            }
          }
        },
        {
          "type": "object",
          "required": [
            "vertices",
            "edges"
          ],
          "properties": {
            "vertices": {
              "type": "array",
              "items": {
                "type": "integer"
              }
            },
            "edges": {
              "type": "array",
              "items": {
                "type": "array",
                "items": {
                  "type": "integer"
                },
                "minItems": 2,
                "maxItems": 2
              }
            }
          }
        },
        {
          "type": "object",
          "required": [
            "start",
            "end"
          ],
          "properties": {
            "start": {
              "type": "integer"
            },
            "end": {
              "type": "integer"
            }
          }
        },
        {
          "type": "object",
          "required": [
            "success",
            "vertices"
          ],
          "properties": {
            "success": {
              "type": "boolean"
            },
            "vertices": {
              "type": "array",
              "items": {
                "type": "integer"
              }
            }
          }
        }
      ]
    }
  }
}
```

### CommandType values

These enum values represent the different type of commands that are sent

|   Command Type   | Purpose                                      | 
|:----------------:|:---------------------------------------------|
|      `NONE`      | Does nothing                                 |
|   `ADD_STREET`   | Create a Street                              |
|   `MOD_STREET`   | Modify a Street                              |
|   `RM_STREET`    | Remove a Street                              |
| `GENERATE_GRAPH` | Request graph geneeration                    |
|  `GRAPH_UPDATE`  | Send the current graph                       |
|  `PATH_REQUEST`  | Request a path                               |
|  `PATH_RESULT`   | Return the shortest path                     |
|     `ERROR`      | Report a protocol error or application error |
|    `SHUTDOWN`    | Request gracefull termination                |
|      `ACK`       | Confirm successful processing, if needed     |

The `driver` will route messages from the different subprocesses to the appropriate subprocesses. This table represent a
simple routing logic it uses

|   Source   |   Command Type   |   Destination    |
|:----------:|:----------------:|:----------------:|
|   `rgen`   |   `ADD_STREET`   |       `py`       |
|   `rgen`   |   `MOD_STREET`   |       `py`       |
|   `rgen`   |   `RM_STREET`    |       `py`       |
|   `rgen`   | `GENERATE_GRAPH` |       `py`       |
|    `py`    |  `GRAPH_UPDATE`  |       `sp`       |
|  terminal  |  `PATH_REQUEST`  |       `sp`       |
|    `sp`    |  `PATH_RESULT`   |     terminal     |
| Any worker |     `ERROR`      | Logging/Terminal |
