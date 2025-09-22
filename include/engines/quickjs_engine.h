#pragma once

#include "test_runner.h"

// 前向声明 QuickJS 类型
typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;
typedef struct JSValue JSValue;

// QuickJS 引擎 - 轻量级 JavaScript 引擎，无进程开销
class QuickJSEngine : public ScriptEngine {
public:
    QuickJSEngine();
    ~QuickJSEngine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    // QuickJS 内部方法
    bool executeJavaScript(const std::string& script);
    void setupStandardLibraries();
    
    // 成员变量
    JSRuntime* runtime_;
    JSContext* context_;
    bool initialized_;
    bool jit_enabled_;
};