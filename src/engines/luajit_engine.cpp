#include "engines/luajit_engine.h"
#include "timer.h"
#include <iostream>
#include <fstream>
#include <sstream>

// LuaJIT 头文件
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

LuaJITEngine::LuaJITEngine() 
    : lua_state_(nullptr), initialized_(false), jit_enabled_(true) {
}

LuaJITEngine::~LuaJITEngine() {
    cleanup();
}

bool LuaJITEngine::initialize() {
    // 创建 LuaJIT 状态
    lua_state_ = luaL_newstate();
    if (!lua_state_) {
        std::cerr << "Failed to create LuaJIT state" << std::endl;
        return false;
    }
    
    // 打开标准库
    luaL_openlibs(lua_state_);
    
    // 设置标准库和配置
    setupStandardLibraries();
    
    std::cout << "LuaJIT Engine initialized" << std::endl;
    initialized_ = true;
    return true;
}

void LuaJITEngine::cleanup() {
    if (initialized_ && lua_state_) {
        lua_close(lua_state_);
        lua_state_ = nullptr;
        initialized_ = false;
    }
}

void LuaJITEngine::setupStandardLibraries() {
    // LuaJIT 的标准库已经通过 luaL_openlibs 加载
    // 简化实现 - 不进行复杂的 JIT 配置
}

void LuaJITEngine::enableLuaJIT() {
    // 启用 JIT 编译 - 简化版本，只启用基本JIT
    const char* enable_code = R"(
        if jit then 
            jit.on()
        end
    )";
    
    if (luaL_dostring(lua_state_, enable_code) != 0) {
        // 如果出错，静默忽略
        lua_pop(lua_state_, 1);
    }
    jit_enabled_ = true;
}

void LuaJITEngine::disableLuaJIT() {
    // 禁用 JIT 编译
    const char* disable_code = R"(
        if jit then 
            jit.off()
        end
    )";
    
    if (luaL_dostring(lua_state_, disable_code) != 0) {
        lua_pop(lua_state_, 1);
    }
    jit_enabled_ = false;
}

bool LuaJITEngine::executeScript(const std::string& script) {
    if (!initialized_) {
        return false;
    }
    
    return executeLuaCode(script);
}

bool LuaJITEngine::executeLuaCode(const std::string& code) {
    // 执行 Lua 代码
    int result = luaL_dostring(lua_state_, code.c_str());
    
    if (result != 0) {
        // 获取错误信息
        const char* error_msg = lua_tostring(lua_state_, -1);
        if (error_msg) {
            std::cerr << "LuaJIT Error: " << error_msg << std::endl;
        }
        lua_pop(lua_state_, 1); // 弹出错误信息
        return false;
    }
    
    return true;
}

bool LuaJITEngine::executeFile(const std::string& filename) {
    if (!initialized_) {
        return false;
    }
    
    // 使用 luaL_dofile 执行文件
    int result = luaL_dofile(lua_state_, filename.c_str());
    
    if (result != 0) {
        const char* error_msg = lua_tostring(lua_state_, -1);
        if (error_msg) {
            std::cerr << "LuaJIT File Error: " << error_msg << std::endl;
        }
        lua_pop(lua_state_, 1);
        return false;
    }
    
    return true;
}

std::string LuaJITEngine::getName() const {
    return jit_enabled_ ? "LuaJIT+JIT" : "LuaJIT-JIT";
}

bool LuaJITEngine::supportsJIT() const {
    return true; // LuaJIT 原生支持 JIT
}

void LuaJITEngine::enableJIT(bool enable) {
    if (!initialized_) {
        jit_enabled_ = enable;
        return;
    }
    
    if (enable && !jit_enabled_) {
        enableLuaJIT();
    } else if (!enable && jit_enabled_) {
        disableLuaJIT();
    }
}