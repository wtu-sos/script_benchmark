#include "engines/lua_engine.h"
#include <iostream>

LuaEngine::LuaEngine() : L(nullptr) {
}

LuaEngine::~LuaEngine() {
    cleanup();
}

bool LuaEngine::initialize() {
    L = luaL_newstate();
    if (!L) {
        std::cerr << "Failed to create Lua state" << std::endl;
        return false;
    }
    
    // 打开标准库
    luaL_openlibs(L);
    
    std::cout << "Lua Engine initialized" << std::endl;
    return true;
}

void LuaEngine::cleanup() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

bool LuaEngine::executeScript(const std::string& script) {
    if (!L) {
        return false;
    }
    
    int result = luaL_loadstring(L, script.c_str());
    if (result != LUA_OK) {
        std::cerr << "Lua load error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    
    result = lua_pcall(L, 0, 0, 0);
    if (result != LUA_OK) {
        std::cerr << "Lua execution error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

bool LuaEngine::executeFile(const std::string& filename) {
    if (!L) {
        return false;
    }
    
    int result = luaL_loadfile(L, filename.c_str());
    if (result != LUA_OK) {
        std::cerr << "Lua load file error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    
    result = lua_pcall(L, 0, 0, 0);
    if (result != LUA_OK) {
        std::cerr << "Lua execution error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    
    return true;
}

std::string LuaEngine::getName() const {
    return "Lua";
}

bool LuaEngine::supportsJIT() const {
    return false; // 标准 Lua 不支持 JIT
}

void LuaEngine::enableJIT(bool enable) {
    // 标准 Lua 不支持 JIT，什么也不做
}