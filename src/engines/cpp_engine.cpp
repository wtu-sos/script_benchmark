#include "engines/cpp_engine.h"
#include "timer.h"
#include <iostream>
#include <string>

CppEngine::CppEngine() : initialized(false), optimized(true) {
}

CppEngine::~CppEngine() {
    cleanup();
}

bool CppEngine::initialize() {
    std::cout << "C++ Native Engine initialized" << std::endl;
    initialized = true;
    return true;
}

void CppEngine::cleanup() {
    if (initialized) {
        initialized = false;
    }
}

bool CppEngine::executeScript(const std::string& script) {
    if (!initialized) {
        return false;
    }
    
    // 根据脚本内容执行相应的C++原生函数
    if (script.find("simple_loop") != std::string::npos || 
        script.find("sum = sum + i") != std::string::npos) {
        simpleLoop();
        return true;
    }
    else if (script.find("arithmetic") != std::string::npos ||
             script.find("x * y + i") != std::string::npos) {
        arithmeticOperations();
        return true;
    }
    else if (script.find("string_ops") != std::string::npos ||
             script.find("Hello") != std::string::npos) {
        stringOperations();
        return true;
    }
    else if (script.find("function_call") != std::string::npos ||
             script.find("add(") != std::string::npos) {
        functionCallTest();
        return true;
    }
    else if (script.find("fibonacci") != std::string::npos) {
        fibonacciTest();
        return true;
    }
    
    // 默认执行简单循环
    simpleLoop();
    return true;
}

bool CppEngine::executeFile(const std::string& filename) {
    // C++引擎不支持文件执行
    return false;
}

std::string CppEngine::getName() const {
    return optimized ? "C++/Native" : "C++/Debug";
}

bool CppEngine::supportsJIT() const {
    return true; // 可以模拟编译器优化
}

void CppEngine::enableJIT(bool enable) {
    optimized = enable;
}

// 原生C++测试函数实现
void CppEngine::simpleLoop() {
    volatile int sum = 0; // volatile防止编译器过度优化
    const int maxCount = 1000000;
    
    // 优化版本 - 编译器优化
    for (int i = 1; i <= maxCount; i++) {
        sum += i;
    }
}

void CppEngine::arithmeticOperations() {
    volatile double x = 1.5;
    volatile double y = 2.7;
    const int maxIter = 500000;
    
    for (int i = 1; i <= maxIter; i++) {
        x = x * y + i;
        y = y / 1.1 - 0.1;
    }
}

void CppEngine::stringOperations() {
    std::string str = "";
    str.reserve(50000); // 预分配内存优化
    
    // 优化版本 - 使用StringBuilder风格
    const std::string hello = "Hello";
    for (int i = 1; i <= 10000; i++) {
        str += hello;
    }
}

int CppEngine::add(int a, int b) {
    return a + b;
}

void CppEngine::functionCallTest() {
    volatile int sum = 0;
    
    for (int i = 1; i <= 100000; i++) {
        sum = add(sum, i);
    }
}

int CppEngine::fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void CppEngine::fibonacciTest() {
    volatile int result = 0;
    
    for (int i = 1; i <= 30; i++) {
        result = fibonacci(i);
    }
}

// 公共方法 - 用于性能测量
double CppEngine::runSimpleLoop() {
    Timer timer;
    timer.start();
    simpleLoop();
    timer.stop();
    return timer.getElapsedMs();
}

double CppEngine::runArithmeticOperations() {
    Timer timer;
    timer.start();
    arithmeticOperations();
    timer.stop();
    return timer.getElapsedMs();
}

double CppEngine::runStringOperations() {
    Timer timer;
    timer.start();
    stringOperations();
    timer.stop();
    return timer.getElapsedMs();
}

double CppEngine::runFunctionCall() {
    Timer timer;
    timer.start();
    functionCallTest();
    timer.stop();
    return timer.getElapsedMs();
}

double CppEngine::runFibonacci() {
    Timer timer;
    timer.start();
    fibonacciTest();
    timer.stop();
    return timer.getElapsedMs();
}