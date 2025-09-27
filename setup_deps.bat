@echo off
echo Downloading and setting up script engine dependencies...

REM 创建 third_party 目录
if not exist "third_party" mkdir third_party
cd third_party

REM 下载 Lua 5.4.6
echo Downloading Lua 5.4.6...
if not exist "lua" (
    git clone https://github.com/lua/lua.git
    cd lua
    git checkout v5.4.6
    cd ..
) else (
    echo Lua already exists, skipping...
)

REM 下载 LuaJIT (可选)
echo Downloading LuaJIT...
if not exist "luajit" (
    git clone https://github.com/LuaJIT/LuaJIT.git luajit
    cd luajit
    REM 使用稳定分支
    git checkout v2.1
    REM 编译 LuaJIT (需要 MSVC)
    call src\msvcbuild.bat
    cd ..
) else (
    echo LuaJIT already exists, skipping...
)

REM 下载 AngelScript (可选)
echo Downloading AngelScript...
if not exist "angelscript" (
    git clone https://github.com/codecat/angelscript-mirror.git angelscript
) else (
    echo AngelScript already exists, skipping...
)

REM 下载 QuickJS (可选)
echo Downloading QuickJS...
if not exist "quickjs" (
    git clone https://github.com/bellard/quickjs.git
) else (
    echo QuickJS already exists, skipping...
)

cd ..

echo.
echo Dependencies setup complete!
echo.
echo To build the project:
echo   mkdir build
echo   cd build
echo   cmake .. -DENABLE_LUA=ON
echo   cmake --build . --config Release
echo.
echo To enable additional engines:
echo   cmake .. -DENABLE_LUA=ON -DENABLE_LUAJIT=ON -DENABLE_ANGELSCRIPT=ON
echo.