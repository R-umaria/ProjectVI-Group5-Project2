# Fleet Telemetry System

Client-server fleet telemetry system for CSCN73060. The client replays aircraft telemetry records over TCP, and the server processes concurrent client sessions, computes fuel consumption statistics, and stores completed flight results.

## Current project structure

- `src/client/` - client source code
- `src/server/` - server source code
- `src/shared/` - shared packet, config, logging, and utility code
- `config/` - client and server configuration files
- `data/sample/` - telemetry input files used by the client
- `output/logs/server/` - server log file
- `output/logs/clients/` - one log file per client session
- `output/stats/` - generated CSV statistics files
- `output/performance/` - performance-test artifacts
- `output/state/` - runtime state used for generated aircraft IDs
- `LoadTest_Batch.bat`, `EnduranceTest_Batch.bat`, `SpikeTest_Batch.bat` - the only batch files kept in the repo

## Output locations

All generated runtime files are now written under the project root `output/` directory.

- Server log: `output/logs/server/server.log`
- Client logs: `output/logs/clients/<Aircraft_ID>.log`
- Flight statistics CSV: `output/stats/flight_stats.csv`
- Aircraft history CSV: `output/stats/aircraft_history.csv`

This keeps logs and CSV files in one place even when `Client.exe` or `Server.exe` is started from `x64\Release`.

## Build the solution

1. Open `FleetTelemetrySystem.sln` in Visual Studio.
2. Select `Release | x64`.
3. Build the solution.
4. After build, the executables are generated in `x64\Release\`.

Expected executables after build:

- `x64\Release\Server.exe`
- `x64\Release\Client.exe`

## Start the server

### Option 1 - from Visual Studio

1. Set `ServerApp` as the startup project.
2. Run in `Release | x64`.

### Option 2 - from the built executable

Open Command Prompt in `x64\Release` and run:

```bat
Server.exe
```

Optional server arguments:

```bat
Server.exe --bind-ip 0.0.0.0 --listen-port 54000
```

## Start the client

Open Command Prompt in `x64\Release` and run one of the following.

### Single client

```bat
Client.exe --server-ip localhost --server-port 54000 --send-interval-ms 1000
```

If `--telemetry-file` is not provided, the client selects a random telemetry file from `data/sample/`.

Example with an explicit file:

```bat
Client.exe --server-ip localhost --server-port 54000 --telemetry-file data/sample/telemetry_1.txt --send-interval-ms 1000
```

### Multiple clients using the built-in client launcher

```bat
Client.exe --client-count 3 --aircraft-start 1 --aircraft-end 3 --send-interval-ms 1000 --server-ip localhost --server-port 54000
```

This command spawns three client processes. The generated aircraft IDs use the simple format:

- `Aircraft_001`
- `Aircraft_002`
- `Aircraft_003`

## Batch test files

Only these batch files are kept in the repo:

- `LoadTest_Batch.bat`
- `EnduranceTest_Batch.bat`
- `SpikeTest_Batch.bat`

### How to use the batch files

1. Build the solution first.
2. Copy the required batch file into `x64\Release`.
3. Open Command Prompt in `x64\Release`.
4. Run the batch file.

### Load test

Default behavior matches the professor-style load script by spawning multiple `Client.exe` processes.

```bat
LoadTest_Batch.bat --server localhost --port 54000
```

Optional arguments:

```bat
LoadTest_Batch.bat --count 25 --server localhost --port 54000 --send-interval-ms 25
```

### Endurance test

```bat
EnduranceTest_Batch.bat --server localhost --port 54000
```

Optional arguments:

```bat
EnduranceTest_Batch.bat --count 100 --timeout-seconds 250 --server localhost --port 54000 --send-interval-ms 25
```

### Spike test

```bat
SpikeTest_Batch.bat --server localhost --port 54000
```

Optional arguments:

```bat
SpikeTest_Batch.bat --count 100 --server localhost --port 54000 --send-interval-ms 25
```

### Optional telemetry-file override for any batch file

```bat
LoadTest_Batch.bat --server localhost --port 54000 --telemetry-file data/sample/telemetry_2.txt
```

If `--telemetry-file` is omitted, each spawned client chooses a random telemetry file automatically.

## Notes about IDs and logs

- Automatically generated aircraft IDs use the format `Aircraft_001`, `Aircraft_002`, and so on.
- Each client session writes to its own log file in `output/logs/clients/`.
- The server writes its log to `output/logs/server/server.log`.
- Completed flight results are appended to `output/stats/flight_stats.csv`.
- Cumulative per-aircraft history is stored in `output/stats/aircraft_history.csv`.

## Default configuration files

- Client config: `config/client.config.json`
- Server config: `config/server.config.json`

The client and server can be started with config defaults only, or you can override settings with command-line arguments.
