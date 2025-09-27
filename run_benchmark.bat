@echo off
echo =======================================================
echo            脚本语言性能基准测试
echo =======================================================
echo.
echo 测试选项：
echo   1. 完整基准测试 (所有引擎，所有测试)
echo   2. 简单循环测试
echo   3. 算术运算测试  
echo   4. 字符串操作测试
echo   5. 函数调用测试
echo   6. 斐波那契递归测试
echo.

set /p choice="请选择测试 (1-6): "

cd build\Release

if "%choice%"=="1" (
    echo 运行完整基准测试...
    script_benchmark.exe
) else if "%choice%"=="2" (
    echo 运行简单循环测试...
    script_benchmark.exe simple_loop
) else if "%choice%"=="3" (
    echo 运行算术运算测试...
    script_benchmark.exe arithmetic
) else if "%choice%"=="4" (
    echo 运行字符串操作测试...
    script_benchmark.exe string_ops
) else if "%choice%"=="5" (
    echo 运行函数调用测试...
    script_benchmark.exe function_call
) else if "%choice%"=="6" (
    echo 运行斐波那契递归测试...
    script_benchmark.exe fibonacci
) else (
    echo 无效选择，运行完整测试...
    script_benchmark.exe
)

echo.
echo 测试完成！查看 BENCHMARK_RESULTS.md 了解详细分析。
pause