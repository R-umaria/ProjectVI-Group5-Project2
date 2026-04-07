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

Example:

```bash
ClientApp.exe --server-ip 192.168.1.10 --server-port 54000 --telemetry-file data/sample/telemetry_1.txt --aircraft-id AIR-1001 --send-interval-ms 25
```

Supported arguments:

- `--server-ip`
- `--server-port`
- `--telemetry-file`
- `--aircraft-id`
- `--send-interval-ms`

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

- Each client execution represents one flight session.
- The server appends completed flights to `output/stats/flight_stats.csv`.
- Logs are written to `output/logs/` and mirrored to the console.
- For distributed performance testing, update client IP settings to the actual server machine address.


## Client identity and log files
- Each client process now receives a unique aircraft ID at startup unless `--aircraft-id` is explicitly provided.
- Each client session writes to its own log file in `output/logs/`, with the filename including the aircraft ID, process ID, and session timestamp.
- Load, spike, and endurance batch scripts now pass unique `--aircraft-id` values so concurrent test runs remain analytically valid.
