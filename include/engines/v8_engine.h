#pragma once

#include "test_runner.h"

// 前向声明，避免包含复杂的 V8 头文件
namespace v8 {
    class Isolate;
    class Context;
    template<class T> class Local;
    template<class T> class Global;
}

class V8Engine : public ScriptEngine {
public:
    V8Engine();
    ~V8Engine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    v8::Isolate* isolate;
    v8::Global<v8::Context>* context;
    bool jit_enabled;
    bool initialized;

    // JavaScript 执行方法
    bool executeJavaScript(const std::string& script);
    
    // 初始化 V8 平台
    static bool initializePlatform();
    static void shutdownPlatform();
    static bool platform_initialized;
};