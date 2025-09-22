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
    # LuaJIT 需要手动编译，这里假设已编译好
    find_library(LUAJIT_LIB luajit PATHS third_party/luajit/src)
    if(LUAJIT_LIB)
        include_directories(third_party/luajit/src)
        list(APPEND SOURCES src/engines/luajit_engine.cpp)
        add_definitions(-DENABLE_LUAJIT)
    else()
        message(WARNING "LuaJIT library not found, please build it first")
    endif()
endif()

# AngelScript 支持
if(ENABLE_ANGELSCRIPT)
    add_subdirectory(third_party/angelscript)
    list(APPEND SOURCES 
        src/engines/angelscript_engine.cpp
        src/engines/angelscript_jit.cpp
        third_party/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
    )
    include_directories(third_party/angelscript/sdk/add_on)
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

if(ENABLE_LUAJIT AND LUAJIT_LIB)
    target_link_libraries(script_benchmark ${LUAJIT_LIB})
endif()

if(ENABLE_ANGELSCRIPT)
    target_link_libraries(script_benchmark angelscript)
endif()

if(ENABLE_V8)
    target_link_libraries(script_benchmark v8_mock)
endif()

if(ENABLE_QUICKJS)
    target_link_libraries(script_benchmark quickjs)
endif()
