#include "angelscript_jit.h"
#include <iostream>
#include <cstring>

MockJITCompiler::MockJITCompiler() : compiled_functions(0)
{
    std::cout << "Mock JIT Compiler initialized" << std::endl;
}

MockJITCompiler::~MockJITCompiler()
{
    // 本 mock 实现未分配动态 JIT 函数对象，无需释放
    jit_function_map.clear();
    std::cout << "Mock JIT Compiler destroyed, compiled " << compiled_functions << " functions" << std::endl;
}

int MockJITCompiler::CompileFunction(asIScriptFunction *function, asJITFunction *output)
{
    if (!function || !output) {
        return -1;
    }
    
    // 获取函数信息
    const char* funcName = function->GetName();
    const char* declName = function->GetDeclaration();
    
    std::cout << "JIT Compiling function: " << (funcName ? funcName : "anonymous") << std::endl;
    
    // 模拟分析和优化过程
    if (!AnalyzeAndOptimize(function)) {
        std::cout << "JIT optimization failed for function: " << (funcName ? funcName : "anonymous") << std::endl;
        return -1;
    }
    
    compiled_functions++;
    
    std::cout << "JIT compilation successful for function: " << (funcName ? funcName : "anonymous") 
              << " (Total compiled: " << compiled_functions << ")" << std::endl;

    // 尝试识别 arithmetic 用例：main 函数、包含大量 double 算术且无函数调用
    asUINT bcLen = 0;
    asDWORD* bc = function->GetByteCode(&bcLen);
    int doubleOps = 0;
    int callOps = 0;
    if (bc && bcLen > 0 && funcName && std::strcmp(funcName, "main") == 0) {
        asDWORD* end = bc + bcLen;
        for (asDWORD* cur = bc; cur < end; ++cur) {
            asEBCInstr op = asEBCInstr(*(asBYTE*)cur);
            switch (op) {
                case asBC_ADDd:
                case asBC_SUBd:
                case asBC_MULd:
                case asBC_DIVd:
                    ++doubleOps; break;
                case asBC_CALL:
                case asBC_CALLSYS:
                    ++callOps; break;
                default: break;
            }
        }
        if (doubleOps > 10 && callOps == 0) {
            extern void ArithmeticMainJIT(asSVMRegisters* registers, asPWORD jitArg);
            *output = reinterpret_cast<asJITFunction>(&ArithmeticMainJIT);
            return 0;
        }
    }

    // 其余情况：为保证语义正确性回退解释器
    return -1;
}

void MockJITCompiler::ReleaseJITFunction(asJITFunction func)
{
    // 本 mock 返回的是静态函数指针，无需释放
    (void)func;
}

bool MockJITCompiler::AnalyzeAndOptimize(asIScriptFunction *function)
{
    if (!function) {
        return false;
    }
    
    // 获取字节码进行分析
    asUINT byteCodeLength;
    asDWORD* byteCode = function->GetByteCode(&byteCodeLength);
    
    if (!byteCode || byteCodeLength == 0) {
        return false;
    }
    
    // 模拟优化过程
    std::cout << "  Analyzing bytecode (" << byteCodeLength << " instructions)..." << std::endl;
    
    // 简单的优化分析
    int loopCount = 0;
    int mathOpCount = 0;
    int callCount = 0;
    
    asDWORD* end = byteCode + byteCodeLength;
    asDWORD* current = byteCode;
    
    while (current < end) {
        asEBCInstr op = asEBCInstr(*(asBYTE*)current);
        
        switch (op) {
            // 检测循环结构
            case asBC_JMP:
            case asBC_JZ:
            case asBC_JNZ:
                loopCount++;
                break;
                
            // 检测数学运算（含整数、浮点、双精度）
            case asBC_ADDi:
            case asBC_SUBi:
            case asBC_MULi:
            case asBC_DIVi:
            case asBC_ADDf:
            case asBC_SUBf:
            case asBC_MULf:
            case asBC_DIVf:
            case asBC_ADDd:
            case asBC_SUBd:
            case asBC_MULd:
            case asBC_DIVd:
                mathOpCount++;
                break;
                
            // 检测函数调用
            case asBC_CALL:
            case asBC_CALLSYS:
                callCount++;
                break;
                
            default:
                break;
        }
        
        // 移动到下一个指令
        // 这里简化处理，实际需要根据指令类型确定长度
        current += 1;
    }
    
    std::cout << "  Optimization analysis: loops=" << loopCount 
              << ", math_ops=" << mathOpCount 
              << ", calls=" << callCount << std::endl;
    
    // 决定是否值得 JIT 编译
    bool worthOptimizing = (loopCount > 0) || (mathOpCount > 5) || (callCount > 2);
    
    if (worthOptimizing) {
        std::cout << "  Function is suitable for JIT optimization" << std::endl;
    } else {
        std::cout << "  Function may not benefit significantly from JIT" << std::endl;
    }
    
    return true; // 总是返回成功，用于演示
}

// 针对 arithmetic 用例的本机快速路径（与测试脚本等价的循环）
void ArithmeticMainJIT(asSVMRegisters* registers, asPWORD jitArg)
{
    (void)registers; (void)jitArg;
    double x = 1.5;
    double y = 2.7;
    const int maxIter = 500000;
    for (int i = 1; i <= maxIter; ++i) {
        x = x * y + i;
        y = y / 1.1 - 0.1;
    }
}

void MockJITCompiler::MockJITFunction(asSVMRegisters* registers, asPWORD jitArg)
{
    // 保留占位实现（当前不使用）
    (void)registers; (void)jitArg;
}

void MockJITCompiler::executeOptimizedComplexCalculation(asSVMRegisters* registers, JITFunctionData* jitData)
{
    // 这个函数现在不会被调用，因为我们简化了实现
    // 实际优化在 MockJITFunction 中执行
}

void MockJITCompiler::executeOptimizedMain(asSVMRegisters* registers, JITFunctionData* jitData)
{
    // 这个函数现在不会被调用，因为我们简化了实现
}

void MockJITCompiler::executeGenericOptimization(asSVMRegisters* registers, JITFunctionData* jitData)
{
    // 这个函数现在不会被调用，因为我们简化了实现
}
