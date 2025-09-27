#pragma once

#include "test_runner.h"
#include <Python.h>
#include <string>

/**
 * Python3 脚本引擎
 * 使用 CPython 解释器执行 Python 代码
 */
class PythonEngine : public ScriptEngine {
public:
    PythonEngine();
    virtual ~PythonEngine();

    // ScriptEngine 接口实现
    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

private:
    bool initialized_;
    
    // Python 相关的辅助方法
    void setupStandardModules();
    bool executePythonCode(const std::string& code);
    void handlePythonError();
};