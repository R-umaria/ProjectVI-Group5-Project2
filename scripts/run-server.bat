@echo off
setlocal
pushd %~dp0..\

set "EXE=x64\Debug\Server.exe"
if not exist "%EXE%" set "EXE=x64\Release\Server.exe"
if not exist "%EXE%" (
  echo Build the solution first from Visual Studio.
  popd
  exit /b 1
)

start "ServerApp" "%EXE%" %*
popd
endlocal
