@echo off
echo ========================================
echo Script Language Performance Benchmark
echo ========================================
echo.

cd build\Release

echo 运行完整基准测试...
echo.
echo ================ 完整测试结果 ================
script_benchmark.exe
echo.

echo ================ 单项测试详情 ================
echo.

echo [1/6] 简单循环测试 (1,000,000 次累加)
script_benchmark.exe simple_loop
echo.

echo [2/6] 算术运算测试 (500,000 次浮点运算)
script_benchmark.exe arithmetic
echo.

echo [3/6] 字符串操作测试 (10,000 次字符串拼接)
script_benchmark.exe string_ops
echo.

echo [4/6] 函数调用测试 (100,000 次函数调用)
script_benchmark.exe function_call
echo.

echo [5/6] 斐波那契递归测试 (计算 fib(1) 到 fib(30))
script_benchmark.exe fibonacci
echo.

echo [6/6] AngelScript JIT 对比测试
echo 注意：这将显示 JIT 编译器的详细分析过程
script_benchmark.exe
echo.

echo ========================================
echo 所有测试完成！
echo ========================================
pause