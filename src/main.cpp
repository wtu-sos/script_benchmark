#include <iostream>
#include <memory>
#include "test_runner.h"

#ifdef ENABLE_LUA
#include "engines/lua_engine.h"
#endif

#ifdef ENABLE_LUAJIT
#include "engines/luajit_engine.h"
#endif

#ifdef ENABLE_ANGELSCRIPT
#include "engines/angelscript_engine.h"
#endif

#ifdef ENABLE_V8
#include "engines/v8_engine.h"
#endif

#ifdef ENABLE_QUICKJS
#include "engines/quickjs_engine.h"
#endif

int main(int argc, char* argv[]) {
    std::cout << "Script Language Performance Benchmark\n";
    std::cout << "======================================\n\n";
    
    TestRunner runner;
    
    // 添加可用的脚本引擎
#ifdef ENABLE_LUA
    std::cout << "Adding Lua engine...\n";
    runner.addEngine(std::make_unique<LuaEngine>());
#endif

#ifdef ENABLE_LUAJIT
    std::cout << "Adding LuaJIT engine...\n";
    runner.addEngine(std::make_unique<LuaJITEngine>());
#endif

#ifdef ENABLE_ANGELSCRIPT
    std::cout << "Adding AngelScript engine...\n";
    runner.addEngine(std::make_unique<AngelScriptEngine>());
#endif

#ifdef ENABLE_V8
    std::cout << "Adding V8 engine...\n";
    runner.addEngine(std::make_unique<V8Engine>());
#endif

#ifdef ENABLE_QUICKJS
    std::cout << "Adding QuickJS engine...\n";
    runner.addEngine(std::make_unique<QuickJSEngine>());
#endif

    std::cout << "\nStarting benchmark tests...\n";
    
    if (argc > 1) {
        // 运行指定测试
        std::string test_name = argv[1];
        std::cout << "Running specific test: " << test_name << std::endl;
        runner.runTest(test_name);
    } else {
        // 运行所有测试
        runner.runAllTests();
    }
    
    // 输出结果
    runner.printResults();
    
    return 0;
}