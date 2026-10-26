@echo off
setlocal

where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] cmake not found. Install CMake and add it to PATH.
  pause
  exit /b 1
)

where qmake6 >nul 2>nul
if errorlevel 1 (
  echo [ERROR] qmake6 not found. Add Qt 6 msvc2022_64\bin to PATH.
  pause
  exit /b 1
)

for /f "usebackq delims=" %%i in (`qmake6 -query QT_INSTALL_PREFIX`) do set "QT_PREFIX=%%i"
echo [INFO] Qt prefix: %QT_PREFIX%

if not exist build mkdir build
pushd build

cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_PREFIX%" ..
if errorlevel 1 goto :failed

cmake --build . --config Debug --parallel
if errorlevel 1 goto :failed

cmake --install . --config Debug
if errorlevel 1 goto :failed

popd
echo.
echo [OK] Build and install completed.
echo      Run: install\bin\QtGraphEngineKit.exe
echo      Start MySQL80 and Redis before running.
pause
exit /b 0

:failed
popd
echo.
echo [ERROR] Configure/build/install failed. Read the messages above.
pause
exit /b 1
