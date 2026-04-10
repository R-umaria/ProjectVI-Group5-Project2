@echo off
pushd %~dp0\..
call SpikeTest_Batch.bat %*
popd
