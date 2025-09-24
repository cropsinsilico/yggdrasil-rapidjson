for %%x in (serialize yggdrasil units) do echo %%x
rem @setlocal
rem setlocal EnableDelayedExpansion

rem set examples=serialize yggdrasil units

rem for %%example in (%examples%) do (
rem   rem echo "Building %%example"
rem   rem set builddir="example\%%example\build"
rem   rem if not exist "!builddir!" mkdir "!builddir!"
rem   rem if !errorlevel! neq 0 exit /b !errorlevel!
rem   rem cd "!builddir!"
rem   rem cmake -G "Ninja" ^
rem   rem       -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
rem   rem       ..
rem   rem if !errorlevel! neq 0 exit /b !errorlevel!
rem   rem cmake --build . --config Debug
rem   rem if !errorlevel! neq 0 exit /b !errorlevel!

rem   rem echo "Running %%example"
rem   rem "%%example.exe"
rem   rem if !errorlevel! neq 0 exit /b !errorlevel!

rem   rem cd ..\..\..
rem )

rem @endlocal
