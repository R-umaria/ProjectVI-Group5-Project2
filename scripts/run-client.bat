@echo off
setlocal
pushd %~dp0..\
if not exist x64\Debug\ClientApp.exe (
  echo Build the solution first from Visual Studio.
  popd
  exit /b 1
)
start "ClientApp" x64\Debug\ClientApp.exe
popd
endlocal
