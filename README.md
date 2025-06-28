# 🐢 Slow Protocol Demo

## Participants

- José Carlos Andrade do Nascimento — nº USP 12549450
- Felipe Carneiro Machado — nº USP 14569373
- Guilherme Rebecchi dos Santos — nº USP 12550107

## 🚀 Starting Up Everything

The main entry point of the application is the `main.cpp`.

This project is a demo of our UDP client and transaction manager.

## Compiling it

You must have `make` installed in your system. Then:

```bash
make all
./bin/app # starts the application
```

Note: The first data ("Hello World") will pretty much work everytime. However, the second data (with revive) may not work sometimes due to the expiration time given by the sttl field from the server. Sometimes the time will expire before it tries to revive the connection depending on how long the code actually takes each time to run, which means the revive will fail. If you try a bunch of times, some of them will work.

## ⚙️ How It Works

You can set whatever flow you want. However, a working example is in `main.go`, in which it connects, sends a message, disconnects, sends another message with revive (if the connection time is still on) and disconnects again.

We also made a logger package to make it easier for the user to understand what's going on.

### 1. Connection Setup

We begin by connecting through the Transaction Manager, which handles:

- Connection initialization
- Session management
- Status monitoring

```cpp
  if (!transaction_manager->connect() ) {
        Log(LogLevel::ERROR, "connect failed. cancelling operation");
        exit(EXIT_FAILURE);
    }
```

### 2. Sending Data

To send data to the server, use the send_data method from the transaction manager.

The Transaction Manager will:

- Send the data in packages
- Handle acknowledgments
- Manage retransmissions (using ack and seqnum logic)

```cpp
  if (!transaction_manager->send_data("hello world") ) {
        Log(LogLevel::ERROR, "data sending failed. cancelling operation");
        exit(EXIT_FAILURE);
    }
```

### 3. Disconnecting

After sending data, you must disconnect.

This will:

- Send a disconnect package to the server
- The client will wait for an acknowledgment

```cpp
  if (!transaction_manager->disconnect() ) {
        Log(LogLevel::ERROR, "disconnect failed. cancelling operation");
        exit(EXIT_FAILURE);
    }
```

### 4. Session Revival

If the session's STTL (session time to live) is still valid:

You can send more data using the revive flag set to true

This allows sending more data without reconnecting

```cpp
  if (!transaction_manager->send_data("hello world again", true) ) {
        Log(LogLevel::ERROR, "data sending failed. cancelling operation");
        exit(EXIT_FAILURE);
    }
```

After that, don't forget to disconnect again to properly clean up the session (see section 3).

## 📁 Project Structure and Module Implementation

### Directory Organization

The project follows a modular C++ architecture with clear separation of concerns:

```text
redes-slow-protocol/
├── include/           # Global public headers (visible to all modules)
├── src/              # Private implementation files organized by module
│   ├── main.cpp      # Contains a demo for the protocol library
│   ├── client/       # UDP client implementation, isolates networking
│   ├── logger/       # Logging system for easy debug and insight into the package
│   ├── package_builder/  # Protocol packagedata type definition, serialization and deserialization
│   └── transaction/  # Session and transaction management
├── bin/              # Compiled executable output
├── build/            # Object files and intermediate build artifacts
└── Makefile          # Build configuration
```

### Module Architecture

#### 🔄 **Core Modules**

**1. Logger Module** (`include/logger.hpp`, `src/logger/`)

- **Purpose**: Centralized logging system with configurable log levels
- **Interface**: Simple `Log(LogLevel, std::string)` function
- **Implementation**: Console output with timestamped messages
- **Log Levels**: INFO, WARNING, ERROR

**2. SlowPackage Module** (`include/slow_package.hpp`, `src/package_builder/`)

- **Purpose**: Implements the custom protocol packet structure
- **Key Components**:
  - `SlowPackage` DTO (Data Transfer Object) as a class
  - Serialization and deserialization
- **Features**: Serialization/deserialization, type definition

**3. Package Builder Module** (`include/package_builder.hpp`, `src/package_builder/`)

- **Purpose**: Factory functions for creating protocol packets
- **Key Functions**:
  - `conectPackage()`: Connection initiation
  - `disconnectPackage()`: Session termination
  - `fragmentedDataPackages()`: Data fragmentation for large payloads
  - `fragmentedRevivePackages()`: Session revival with existing session data
- **Features**: Data fragmentation and easy package building without boilerplate

**4. UDP Client Module** (`include/udp_client.hpp`, `src/client/`)

- **Purpose**: Low-level UDP socket communication
- **Features**:
  - Connection setup and management
  - Binary data transmission (`send_bytes()`)
  - Character data transmission (`send_chars()`)
  - Configurable receive timeouts
  - Non-blocking receive operations

**5. Transaction Module** (`include/transaction.hpp`, `src/transaction/`)

- **Purpose**: High-level session and transaction management
- **Key Responsibilities**:
  - Connection lifecycle management
  - Session state tracking (OFFLINE, CONNECTED, EXPIRED, CONNECTING)
  - Automatic retransmission with acknowledgment logic
  - Thread-safe buffer management for incoming packets
  - Background listener thread for continuous packet reception

#### 🔄 **Data Flow Architecture**

```text
main.cpp (Your application)
    ↓ 
Transaction Manager (session orchestration) -> Package Builder (protocol packet creation)
    ↓
UDP Client (network communication)
    ↓
Network Layer
```

#### 🧵 **Threading Model**

- **Main Thread**: Application logic and user interaction
- **Listener Thread**: Background packet reception and buffer management
- **Thread Safety**: Mutex-protected shared resources (connection status, receiver buffer)

#### 🔐 **Session Management**

The transaction manager implements a stateful session protocol with:

- **Session UUID**: 16-byte unique identifier
- **STTL (Session Time To Live)**: Automatic session expiration
- **Sequence Numbers**: Ordered packet delivery and acknowledgment
- **Revive Mechanism**: Reuse existing sessions without reconnection
- **Connection States**: Finite state machine for connection lifecycle

This modular design provides clear separation between network communication, protocol implementation, and application logic, making the codebase maintainable and extensible.
