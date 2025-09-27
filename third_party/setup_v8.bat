@echo off
setlocal

REM Change to the directory of this script (third_party)
cd /d %~dp0

REM Clone depot_tools if missing
if not exist "depot_tools" (
  echo [setup_v8] Cloning depot_tools...
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git || goto :error
)

REM Ensure depot_tools uses local toolchain
set DEPOT_TOOLS_WIN_TOOLCHAIN=0

REM Prepend depot_tools to PATH for this session
set PATH=%CD%\depot_tools;%PATH%

REM Fetch V8 source if missing (optional, gclient sync will also pull)
if not exist "v8" (
  echo [setup_v8] Fetching V8...
  fetch v8 || echo [setup_v8] fetch failed or not available, will rely on gclient sync
) else (
  echo [setup_v8] V8 already fetched, skipping fetch
)

REM Force recreate a valid .gclient at parent (third_party)
echo [setup_v8] Writing fresh .gclient in third_party...
del /f /q .gclient 2>nul
rmdir /S /Q .gclient_entries 2>nul
(
  echo solutions = [
  echo   { "name": "v8", "url": "https://chromium.googlesource.com/v8/v8.git", "deps_file": "DEPS", "managed": True, "custom_deps": { }, "custom_vars": { } },
  echo ]
) > .gclient

REM Sync & run hooks
echo [setup_v8] Syncing V8 dependencies...
call "%CD%\depot_tools\gclient.bat" sync || goto :error
echo [setup_v8] Running hooks...
call "%CD%\depot_tools\gclient.bat" runhooks || goto :error

REM Enter v8 directory
cd v8 || goto :error

REM Clean previous build if exists (to ensure GN args take effect)
if exist "out\win-release" (
  echo [setup_v8] Cleaning previous build outputs...
  call "%~dp0depot_tools\ninja.bat" -C out\win-release -t clean >nul 2>nul
)

REM Generate GN build files (correctly disable Temporal support)
echo [setup_v8] Generating GN config...
call "%~dp0depot_tools\gn.bat" gen out\win-release --args="is_component_build=false is_debug=false target_cpu=\"x64\" v8_static_library=true v8_monolithic=true v8_use_external_startup_data=false use_custom_libcxx=false treat_warnings_as_errors=false v8_enable_temporal_support=false temporal_capi_use=false extra_cflags=[\"-D_SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING\"]" || goto :error

REM Build v8_monolith static library (use 8 parallel jobs)
echo [setup_v8] Building v8_monolith with 8 parallel jobs (this may take a while)...
call "%~dp0depot_tools\ninja.bat" -C out\win-release -j 8 v8_monolith || goto :error

REM Ensure monolith lib at the expected path for CMake
if not exist "out\win-release\v8_monolith.lib" (
  if exist "out\win-release\obj\v8_monolith.lib" (
    copy /Y "out\win-release\obj\v8_monolith.lib" "out\win-release\v8_monolith.lib" >nul
  )
)

echo [setup_v8] Done.
echo [setup_v8] Library: %CD%\out\win-release\v8_monolith.lib
echo [setup_v8] Includes: %CD%\include\

goto :eof

:error
echo [setup_v8] Failed. See errors above.
exit /b 1