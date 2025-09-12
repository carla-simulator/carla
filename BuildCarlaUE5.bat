@echo off
REM CARLA UE5.5专用构建脚本 - Visual Studio 2022版本

setlocal enabledelayedexpansion

REM 设置日志文件
set LOG_FILE=%~dp0BuildCarlaUE5.log
>"%LOG_FILE%" echo 开始构建CARLA UE5.5...
echo 构建日志将保存在：%LOG_FILE%

REM 检查是否以管理员权限运行
NET SESSION >nul 2>&1
if %errorLevel% neq 0 (
    echo 请以管理员权限运行此脚本！
    pause
    exit /b 1
)

REM 定义VS安装路径（根据实际情况修改）
set VS_INSTALL_DIR=D:\VS2022\Community

REM 激活Visual Studio 2022开发环境
echo 正在激活Visual Studio 2022开发环境...
echo.>>"%LOG_FILE%"
echo ===== 激活Visual Studio 2022开发环境 =====>>"%LOG_FILE%"
call "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat" >>"%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo 无法激活Visual Studio环境！错误代码：%errorLevel%
    echo 详细信息请查看日志文件
    pause
    exit /b 1
)

REM 验证C++编译器是否可用
echo 正在验证C++编译器...
echo.>>"%LOG_FILE%"
echo ===== 验证C++编译器 =====>>"%LOG_FILE%"
where cl.exe >>"%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo 未找到C++编译器(cl.exe)！
    echo 请确保已安装Visual Studio 2022的"使用C++的桌面开发"工作负载
    echo 详细信息请查看日志文件
    pause
    exit /b 1
) else (
    echo 已找到C++编译器：
    where cl.exe
    where cl.exe >>"%LOG_FILE%"
)

REM 验证CMake是否可用
echo.>>"%LOG_FILE%"
echo ===== 验证CMake =====>>"%LOG_FILE%"
where cmake.exe >>"%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo 未找到CMake！请安装CMake并添加到环境变量PATH中
    echo 详细信息请查看日志文件
    pause
    exit /b 1
) else (
    echo 已找到CMake：
    where cmake.exe
    where cmake.exe >>"%LOG_FILE%"
)

REM 设置UE5根目录（根据实际情况修改）
set UE5_ROOT=F:\Epic Games\UE_5.5

REM 检查UE5是否正确安装
echo.>>"%LOG_FILE%"
echo ===== 检查UE5安装 =====>>"%LOG_FILE%"
if not exist "%UE5_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" (
    echo 未找到Unreal Engine 5.5！
    echo 请设置正确的UE5_ROOT路径
    echo 详细信息请查看日志文件
    pause
    exit /b 1
) else (
    echo 已找到Unreal Engine 5.5：%UE5_ROOT%
    echo 已找到Unreal Engine 5.5：%UE5_ROOT% >>"%LOG_FILE%"
)

REM 设置环境变量
echo.>>"%LOG_FILE%"
echo ===== 设置环境变量 =====>>"%LOG_FILE%"
setx UE5_ROOT "%UE5_ROOT%" /M
if %errorLevel% neq 0 (
    echo 警告：无法永久设置UE5_ROOT环境变量
    echo 请手动设置系统环境变量UE5_ROOT=%UE5_ROOT%
    echo 警告信息 >>"%LOG_FILE%"
)

echo 环境变量PATH：>>"%LOG_FILE%"
echo %PATH% >>"%LOG_FILE%"

REM 运行CarlaSetup.bat
echo.>>"%LOG_FILE%"
echo ===== 运行CarlaSetup.bat =====>>"%LOG_FILE%"
echo 正在运行CarlaSetup.bat...
echo 注意：这将开始CARLA的完整构建过程，可能需要几个小时
echo 请确保有足够的磁盘空间（至少225GB）和稳定的网络连接
pause

call "%~dp0CarlaSetup.bat" >>"%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo CarlaSetup.bat执行失败！错误代码：%errorLevel%
    echo 详细信息请查看日志文件：%LOG_FILE%
    pause
    exit /b 1
) else (
    echo.>>"%LOG_FILE%"
echo ===== 构建完成 =====>>"%LOG_FILE%"
echo CARLA UE5.5构建成功！
echo 详细构建日志：%LOG_FILE%
)

pause
endlocal