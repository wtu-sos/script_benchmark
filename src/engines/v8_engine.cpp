#include "engines/v8_engine.h"
#include "../third_party/v8/v8_real.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

// 使用真实的 V8 引擎实现
bool V8Engine::platform_initialized = false;

V8Engine::V8Engine() : isolate(nullptr), context(nullptr), jit_enabled(true), initialized(false) {
    // 创建真实的 V8 引擎实例
    real_v8_engine = std::make_unique<RealV8Engine>();
}

V8Engine::~V8Engine() {
    cleanup();
}

bool V8Engine::initialize() {
    if (!real_v8_engine) {
        std::cerr << "Failed to create V8 engine" << std::endl;
        return false;
    }
    
    if (!real_v8_engine->initialize()) {
        std::cerr << "Failed to initialize V8 engine" << std::endl;
        return false;
    }

    platform_initialized = true;
    initialized = true;
    return true;
}

void V8Engine::cleanup() {
    if (initialized) {
        if (real_v8_engine) {
            real_v8_engine->cleanup();
        }
        initialized = false;
        isolate = nullptr;
        if (context) {
            delete context;
            context = nullptr;
        }
    }
}

bool V8Engine::executeScript(const std::string& script) {
    if (!initialized || !real_v8_engine) {
        return false;
    }

    try {
        return real_v8_engine->executeScript(script);
    } catch (const std::exception& e) {
        std::cerr << "JavaScript execution error: " << e.what() << std::endl;
        return false;
    }
}

bool V8Engine::executeFile(const std::string& filename) {
    if (!initialized || !real_v8_engine) {
        std::cerr << "V8 engine not initialized" << std::endl;
        return false;
    }

    return real_v8_engine->executeFile(filename);
}

std::string V8Engine::getName() const {
    if (real_v8_engine) {
        return real_v8_engine->getName();
    }
    return jit_enabled ? "V8+JIT" : "V8-JIT";
}

bool V8Engine::supportsJIT() const {
    if (real_v8_engine) {
        return real_v8_engine->supportsJIT();
    }
    return true; // V8 原生支持 JIT
}

void V8Engine::enableJIT(bool enable) {
    jit_enabled = enable;
    if (real_v8_engine) {
        real_v8_engine->enableJIT(enable);
    }
}

bool V8Engine::initializePlatform() {
    if (platform_initialized) {
        return true;
    }

    // 通过真实的 V8 引擎初始化平台
    platform_initialized = true;
    return true;
}

void V8Engine::shutdownPlatform() {
    if (platform_initialized) {
        platform_initialized = false;
    }
}
