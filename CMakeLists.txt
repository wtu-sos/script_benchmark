cmake_minimum_required(VERSION 3.16)
project(script_benchmark VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W3)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -O2)
endif()

# 选项开关
option(ENABLE_LUA "Enable Lua scripting support" ON)
option(ENABLE_LUAJIT "Enable LuaJIT support" ON)
option(ENABLE_ANGELSCRIPT "Enable AngelScript support" ON)
option(ENABLE_PYTHON "Enable Python3 support" ON)
option(ENABLE_QUICKJS "Enable QuickJS JavaScript support" OFF)
option(ENABLE_V8 "Enable V8 JavaScript support" ON)

# 主要源文件
set(SOURCES
    src/main.cpp
    src/engines/cpp_engine.cpp
    src/engines/luajit_engine.cpp
    src/engines/angelscript_engine.cpp
    src/engines/python_engine.cpp
    src/engines/quickjs_engine.cpp
    src/test_runner.cpp
    src/timer.cpp
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
    add_subdirectory(third_party/angelsea)
    list(APPEND SOURCES 
        src/engines/angelscript_engine.cpp
        third_party/angelsea/vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
    )
    include_directories(third_party/angelsea/vendor/angelscript/sdk/angelscript/include)
    include_directories(third_party/angelsea/vendor/angelscript/sdk/add_on)
    add_definitions(-DENABLE_ANGELSCRIPT)
endif()

# QuickJS 支持（关闭，改用 V8）
if(ENABLE_QUICKJS)
    add_definitions(-DENABLE_QUICKJS)
    message(WARNING "QuickJS enabled, but this build uses V8 by default. Disable ENABLE_QUICKJS to avoid conflicts.")
endif()

# V8 支持（嵌入式链接 v8_monolith.lib）
if(ENABLE_V8)
    set(V8_ROOT "${CMAKE_SOURCE_DIR}/third_party/v8")
    include_directories(${V8_ROOT}/include)
    list(APPEND SOURCES src/engines/v8_engine.cpp)
    add_definitions(-DENABLE_V8)
    set(V8_LIB "${V8_ROOT}/out/win-release/v8_monolith.lib")
    if(EXISTS ${V8_LIB})
        message(STATUS "Linking V8: ${V8_LIB}")
        set(V8_LINK_LIB ${V8_LIB})
    else()
        message(FATAL_ERROR "V8 library not found at ${V8_LIB}. Please run third_party/setup_v8.bat")
    endif()
endif()

# Python 支持
if(ENABLE_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    if(Python3_FOUND)
        list(APPEND SOURCES src/engines/python_engine.cpp)
        include_directories(${Python3_INCLUDE_DIRS})
        add_definitions(-DENABLE_PYTHON)
        message(STATUS "Found Python3: ${Python3_VERSION}")
        message(STATUS "Python3 Include Dirs: ${Python3_INCLUDE_DIRS}")
        message(STATUS "Python3 Libraries: ${Python3_LIBRARIES}")
    else()
        message(WARNING "Python3 development headers not found")
    endif()
endif()


# 主执行文件
add_executable(script_benchmark ${SOURCES})

# 确保使用 C++20（MSVC 显式开启 /std:c++20）
if(MSVC)
    target_compile_options(script_benchmark PRIVATE /std:c++20 /Zc:__cplusplus $<$<CONFIG:Release>:/O2>)
endif()

# 链接 V8
if(ENABLE_V8)
    target_link_libraries(script_benchmark ${V8_LINK_LIB})
endif()

# Windows 下需要链接 ws2_32 等（QuickJS 使用 socket/线程时可能需要）。本项目只用到基础功能，通常无需额外库。

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
    target_link_libraries(script_benchmark angelscript angelsea)
endif()

# QuickJS 这里采用静态编译入可执行文件，不需要单独链接库


if(ENABLE_PYTHON)
    if(Python3_FOUND)
        target_link_libraries(script_benchmark ${Python3_LIBRARIES})
    endif()
endif()