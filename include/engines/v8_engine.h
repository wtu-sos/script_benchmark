#pragma once

#include <string>
#include "test_runner.h"

// 前向声明，避免头中包含 v8 头文件，减少编译耦合
namespace v8 {
class Isolate;
class Platform;
class ArrayBuffer;
class Context;
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
    bool initialized_;
    bool jit_enabled_;

    v8::Isolate* isolate_;
    void* allocator_; // v8::ArrayBuffer::Allocator*

    // 使用持久化上下文
    void* context_persistent_; // 存储为 void*，实现中转换

    static bool platform_initialized_;
    static void ensurePlatform();
};
