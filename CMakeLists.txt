cmake_minimum_required(VERSION 3.16)
project(script_benchmark VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引擎选项
option(ENABLE_LUA "Enable Lua engine" ON)
option(ENABLE_LUAJIT "Enable LuaJIT engine" OFF)  # 暂时禁用避免配置复杂度
option(ENABLE_V8 "Enable V8 engine" OFF)          # 暂时禁用避免配置复杂度
option(ENABLE_ANGELSCRIPT "Enable AngelScript engine" ON)
option(ENABLE_PYTHON "Enable Python engine" ON)

# 将选项作为预处理器定义传递给编译器
if(ENABLE_LUA)
    add_definitions(-DENABLE_LUA)
endif()

if(ENABLE_LUAJIT)
    add_definitions(-DENABLE_LUAJIT)
endif()

if(ENABLE_V8)
    add_definitions(-DENABLE_V8)
endif()

if(ENABLE_ANGELSCRIPT)
    add_definitions(-DENABLE_ANGELSCRIPT)
endif()

if(ENABLE_PYTHON)
    add_definitions(-DENABLE_PYTHON)
endif()

# 全局设置：强制所有目标使用静态运行时库以匹配 V8
if(MSVC)
    # 必须在任何 add_subdirectory 之前设置，以影响所有子项目
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    
    # 全局编译选项 - 仅应用到C++编译器，不影响汇编器
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/W3>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++20>)  
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
    add_compile_options($<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:/O2>)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -O2)
endif()

# 主要源文件
set(SOURCES
    src/main.cpp
    src/engines/cpp_engine.cpp
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
endif()

# LuaJIT 支持
if(ENABLE_LUAJIT)
    # 使用手工构建的 LuaJIT 库
    set(LUAJIT_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/third_party/luajit/src/lua51.lib")
    
    if(EXISTS ${LUAJIT_LIBRARY_PATH})
        include_directories(third_party/luajit/src)
        list(APPEND SOURCES src/engines/luajit_engine.cpp)
        message(STATUS "Found LuaJIT library at: ${LUAJIT_LIBRARY_PATH}")
    else()
        message(WARNING "LuaJIT library not found, please build it first using msvcbuild.bat")
    endif()
endif()

# AngelScript 支持
if(ENABLE_ANGELSCRIPT)
    # 让 Angelsea 使用自己 vendored 的 AngelScript（包含内部 API）
    set(ASEA_ANGELSCRIPT_SYSTEM OFF CACHE BOOL "")
    # 完全禁用 MIR JIT 功能，避免链接错误
    set(ASEA_MIR_SYSTEM OFF CACHE BOOL "" FORCE)
    set(ASEA_ENABLE_JIT OFF CACHE BOOL "" FORCE)
    add_subdirectory(third_party/angelsea)
    list(APPEND SOURCES 
        src/engines/angelscript_engine.cpp
        third_party/angelsea/vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
    )
    include_directories(third_party/angelsea/vendor/angelscript/sdk/angelscript/include)
    include_directories(third_party/angelsea/vendor/angelscript/sdk/add_on)
endif()

# QuickJS 支持（关闭，改用 V8）
if(ENABLE_QUICKJS)
    message(WARNING "QuickJS enabled, but this build uses V8 by default. Disable ENABLE_QUICKJS to avoid conflicts.")
endif()

# V8 支持（嵌入式链接 v8_monolith.lib）
if(ENABLE_V8)
    set(V8_ROOT "${CMAKE_SOURCE_DIR}/third_party/v8")
    include_directories(${V8_ROOT}/include)
    list(APPEND SOURCES src/engines/v8_engine.cpp)
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
        message(STATUS "Found Python3: ${Python3_VERSION}")
        message(STATUS "Python3 Include Dirs: ${Python3_INCLUDE_DIRS}")
        message(STATUS "Python3 Libraries: ${Python3_LIBRARIES}")
    else()
        message(WARNING "Python3 development headers not found")
    endif()
endif()


# 主执行文件
add_executable(script_benchmark ${SOURCES})

# 添加 V8 测试程序
if(ENABLE_V8)
    add_executable(test_v8 test_v8.cpp src/engines/v8_engine.cpp)
    include_directories(.)
    target_link_libraries(test_v8 ${V8_LINK_LIB})
    if(WIN32)
        target_link_libraries(test_v8 
            winmm.lib      # timeGetTime 等时间函数
            dbghelp.lib    # 符号调试功能
            ws2_32.lib     # 网络功能
            advapi32.lib   # 高级 API
        )
    endif()
    if(MSVC)
        set_property(TARGET test_v8 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# 确保主目标使用静态运行时库
if(MSVC)
    set_property(TARGET script_benchmark PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# 链接 V8
if(ENABLE_V8)
    target_link_libraries(script_benchmark ${V8_LINK_LIB})
    # V8 需要的 Windows 系统库
    if(WIN32)
        target_link_libraries(script_benchmark 
            winmm.lib      # timeGetTime 等时间函数
            dbghelp.lib    # 符号调试功能
            ws2_32.lib     # 网络功能
            advapi32.lib   # 高级 API
        )
    endif()
endif()

# Windows 下需要链接 ws2_32 等（QuickJS 使用 socket/线程时可能需要）。本项目只用到基础功能，通常无需额外库。

# 链接库
if(ENABLE_LUA)
    target_link_libraries(script_benchmark lua)
    # Lua 需要的 C 运行时库
    if(WIN32)
        target_link_libraries(script_benchmark 
            legacy_stdio_definitions.lib  # Lua 需要的旧版 stdio 函数
            msvcrt.lib                    # C 运行时库
            oldnames.lib                  # 旧名称兼容性
        )
    endif()
endif()

if(ENABLE_LUAJIT)
    if(EXISTS ${LUAJIT_LIBRARY_PATH})
        target_link_libraries(script_benchmark ${LUAJIT_LIBRARY_PATH})
        # LuaJIT 也需要 C 运行时库
        if(WIN32)
            target_link_libraries(script_benchmark 
                legacy_stdio_definitions.lib
                msvcrt.lib
                oldnames.lib
            )
        endif()
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