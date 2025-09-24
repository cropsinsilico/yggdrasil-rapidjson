@setlocal
setlocal EnableDelayedExpansion

set examples=serialize yggdrasil units

for %%example in (%examples%) do (
  echo "Building %%example"
  set builddir="example\%%example\build"
  if not exist "!builddir!" mkdir "!builddir!"
  if !errorlevel! neq 0 exit /b !errorlevel!
  cd "!builddir!"
  cmake -G "Ninja" ^
        -D CMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
        ..
  if !errorlevel! neq 0 exit /b !errorlevel!
  cmake --build . --config Debug
  if !errorlevel! neq 0 exit /b !errorlevel!

  echo "Running %%example"
  "%%example.exe"
  if !errorlevel! neq 0 exit /b !errorlevel!

  cd ..\..\..
)

@endlocal
