#pragma once

#include "test_runner.h"

// 前向声明 LuaJIT 类型
struct lua_State;

// LuaJIT 引擎 - 使用 LuaJIT 的 JIT 编译器
class LuaJITEngine : public ScriptEngine {
public:
    LuaJITEngine();
    ~LuaJITEngine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    // LuaJIT 内部方法
    bool executeLuaCode(const std::string& code);
    void setupStandardLibraries();
    void enableLuaJIT();
    void disableLuaJIT();
    
    // 成员变量
    lua_State* lua_state_;
    bool initialized_;
    bool jit_enabled_;
};