#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

// 测试结果结构
struct TestResult {
    std::string test_name;
    std::string engine_name;
    double elapsed_ms;
    bool success;
    std::string error_msg;
};

// 脚本引擎接口
class ScriptEngine {
public:
    virtual ~ScriptEngine() = default;
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    virtual bool executeScript(const std::string& script) = 0;
    virtual bool executeFile(const std::string& filename) = 0;
    virtual std::string getName() const = 0;
    virtual bool supportsJIT() const = 0;
    virtual void enableJIT(bool enable) = 0;
};

// 测试运行器
class TestRunner {
public:
    TestRunner();
    ~TestRunner();

    void addEngine(std::unique_ptr<ScriptEngine> engine);
    void runAllTests();
    void runTest(const std::string& test_name);
    void printResults() const;

private:
    std::vector<std::unique_ptr<ScriptEngine>> engines;
    std::vector<TestResult> results;

    // 基准测试函数
    TestResult runSimpleLoop(ScriptEngine* engine);
    TestResult runFunctionCall(ScriptEngine* engine);
    TestResult runStringOperations(ScriptEngine* engine);
    TestResult runArithmeticOperations(ScriptEngine* engine);
    TestResult runFibonacci(ScriptEngine* engine);
    TestResult runColdStart(ScriptEngine* engine);
    TestResult runJITComparison(ScriptEngine* engine);
    
    // JIT 版本的基础测试
    TestResult runSimpleLoopWithJIT(ScriptEngine* engine);
    TestResult runArithmeticOperationsWithJIT(ScriptEngine* engine);
    TestResult runStringOperationsWithJIT(ScriptEngine* engine);
    TestResult runFunctionCallWithJIT(ScriptEngine* engine);
    TestResult runFibonacciWithJIT(ScriptEngine* engine);
};
