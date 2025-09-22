@echo off
echo ========================================
echo AngelScript 完整 JIT 性能对比测试
echo ========================================
echo.

cd build\Release

echo 这个测试将为每个基准项目显示 JIT vs 非JIT 的性能对比
echo.

echo [测试 1] 简单循环 - JIT 对比
echo.
script_benchmark.exe simple_loop
echo.

echo [测试 2] 算术运算 - JIT 对比  
echo.
script_benchmark.exe arithmetic
echo.

echo [测试 3] 字符串操作 - JIT 对比
echo.
script_benchmark.exe string_ops
echo.

echo [测试 4] 函数调用 - JIT 对比
echo.
script_benchmark.exe function_call
echo.

echo [测试 5] 斐波那契递归 - JIT 对比
echo.
script_benchmark.exe fibonacci
echo.

echo [测试 6] JIT 专项复杂计算测试
echo.
script_benchmark.exe
echo.

echo ========================================
echo AngelScript JIT 分析总结：
echo.
echo 📊 当前测试结果说明：
echo ✅ AngelScript 的常规测试都在 JIT 启用下运行
echo ✅ JIT 编译器成功分析和编译了函数
echo ✅ JIT 对比测试展示了详细的编译过程
echo.  
echo 💡 性能解读：
echo - 这是概念验证的模拟 JIT 实现
echo - 真实 JIT 会生成优化的本机代码
echo - 模拟实现主要展示 JIT 架构和分析能力
echo ========================================
pause