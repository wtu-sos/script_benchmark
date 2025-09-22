#pragma once

#include "test_runner.h"
#include <angelscript.h>
#include <unordered_map>

// 前向声明
class MockJITCompiler;

class AngelScriptEngine : public ScriptEngine {
public:
    AngelScriptEngine();
    ~AngelScriptEngine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    asIScriptEngine* engine;
    asIScriptContext* ctx;
    bool jit_enabled;
    MockJITCompiler* jit_compiler;

    // 编译产物缓存（按脚本内容哈希）
    std::unordered_map<size_t, asIScriptModule*> module_cache;
    std::unordered_map<size_t, asIScriptFunction*> func_cache;

    // 消息回调函数
    void messageCallback(const asSMessageInfo* msg);
    static void messageCallbackStatic(const asSMessageInfo* msg, void* param);
    void registerStringType();
    
    // JIT 编译相关
    void triggerJITCompilation(asIScriptModule* module);
    
    // Lua 到 AngelScript 语法转换
    std::string convertLuaToAngelScript(const std::string& luaScript);
    std::string convertLuaVarsToAngelScript(const std::string& code);
};
