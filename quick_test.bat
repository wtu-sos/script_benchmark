@echo off
echo ========================================
echo 快速性能测试 - 主要测试项目
echo ========================================
echo.

cd build\Release

echo [测试 1] 简单循环性能
script_benchmark.exe simple_loop
echo.

echo [测试 2] 字符串操作性能 (AngelScript 优势项目)
script_benchmark.exe string_ops
echo.

echo [测试 3] 斐波那契递归性能
script_benchmark.exe fibonacci
echo.

echo ========================================
echo 快速测试完成！
echo 
echo 提示：
echo - 运行 run_all_tests.bat 查看完整测试
echo - 运行 script_benchmark.exe 查看所有测试汇总
echo ========================================
pause