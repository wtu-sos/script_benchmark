#include "engines/quickjs_engine.h"
#include "timer.h"
#include <iostream>
#include <fstream>
#include <sstream>

// QuickJS 头文件
extern "C" {
#include "quickjs.h"
#include "quickjs-libc.h"
}

QuickJSEngine::QuickJSEngine() 
    : runtime_(nullptr), context_(nullptr), initialized_(false), jit_enabled_(false) {
}

QuickJSEngine::~QuickJSEngine() {
    cleanup();
}

bool QuickJSEngine::initialize() {
    // 创建 QuickJS 运行时
    runtime_ = JS_NewRuntime();
    if (!runtime_) {
        std::cerr << "Failed to create QuickJS runtime" << std::endl;
        return false;
    }
    
    // 创建 JavaScript 上下文
    context_ = JS_NewContext(runtime_);
    if (!context_) {
        std::cerr << "Failed to create QuickJS context" << std::endl;
        JS_FreeRuntime(runtime_);
        runtime_ = nullptr;
        return false;
    }
    
    // 设置标准库
    setupStandardLibraries();
    
    std::cout << "QuickJS JavaScript Engine initialized" << std::endl;
    initialized_ = true;
    return true;
}

void QuickJSEngine::cleanup() {
    if (initialized_) {
        if (context_) {
            JS_FreeContext(context_);
            context_ = nullptr;
        }
        if (runtime_) {
            JS_FreeRuntime(runtime_);
            runtime_ = nullptr;
        }
        initialized_ = false;
    }
}

void QuickJSEngine::setupStandardLibraries() {
    // 添加标准库支持
    js_init_module_std(context_, "std");
    js_init_module_os(context_, "os");
    
    // 添加基本的全局对象
    JSValue global = JS_GetGlobalObject(context_);
    
    // 添加 console.log 支持
    const char* console_code = R"(
        globalThis.console = {
            log: function(...args) {
                print(args.join(' '));
            }
        };
    )";
    
    JSValue result = JS_Eval(context_, console_code, strlen(console_code), 
                           "<console>", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        // 静默忽略错误
    }
    JS_FreeValue(context_, result);
    JS_FreeValue(context_, global);
}

bool QuickJSEngine::executeScript(const std::string& script) {
    if (!initialized_) {
        return false;
    }
    
    return executeJavaScript(script);
}

bool QuickJSEngine::executeJavaScript(const std::string& script) {
    // 执行 JavaScript 代码
    JSValue result = JS_Eval(context_, script.c_str(), script.length(), 
                           "<script>", JS_EVAL_TYPE_GLOBAL);
    
    bool success = !JS_IsException(result);
    
    if (!success) {
        // 获取异常信息
        JSValue exception = JS_GetException(context_);
        const char* error_str = JS_ToCString(context_, exception);
        if (error_str) {
            std::cerr << "QuickJS Error: " << error_str << std::endl;
            JS_FreeCString(context_, error_str);
        }
        JS_FreeValue(context_, exception);
    }
    
    JS_FreeValue(context_, result);
    
    // 运行垃圾回收
    JS_RunGC(runtime_);
    
    return success;
}

bool QuickJSEngine::executeFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return executeScript(buffer.str());
}

std::string QuickJSEngine::getName() const {
    return jit_enabled_ ? "QuickJS+Opt" : "QuickJS";
}

bool QuickJSEngine::supportsJIT() const {
    return false; // QuickJS 不支持 JIT，但可以模拟优化
}

void QuickJSEngine::enableJIT(bool enable) {
    jit_enabled_ = enable;
    // QuickJS 不支持真正的 JIT，但我们可以设置一些优化选项
}