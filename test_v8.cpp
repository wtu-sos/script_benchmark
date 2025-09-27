#include <iostream>
#include "src/engines/v8_engine.h"

int main() {
    std::cout << "Testing V8 Engine..." << std::endl;
    
    V8Engine v8_engine;
    std::cout << "V8Engine object created" << std::endl;
    
    bool success = v8_engine.initialize();
    std::cout << "V8Engine initialize() returned: " << (success ? "true" : "false") << std::endl;
    
    if (success) {
        std::cout << "V8 Engine successfully initialized" << std::endl;
    } else {
        std::cout << "V8 Engine initialization failed" << std::endl;
    }
    
    std::cout << "Test completed" << std::endl;
    return 0;
}