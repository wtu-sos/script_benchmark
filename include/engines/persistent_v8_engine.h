#pragma once

#include "test_runner.h"
#include <memory>
#include <string>

// 前向声明
class PersistentNodeEngine;

// 持久化 V8 引擎 - 通过长期运行的 Node.js 进程来避免启动开销
class PersistentV8Engine : public ScriptEngine {
public:
    PersistentV8Engine();
    ~PersistentV8Engine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    std::unique_ptr<PersistentNodeEngine> node_engine_;
    bool jit_enabled_;
    bool initialized_;
};