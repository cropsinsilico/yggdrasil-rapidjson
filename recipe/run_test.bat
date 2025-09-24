@setlocal

set examples=serialize yggdrasil units

(for %%example in (%examples%) do (
     echo Building %%example
     if not exist "example\%%example\build" mkdir example\%%example\build
     cd example\%%example\build
     cmake -G "Ninja" ^
           -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
           ..
     if errorlevel 1 exit 1
     cmake --build . --config Debug
     if errorlevel 1 exit 1

     echo Running %%example
     %%example.exe
     if errorlevel 1 exit 1

     cd ..\..\..
))
@endlocal
