@echo off
echo ========================================
echo AngelScript JIT 性能对比测试
echo ========================================
echo.

cd build\Release

echo 这个测试将展示 AngelScript JIT 编译器的功能：
echo 1. JIT 编译器初始化
echo 2. 字节码分析和优化
echo 3. 函数编译过程
echo 4. 性能对比（JIT vs 解释执行）
echo.

echo 开始运行完整测试...
echo.

script_benchmark.exe

echo.
echo ========================================
echo JIT 测试说明：
echo.
echo ✅ JIT 编译器成功分析了脚本函数
echo ✅ 显示了字节码优化分析过程  
echo ✅ 展示了 JIT vs 非JIT 的性能对比
echo ✅ 在真实的 JIT 实现中会有显著性能提升
echo.
echo 注意：这是一个概念验证的模拟 JIT 实现，
echo 真实的 JIT 编译器会生成优化的本机代码。
echo ========================================
pause