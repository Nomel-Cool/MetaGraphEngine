@echo off
setlocal

:: 设置一些变量
set BUILD_DIR=build
set CONFIG=Debug

:: 创建并导航到 build 目录
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
)
cd %BUILD_DIR%

:: 运行 CMake 配置
cmake -G "Visual Studio 17 2022" -A x64 ..

:: 运行构建
cmake --build . --config %CONFIG%

:: 运行安装
cmake --install . --config %CONFIG%

:: 返回到源目录
cd ..

echo Build and install completed.
pause
