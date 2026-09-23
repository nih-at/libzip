@echo off
set "GENERATOR_PLATFORM=%PLATFORM%"
if not "%PLATFORM%"=="ARM" exit /b 0

if "%TRIPLET%"=="arm-windows" goto desktop
if "%TRIPLET%"=="arm-uwp" goto uwp
echo Unexpected ARM triplet: %TRIPLET%
exit /b 1

:desktop
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64_arm 10.0.19041.0
if errorlevel 1 exit /b 1
goto configured

:uwp
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64_arm store 10.0.19041.0
if errorlevel 1 exit /b 1

:configured
if not "%WindowsSDKVersion%"=="10.0.19041.0\" goto invalid_environment
if not "%UCRTVersion%"=="10.0.19041.0" goto invalid_environment
if not "%VSCMD_ARG_HOST_ARCH%"=="x64" goto invalid_environment
if not "%VSCMD_ARG_TGT_ARCH%"=="arm" goto invalid_environment
if "%TRIPLET%"=="arm-windows" if not "%VSCMD_ARG_APP_PLAT%"=="Desktop" goto invalid_environment
if "%TRIPLET%"=="arm-uwp" if not "%VSCMD_ARG_APP_PLAT%"=="UWP" goto invalid_environment
for %%L in ("%WindowsSdkDir%Lib\10.0.19041.0\um\arm\kernel32.lib" "%WindowsSdkDir%Lib\10.0.19041.0\um\arm\WindowsApp.lib" "%WindowsSdkDir%Lib\10.0.19041.0\ucrt\arm\ucrt.lib") do if not exist "%%~L" goto invalid_environment

rem Keep vcpkg's clean child on the explicitly selected ARM SDK.
set "VCPKG_OVERLAY_TRIPLETS=%APPVEYOR_BUILD_FOLDER%\.github\appveyor-triplets"
set "GENERATOR_PLATFORM=ARM,version=10.0.19041.0"
set "CMAKE_OPTS=%CMAKE_OPTS% -DVCPKG_TARGET_TRIPLET=%TRIPLET%"
echo ARM build SDK: %WindowsSDKVersion% (%VSCMD_ARG_APP_PLAT%)
exit /b 0

:invalid_environment
echo ARM SDK 10.0.19041.0 initialization or library check failed.
exit /b 1
