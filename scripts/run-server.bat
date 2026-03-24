@echo off
setlocal
pushd %~dp0..\
if not exist x64\Debug\ServerApp.exe (
  echo Build the solution first from Visual Studio.
  popd
  exit /b 1
)
start "ServerApp" x64\Debug\ServerApp.exe
popd
endlocal
