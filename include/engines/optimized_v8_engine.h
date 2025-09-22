#pragma once

#include "test_runner.h"
#include <memory>

// 前向声明，避免包含复杂的 V8 头文件
namespace v8 {
    class Isolate;
    class Context;
    template<class T> class Local;
    template<class T> class Global;
}

// 前向声明真实的 V8 引擎实现类
class RealV8Engine;

// 优化的 V8 引擎 - 减少进程启动开销
class OptimizedV8Engine : public ScriptEngine {
public:
    OptimizedV8Engine();
    ~OptimizedV8Engine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    // 批量执行优化
    bool executeBatchedScript(const std::string& script);
    void warmupEngine();
    
    // 原有的 V8 接口（为了兼容性保留）
    v8::Isolate* isolate;
    v8::Global<v8::Context>* context;
    bool jit_enabled;
    bool initialized;
    
    // 优化相关
    bool engine_warmed_up;
    
    // 真实的 V8 引擎实现
    std::unique_ptr<RealV8Engine> real_v8_engine;
    
    // 初始化 V8 平台
    static bool initializePlatform();
    static void shutdownPlatform();
    static bool platform_initialized;
};