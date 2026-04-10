@echo off
pushd %~dp0\..
call EnduranceTest_Batch.bat %*
popd
