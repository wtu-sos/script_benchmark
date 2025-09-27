#include "engines/python_engine.h"
#include "timer.h"
#include <iostream>
#include <fstream>
#include <sstream>

PythonEngine::PythonEngine() : initialized_(false) {
}

PythonEngine::~PythonEngine() {
    cleanup();
}

bool PythonEngine::initialize() {
    if (initialized_) {
        return true;
    }
    
    // 初始化 Python 解释器
    if (!Py_IsInitialized()) {
        Py_Initialize();
        
        if (!Py_IsInitialized()) {
            std::cerr << "Failed to initialize Python interpreter" << std::endl;
            return false;
        }
    }
    
    // 设置标准模块
    setupStandardModules();
    
    std::cout << "Python Engine initialized (Version: " << Py_GetVersion() << ")" << std::endl;
    initialized_ = true;
    return true;
}

void PythonEngine::cleanup() {
    if (initialized_) {
        // 注意：通常不需要调用 Py_Finalize()，因为可能影响其他 Python 组件
        // 在生产环境中，Python 解释器通常在程序结束时自动清理
        initialized_ = false;
    }
}

void PythonEngine::setupStandardModules() {
    // Python 标准库默认已加载，这里可以添加额外的配置
    // 例如设置 sys.path 或导入常用模块
    
    const char* setup_code = R"(
import sys
import math
import time
    )";
    
    PyRun_SimpleString(setup_code);
}

bool PythonEngine::executeScript(const std::string& script) {
    if (!initialized_) {
        return false;
    }
    
    return executePythonCode(script);
}

bool PythonEngine::executePythonCode(const std::string& code) {
    // 执行 Python 代码
    int result = PyRun_SimpleString(code.c_str());
    
    if (result != 0) {
        // 处理错误
        handlePythonError();
        return false;
    }
    
    return true;
}

void PythonEngine::handlePythonError() {
    if (PyErr_Occurred()) {
        PyObject* ptype, * pvalue, * ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        
        if (pvalue != NULL) {
            PyObject* str_exc_value = PyObject_Str(pvalue);
            if (str_exc_value != NULL) {
                const char* error_msg = PyUnicode_AsUTF8(str_exc_value);
                if (error_msg) {
                    std::cerr << "Python Error: " << error_msg << std::endl;
                }
                Py_DECREF(str_exc_value);
            }
        }
        
        // 清理错误状态
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
        PyErr_Clear();
    }
}

bool PythonEngine::executeFile(const std::string& filename) {
    if (!initialized_) {
        return false;
    }
    
    // 读取文件内容
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open Python file: " << filename << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return executePythonCode(buffer.str());
}

std::string PythonEngine::getName() const {
    return "Python3";
}

bool PythonEngine::supportsJIT() const {
    // CPython 不支持 JIT，这里返回 false
    // 注意：PyPy 支持 JIT，但我们这里实现的是 CPython
    return false;
}

void PythonEngine::enableJIT(bool enable) {
    // CPython 不支持 JIT，此方法为空实现
    (void)enable; // 避免未使用参数警告
}