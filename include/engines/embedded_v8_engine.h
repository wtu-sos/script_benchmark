#pragma once

#include "test_runner.h"
#include <memory>

// 前向声明 V8 类型
namespace v8 {
    class Isolate;
    class Context;
    template<class T> class Local;
    template<class T> class Global;
    class Platform;
    class ArrayBuffer;
}

// 内嵌 V8 引擎 - 直接使用 V8 C++ API
class EmbeddedV8Engine : public ScriptEngine {
public:
    EmbeddedV8Engine();
    ~EmbeddedV8Engine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    // V8 初始化和清理
    static bool initializePlatform();
    static void shutdownPlatform();
    
    // V8 脚本执行
    bool executeJavaScript(const std::string& script);
    
    // 成员变量
    static std::unique_ptr<v8::Platform> platform_;
    static bool platform_initialized_;
    
    v8::Isolate* isolate_;
    v8::Global<v8::Context>* context_;
    bool jit_enabled_;
    bool initialized_;
};