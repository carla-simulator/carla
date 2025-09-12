@echo off
REM Visual Studio 2022环境诊断脚本 - 用于检查CARLA构建所需的C++开发组件

setlocal enabledelayedexpansion

REM 设置日志文件
set LOG_FILE=%~dp0CheckVSEnvironment.log
REM 创建空日志文件
echo Visual Studio 2022环境诊断报告 > "%LOG_FILE%"

echo ======================================================
echo Visual Studio 2022环境诊断工具 - CARLA UE5.5版
echo ======================================================
echo 诊断日志将保存在：%LOG_FILE%
echo.

REM 检查操作系统版本
echo 1. 检查操作系统版本...
echo.>>"%LOG_FILE%"
echo ===== 操作系统信息 =====>>"%LOG_FILE%"
ver >>"%LOG_FILE%"
echo 系统版本：%OS%
echo 系统版本已记录到日志文件

echo.

REM 检查Visual Studio 2022是否安装
echo 2. 检查Visual Studio 2022安装情况...
echo.>>"%LOG_FILE%"
echo ===== Visual Studio 2022安装检查 =====>>"%LOG_FILE%"

set VS_PATHS="D:\VS2022\Community" "C:\Program Files\Microsoft Visual Studio\2022\Community" "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community"

set VS_INSTALLED=false
for %%p in (%VS_PATHS%) do (
    if exist "%%~p\Common7\IDE\devenv.exe" (
        set VS_INSTALL_DIR=%%~p
        set VS_INSTALLED=true
        goto vs_found
    )
)

:vs_found
if %VS_INSTALLED% equ true (
    echo [成功] 找到Visual Studio 2022安装：%VS_INSTALL_DIR%
    echo Visual Studio 2022安装路径：%VS_INSTALL_DIR% >>"%LOG_FILE%"
) else (
    echo [错误] 未找到Visual Studio 2022安装！
    echo 请先安装Visual Studio 2022 Community版本。 >>"%LOG_FILE%"
    echo 推荐下载地址：https://visualstudio.microsoft.com/downloads/
    goto exit_script
)

echo.

REM 检查MSVC工具链版本
echo 3. 检查MSVC工具链版本...
echo.>>"%LOG_FILE%"
echo ===== MSVC工具链版本检查 =====>>"%LOG_FILE%"

if exist "%VS_INSTALL_DIR%\VC\Tools\MSVC" (
    echo 已找到MSVC工具链目录：
    dir /b "%VS_INSTALL_DIR%\VC\Tools\MSVC" | findstr /r "^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$"
    dir /b "%VS_INSTALL_DIR%\VC\Tools\MSVC" | findstr /r "^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$" >>"%LOG_FILE%"
) else (
    echo [错误] 未找到MSVC工具链目录！
    echo 这表明未安装Visual Studio的C++开发组件。 >>"%LOG_FILE%"
    echo 请通过Visual Studio Installer安装"使用C++的桌面开发"工作负载。
    goto exit_script
)

echo.

REM 检查cl.exe编译器是否存在
echo 4. 检查C++编译器(cl.exe)是否存在...
echo.>>"%LOG_FILE%"
echo ===== C++编译器(cl.exe)检查 =====>>"%LOG_FILE%"

set CL_FOUND=false
for /d %%d in ("%VS_INSTALL_DIR%\VC\Tools\MSVC\*.*") do (
    if exist "%%d\bin\Hostx64\x64\cl.exe" (
        set CL_PATH=%%d\bin\Hostx64\x64\cl.exe
        set CL_FOUND=true
        echo [成功] 在%%d\bin\Hostx64\x64\目录下找到cl.exe
        echo 找到cl.exe编译器：!CL_PATH! >>"%LOG_FILE%"
    )
    if exist "%%d\bin\Hostx86\x86\cl.exe" (
        set CL_PATH=%%d\bin\Hostx86\x86\cl.exe
        set CL_FOUND=true
        echo [成功] 在%%d\bin\Hostx86\x86\目录下找到cl.exe
        echo 找到cl.exe编译器：!CL_PATH! >>"%LOG_FILE%"
    )
)

if %CL_FOUND% equ false (
    echo [错误] 未找到C++编译器(cl.exe)！
    echo 这是构建CARLA的关键组件，请确保安装了Visual Studio的C++开发组件。 >>"%LOG_FILE%"
    echo 解决方案：打开Visual Studio Installer，修改安装，勾选"使用C++的桌面开发"工作负载。
    goto exit_script
)

echo.

REM 检查Windows SDK是否安装
echo 5. 检查Windows SDK是否安装...
echo.>>"%LOG_FILE%"
echo ===== Windows SDK检查 =====>>"%LOG_FILE%"

set SDK_PATHS=^"C:\Program Files (x86)\Windows Kits\10^" ^"C:\Program Files\Windows Kits\10^" ^"D:\Windows Kits\10^")

set SDK_INSTALLED=false
for %%p in (%SDK_PATHS%) do (
    if exist "%%~p\Include" (
        set SDK_INSTALL_DIR=%%~p
        set SDK_INSTALLED=true
        echo [成功] 找到Windows SDK安装：%SDK_INSTALL_DIR%
        echo Windows SDK安装路径：%SDK_INSTALL_DIR% >>"%LOG_FILE%"
        
        echo Windows SDK版本：
        dir /b "%SDK_INSTALL_DIR%\Include" | findstr /r "^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$"
        dir /b "%SDK_INSTALL_DIR%\Include" | findstr /r "^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$" >>"%LOG_FILE%"
        goto sdk_found
    )
)

:sdk_found
if %SDK_INSTALLED% equ false (
    echo [警告] 未找到Windows SDK安装！
    echo 构建CARLA可能需要Windows SDK。请通过Visual Studio Installer安装。 >>"%LOG_FILE%"
)

echo.

REM 检查vcvars64.bat是否存在
echo 6. 检查vcvars64.bat是否存在...
echo.>>"%LOG_FILE%"
echo ===== vcvars64.bat检查 =====>>"%LOG_FILE%"

if exist "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat" (
    echo [成功] 找到vcvars64.bat：%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat
    echo 找到vcvars64.bat：%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat >>"%LOG_FILE%"
) else (
    echo [错误] 未找到vcvars64.bat！
    echo 这表明Visual Studio的C++开发组件安装不完整。 >>"%LOG_FILE%"
    echo 请重新安装"使用C++的桌面开发"工作负载。
    goto exit_script
)

echo.

REM 检查CMake是否安装
echo 7. 检查CMake是否安装...
echo.>>"%LOG_FILE%"
echo ===== CMake检查 =====>>"%LOG_FILE%"

where cmake.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('where cmake.exe') do set CMAKE_PATH=%%i
    echo [成功] 找到CMake：%CMAKE_PATH%
    echo 找到CMake：%CMAKE_PATH% >>"%LOG_FILE%"
    
    REM 获取CMake版本
    for /f "tokens=3" %%v in ('cmake --version ^| findstr /i "version"') do set CMAKE_VERSION=%%v
    echo CMake版本：%CMAKE_VERSION%
    echo CMake版本：%CMAKE_VERSION% >>"%LOG_FILE%"
) else (
    echo [警告] 未找到CMake！
    echo 构建CARLA需要CMake 3.19或更高版本。 >>"%LOG_FILE%"
    echo 推荐下载地址：https://cmake.org/download/
)

echo.

REM 检查磁盘空间
echo 8. 检查磁盘空间...
echo.>>"%LOG_FILE%"
echo ===== 磁盘空间检查 =====>>"%LOG_FILE%"

for /f "tokens=3" %%s in ('dir %~d0 /-c ^| findstr /i "bytes free"') do set FREE_SPACE=%%s
set FREE_SPACE_GB=%FREE_SPACE:~0,-9%
echo 当前磁盘可用空间：约%FREE_SPACE_GB%GB
if %FREE_SPACE_GB% gtr 170 (
    echo [成功] 磁盘空间充足（≥170GB）
    echo 磁盘空间：约%FREE_SPACE_GB%GB [充足] >>"%LOG_FILE%"
) else (
    echo [警告] 磁盘空间不足！
    echo 构建CARLA需要至少170GB的可用空间，当前仅有约%FREE_SPACE_GB%GB。 >>"%LOG_FILE%"
    echo 请清理磁盘以确保有足够空间。
)

echo.

REM 生成诊断报告摘要
echo ======================================================
echo 诊断报告摘要
if %CL_FOUND% equ true (
    echo [√] C++编译器(cl.exe)：已找到
) else (
    echo [×] C++编译器(cl.exe)：未找到 - 必须安装
)
if %VS_INSTALLED% equ true (
    echo [√] Visual Studio 2022：已安装
) else (
    echo [×] Visual Studio 2022：未安装 - 必须安装
)
if %SDK_INSTALLED% equ true (
    echo [√] Windows SDK：已安装
) else (
    echo [!] Windows SDK：未找到 - 建议安装
)
where cmake.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo [√] CMake：已安装
) else (
    echo [!] CMake：未找到 - 建议安装
)
if %FREE_SPACE_GB% gtr 170 (
    echo [√] 磁盘空间：充足
) else (
    echo [!] 磁盘空间：不足
)
echo ======================================================
echo 详细报告请查看：%LOG_FILE%
echo.
echo 解决方案总结：
if %CL_FOUND% equ false (
    echo 1. 打开Visual Studio Installer
    echo 2. 选择修改Visual Studio 2022
    echo 3. 勾选"使用C++的桌面开发"工作负载
    echo 4. 在右侧组件中确保勾选了所有必要的C++工具
    echo 5. 点击修改按钮开始安装
) else (
    echo 您的Visual Studio环境基本满足CARLA构建要求！
    echo 建议运行BuildCarlaUE5.bat脚本开始构建过程。
)
echo ======================================================

echo 详细信息已保存到日志文件。

:exit_script
echo.
pause
endlocal