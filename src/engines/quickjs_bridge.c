// QuickJS C bridge to be called from C++ without including quickjs.h there
// Build this file as C (not C++). It wraps QuickJS APIs and exposes opaque handles.

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define QJS_API __declspec(dllexport)
#else
#define QJS_API
#endif

// Opaque handle types (matching the header that C++ will include)
typedef void* QJS_Runtime;
typedef void* QJS_Context;

// Include QuickJS only in C source to avoid MSVC C++ C99 incompatibilities
#include "quickjs.h"
#include "quickjs-libc.h"

#ifdef __cplusplus
extern "C" {
#endif

QJS_API QJS_Runtime qjs_new_runtime(void) {
    JSRuntime* rt = JS_NewRuntime();
    return (QJS_Runtime)rt;
}

QJS_API QJS_Context qjs_new_context(QJS_Runtime rt) {
    JSContext* ctx = JS_NewContext((JSRuntime*)rt);
    return (QJS_Context)ctx;
}

QJS_API void qjs_free_runtime(QJS_Runtime rt) {
    if (rt) JS_FreeRuntime((JSRuntime*)rt);
}

QJS_API void qjs_free_context(QJS_Context ctx) {
    if (ctx) JS_FreeContext((JSContext*)ctx);
}

QJS_API void qjs_install_helpers(QJS_Context ctx) {
    js_std_add_helpers((JSContext*)ctx, 0, NULL);
}

QJS_API int qjs_eval(QJS_Context ctx, const char* code, const char* filename) {
    JSContext* c = (JSContext*)ctx;
    if (!c || !code) return 0;
    const char* fname = filename ? filename : "<script>";
    JSValue v = JS_Eval(c, code, (size_t)strlen(code), fname, JS_EVAL_TYPE_GLOBAL);
    int ok = !JS_IsException(v);
    if (!ok) {
        JSValue ex = JS_GetException(c);
        const char* msg = JS_ToCString(c, ex);
        if (msg) {
            fprintf(stderr, "QuickJS Error: %s\n", msg);
            JS_FreeCString(c, msg);
        }
        JS_FreeValue(c, ex);
    }
    JS_FreeValue(c, v);
    return ok;
}

QJS_API void qjs_run_gc(QJS_Runtime rt) {
    if (rt) JS_RunGC((JSRuntime*)rt);
}

#ifdef __cplusplus
}
#endif
