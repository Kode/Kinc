@echo off
REM
REM build.cmd - rebuild generated ETW tracing files from LibOVREvents.man
REM

REM assume mc.exe is in a path relative to xperf.exe
for /f "delims=" %%a in ('where /F Xperf') do set XPERF_PATH=%%~dpa

set OSTYPE=x86
if not "%PROCESSOR_ARCHITECTURE%"=="x86" set OSTYPE=x64
if not "%PROCESSOR_ARCHITEW6432%"=="" set OSTYPE=x64

set MC="%XPERF_PATH%..\bin\%OSTYPE%\mc.exe"

%MC% -v -a -A -n -um .\LibOVREvents.man -h . -z LibOVREvents
