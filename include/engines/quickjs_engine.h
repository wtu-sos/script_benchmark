#pragma once

#include <string>
#include "test_runner.h"

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
    bool initialized_;
    bool jit_enabled_;

    bool ensureQjsAvailable();
};
