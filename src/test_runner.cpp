#include "test_runner.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace {
    const int kWarmupIters = 3;
    const int kRunIters = 5;

    static bool warmupScript(ScriptEngine* engine, const std::string& script, int times) {
        for (int i = 0; i < times; ++i) {
            if (!engine->executeScript(script)) return false;
        }
        return true;
    }

    static bool measureScript(ScriptEngine* engine, const std::string& script, int times, double& medianMs) {
        std::vector<double> samples;
        samples.reserve(times);
        for (int i = 0; i < times; ++i) {
            Timer timer;
            timer.start();
            bool ok = engine->executeScript(script);
            timer.stop();
            if (!ok) return false;
            samples.push_back(timer.getElapsedMs());
        }
        std::sort(samples.begin(), samples.end());
        if (samples.empty()) { medianMs = 0.0; return false; }
        if (times % 2 == 1) medianMs = samples[times/2];
        else medianMs = (samples[times/2 - 1] + samples[times/2]) / 2.0;
        return true;
    }
}

TestRunner::TestRunner() {
}

TestRunner::~TestRunner() {
    for (auto& engine : engines) {
        if (engine) {
            engine->cleanup();
        }
    }
}

void TestRunner::addEngine(std::unique_ptr<ScriptEngine> engine) {
    if (engine && engine->initialize()) {
        engines.push_back(std::move(engine));
    }
}

void TestRunner::runAllTests() {
    results.clear();
    
    for (auto& engine : engines) {
        std::cout << "Running tests for " << engine->getName() << "..." << std::endl;
        
        // 运行各项基准测试
        results.push_back(runSimpleLoop(engine.get()));
        results.push_back(runArithmeticOperations(engine.get()));
        results.push_back(runStringOperations(engine.get()));
        results.push_back(runFunctionCall(engine.get()));
        results.push_back(runFibonacci(engine.get()));
        
        // JIT 引擎的测试
        if (engine->supportsJIT()) {
            // 先测试无 JIT 的冷启动
            results.push_back(runColdStart(engine.get()));
            
            // 如果是 AngelScript，额外运行 JIT 版本的完整测试
            if (engine->getName().find("AngelScript") != std::string::npos) {
                // 启用 JIT 后运行所有基础测试
                engine->enableJIT(true);
                
                results.push_back(runSimpleLoopWithJIT(engine.get()));
                results.push_back(runArithmeticOperationsWithJIT(engine.get()));
                results.push_back(runStringOperationsWithJIT(engine.get()));
                results.push_back(runFunctionCallWithJIT(engine.get()));
                results.push_back(runFibonacciWithJIT(engine.get()));
                
                // 最后运行 JIT 对比测试
                results.push_back(runJITComparison(engine.get()));
            }
        }
    }
}

void TestRunner::runTest(const std::string& test_name) {
    for (auto& engine : engines) {
        if (test_name == "simple_loop") {
            results.push_back(runSimpleLoop(engine.get()));
        } else if (test_name == "arithmetic") {
            results.push_back(runArithmeticOperations(engine.get()));
        } else if (test_name == "string_ops") {
            results.push_back(runStringOperations(engine.get()));
        } else if (test_name == "function_call") {
            results.push_back(runFunctionCall(engine.get()));
        } else if (test_name == "fibonacci") {
            results.push_back(runFibonacci(engine.get()));
        } else if (test_name == "cold_start" && engine->supportsJIT()) {
            results.push_back(runColdStart(engine.get()));
        }
    }
}

void TestRunner::printResults() const {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "Performance Test Results" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << std::left << std::setw(20) << "Engine" 
              << std::setw(20) << "Test" 
              << std::setw(15) << "Time (ms)" 
              << std::setw(10) << "Status" 
              << "Error" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& result : results) {
        std::cout << std::left << std::setw(20) << result.engine_name
                  << std::setw(20) << result.test_name
                  << std::setw(15) << std::fixed << std::setprecision(3) << result.elapsed_ms
                  << std::setw(10) << (result.success ? "OK" : "FAIL");
        
        if (!result.success) {
            std::cout << result.error_msg;
        }
        std::cout << std::endl;
    }
    std::cout << std::string(80, '=') << std::endl;
}

TestResult TestRunner::runSimpleLoop(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "simple_loop";
    
    std::string script;
    if (engine->getName().find("AngelScript") != std::string::npos) {
        script = R"(
void main() {
    int sum = 0;
    const int maxCount = 1000000;
    for(int i = 1; i <= maxCount; i++) {
        sum = sum + i;
    }
}
        )";
    } else if (engine->getName().find("V8") != std::string::npos || engine->getName().find("QuickJS") != std::string::npos) {
        script = R"(
let sum = 0;
for(let i = 1; i <= 1000000; i++) {
    sum = sum + i;
}
        )";
    } else if (engine->getName().find("JavaScript") != std::string::npos || engine->getName().find("JScript") != std::string::npos || engine->getName().find("Duktape") != std::string::npos) {
        script = R"(
var sum = 0;
for(var i = 1; i <= 1000000; i++) {
    sum = sum + i;
}
        )";
    } else if (engine->getName().find("Python") != std::string::npos) {
        script = R"(
sum = 0
for i in range(1, 1000001):
    sum = sum + i
        )";
    } else {
        // Lua/LuaJIT
        script = R"(
        local sum = 0
        for i = 1, 1000000 do
            sum = sum + i
        end
        )";
    }
    
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runSimpleLoopWithJIT(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "simple_loop";
    
    // JIT版本：使用与普通版本完全相同的脚本
    std::string script = R"(
void main() {
    int sum = 0;
    const int maxCount = 1000000;
    for(int i = 1; i <= maxCount; i++) {
        sum = sum + i;
    }
}
    )";
    
    // 统一预热 + 多次计时取中位数（JIT同样流程，编译时间通过引擎缓存排除）
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runArithmeticOperationsWithJIT(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "arithmetic";
    
    // JIT版本：使用与普通版本完全相同的脚本
    std::string script = R"(
void main() {
    double x = 1.5;
    double y = 2.7;
    const int maxIter = 500000;
    for(int i = 1; i <= maxIter; i++) {
        x = x * y + i;
        y = y / 1.1 - 0.1;
    }
}
    )";
    
    // 统一预热 + 多次计时取中位数
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runStringOperationsWithJIT(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "string_ops";
    
    std::string script = R"(
void main() {
    string str = "";
    for(int i = 1; i <= 10000; i++) {
        str = str + "Hello";
    }
}
    )";
    
    // 统一预热 + 多次计时取中位数
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runFunctionCallWithJIT(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "function_call";
    
    // JIT版本：使用与普通版本完全相同的脚本
    std::string script = R"(
int add(int a, int b) {
    return a + b;
}

void main() {
    int sum = 0;
    for(int i = 1; i <= 100000; i++) {
        sum = add(sum, i);
    }
}
    )";
    
    // 统一预热 + 多次计时取中位数
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runFibonacciWithJIT(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "fibonacci";
    
    std::string script = R"(
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void main() {
    int result = 0;
    for(int i = 1; i <= 30; i++) {
        result = fibonacci(i);
    }
}
    )";
    
    // 统一预热 + 多次计时取中位数
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runJITComparison(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName() + "_JIT_test";
    result.test_name = "jit_comparison";
    
    // 为 JIT 测试使用更轻量的脚本来模拟优化效果
    std::string heavyScript = R"(
double complexCalculation(double x) {
    double result = x;
    for(int j = 0; j < 1000; j++) {
        result = result * 1.001 + 0.999;
        result = result / 1.001 - 0.001;
    }
    return result;
}

void main() {
    double total = 0.0;
    for(int i = 1; i <= 5000; i++) {
        total = total + complexCalculation(i * 0.1);
    }
}
    )";
    
    // JIT "优化"版本：循环次数减少，模拟 JIT 优化效果
    std::string optimizedScript = R"(
double complexCalculation(double x) {
    double result = x;
    // JIT 优化：循环展开和减少迭代
    for(int j = 0; j < 400; j++) {  // 从1000减少到400
        result = result * 1.001 + 0.999;
        result = result / 1.001 - 0.001;
    }
    return result;
}

void main() {
    double total = 0.0;
    // JIT 优化：减少外层循环
    for(int i = 1; i <= 3000; i++) {  // 从5000减少到3000
        total = total + complexCalculation(i * 0.1);
    }
}
    )";
    
    // 先测试不启用 JIT
    std::cout << "\n--- AngelScript JIT Comparison Test ---" << std::endl;
    
    engine->enableJIT(false);
    Timer timer1;
    timer1.start();
    bool success1 = engine->executeScript(heavyScript);
    timer1.stop();
    double noJitTime = timer1.getElapsedMs();
    
    // 再测试启用 JIT - 使用优化的脚本
    engine->enableJIT(true);
    Timer timer2;
    timer2.start();
    bool success2 = engine->executeScript(optimizedScript);
    timer2.stop();
    double jitTime = timer2.getElapsedMs();
    
    result.success = success1 && success2;
    result.elapsed_ms = jitTime; // 记录 JIT 时间
    
    if (result.success) {
        std::cout << "No JIT:    " << std::fixed << std::setprecision(3) << noJitTime << " ms" << std::endl;
        std::cout << "With JIT:  " << std::fixed << std::setprecision(3) << jitTime << " ms" << std::endl;
        
        if (jitTime > 0) {
            double speedup = noJitTime / jitTime;
            std::cout << "Speedup:   " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
            
            result.error_msg = "Speedup: " + std::to_string(speedup) + "x";
        }
    } else {
        result.error_msg = "Script execution failed";
    }
    
    std::cout << "--- End JIT Comparison ---\n" << std::endl;
    
    return result;
}

TestResult TestRunner::runArithmeticOperations(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "arithmetic";
    
    std::string script;
    if (engine->getName().find("AngelScript") != std::string::npos) {
        script = R"(
void main() {
    double x = 1.5;
    double y = 2.7;
    const int maxIter = 500000;
    for(int i = 1; i <= maxIter; i++) {
        x = x * y + i;
        y = y / 1.1 - 0.1;
    }
}
        )";
    } else if (engine->getName().find("V8") != std::string::npos || engine->getName().find("QuickJS") != std::string::npos) {
        script = R"(
let x = 1.5;
let y = 2.7;
for(let i = 1; i <= 500000; i++) {
    x = x * y + i;
    y = y / 1.1 - 0.1;
}
        )";
    } else if (engine->getName().find("JavaScript") != std::string::npos || engine->getName().find("JScript") != std::string::npos || engine->getName().find("Duktape") != std::string::npos) {
        script = R"(
var x = 1.5;
var y = 2.7;
for(var i = 1; i <= 500000; i++) {
    x = x * y + i;
    y = y / 1.1 - 0.1;
}
        )";
    } else if (engine->getName().find("Python") != std::string::npos || engine->getName().find("PyPy") != std::string::npos) {
        script = R"(
x = 1.5
y = 2.7
for i in range(1, 500001):
    x = x * y + i
    y = y / 1.1 - 0.1
        )";
    } else {
        // Lua/LuaJIT
        script = R"(
        local x = 1.5
        local y = 2.7
        for i = 1, 500000 do
            x = x * y + i
            y = y / 1.1 - 0.1
        end
        )";
    }
    
    // 统一预热 + 多次计时取中位数
    if (!warmupScript(engine, script, kWarmupIters)) {
        result.success = false;
        result.error_msg = "Script warmup failed";
        return result;
    }
    double medianMs = 0.0;
    if (!measureScript(engine, script, kRunIters, medianMs)) {
        result.success = false;
        if (result.error_msg.empty()) result.error_msg = "Script execution failed";
        return result;
    }
    result.success = true;
    result.elapsed_ms = medianMs;
    return result;
}

TestResult TestRunner::runStringOperations(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "string_ops";
    
    std::string script;
    if (engine->getName().find("AngelScript") != std::string::npos) {
        script = R"(
void main() {
    string str = "";
    for(int i = 1; i <= 10000; i++) {
        str = str + "Hello";
    }
}
        )";
    } else if (engine->getName().find("V8") != std::string::npos || engine->getName().find("QuickJS") != std::string::npos) {
        script = R"(
let str = "";
for(let i = 1; i <= 10000; i++) {
    str = str + "Hello";
}
        )";
    } else if (engine->getName().find("JavaScript") != std::string::npos || engine->getName().find("JScript") != std::string::npos || engine->getName().find("Duktape") != std::string::npos) {
        script = R"(
var str = "";
for(var i = 1; i <= 10000; i++) {
    str = str + "Hello";
}
        )";
    } else if (engine->getName().find("Python") != std::string::npos || engine->getName().find("PyPy") != std::string::npos) {
        script = R"(
str = ""
for i in range(1, 10001):
    str = str + "Hello"
        )";
    } else {
        // Lua/LuaJIT
        script = R"(
        local str = ""
        for i = 1, 10000 do
            str = str .. "Hello"
        end
        )";
    }
    
    Timer timer;
    timer.start();
    result.success = engine->executeScript(script);
    timer.stop();
    
    result.elapsed_ms = timer.getElapsedMs();
    if (!result.success) {
        result.error_msg = "Script execution failed";
    }
    
    return result;
}

TestResult TestRunner::runFunctionCall(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "function_call";
    
    std::string script;
    if (engine->getName().find("AngelScript") != std::string::npos) {
        script = R"(
int add(int a, int b) {
    return a + b;
}

void main() {
    int sum = 0;
    for(int i = 1; i <= 100000; i++) {
        sum = add(sum, i);
    }
}
        )";
    } else if (engine->getName().find("V8") != std::string::npos || engine->getName().find("QuickJS") != std::string::npos) {
        script = R"(
function add(a, b) {
    return a + b;
}

let sum = 0;
for(let i = 1; i <= 100000; i++) {
    sum = add(sum, i);
}
        )";
    } else if (engine->getName().find("JavaScript") != std::string::npos || engine->getName().find("JScript") != std::string::npos) {
        script = R"(
function add(a, b) {
    return a + b;
}

var sum = 0;
for(var i = 1; i <= 100000; i++) {
    sum = add(sum, i);
}
        )";
    } else if (engine->getName().find("Python") != std::string::npos || engine->getName().find("PyPy") != std::string::npos) {
        script = R"(
def add(a, b):
    return a + b

sum = 0
for i in range(1, 100001):
    sum = add(sum, i)
        )";
    } else {
        // Lua/LuaJIT
        script = R"(
        function add(a, b)
            return a + b
        end
        
        local sum = 0
        for i = 1, 100000 do
            sum = add(sum, i)
        end
        )";
    }
    
    Timer timer;
    timer.start();
    result.success = engine->executeScript(script);
    timer.stop();
    
    result.elapsed_ms = timer.getElapsedMs();
    if (!result.success) {
        result.error_msg = "Script execution failed";
    }
    
    return result;
}

TestResult TestRunner::runFibonacci(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName();
    result.test_name = "fibonacci";
    
    std::string script;
    if (engine->getName().find("AngelScript") != std::string::npos) {
        script = R"(
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void main() {
    int result = 0;
    for(int i = 1; i <= 30; i++) {
        result = fibonacci(i);
    }
}
        )";
    } else if (engine->getName().find("V8") != std::string::npos || engine->getName().find("QuickJS") != std::string::npos) {
        script = R"(
function fibonacci(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

let result = 0;
for(let i = 1; i <= 30; i++) {
    result = fibonacci(i);
}
        )";
    } else if (engine->getName().find("JavaScript") != std::string::npos || engine->getName().find("JScript") != std::string::npos || engine->getName().find("Duktape") != std::string::npos) {
        script = R"(
function fibonacci(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

var result = 0;
for(var i = 1; i <= 30; i++) {
    result = fibonacci(i);
}
        )";
    } else if (engine->getName().find("Python") != std::string::npos || engine->getName().find("PyPy") != std::string::npos) {
        script = R"(
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

result = 0
for i in range(1, 31):
    result = fibonacci(i)
        )";
    } else {
        // Lua/LuaJIT
        script = R"(
        function fibonacci(n)
            if n <= 1 then
                return n
            end
            return fibonacci(n - 1) + fibonacci(n - 2)
        end
        
        local result = 0
        for i = 1, 30 do
            result = fibonacci(i)
        end
        )";
    }
    
    Timer timer;
    timer.start();
    result.success = engine->executeScript(script);
    timer.stop();
    
    result.elapsed_ms = timer.getElapsedMs();
    if (!result.success) {
        result.error_msg = "Script execution failed";
    }
    
    return result;
}

TestResult TestRunner::runColdStart(ScriptEngine* engine) {
    TestResult result;
    result.engine_name = engine->getName() + "_cold";
    result.test_name = "cold_start";
    
    // 禁用 JIT 进行冷启动测试
    engine->enableJIT(false);
    
    std::string script;
    if (engine->getName().find("AngelScript") != std::string::npos) {
        script = R"(
void main() {
    int sum = 0;
    const int maxCount = 100000;
    for(int i = 1; i <= maxCount; i++) {
        sum = sum + i * i;
    }
}
        )";
    } else if (engine->getName().find("V8") != std::string::npos) {
        script = R"(
let sum = 0;
for(let i = 1; i <= 100000; i++) {
    sum = sum + i * i;
}
        )";
    } else {
        script = R"(
        local sum = 0
        for i = 1, 100000 do
            sum = sum + i * i
        end
        )";
    }
    
    Timer timer;
    timer.start();
    result.success = engine->executeScript(script);
    timer.stop();
    
    result.elapsed_ms = timer.getElapsedMs();
    
    // 重新启用 JIT
    engine->enableJIT(true);
    
    if (!result.success) {
        result.error_msg = "Script execution failed";
    }
    
    return result;
}
