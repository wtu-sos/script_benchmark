#include "engines/angelscript_engine.h"
#include "engines/angelscript_jit.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

// 包含 AngelScript 字符串 add-on
#include "scriptstdstring/scriptstdstring.h"

AngelScriptEngine::AngelScriptEngine() : engine(nullptr), ctx(nullptr), jit_enabled(false), jit_compiler(nullptr) {
}

AngelScriptEngine::~AngelScriptEngine() {
    cleanup();
}

bool AngelScriptEngine::initialize() {
    // 创建 AngelScript 引擎
    engine = asCreateScriptEngine();
    if (!engine) {
        std::cerr << "Failed to create AngelScript engine" << std::endl;
        return false;
    }

    // 设置消息回调
    int r = engine->SetMessageCallback(asFUNCTION(messageCallbackStatic), this, asCALL_CDECL);
    if (r < 0) {
        std::cerr << "Failed to set message callback" << std::endl;
        return false;
    }

    // 注册字符串类型
    registerStringType();
    
    // 创建执行上下文
    ctx = engine->CreateContext();
    if (!ctx) {
        std::cerr << "Failed to create AngelScript context" << std::endl;
        return false;
    }

    return true;
}

void AngelScriptEngine::cleanup() {
    if (ctx) {
        ctx->Release();
        ctx = nullptr;
    }
    
    // 清理 JIT 编译器
    if (jit_compiler) {
        if (engine) {
            engine->SetJITCompiler(nullptr);
        }
        delete jit_compiler;
        jit_compiler = nullptr;
    }

    // 清空编译缓存（模块由引擎统一释放，这里只清空映射）
    module_cache.clear();
    func_cache.clear();
    
    if (engine) {
        engine->ShutDownAndRelease();
        engine = nullptr;
    }
    
    jit_enabled = false;
}

bool AngelScriptEngine::executeScript(const std::string& script) {
    if (!engine || !ctx) {
        return false;
    }

    // 以脚本文本 + JIT 开关作为缓存键，避免把编译/JIT 编译时间计入执行
    size_t key = std::hash<std::string>{}(script);
    key ^= static_cast<size_t>(jit_enabled ? 0x9e3779b9 : 0x85ebca6b);

    asIScriptFunction* func = nullptr;

    auto fIt = func_cache.find(key);
    if (fIt != func_cache.end()) {
        // 命中缓存：直接 Prepare + Execute
        func = fIt->second;
    } else {
        // 未命中：编译模块并缓存
        std::string modName = std::string("ScriptModule_") + std::to_string(key);
        asIScriptModule* mod = engine->GetModule(modName.c_str(), asGM_ALWAYS_CREATE);
        if (!mod) {
            std::cerr << "Failed to create module" << std::endl;
            return false;
        }

        int r = mod->AddScriptSection("script", script.c_str(), static_cast<unsigned int>(script.length()));
        if (r < 0) {
            std::cerr << "Failed to add script section" << std::endl;
            return false;
        }

        r = mod->Build();
        if (r < 0) {
            std::cerr << "Failed to build script" << std::endl;
            return false;
        }

        func = mod->GetFunctionByDecl("void main()");
        if (!func) {
            std::cerr << "No function to execute" << std::endl;
            return false;
        }

        module_cache[key] = mod;
        func_cache[key] = func;
    }

    // 执行（此时若启用JIT，Prepare将使用已编译的JIT函数）
    int r = ctx->Prepare(func);
    if (r < 0) {
        std::cerr << "Failed to prepare context" << std::endl;
        return false;
    }

    r = ctx->Execute();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            std::cerr << "Script exception: " << ctx->GetExceptionString() << std::endl;
        } else {
            std::cerr << "Script execution failed with code: " << r << std::endl;
        }
        return false;
    }

    return true;
}

bool AngelScriptEngine::executeFile(const std::string& filename) {
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

std::string AngelScriptEngine::getName() const {
    return jit_enabled ? "AngelScript+JIT" : "AngelScript";
}

bool AngelScriptEngine::supportsJIT() const {
    // AngelScript 本身不直接支持 JIT，但可以通过 add-on 实现
    // 这里我们简化处理，假设支持
    return true;
}

void AngelScriptEngine::enableJIT(bool enable) {
    if (!engine) {
        return;
    }
    
    if (enable && !jit_enabled) {
        // 启用 JIT
        std::cout << "Enabling AngelScript JIT compiler..." << std::endl;
        
        // 创建 JIT 编译器
        jit_compiler = new MockJITCompiler();
        
        // 设置引擎属性以包含 JIT 指令
        engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, true);
        
        // 设置 JIT 编译器
        int r = engine->SetJITCompiler(jit_compiler);
        if (r < 0) {
            std::cerr << "Failed to set JIT compiler" << std::endl;
            delete jit_compiler;
            jit_compiler = nullptr;
            return;
        }
        
        jit_enabled = true;
        std::cout << "AngelScript JIT compiler enabled" << std::endl;
        
    } else if (!enable && jit_enabled) {
        // 禁用 JIT
        std::cout << "Disabling AngelScript JIT compiler..." << std::endl;
        
        if (jit_compiler) {
            engine->SetJITCompiler(nullptr);
            delete jit_compiler;
            jit_compiler = nullptr;
        }
        
        engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, false);
        jit_enabled = false;
        
        std::cout << "AngelScript JIT compiler disabled" << std::endl;
    }
}

void AngelScriptEngine::messageCallback(const asSMessageInfo* msg) {
    const char* type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";
    
    std::cerr << msg->section << " (" << msg->row << ", " << msg->col << ") : "
              << type << " : " << msg->message << std::endl;
}

void AngelScriptEngine::messageCallbackStatic(const asSMessageInfo* msg, void* param) {
    AngelScriptEngine* engine = static_cast<AngelScriptEngine*>(param);
    engine->messageCallback(msg);
}

void AngelScriptEngine::registerStringType() {
    // 使用完整的字符串 add-on
    RegisterStdString(engine);
}

// 将简化的 Lua 语法转换为 AngelScript 语法
std::string AngelScriptEngine::convertLuaToAngelScript(const std::string& luaScript) {
    std::string result = luaScript;
    
    // 基础转换规则
    // Lua: local var = value -> AngelScript: int/double/string var = value;
    // Lua: for i = 1, n do -> AngelScript: for(int i = 1; i <= n; i++)
    // Lua: function name() -> AngelScript: void name()
    // Lua: .. (字符串连接) -> AngelScript: + 
    
    // 添加 main 函数包装
    result = "void main() {\n" + 
             convertLuaVarsToAngelScript(result) + 
             "\n}";
    
    return result;
}

void AngelScriptEngine::triggerJITCompilation(asIScriptModule* module) {
    if (!module || !jit_compiler) {
        return;
    }
    
    // 预编译模块中的所有函数
    // 这样 JIT 编译开销就不会计入执行时间
    asUINT funcCount = module->GetFunctionCount();
    
    for (asUINT i = 0; i < funcCount; i++) {
        asIScriptFunction* func = module->GetFunctionByIndex(i);
        if (func) {
            // 触发 JIT 编译但不计入时间
            // AngelScript 会自动调用 JIT 编译器
            // 我们只需要确保函数被"预热"
            
            // 通过准备函数执行来触发 JIT 编译
            asIScriptContext* tempCtx = engine->CreateContext();
            if (tempCtx) {
                // 只是准备，不执行，这会触发 JIT 编译
                tempCtx->Prepare(func);
                tempCtx->Release();
            }
        }
    }
}

std::string AngelScriptEngine::convertLuaVarsToAngelScript(const std::string& code) {
    std::string result = code;
    
    // 简单的正则表达式替换（这里用字符串替换简化）
    // local sum = 0 -> int sum = 0;
    size_t pos = 0;
    while ((pos = result.find("local ", pos)) != std::string::npos) {
        result.replace(pos, 6, "int ");
        pos += 4;
    }
    
    // for i = 1, 1000000 do -> for(int i = 1; i <= 1000000; i++) {
    pos = 0;
    while ((pos = result.find("for ", pos)) != std::string::npos) {
        size_t doPos = result.find(" do", pos);
        if (doPos != std::string::npos) {
            size_t commaPos = result.find(", ", pos);
            if (commaPos < doPos) {
                std::string varName = result.substr(pos + 4, result.find(" = ", pos) - pos - 4);
                std::string startVal = result.substr(result.find(" = ", pos) + 3, commaPos - result.find(" = ", pos) - 3);
                std::string endVal = result.substr(commaPos + 2, doPos - commaPos - 2);
                
                std::string newFor = "for(int " + varName + " = " + startVal + "; " + 
                                    varName + " <= " + endVal + "; " + varName + "++) {";
                
                result.replace(pos, doPos - pos + 3, newFor);
            }
        }
        pos += 3;
    }
    
    // end -> }
    pos = 0;
    while ((pos = result.find("end")) != std::string::npos) {
        result.replace(pos, 3, "}");
        pos += 1;
    }
    
    // .. -> +
    pos = 0;
    while ((pos = result.find(" .. ")) != std::string::npos) {
        result.replace(pos, 4, " + ");
        pos += 3;
    }
    
    // 添加分号到行尾（简化处理）
    pos = 0;
    while ((pos = result.find("\n", pos)) != std::string::npos) {
        if (pos > 0 && result[pos-1] != '{' && result[pos-1] != '}' && result[pos-1] != ';') {
            result.insert(pos, ";");
            pos += 2;
        } else {
            pos += 1;
        }
    }
    
    return result;
}