@echo off

REM 简化版CARLA设置脚本
REM 直接激活Visual Studio环境并运行CarlaSetup.bat

REM 检查是否以管理员权限运行
NET SESSION >nul 2>&1
if %errorLevel% neq 0 (
    echo 请以管理员权限运行此脚本！
    pause
    exit /b 1
)

REM 激活Visual Studio 2022开发环境
echo 正在激活Visual Studio 2022开发环境...
call "D:\VS2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorLevel% neq 0 (
    echo 无法激活Visual Studio环境！错误代码：%errorLevel%
    pause
    exit /b 1
)

REM 验证C++编译器
echo 正在检查C++编译器...
where cl.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo 未找到C++编译器(cl.exe)！
    pause
    exit /b 1
) else (
    echo 已找到C++编译器：
    where cl.exe
)

REM 运行CarlaSetup.bat
echo 正在运行CarlaSetup.bat...
call "%~dp0CarlaSetup.bat"

pause