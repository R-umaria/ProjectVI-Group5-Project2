@echo off
setlocal
pushd %~dp0..\

set "EXE=x64\Debug\Client.exe"
if not exist "%EXE%" set "EXE=x64\Release\Client.exe"
if not exist "%EXE%" (
  echo Build the solution first from Visual Studio.
  popd
  exit /b 1
)

start "ClientApp" "%EXE%" %*
popd
endlocal
