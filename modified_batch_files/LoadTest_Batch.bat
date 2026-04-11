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

set /A count=25
if not "%~1"=="" set /A count=%~1

set /A aircraftStart=1
if not "%~2"=="" set /A aircraftStart=%~2

set "aircraftPrefix=AIRCRAFT"
if not "%~3"=="" set "aircraftPrefix=%~3"

set "serverIp=%~4"
set "serverPort=%~5"
set "telemetryFile=%~6"

set "extraArgs="
if not "%serverIp%"=="" set "extraArgs=!extraArgs! --server-ip %serverIp%"
if not "%serverPort%"=="" set "extraArgs=!extraArgs! --server-port %serverPort%"
if not "%telemetryFile%"=="" set "extraArgs=!extraArgs! --telemetry-file ""%telemetryFile%"""

set /A index=0
:while
if !index! lss %count% (
    set /A aircraftNumber=%aircraftStart% + !index!
    start "Client-!aircraftNumber!" /min "%EXE%" --aircraft-id %aircraftPrefix%-!aircraftNumber! !extraArgs!
    set /A index=!index! + 1
    @echo Spawned %aircraftPrefix%-!aircraftNumber!
    goto :while
)

popd
endlocal
