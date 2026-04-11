Hotfix summary

1. Client.exe now chooses a random telemetry file from data/sample whenever no explicit --telemetry-file is provided. This keeps the professor batch files compatible while avoiding the same telemetry input for every spawned client.
2. Auto-generated client IDs now use the simple format AIR_001, AIR_002, etc. The next value is assigned through a small shared counter file so simultaneous launches do not reuse the same local ID.
3. Server memory cleanup was tightened in AircraftSessionManager:
   - active session state is moved out and destroyed as soon as a client disconnects
   - the active-session hash table is compacted after removals
   - long-term per-aircraft cumulative history is persisted to output/stats/aircraft_history.csv instead of being kept in RAM for the whole run

Optional for multi-PC distributed tests
- To avoid AIR_001 collisions across different client PCs, set a different offset on each client PC before running the professor batch files. Example:
  set FLEET_AIRCRAFT_ID_OFFSET=1000
- You can reset the local counter with scripts\reset-client-state.bat
