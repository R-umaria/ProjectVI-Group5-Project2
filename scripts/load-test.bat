@echo off
pushd %~dp0\..
call LoadTest_Batch.bat %*
popd
