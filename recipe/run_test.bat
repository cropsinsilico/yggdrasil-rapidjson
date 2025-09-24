@setlocal
setlocal EnableDelayedExpansion

set examples=serialize yggdrasil units

for %%example in (%examples%) do (
  rem echo HERE
  rem dir
  rem echo "Building %%example"
  set builddir="example\%%example\build"
  rem if not exist "!builddir!" mkdir "!builddir!"
  rem if !errorlevel! neq 0 exit /b !errorlevel!
  rem cd "!builddir!"
  rem cmake -G "Ninja" ^
  rem       -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
  rem       ..
  rem if !errorlevel! neq 0 exit /b !errorlevel!
  rem cmake --build . --config Debug
  rem if !errorlevel! neq 0 exit /b !errorlevel!

  rem echo "Running %%example"
  rem "%%example.exe"
  rem if !errorlevel! neq 0 exit /b !errorlevel!

  cd ..\..\..
)

@endlocal
