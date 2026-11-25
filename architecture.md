# Architecture

## Workers

The worker is responsible for monitoring a communication connection and processing incoming commands.

It uses a polling-based event loop to detect incoming data, receive commands, and delegating their execution to a
`CommandDispatcher`.

The architecture is divided into three main components:

- `Worker`: Controls the execution loop and manages the worker's lifecycle.
- `transport::Connection`: Encapsulates the communication file descriptor and handles sending and receiving commands.
- `command::CommandDispatcher`: Maps commands types to their corresponding handlers and executes them.

This separation allows communication management, evet-loop execution, and command processing to evolve independently.

### Worker Execution Flow

When `Worker::run()` is called, it first checks whether the worker is already running.

The atomic `running_.exchange(true)` operation sets the running state to true and returns its previous value. If the
worker was already running, the method returns immediately, preventing multiple execution loops from being started.

Once initialized, the worker configures a `pollfd` structure to monitor the connection's file descriptor for incoming
data (`POLLIN`).

It then enters its main execution loop

#### Polling

The worker uses POSIX `poll()` system call with a 100ms timeout.  
This allows it to wait efficiently for incoming data without continuously polling the connection. The timeout also
ensures that the worker periodically checks its running state, allowing externals stop requests to be observed.

The polling results are handled as:

- **Timeout**: No activity occurred within the timeout period. The worker continues waiting.
- **Interrupted system call (EINTR)**: The polling operation was interrupted by a signal. The worker retries.
- **Other polling errors**: The error is logged, and the exeuction loop is terminated.
- **Available events**: The worker inspects the returned events.

Before attempting to receive data, the worker checks for invalid descriptors, connection hangups, and communication
errors (`POLLNVAL`, `POLLHUP`, and `POLLERR`).

If an error is detected, the worker logs it and exits the loop.

#### Receiving Commands

When `POLLIN` indicates that the data is available, the worker calls `Connection::recv()` to receive a command. If the
operation succeeds, the received command is passed to `CommandDispatcher::dispatch()`.

The worker does not need to understand individual command types or their payloads. Its responsibility ends at
successfully receiving and forwarding the command.  
This keeps the main execution loop independent of the actual functionality implemented by individual commands.

### CommandDispatcher

The `CommandDispatcher` provides a centralized mechanism for registering and executing command handlers. It maintains an
`unordered_map` associating each `CommandType` with a corresponding handler function.

Handlers can be registered in three forms:

- Handlers that do not require a payload.
- Handlers that receive a generic payload.
- Handlers that receive a specific payload type

For typed handlers, the dispatcher internally extracts the expected type from the payload using `std::get_if<T>()`. If
the payload does not contain the expected type, an exception is thrown.

When a command is dispatched, the dispatcher looks up its type in the registered handlers map. If no handler is
registered, a warning is logged. Otherwise, the corresponding handler is invoked.

This design decouples command reception from command execution and allows new functionality to be introduced through
handler registration without modifying the worker's event loop.

### Workers Shutdown

The worker registers a handler for the `Shutdown` command during construction.

This handler captures the worker instance and invokes `Worker::stop()` when a shutdown command is dispatched. The
`stop()` method sets the atomic `running_` flag to `false`.

Since the command processing occurs inside the worker's execution loop, the shutdown handler executes on the worker
thread. After the handlers returns, the loop is evaluated again and the worker exits.

External stop requests are also supported. Because the worker uses a 100ms polling timeout, it periodically checks the
running state even when no incoming commands are available.

### Connection and Resource Ownership

The `Connection` class encapsulates the underlying communication file descriptor. It is responsible for providing access
to the descriptor and implementing the operations required to send and receive commands.

The class follows the **RAII** principle: it owns the file descriptor and closes it when destroyed.  
*Copy construction* and *copy assignment* are disabled to prevent multiple objects from unintentionally owning the same
descriptor.  
*Move construction* and *move assignment* are supported, allowing ownership to be transferred safely between objects.
The moved-from instance is left with an invalid descriptor (-1).

The worker receives the connection through a reference and does not own it. Therefore, the connection must remain alive
throughout the worker's execution.

### Design Rationale

The architecture provides several benefits:

- Separation of responsibilities between communication, event monitoring, and command execution.
- Efficient waiting for incoming data through `poll()`
- Periodic running-state checks through a polling timeout.
- Extensibility through dynamically registered command handlers.
- Type-specific payload handling through the command dispatcher.
- Automatic resource cleanup through RAII.
- Explicit ownership transfer through move semantics.

Overall, the worker acts as the coordinator for the communication-processing pipeline, while the connection and
dispatcher encapsulate the lower-level details of communication and command execution.

## Communication

The communication layer provides Inter-Process Communication through POSIX **Unix Domain** sockets.

The implementation uses socket-pairs created with `socketpair(AF_UNIX, SOCK_SEQPACKET, 0,...)` system call.

The communication is organized into four layers:

- **Command**: Defines the logical structure of messages exchanged between processes.
- **Serializer**: Converts commands between C++ and JSON.
- **Connection**: Provides a high-level interface for sending and receiving commands.
- **Protocol**: Implements the underlying byte-level communication using POSIX socket operations.

This separation allows the application to work with strongly typed command objects without dealing directly with serialization or socket operations.

### Command Representation

All messages are represented by a common `Command` structure containing:

- `version`: Identifies the command protocol version
- `request_id`: Identifies the request
- `type`: Identifies the `CommandType`
- `payload`: Contains the Command-specific data

The payload is represented using `std::variant`, which allows it to hold one of several predefined types. A `std::monostate` alternative represents commands without additional data.

This approach provides type-safety payload handling without requiring manual memory management or dynamically allocated payload pointers.

The command type and payload variant serve different purposes: the command type identifies the operation, while the variant identifies the actual C++ type stored in the payload.

### Serialization

Serialization is implemented using [nlohman/json](https://json.nlohmann.me/) library.

The `to_json()` implementation converts a `Command` into a JSON object containing its version, request ID, command type, and payload.

The payload is serialized using `std::visit()`, which examines the active alternative of the variant. If the active alternative is `std::monostate`, the JSON payload is represented as `null`. Otherwise, the corresponding structure is serialized using its registered JSON conversion.

Deserialization follows the reverse process. The `from_json()` implementation extracts the command metadata and uses `get_payload()` to determine which payload structure should be reconstructed.

The command type acts as the discriminator for selecting the expected payload type. This means both communicating processes must agree on the mapping between command types and payload structures.

### Connection

The connection class encapsulates ownership of a socket file descriptor and provides the operations required to exchange commands. Its main interface consists of:

- `fd()`: Provides access to the underlying file descriptor.
- `send()`: Serializes and transmits a command
- `recv()`: Receives and reconstructs a command

#### Sending

When `transport::Connection::send()` is called:

1. The C++ command is converted into a JSON object.
2. The JSON object is serialized into a string using `dump()`.
3. The string is exposed as a byte span
4. The bytes are passed to the `transport::protocol::send` operation.

The connection does not implement the actual socket transmission. It delegates this responsibility to the protocol layer.

#### Receiving

When `transport::Connection::recv()` is called:

1. The protocol layers receives the incoming packet into a byte vector.
2. The received bytes are converted into a JSON representation
3. The JSON object is parsed.
4. The command metadata and payload are reconstructed.
5. The resulting `command::Command` is returned to the caller

This keeps serialization and communication details encapsulated withing the connection abstraction.

### Underlying Communication: Unix Domain Socketpair

The application uses POSIX `socketpair()` with the following parameters:

- `AF_UNIX`: Uses Unix domain sockets for local inter-process communication
- `SOCK_SEQPACKET`: Provides connection-oriented, reliable, ordered, message-oriented communication

The socketpair creates two connected file descriptors. Each endpoints can both send and receive data (full-duplex).

Using `SOCK_SEQPACKET` is an important design decision because it preserves message boundaries. Unlike `SOCK_STREAM` which exposes a continuous byte stream, `SOCK_SEQPACKET` maintains individual message boundaries.

This means each serialized command can be transmitted as one logical packet, eliminating the need for additional application-level framing mechanism to identify message boundaries.

#### Protocol Implementation

The protocol layer provides low-level send and receive operations

**Sending**

THe send operation validates the message size before transmitting. Messages must be non-empty and cannot exceed `MAX_MESSAGE_SIZE`.

Transmission uses the POSIX `send()` system call with `MSG_NOSIGNAL`. This prevents the process from receiving `SIGPIPE` when writing to a disconnected socket.

The operation is considered successful only when the completed message has been transmitted. Interrupted system calls (EINTR) are retried. Other errors are logged and reported as failures.

**Receiving**

The receive operation uses `recvmsg()` to read an individual packet.

A buffer sized to `MAX_MESSAGE_SIZE` is provided to the system call.

After reception, the implementation checks for:

- Connection closure
- Interrupted system calls
- Communication errors
- Message truncation (MSG_TRUNC)

If a message exceeds the receiving buffer, it is rejected rather than passed to the JSON parser.

Successful reception resizes the buffer to the actual number of received bytes before returning it to the connection layer.

#### File descriptor ownership

The Connection class owns its underlying file descriptor and follows the RAII principle.

Copy operations are disabled to prevent multiple objects from unintentionally owning the same descriptor.

Move operations are supported to transfer ownership safely. The moved-from object is left with an invalid descriptor (-1).

When the connection is destroyed, its descriptor is automatically closed.

### Design Rationale

The communication architecture provides several benefits:

- Separation of responsibilities: Command representation, serialization, communication, and socket operations are independent layers.
- **Type-safe messages**: `std::variant` provides a controlled set of supported payload types.
- **Structured serialization**: JSON provides a clear representation of commands without depending on C++ object memory layout.
- **Message boundaries**: `SOCK_SEQPACKET` preserves individual command boundaries without requiring custom framing.
- **Bounded message sizes**: `MAX_MESSAGE_SIZE` limites the amount of data handled by the protocol.
- **Resource Ownership**: RAII and move semantics provide explicit and predictable file descriptor ownership.
- **Simplified application code**: Higher-level components interact with `Command` objects instead of dealing with raw bytes and socket operations.

Overall, the communication layer provides a command-oriented abstraction over POSIX inter-process communication. It allows the application to exchange structured messages while keeping the underlying serialization and transport mechanics hidden from the rest of the system.

## Driver

The driver is responsible for starting, managing, and coordinating the application's subprocesses.

It provides centralized process management and communication routing through four main components:

- `driver::ProcessManager`: Owns and manages subprocesses.
- `driver::Process`: Represents an individual subprocess and encapsulates its PID, lifecycle operations, and communication connection.
- `driver::ConnectionLoop`: Runs a dedicated monitoring thread that detects and receives incoming  commands from subprocesses.
- `driver::Router`: Handles received commands and determines how messages should be routed between subprocesses.

Each subprocess has an independent communication connection to the main process, established through a POSIX Unix Domain socket pair.

The `driver` acts as a central communication coordinator. Subprocesses communicate with one another through the driver rather than directly accessing each other's connections.

### Process Manager

The `driver::ProcessManager` maintains an association between logical worker identifiers and their corresponding `driver::Process` objects.

It owns the subprocess instances and provides access to the individual processes through their `driver::WorkerId`. This allows the rest of the application to interact with subprocesses using logical identifiers instead of managing operating-system process IDs directly.

Its main responsibilities are:

- Starting the configured subprocesses.
- Maintaining ownership of the process instances.
- Providing access to individual processes.
- Making process connections available to the communication-monitoring system.
- Receive and forward user inputs.

### Process

The `driver::Process` class represents an individual operating-system subprocess.

It encapsulates the process ID, exit status, lifecycle state and communication connection.

#### Process Creation

Subprocesses are created through `driver::make_process()`.

The creation sequence is:

1. Validate the process configuration
2. Create a pair of connected Unix domain sockets.
3. Call POSIX `fork()` system call to create the child process.
4. In the child, close the parent's socket endpoint.
5. Execute the configured executable using POSIX `execv()` system call.
6. In the parent, construct a `driver::Process` object using the parents socket endpoint and child PID.

The socket pair is created before `fork()`, allowing both processes to inherit the connected endpoints.

The child receives its communication file descriptor through the `--socket` command-line argument.

The parent retains the other endpoint inside the `Process` object.

#### Process Communication

The class provides three important lifecycle operations:

- `driver::Process::request_shutdown()`: Send an application-level `Shutdown` command to the subprocess.
- `driver::Process::terminate()`: Requests process termination through `SIGTERM`.
- `driver::Process::wait()`: Waits for the process termination and collects its exit status

The destructor provides an additional cleanup mechanism by attempting to terminate and wait for a subprocess that has not already been collected.

The class follows move-only ownership semantics to prevent multiple objects from unintentionally managing the same subprocess and communication resource.

### Connection Loop

The `driver::ConnectionLoop` is responsible for monitoring the communication connections associated with all managed subprocesses. It executes independently of the main application thread through a `std::jthread`.

#### Connection Registration

The monitoring loop maintains a map associating each `WorkerId` to a reference of its corresponding `Connection`.

> [!IMPORTANT] 
> The connection themselves remain owned by the `driver::Process` object managed by `driver::ProcessManager`.

> [!NOTE] 
> The `driver::add_connections()` helper registers the connection of all managed subprocesses with the monitoring loop.

#### Monitoring

THe monitoring thread periodically builds a snapshot of the registered connections and uss the POSIX `poll()` system call to monitor all their file descriptors simultaneously. Each descriptor is monitored for incoming data through `POLLIN`.

When activity is detected, the monitoring loop uses the corresponding index in its worker identifier collection to determine which subprocess generated the event.

It then receives the command through the corresponding `Connection`

#### Command Reception

Successfully received commands are passed to the `router` through: `driver::Router::handle(source, command)`.

The source worker identifier is included so that the router can distinguish which subprocess generated the message.

The connection monitoring does not implement application-specific routing rules. Its responsibility is limited to monitoring, receiving, and forwarding incoming commands to the router.

#### Thread Lifecycle

The monitoring loop uses `std::jthread` and its stop token to support cooperative termination.

A stop request causes the monitoring loop to exit, after which the monitoring thread must be joined before its synchronization objects and referenced connection are destroyed.

### Router

The `driver::Router` is responsible for handling incoming commands and coordinate communication between subprocesses.

It maintains a reference to `driver::ProcessManager`, allowing to access the managed processes and send commands through their connections.

Its main function `driver::Router::handle(source, command)`. The `source` identifier provides context about which process generated the command, while the command itself provides the message type and payload.

Based on this information, the router determines how the message should be handled.

Private function `forward(desination, command)` method provides the mechanism for forwarding commands to another subprocess.

It identifies the destination process through `ProcessManager` and delegates transmission to `driver::Process::send()`. This keeps routing logic independent of process creation, serialization, and low-level socket operations.

### Communication Flow

The overall communication process follows this sequence:

1. A subprocess sends a command through its communication connection.
2. The `driver::ConnectionLoop` detects incoming data using `poll().
3. The monitoring loop identifies the source subprocess.
4. The command is received and reconstructed by the `Connection`.
5. The command and source identifier are passed to `driver::Router::handle()`
6. The router determines the appropriate handling or destination.
7. If forwarding is required, the router retrieves the destination process through `driver::ProcessManager`.
8. The destination process sends the command through its own communication connection.

### Architectural Rationale

The architecture separates process lifecycle management from communication monitoring and message routing.

- **ProcessManager** centralizes subprocess ownership and access
- **Process** encapsulates individual process resources and lifecycle operations
- **ConnectionLoop** provides asynchronous monitoring of multiple communication endpoints using a single dedicated thread.
- **Router** centralizes message handling and routing decisions.

This separation makes the application easier to extend because subprocess management, communication handling and routing policies can evolve independently.

