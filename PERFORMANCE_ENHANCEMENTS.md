# Performance Enhancements Applied

## Goals
These changes keep the project aligned with the client-server fleet telemetry requirements and the existing SRS/SDD while reducing unnecessary CPU, lock contention, and synchronous I/O.

## Implemented changes

### Client
- Restored **one client process = one flight execution** by simplifying `src/client/main.cpp`.
- Preserved `--aircraft-id` values from scripts instead of overriding them internally.
- Improved auto-generated aircraft IDs so they are less collision-prone when no ID is provided.
- Stopped forcing a random telemetry file every run.
  - The configured or command-line file is used first.
  - Random selection is only used when the file is missing or `--random-telemetry-file` is supplied.
- Removed per-record console printing from the default execution path.
  - `--verbose-records` can be used for debug runs.
- Changed connection failure and send failure handling to **terminate safely** instead of continuing in local display-only mode.

### Shared logging
- Reworked `Logger` to keep the log file open instead of reopening it for every message.
- Replaced `std::endl` with newline writes to reduce forced flushing.
- Kept thread-safe logging.

### Server
- Removed the growing detached-thread container in `ServerApp.cpp`.
- Reduced noisy progress logging in `ClientHandler.cpp`.
- Added malformed-packet log suppression after the first few packets per client session.
- Reworked telemetry session handling so updates are no longer serialized behind one global lock.
  - Session lookup/creation uses a short map lock.
  - Each active aircraft now has its own lock.
- Refactored `TelemetryProcessor` to operate on a single flight state instead of owning a global map.
- Added asynchronous completed-flight CSV persistence in `StatisticsStore` so disk writes are moved off the hot path.

### Packet parsing
- Replaced vector-based packet splitting in `Packet::Deserialize` with direct comma position parsing to reduce allocations.

## Validation performed
- Source-level build validation using `g++ -std=c++17` for both the client and server translation units on this environment.
- The project remains structured for Visual Studio solution builds.
