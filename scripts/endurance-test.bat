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

set /A count=100
set /A delaySeconds=250
set /A wave=1

:loop
set /A index=1
:spawn
if !index! leq %count% (
  start "Endurance-!wave!-!index!" /min "%EXE%" --aircraft-id AIR-END-!wave!-!index!
  set /A index=!index!+1
  goto :spawn
)

set /A wave=!wave!+1
timeout /t %delaySeconds% > NUL
goto :loop
