# Fleet Telemetry System

A working Visual Studio baseline for the CSCN73060 client-server fleet telemetry project.

## What is implemented

- TCP client-server communication over sockets
- Thread-per-client server listener and worker model
- Client telemetry file replay with validation and ordered transmission
- Server-side packet parsing and per-aircraft flight tracking
- Real-time fuel consumption calculation using fuel delta over elapsed time
- End-of-flight CSV persistence without overwriting prior completed flights
- Config-driven startup plus optional command-line overrides
- Windows batch scripts for local runs and performance-test style launches

## Solution layout

- **ClientApp**: console client that reads telemetry data and sends packets to the server
- **ServerApp**: console server for multi-client telemetry processing
- **SharedLib**: shared packet, config, logging, and utility code

## Open in Visual Studio

1. Open `FleetTelemetrySystem.sln`
2. Select **Debug** or **Release**
3. Build the solution
4. Set `ServerApp` as startup project to run the server, or `ClientApp` to run the client

## Runtime folders

- `config/` for client and server configuration
- `data/` for telemetry inputs
- `output/logs/` for logs
- `output/stats/` for generated statistics
- `output/performance/` for performance test artifacts

## Client usage

Default run uses `config/client.config.json`.

Single client example:

```bash
ClientApp.exe --server-ip 192.168.1.10 --server-port 54000 --telemetry-file data/sample/telemetry_1.txt --aircraft-id AIRCRAFT-1001 --send-interval-ms 25
```

Launcher example for a deterministic ID range:

```bash
ClientApp.exe --client-count 25 --aircraft-start 1001 --aircraft-end 1025 --aircraft-prefix AIRCRAFT --server-ip 192.168.1.10 --server-port 54000 --telemetry-file data/sample/telemetry_1.txt
```

When launcher arguments are used, the parent process spawns separate client processes. Each child process still represents one flight session.

Supported single-client arguments:

- `--server-ip`
- `--server-port`
- `--telemetry-file`
- `--aircraft-id`
- `--send-interval-ms`

Supported launcher arguments:

- `--client-count`
- `--aircraft-start`
- `--aircraft-end`
- `--aircraft-prefix`

## Server usage

Default run uses `config/server.config.json`.

Example:

```bash
ServerApp.exe --bind-ip 0.0.0.0 --listen-port 54000 --stats-file output/stats/flight_stats.csv
```

Supported arguments:

- `--bind-ip`
- `--listen-port`
- `--stats-file`
- `--log-file`

## Notes

- Each spawned child client process represents one flight session.
- The server appends completed flights to `output/stats/flight_stats.csv`.
- Logs are written to `output/logs/` and mirrored to the console.
- For distributed performance testing, update client IP settings to the actual server machine address.

## Batch files for professor-style testing

The repo root now includes batch files aligned with the project instructions:

- `LoadTest_Batch.bat`
- `EnduranceTest_Batch.bat`
- `SpikeTest_Batch.bat`

Examples:

```bat
LoadTest_Batch.bat 25 1001 AIRCRAFT 192.168.1.10 54000 data\sample\telemetry_1.txt
```

This spawns 25 client processes with IDs `AIRCRAFT-1001` through `AIRCRAFT-1025`.

```bat
EnduranceTest_Batch.bat 100 2001 250 AIR-END 192.168.1.10 54000 data\sample\telemetry_2.txt
```

This continuously spawns waves of 100 clients, starting wave 1 at `AIR-END-2001`, then the next wave continues the numbering.

## Client identity and log files
- Each client process receives a unique aircraft ID at startup unless `--aircraft-id` is explicitly provided.
- Each client session writes to its own log file in `output/logs/`.
- Load, spike, and endurance batch scripts pass deterministic `--aircraft-id` values so concurrent test runs remain analytically valid and easy to trace.
