@echo off
REM 简化版Visual Studio环境检查脚本

setlocal

REM 直接显示在控制台而不是写入日志文件
echo ======================================================
echo 简化版Visual Studio 2022环境检查

echo 1. 检查操作系统版本...
echo 系统版本：%OS%
ver

echo.
echo 2. 查找Visual Studio 2022安装位置...
for %%p in ("D:\VS2022\Community" "C:\Program Files\Microsoft Visual Studio\2022\Community" "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community") do (
    if exist "%%~p\Common7\IDE\devenv.exe" (
        echo 找到VS2022安装在：%%~p
        set VS_FOUND=true
    )
)
if not defined VS_FOUND (
    echo 未找到Visual Studio 2022安装！
)

echo.
echo 3. 查找cl.exe编译器...
where cl.exe
if %errorLevel% neq 0 (
    echo 未找到cl.exe编译器！
    echo 这是构建CARLA的关键组件缺失。
    echo 解决方案：打开Visual Studio Installer，修改安装，勾选"使用C++的桌面开发"工作负载。
) else (
    echo 已找到cl.exe编译器！
    cl.exe 2>&1 | findstr /i "Version"
)

echo.
echo 4. 检查CMake是否安装...
where cmake.exe
if %errorLevel% neq 0 (
    echo 未找到CMake！
    echo 构建CARLA需要CMake 3.24或更高版本。
) else (
    cmake --version
)

echo.
echo ======================================================
echo 环境检查完成！

pause