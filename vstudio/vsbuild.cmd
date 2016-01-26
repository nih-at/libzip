@echo off
setlocal enableextensions enabledelayedexpansion

rem ---------------------------------------------------------------------------
rem Check that required commands are in the PATH.
rem ---------------------------------------------------------------------------
set CHECK_CMD=cmake.exe
set CHECK_DESC=CMake
call :check_installed
set CHECK_CMD=msbuild.exe
set CHECK_DESC=MSBuild
call :check_installed

rem ---------------------------------------------------------------------------
rem Parse command-line arguments.
rem %1: Command ("build" or "clean")
rem %2: Generator (e.g. "Visual Studio 12" for VS2013)
rem %3: Platform toolset (e.g. "v120_xp" for VS2013 toolset for Windows XP)
rem ---------------------------------------------------------------------------
if "%1"=="clean" (
	echo Cleaning
	if exist zlib\installed rmdir /s /q zlib\installed
	if errorlevel 1 goto exit_failure
	if exist zlib\build rmdir /s /q zlib\build
	if errorlevel 1 goto exit_failure
	if exist ..\build rmdir /s /q ..\build
	if errorlevel 1 goto exit_failure
	if exist ..\regress\bigzero.zip del ..\regress\bigzero.zip
	if errorlevel 1 goto exit_failure
	if exist ..\regress\manyfiles.zip del ..\regress\manyfiles.zip
	if errorlevel 1 goto exit_failure
	echo Done
	exit /b 0
) else if "%1"=="build" (
	set CMAKE_GENERATOR=%2
	set CMAKE_TOOLSET=%3
	set LIBZIP_RUN_TESTS=false
) else if "%1"=="build+test" (
	set CMAKE_GENERATOR=%2
	set CMAKE_TOOLSET=%3
	set LIBZIP_RUN_TESTS=true
) else (
	echo Invalid command "%1"
	exit /b 1
)

rem ---------------------------------------------------------------------------
rem If we're running tests, we'll also need a Perl interpreter.
rem ---------------------------------------------------------------------------
if "%LIBZIP_RUN_TESTS%"=="true" (
	set CHECK_CMD=perl.exe
	set CHECK_DESC=a Perl interpreter (to run tests)
	call :check_installed
)

rem ---------------------------------------------------------------------------
rem Configure and build zlib.
rem ---------------------------------------------------------------------------
pushd zlib
for /f %%p in (".\installed") do set ZLIB_INSTALL_PATH=%%~fp
echo zlib will be "installed" to %ZLIB_INSTALL_PATH%
if not exist build (
	mkdir build
	if errorlevel 1 popd & goto exit_failure
)
cd build
if errorlevel 1 popd & goto exit_failure
echo Configuring zlib
cmake .. -G %CMAKE_GENERATOR% -T %CMAKE_TOOLSET% -DCMAKE_INSTALL_PREFIX="%ZLIB_INSTALL_PATH%"
if errorlevel 1 popd & goto exit_failure
echo Building zlib
msbuild /P:Configuration=Debug INSTALL.vcxproj
if errorlevel 1 popd & goto exit_failure
msbuild /P:Configuration=Release INSTALL.vcxproj
if errorlevel 1 popd & goto exit_failure
popd

rem ---------------------------------------------------------------------------
rem regress/CMakeLists.txt thinks the Perl script to run a test is called
rem runtest, but on Windows we have to explicitly start the Perl interpreter
rem because cmd.exe doesn't do "shebang".
rem Fix this up before configuring libzip.
rem ---------------------------------------------------------------------------
if "%LIBZIP_RUN_TESTS%"=="true" (
	echo Fixing up runtest script for Windows
	pushd ..\regress
	rem Get the full path to the Perl interpreter, with backslashes replaced by
	rem forward slashes.
	for /f "usebackq tokens=*" %%a in (`where perl.exe`) do set PERL_PATH=%%a
	set PERL_PATH=!PERL_PATH:\=/!
	echo Path to Perl interpreter is !PERL_PATH!

	rem Fix up the CMakeLists.txt file.
	perl -p -e "s|\$\{CMAKE_CURRENT_SOURCE_DIR\}/runtest|!PERL_PATH! \$\{CMAKE_CURRENT_BINARY_DIR\}/runtest|;" CMakeLists.txt > CMakeLists.fixed.txt
	if errorlevel 1 popd & goto exit_failure
	rename CMakeLists.txt CMakeLists.orig.txt
	if errorlevel 1 popd & goto exit_failure
	rename CMakeLists.fixed.txt CMakeLists.txt
	if errorlevel 1 popd & goto exit_failure
	popd
)

rem ---------------------------------------------------------------------------
rem Prepare the build directory and run CMake to configure the project.
rem ---------------------------------------------------------------------------
pushd ..
if not exist build (
	echo Creating build directory
	mkdir build
	if errorlevel 1 popd & goto exit_failure
)
cd build
if errorlevel 1 popd & goto exit_failure
cmake .. -G %CMAKE_GENERATOR% -T %CMAKE_TOOLSET% -DCMAKE_PREFIX_PATH="%ZLIB_INSTALL_PATH%"
if errorlevel 1 popd & goto exit_failure
call :revert_cmakelists

rem ---------------------------------------------------------------------------
rem Build libzip.
rem ---------------------------------------------------------------------------
msbuild /P:Configuration=Debug ALL_BUILD.vcxproj
if errorlevel 1 popd & goto exit_failure
msbuild /P:Configuration=Release ALL_BUILD.vcxproj
if errorlevel 1 popd & goto exit_failure
popd

rem ---------------------------------------------------------------------------
rem Copy DLLs so zipcmp/zipmerge can run.
rem ---------------------------------------------------------------------------
echo Copying DLLs
copy zlib\installed\bin\zlibd.dll ..\build\src\Debug
if errorlevel 1 goto exit_failure
copy zlib\installed\bin\zlib.dll ..\build\src\Release
if errorlevel 1 goto exit_failure
copy ..\build\lib\Release\zip.dll ..\build\src\Release
if errorlevel 1 goto exit_failure
copy ..\build\lib\Debug\zip.dll ..\build\src\Debug
if errorlevel 1 goto exit_failure

rem ---------------------------------------------------------------------------
rem Run the tests, if required.
rem ---------------------------------------------------------------------------
if "%LIBZIP_RUN_TESTS%"=="true" (
	echo Copying libraries for tests
	pushd ..\build\regress
	copy ..\..\vstudio\zlib\installed\bin\zlib.dll .
	if errorlevel 1 popd & goto exit_failure
	copy ..\lib\Release\zip.dll .
	if errorlevel 1 popd & goto exit_failure
	copy Release\*.exe .
	if errorlevel 1 popd & goto exit_failure
	echo Extracting test files
	if not exist ..\..\regress\bigzero.zip ziptool ..\..\regress\bigzero-zip.zip cat 0 > ..\..\regress\bigzero.zip
	if errorlevel 1 popd & goto exit_failure
	if not exist ..\..\regress\manyfiles.zip ziptool ..\..\regress\manyfiles-zip.zip cat 0 > ..\..\regress\manyfiles.zip
	if errorlevel 1 popd & goto exit_failure
	echo Generating runtest script
	for /f %%p in ("..\..\regress") do set ABS_SRCDIR=%%~fp
	set ABS_SRCDIR=!ABS_SRCDIR:\=\\!
	perl -p -e "s/@[s]rcdir@/..\\..\\regress/g;s/@[a]bs_srcdir@/!ABS_SRCDIR!/g;s|../../src/zipcmp|..\\..\\src\\Release\\zipcmp|g;" ..\..\regress\runtest.in > runtest
	if errorlevel 1 popd & goto exit_failure
	echo Running tests
	ctest
	if errorlevel 1 popd & goto exit_failure
	popd
)

goto :EOF

:check_installed
where %CHECK_CMD% > nul 2>&1
if "%errorlevel%"=="9009" (
  echo This build script requires where.exe. If running on Windows XP or
  echo earlier, this can be found in the Windows Resource Kit.
  exit /b 1
)
if errorlevel 1 (
  echo Please make sure that %CHECK_DESC% is installed and in your PATH.
  exit /b 1
)
goto :EOF

:revert_cmakelists
if exist ..\regress\CMakeLists.orig.txt (
	del ..\regress\CMakeLists.txt
	rename ..\regress\CMakeLists.orig.txt CMakeLists.txt
)
goto :EOF

:exit_failure
call :revert_cmakelists
echo Build failed.
exit /b 1
