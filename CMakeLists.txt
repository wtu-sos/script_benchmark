cmake_minimum_required(VERSION 3.16)
project(script_benchmark VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W3 /O2)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -O2)
endif()

# 选项开关
option(ENABLE_LUA "Enable Lua scripting support" ON)
option(ENABLE_LUAJIT "Enable LuaJIT support" OFF)
option(ENABLE_ANGELSCRIPT "Enable AngelScript support" OFF)
option(ENABLE_V8 "Enable V8 JavaScript support" OFF)
option(ENABLE_QUICKJS "Enable QuickJS support" OFF)

# 源文件
set(SOURCES
    src/main.cpp
    src/timer.cpp
    src/test_runner.cpp
    src/engines/cpp_engine.cpp
)

# 包含目录
include_directories(include)
include_directories(include/engines)

# Lua 支持
if(ENABLE_LUA)
    add_subdirectory(third_party/lua)
    list(APPEND SOURCES src/engines/lua_engine.cpp)
    add_definitions(-DENABLE_LUA)
endif()

# LuaJIT 支持
if(ENABLE_LUAJIT)
    # 使用手工构建的 LuaJIT 库
    set(LUAJIT_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/third_party/luajit/src/lua51.lib")
    
    if(EXISTS ${LUAJIT_LIBRARY_PATH})
        include_directories(third_party/luajit/src)
        list(APPEND SOURCES src/engines/luajit_engine.cpp)
        add_definitions(-DENABLE_LUAJIT)
        message(STATUS "Found LuaJIT library at: ${LUAJIT_LIBRARY_PATH}")
    else()
        message(WARNING "LuaJIT library not found, please build it first using msvcbuild.bat")
    endif()
endif()

# AngelScript 支持
if(ENABLE_ANGELSCRIPT)
    # 让 Angelsea 使用自己 vendored 的 AngelScript（包含内部 API）
    set(ASEA_ANGELSCRIPT_SYSTEM OFF CACHE BOOL "")
    # 不设置 ASEA_ANGELSCRIPT_ROOT，让它使用默认的 vendor/angelscript/sdk
    # MIR / fmt 默认使用 Angelsea 的 vendored 版本（需初始化其子模块）
    add_subdirectory(third_party/angelsea)

    list(APPEND SOURCES 
        src/engines/angelscript_engine.cpp
        third_party/angelsea/vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
    )
    # 使用 Angelsea 提供的 AngelScript 包含目录
    include_directories(third_party/angelsea/vendor/angelscript/sdk/angelscript/include)
    include_directories(third_party/angelsea/vendor/angelscript/sdk/add_on)
    add_definitions(-DENABLE_ANGELSCRIPT)
endif()

# V8 支持
if(ENABLE_V8)
    add_subdirectory(third_party/v8)
    list(APPEND SOURCES src/engines/v8_engine.cpp)
    add_definitions(-DENABLE_V8)
endif()

# QuickJS 支持
if(ENABLE_QUICKJS)
    add_subdirectory(third_party/quickjs)
    list(APPEND SOURCES src/engines/quickjs_engine.cpp)
    add_definitions(-DENABLE_QUICKJS)
endif()

# 主执行文件
add_executable(script_benchmark ${SOURCES})

# 链接库
if(ENABLE_LUA)
    target_link_libraries(script_benchmark lua)
endif()

if(ENABLE_LUAJIT)
    if(EXISTS ${LUAJIT_LIBRARY_PATH})
        target_link_libraries(script_benchmark ${LUAJIT_LIBRARY_PATH})
    endif()
endif()

if(ENABLE_ANGELSCRIPT)
    # 使用 Angelsea 构建的 AngelScript 目标（包含内部 API）
    target_link_libraries(script_benchmark angelscript angelsea)
endif()

if(ENABLE_V8)
    target_link_libraries(script_benchmark v8_real)
endif()

if(ENABLE_QUICKJS)
    target_link_libraries(script_benchmark quickjs_lib)
endif()
