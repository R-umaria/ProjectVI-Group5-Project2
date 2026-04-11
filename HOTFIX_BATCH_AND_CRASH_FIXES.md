# Hotfix Summary

## Included fixes

1. **Professor batch-file compatibility restored**
   - Client build output renamed to `Client.exe`
   - Server build output renamed to `Server.exe`
   - Root `LoadTest_Batch.bat` and `EnduranceTest_Batch.bat` now match the professor-provided scripts exactly
   - Added `.txt` copies as well

2. **Client runtime path robustness**
   - Client now resolves config and telemetry files relative to the executable/current working directory
   - This allows `Client.exe` to be launched from the professor batch scripts without requiring the solution root as the working directory

3. **Crash/disconnect persistence fix**
   - Completed/partial flight statistics are now written synchronously on disconnect
   - This prevents disconnect results from being lost due to pending async write queues

4. **Session cleanup visibility**
   - Server logs active-session count after disconnect completion
   - Active session memory is released when the aircraft session is removed from the in-memory map

## Expected usage

- Build `Release x64`
- Use `Client.exe` for batch launching
- Use `Server.exe` for the server process
- Keep `config` and `data` folders with the repo layout when running locally
