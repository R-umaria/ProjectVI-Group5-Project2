@echo off
setlocal
pushd %~dp0\..
if exist output\state\aircraft_id_counter.txt del /f /q output\state\aircraft_id_counter.txt
if exist output\state\aircraft_id_counter.lock rmdir /s /q output\state\aircraft_id_counter.lock
echo Reset local client aircraft ID counter.
popd
endlocal
