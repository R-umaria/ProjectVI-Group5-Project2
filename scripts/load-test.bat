@echo off
setlocal enabledelayedexpansion
pushd %~dp0..\

set "EXE=x64\Release\ClientApp.exe"
if not exist "%EXE%" set "EXE=x64\Debug\ClientApp.exe"
if not exist "%EXE%" (
  echo Build ClientApp first.
  popd
  exit /b 1
)

set /A index=1
set /A count=25

:while
if !index! leq %count% (
  start "Client-!index!" /min "%EXE%" --aircraft-id AIR-LT-!index!
  set /A index=!index!+1
  goto :while
)

popd
endlocal
