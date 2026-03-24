# Fleet Telemetry System

A Visual Studio bootstrap for the CSCN73060 client-server fleet telemetry project.

## Solution Layout

- **ClientApp**: console client that reads telemetry data and sends packets to the server.
- **ServerApp**: console server scaffold for multi-client processing.
- **SharedLib**: shared models, logging, config loading, and packet helpers.

## Open in Visual Studio

1. Open `FleetTelemetrySystem.sln`
2. Select **Debug** or **Release**
3. Build the solution
4. Set `ServerApp` as startup project to run the server, or `ClientApp` to run the client

## Runtime Folders

- `config/` for client and server configuration
- `data/` for telemetry inputs
- `output/logs/` for logs
- `output/stats/` for generated statistics
- `output/performance/` for performance test artifacts

## Notes

- This scaffold is intentionally lightweight so the team can iterate quickly.
- The networking and telemetry logic are placeholders that compile cleanly and are ready to extend.
- Project paths are configured relative to the solution directory to reduce setup friction.
