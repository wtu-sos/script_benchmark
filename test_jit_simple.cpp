#include "engines/angelscript_engine.h"
#include "timer.h"
#include <iostream>

int main() {
    AngelScriptEngine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize AngelScript" << std::endl;
        return -1;
    }
    
    std::string script = R"(
void main() {
    int sum = 0;
    const int maxCount = 1000000;
    for(int i = 1; i <= maxCount; i++) {
        sum = sum + i;
    }
}
    )";
    
    std::cout << "\n=== Testing AngelScript JIT Performance ===" << std::endl;
    
    // Test without JIT
    std::cout << "\n--- Without JIT ---" << std::endl;
    engine.enableJIT(false);
    
    Timer timer1;
    timer1.start();
    bool success1 = engine.executeScript(script);
    timer1.stop();
    
    std::cout << "Success: " << (success1 ? "YES" : "NO") << std::endl;
    std::cout << "Time: " << timer1.getElapsedMs() << " ms" << std::endl;
    
    // Test with JIT
    std::cout << "\n--- With JIT ---" << std::endl;
    engine.enableJIT(true);
    
    Timer timer2;
    timer2.start();
    bool success2 = engine.executeScript(script);
    timer2.stop();
    
    std::cout << "Success: " << (success2 ? "YES" : "NO") << std::endl;
    std::cout << "Time: " << timer2.getElapsedMs() << " ms" << std::endl;
    
    if (success1 && success2 && timer1.getElapsedMs() > 0) {
        double speedup = timer1.getElapsedMs() / timer2.getElapsedMs();
        std::cout << "\nSpeedup: " << speedup << "x" << std::endl;
    }
    
    engine.cleanup();
    return 0;
}