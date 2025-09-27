#include "engines/quickjs_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "engines/quickjs_bridge.h"
#ifdef _WIN32
#include <windows.h>
#endif

// 动态加载 qjs_bridge.dll 的函数指针与全局运行时/上下文
static
#ifdef _WIN32
HMODULE
#else
void*
#endif
g_bridge = nullptr;

static QJS_Runtime (*p_qjs_new_runtime)(void) = nullptr;
static QJS_Context (*p_qjs_new_context)(QJS_Runtime) = nullptr;
static void (*p_qjs_free_runtime)(QJS_Runtime) = nullptr;
static void (*p_qjs_free_context)(QJS_Context) = nullptr;
static void (*p_qjs_install_helpers)(QJS_Context) = nullptr;
static int  (*p_qjs_eval)(QJS_Context, const char*, const char*) = nullptr;
static void (*p_qjs_run_gc)(QJS_Runtime) = nullptr;

static QJS_Runtime g_rt = nullptr;
static QJS_Context g_ctx = nullptr;

QuickJSEngine::QuickJSEngine() : initialized_(false), jit_enabled_(false) {}
QuickJSEngine::~QuickJSEngine() { cleanup(); }

bool QuickJSEngine::ensureQjsAvailable() { return true; }

bool QuickJSEngine::initialize() {
#ifdef _WIN32
    // 优先从第三方目录加载，其次从当前目录加载
    g_bridge = LoadLibraryA("third_party\\quickjs\\qjs_bridge.dll");
    if (!g_bridge) {
        g_bridge = LoadLibraryA("qjs_bridge.dll");
    }
    if (!g_bridge) {
        std::cerr << "QuickJS: failed to load qjs_bridge.dll" << std::endl;
        return false;
    }
    // 解析符号
    p_qjs_new_runtime      = (QJS_Runtime (*)(void))GetProcAddress(g_bridge, "qjs_new_runtime");
    p_qjs_new_context      = (QJS_Context (*)(QJS_Runtime))GetProcAddress(g_bridge, "qjs_new_context");
    p_qjs_free_runtime     = (void (*)(QJS_Runtime))GetProcAddress(g_bridge, "qjs_free_runtime");
    p_qjs_free_context     = (void (*)(QJS_Context))GetProcAddress(g_bridge, "qjs_free_context");
    p_qjs_install_helpers  = (void (*)(QJS_Context))GetProcAddress(g_bridge, "qjs_install_helpers");
    p_qjs_eval             = (int (*)(QJS_Context, const char*, const char*))GetProcAddress(g_bridge, "qjs_eval");
    p_qjs_run_gc           = (void (*)(QJS_Runtime))GetProcAddress(g_bridge, "qjs_run_gc");

    if (!p_qjs_new_runtime || !p_qjs_new_context || !p_qjs_free_runtime || !p_qjs_free_context ||
        !p_qjs_install_helpers || !p_qjs_eval || !p_qjs_run_gc) {
        std::cerr << "QuickJS: missing symbols in qjs_bridge.dll" << std::endl;
        FreeLibrary(g_bridge);
        g_bridge = nullptr;
        return false;
    }
#else
    // 非 Windows 平台暂不支持（当前项目面向 Windows）
    return false;
#endif

    // 创建持久化 runtime/context
    g_rt = p_qjs_new_runtime();
    if (!g_rt) {
        std::cerr << "QuickJS: failed to create runtime" << std::endl;
        return false;
    }
    g_ctx = p_qjs_new_context(g_rt);
    if (!g_ctx) {
        std::cerr << "QuickJS: failed to create context" << std::endl;
        p_qjs_free_runtime(g_rt);
        g_rt = nullptr;
        return false;
    }
    p_qjs_install_helpers(g_ctx);

    initialized_ = true;
    std::cout << "QuickJS JavaScript Engine initialized (embedded via DLL bridge)" << std::endl;
    return true;
}

void QuickJSEngine::cleanup() {
#ifdef _WIN32
    if (g_ctx) {
        p_qjs_free_context(g_ctx);
        g_ctx = nullptr;
    }
    if (g_rt) {
        p_qjs_free_runtime(g_rt);
        g_rt = nullptr;
    }
    if (g_bridge) {
        FreeLibrary(g_bridge);
        g_bridge = nullptr;
    }
#endif
    initialized_ = false;
}

bool QuickJSEngine::executeScript(const std::string& script) {
    if (!initialized_) return false;
    if (!g_rt || !g_ctx) return false;
    int ok = p_qjs_eval(g_ctx, script.c_str(), "<script>");
    p_qjs_run_gc(g_rt);
    return ok == 1;
}

bool QuickJSEngine::executeFile(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) return false;
    std::stringstream ss; ss << f.rdbuf();
    return executeScript(ss.str());
}

std::string QuickJSEngine::getName() const { return jit_enabled_ ? "QuickJS+Opt" : "QuickJS"; }
bool QuickJSEngine::supportsJIT() const { return false; }
void QuickJSEngine::enableJIT(bool enable) { jit_enabled_ = enable; }
