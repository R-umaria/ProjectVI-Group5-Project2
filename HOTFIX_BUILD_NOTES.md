# Hotfix Build Notes

This zip fixes the Visual Studio build errors reported after the previous package.

## Fixed

- Removed the accidental dependency on `<random>` in `src/client/ClientApp.cpp`.
- Made fallback telemetry-file selection deterministic by choosing the first sample file in sorted order.
- Fixed Windows launcher code in `src/client/main.cpp`:
  - added `NOMINMAX` before `windows.h`
  - removed invalid `CREATE_MINIMIZED` usage
  - used `STARTUPINFOA.wShowWindow = SW_SHOWMINNOACTIVE` for minimized child launches
  - avoided the `std::max` / Windows macro collision
- Kept the earlier performance improvements and deterministic aircraft ID range behavior.

## Expected client launch modes

### Normal single client
`ClientApp.exe --aircraft-id AIRCRAFT-1001 --server-ip 192.168.1.10 --server-port 54000 --telemetry-file data\\sample\\telemetry_1.txt`

### Internal range launcher
`ClientApp.exe --client-count 25 --aircraft-start 1001 --aircraft-end 1025 --aircraft-prefix AIRCRAFT --server-ip 192.168.1.10 --server-port 54000 --telemetry-file data\\sample\\telemetry_1.txt`

### Professor-style batch launch
`LoadTest_Batch.bat 25 1001 AIRCRAFT 192.168.1.10 54000 data\\sample\\telemetry_1.txt`
