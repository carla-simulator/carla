@echo off
setlocal EnableDelayedExpansion

REM 启动式构建脚本：将 UE5.5 与缓存放在 E 盘，记录日志

set LOG_FILE=%~dp0Build_Start_E.log
echo ===== Start %DATE% %TIME% =====>"%LOG_FILE%"

REM 1) 设置 UE 缓存到 E 盘，减少 C 盘占用
setx UE-SharedDataCachePath E:\UE_DDC >nul 2>nul
setx UE-LocalDataCachePath  E:\UE_DDC_Local >nul 2>nul
if not exist "E:\UE_DDC" mkdir "E:\UE_DDC" >>"%LOG_FILE%" 2>&1
if not exist "E:\UE_DDC_Local" mkdir "E:\UE_DDC_Local" >>"%LOG_FILE%" 2>&1

REM 2) 默认 UE 引擎根目录（可改为你希望的路径，需在 E 盘）
if "%CARLA_UNREAL_ENGINE_PATH%"=="" (
  set UE5_ROOT=E:\UnrealEngine5_carla
  setx CARLA_UNREAL_ENGINE_PATH "!UE5_ROOT!" >nul 2>nul
) else (
  set UE5_ROOT=%CARLA_UNREAL_ENGINE_PATH%
)
echo UE target root: !UE5_ROOT!>>"%LOG_FILE%"

REM 3) 激活 VS 2022 x64 构建环境（如路径不同，请相应修改）
set VS_INSTALL_DIR=D:\VS2022\Community
call "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat" >>"%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
  echo [ERROR] Failed to activate VS build env.>>"%LOG_FILE%"
  echo 请确认已安装 VS2022，且包含 C++ 工作负载。
  exit /b 1
)

REM 4) 选择 Python 3.8（若不存在则交由 CarlaSetup 自动处理）
if exist "D:\Project\soft\Python38\python.exe" (
  set PY_ROOT=D:\Project\soft\Python38
  set PY_ARG=--python-root=!PY_ROOT!
) else (
  set PY_ARG=
)

REM 5) 运行 CarlaSetup（包含：先决条件安装、内容下载、UE 克隆与构建、CARLA 构建、Python API 安装）
echo Running: CarlaSetup.bat !PY_ARG! --interactive>>"%LOG_FILE%"
call "%~dp0CarlaSetup.bat" !PY_ARG! --interactive >>"%LOG_FILE%" 2>&1
set ERR=%errorLevel%
echo ===== End %DATE% %TIME% (err=!ERR!) =====>>"%LOG_FILE%"

if not %ERR%==0 (
  echo 构建失败，详见日志：%LOG_FILE%
  exit /b %ERR%
)

echo 构建完成。日志：%LOG_FILE%
endlocal

