#pragma once

#include "test_runner.h"

// C++ 原生引擎 - 作为性能基准
class CppEngine : public ScriptEngine {
public:
    CppEngine();
    ~CppEngine() override;

    bool initialize() override;
    void cleanup() override;
    bool executeScript(const std::string& script) override;
    bool executeFile(const std::string& filename) override;
    std::string getName() const override;
    bool supportsJIT() const override;
    void enableJIT(bool enable) override;

    // C++ 原生测试方法
    double runSimpleLoop();
    double runArithmeticOperations();
    double runStringOperations();
    double runFunctionCall();
    double runFibonacci();

private:
    bool initialized;
    bool optimized;  // 模拟JIT优化开关
    
    // 原生C++测试函数
    static int add(int a, int b);
    static int fibonacci(int n);
    static void simpleLoop();
    static void arithmeticOperations();
    static void stringOperations();
    static void functionCallTest();
    static void fibonacciTest();
};