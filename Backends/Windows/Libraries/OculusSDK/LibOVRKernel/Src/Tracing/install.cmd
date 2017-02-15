@echo off
setlocal
REM run this script from an Admin shell to set up ETW tracing

set SCRIPTDIR=%~dp0

REM set SDK_MANIFEST_PATH to the SDK install path (e.g. C:\Program Files (x86)\Oculus)
for /f "delims=" %%a in ('reg query "HKLM\SOFTWARE\Wow6432Node\Oculus VR, LLC\Oculus" -v "Base"') do set SDK_INSTALL_PATH=%%a
set SDK_INSTALL_PATH=%SDK_INSTALL_PATH:    Base    REG_SZ    =%
set SDK_MANIFEST_PATH=%SDK_INSTALL_PATH%\oculus-tools\etw

REM Add USERS Read & Execute privileges to the folder
icacls . /grant BUILTIN\Users:(OI)(CI)(RX) >nul
if %errorlevel% equ 0 goto CaclsOk

echo Failed to set cacls, installation may fail

:CaclsOk

for /f "delims=" %%a in ('reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" -v "ProductName"') do set PRODUCT_NAME=%%a
set PRODUCT_NAME=%PRODUCT_NAME: =%
set PRODUCT_NAME=%PRODUCT_NAME:ProductNameREG_SZ=%
set PRODUCT_NAME=%PRODUCT_NAME:dows=%
set PRODUCT_NAME=%PRODUCT_NAME:Enterprise=%
set PRODUCT_NAME=%PRODUCT_NAME:Professional=%
set SHORT_PRODUCT_NAME=%PRODUCT_NAME:.1=%
echo Installing %PRODUCT_NAME% manifests:

rem we only support x64 oses these days
set OSTYPE=x64
set OCUSBVID_SYS=%windir%\System32\drivers\ocusbvid111.sys
if "%SHORT_PRODUCT_NAME%"=="Win7" set OCUSBVID_SYS=%windir%\System32\drivers\ocusbvid109.sys
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto GotOSTYPE
if "%PROCESSOR_ARCHITEW6432%"=="AMD64" goto GotOSTYPE

echo 32-bit OS not supported
exit /b 1

:GotOSTYPE

REM disable paging on x64 systems if stack walks are desired
if %OSTYPE% neq x64 goto SkipRegCheck
for /f "delims=" %%a in ('reg query "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" -v "DisablePagingExecutive"') do set REG_DPA=%%a

if %REG_DPA:~-3% equ 0x1 goto SkipRegCheck
echo ************************
echo DisablePagingExecutive should be set if you want stack tracing to work on %OSTYPE%
echo To disable paging run the following as Administrator:
echo   reg add "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" -v DisablePagingExecutive -d 0x1 -t REG_DWORD -f
echo and reboot
echo ************************

:SkipRegCheck

set RIFTCAMERADRIVER_DIR=%SCRIPTDIR%..\..\..\RiftPTDriver

set USBVID_EVENTS_MAN=%SDK_MANIFEST_PATH%\OVRUSBVidEvents.man
if exist "%RIFTCAMERADRIVER_DIR%\OCUSBVID\OVRUSBVidEvents.man" set USBVID_EVENTS_MAN=%RIFTCAMERADRIVER_DIR%\OCUSBVID\OVRUSBVidEvents.man
if exist "%SCRIPTDIR%OVRUSBVidEvents.man" set USBVID_EVENTS_MAN=%SCRIPTDIR%OVRUSBVidEvents.man

echo Installing %OCUSBVID_SYS% manifest...
REM uninstall any existing manifest first
wevtutil.exe uninstall-manifest "%USBVID_EVENTS_MAN%"
if %errorlevel% neq 0 echo WARNING: This step failed.
wevtutil.exe install-manifest "%USBVID_EVENTS_MAN%" /rf:"%OCUSBVID_SYS%" /mf:"%OCUSBVID_SYS%"
REM make sure it worked
wevtutil get-publisher OVR-USBVid > nul
if %errorlevel% neq 0 echo WARNING: This step failed.
echo Installed %USBVID_EVENTS_MAN%

set LIBOVR_EVENTS_MAN=%SDK_MANIFEST_PATH%\LibOVREvents.man
if exist "%SCRIPTDIR%LibOVREvents.man" set LIBOVR_EVENTS_MAN=%SCRIPTDIR%LibOVREvents.man

REM get rid of stale dll's
del /f /q "%SCRIPTDIR%LibOVRRT*.dll"
set LIBOVR_PATTERN=LibOVRRT*_1.dll
echo Looking for %LIBOVR_PATTERN% dll's
REM this nightmare command copies the newest version of %LIBOVR_PATTERN% into the current directory without prompting...
forfiles /p:"%SDK_INSTALL_PATH%Support\oculus-runtime" /m:%LIBOVR_PATTERN% /c "cmd /c xcopy /y /f /d @path \"%SCRIPTDIR%.\" >nul" >nul 2>nul
if not exist "%SCRIPTDIR%..\..\..\LibOVR\Lib\Windows" goto NoLibOVRSource
forfiles /s /p:"%SCRIPTDIR%..\..\..\LibOVR\Lib\Windows" /m:%LIBOVR_PATTERN% /c "cmd /c xcopy /y /f /d @path \"%SCRIPTDIR%.\" >nul" >nul 2>nul
:NoLibOVRSource
for /f "delims=" %%a in ('dir /b /o:d "%SCRIPTDIR%%LIBOVR_PATTERN%"') do set LIBOVR_DLL=%%a
echo Installing %LIBOVR_DLL% manifest...
REM uninstall any existing manifest first
wevtutil uninstall-manifest "%LIBOVR_EVENTS_MAN%"
if %errorlevel% neq 0 exit /b 1

REM use absolute paths to the RT .dll, otherwise we risk picking up the wrong (e.g. installed) version from %PATH%
echo wevtutil install-manifest "%LIBOVR_EVENTS_MAN%" /rf:"%SCRIPTDIR%%LIBOVR_DLL%" /mf:"%SCRIPTDIR%%LIBOVR_DLL%"
wevtutil install-manifest "%LIBOVR_EVENTS_MAN%" /rf:"%SCRIPTDIR%%LIBOVR_DLL%" /mf:"%SCRIPTDIR%%LIBOVR_DLL%"
REM note we can't do del /f /q "%SCRIPTDIR%%LIBOVR_PATTERN%" here because the binary has to be present for ETW enumeration to work
REM make sure it worked
wevtutil get-publisher OVR-SDK-LibOVR > nul
if %errorlevel% neq 0 exit /b 1
echo Installed %LIBOVR_EVENTS_MAN%

if not exist "%SCRIPTDIR%..\..\..\Tools" exit /b 0

echo You can now start/stop traces with the GUI:
echo   cd %SCRIPTDIR%..\..\..\Tools\TraceScript\ovrtap
echo   .\startovrtap.cmd
echo or (command-line):
echo   cd %SCRIPTDIR%..\..\..\Tools\Xperf
echo   ovrlog
