@echo off
setlocal enabledelayedexpansion
pushd %~dp0

set "EXE=x64\Release\ClientApp.exe"
if not exist "%EXE%" set "EXE=x64\Debug\ClientApp.exe"
if not exist "%EXE%" (
    echo Build ClientApp first.
    popd
    exit /b 1
)

set /A count=100
if not "%~1"=="" set /A count=%~1

set /A aircraftStart=1
if not "%~2"=="" set /A aircraftStart=%~2

set /A timeoutSeconds=250
if not "%~3"=="" set /A timeoutSeconds=%~3

set "aircraftPrefix=AIRCRAFT"
if not "%~4"=="" set "aircraftPrefix=%~4"

set "serverIp=%~5"
set "serverPort=%~6"
set "telemetryFile=%~7"

set "extraArgs="
if not "%serverIp%"=="" set "extraArgs=!extraArgs! --server-ip %serverIp%"
if not "%serverPort%"=="" set "extraArgs=!extraArgs! --server-port %serverPort%"
if not "%telemetryFile%"=="" set "extraArgs=!extraArgs! --telemetry-file ""%telemetryFile%"""

set /A wave=0
:while
set /A wave=!wave! + 1
@echo Wave !wave! started at %time%
set /A index=0
:spawnloop
if !index! lss %count% (
    set /A aircraftNumber=%aircraftStart% + ((!wave! - 1) * %count%) + !index!
    start "Client-!aircraftNumber!" /min "%EXE%" --aircraft-id %aircraftPrefix%-!aircraftNumber! !extraArgs!
    set /A index=!index! + 1
    @echo Spawned %aircraftPrefix%-!aircraftNumber!
    goto :spawnloop
)
timeout /t %timeoutSeconds% > NUL
goto :while
