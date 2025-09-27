#include "engines/v8_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>

#include <v8.h>
#include <libplatform/libplatform.h>

namespace {
    std::unique_ptr<v8::Platform> g_platform;
}

bool V8Engine::platform_initialized_ = false;

V8Engine::V8Engine() : initialized_(false), jit_enabled_(true), isolate_(nullptr), allocator_(nullptr), context_persistent_(nullptr) {}
V8Engine::~V8Engine() { cleanup(); }

void V8Engine::ensurePlatform() {
    if (platform_initialized_) return;
    v8::V8::InitializeICUDefaultLocation(nullptr);
    v8::V8::InitializeExternalStartupData(nullptr);
    g_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(g_platform.get());
    v8::V8::Initialize();
    platform_initialized_ = true;
}

bool V8Engine::initialize() {
    ensurePlatform();

    allocator_ = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = static_cast<v8::ArrayBuffer::Allocator*>(allocator_);
    isolate_ = v8::Isolate::New(create_params);
    if (!isolate_) {
        std::cerr << "V8: failed to create isolate" << std::endl;
        return false;
    }

    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate_);
        v8::Local<v8::Context> context = v8::Context::New(isolate_, nullptr, global);
        v8::Persistent<v8::Context>* persistent = new v8::Persistent<v8::Context>(isolate_, context);
        context_persistent_ = persistent;
    }

    initialized_ = true;
    std::cout << "V8 JavaScript Engine initialized (embedded)" << std::endl;
    return true;
}

void V8Engine::cleanup() {
    if (context_persistent_) {
        auto* p = static_cast<v8::Persistent<v8::Context>*>(context_persistent_);
        p->Reset();
        delete p;
        context_persistent_ = nullptr;
    }
    if (isolate_) {
        isolate_->Dispose();
        isolate_ = nullptr;
    }
    if (allocator_) {
        delete static_cast<v8::ArrayBuffer::Allocator*>(allocator_);
        allocator_ = nullptr;
    }
    initialized_ = false;
}

bool V8Engine::executeScript(const std::string& script) {
    if (!initialized_ || !isolate_ || !context_persistent_) return false;

    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    auto* p = static_cast<v8::Persistent<v8::Context>*>(context_persistent_);
    v8::Local<v8::Context> context = p->Get(isolate_);
    v8::Context::Scope context_scope(context);

    v8::TryCatch try_catch(isolate_);

    v8::Local<v8::String> source;
    if (!v8::String::NewFromUtf8(isolate_, script.c_str(), v8::NewStringType::kNormal).ToLocal(&source)) {
        std::cerr << "V8: failed to create source string" << std::endl;
        return false;
    }

    v8::ScriptOrigin origin(v8::String::NewFromUtf8Literal(isolate_, "<script>"));
    v8::Local<v8::Script> compiled;
    if (!v8::Script::Compile(context, source, &origin).ToLocal(&compiled)) {
        v8::String::Utf8Value error(isolate_, try_catch.Exception());
        std::cerr << "V8 compile error: " << (*error ? *error : "<unknown>") << std::endl;
        return false;
    }

    v8::Local<v8::Value> result;
    if (!compiled->Run(context).ToLocal(&result)) {
        v8::String::Utf8Value error(isolate_, try_catch.Exception());
        std::cerr << "V8 runtime error: " << (*error ? *error : "<unknown>") << std::endl;
        return false;
    }

    return true;
}

bool V8Engine::executeFile(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) return false;
    std::stringstream ss; ss << f.rdbuf();
    return executeScript(ss.str());
}

std::string V8Engine::getName() const { return jit_enabled_ ? "V8+JIT" : "V8-JIT"; }
bool V8Engine::supportsJIT() const { return true; }
void V8Engine::enableJIT(bool enable) { jit_enabled_ = enable; /* 可扩展：传递优化标志 */ }
