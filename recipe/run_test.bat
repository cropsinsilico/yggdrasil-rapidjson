@setlocal
setlocal EnableDelayedExpansion

set examples=serialize yggdrasil units

for %%example in (%examples%) do (
  echo HERE
  dir
  echo "Building %%example"
  set builddir="example\%%example\build"
  if not exist "!builddir!" mkdir "!builddir!"
  if !errorlevel! neq 0 exit /b !errorlevel!
  cd "!builddir!"
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
