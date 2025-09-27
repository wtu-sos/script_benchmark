#pragma once

#ifdef _WIN32
#define QJS_API __declspec(dllimport)
#else
#define QJS_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* QJS_Runtime;
typedef void* QJS_Context;

QJS_API QJS_Runtime qjs_new_runtime(void);
QJS_API QJS_Context qjs_new_context(QJS_Runtime rt);
QJS_API void qjs_free_runtime(QJS_Runtime rt);
QJS_API void qjs_free_context(QJS_Context ctx);
QJS_API void qjs_install_helpers(QJS_Context ctx);
QJS_API int qjs_eval(QJS_Context ctx, const char* code, const char* filename);
QJS_API void qjs_run_gc(QJS_Runtime rt);

#ifdef __cplusplus
}
#endif
