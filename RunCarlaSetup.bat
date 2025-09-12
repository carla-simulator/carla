@echo off

REM 设置日志文件
set LOG_FILE=%~dp0RunCarlaSetup.log

REM 清空旧日志文件
echo. > "%LOG_FILE%"

REM 添加时间戳
echo ====================================================== >> "%LOG_FILE%"
date /t >> "%LOG_FILE%"
time /t >> "%LOG_FILE%"
echo ====================================================== >> "%LOG_FILE%"

REM 检查是否以管理员权限运行
echo 检查管理员权限... >> "%LOG_FILE%"
NET SESSION >nul 2>&1
if %errorLevel% neq 0 (
    echo [错误] 请以管理员权限运行此脚本！ >> "%LOG_FILE%"
    echo 请以管理员权限运行此脚本！
    echo 详细信息请查看日志文件: %LOG_FILE%
    pause
    exit /b 1
) else (
    echo [成功] 已获得管理员权限。 >> "%LOG_FILE%"
)

REM 激活Visual Studio 2022开发环境
echo 尝试激活Visual Studio 2022开发环境... >> "%LOG_FILE%"
call "D:\VS2022\Community\VC\Auxiliary\Build\vcvars64.bat" >> "%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo [错误] 无法激活Visual Studio环境。错误代码：%errorLevel% >> "%LOG_FILE%"
    echo 无法激活Visual Studio环境。检查是否安装了Visual Studio 2022和C++开发组件。
    echo 请通过Visual Studio Installer安装"使用C++的桌面开发"工作负载。
    echo 详细信息请查看日志文件: %LOG_FILE%
    pause
    exit /b 1
) else (
    echo [成功] Visual Studio环境已激活。 >> "%LOG_FILE%"
)

REM 显示当前环境变量中的PATH
echo 当前PATH环境变量： >> "%LOG_FILE%"
echo %PATH% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

REM 验证C++编译器是否可用
echo 验证C++编译器(cl.exe)是否可用... >> "%LOG_FILE%"
where cl.exe >> "%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo [错误] 未找到C++编译器(cl.exe)。请确保安装了Visual Studio 2022的C++开发组件。 >> "%LOG_FILE%"
    echo 未找到C++编译器(cl.exe)。请确保安装了Visual Studio 2022的C++开发组件。
    echo 推荐安装工作负载：使用C++的桌面开发，包括所有必要的组件。
    echo 详细信息请查看日志文件: %LOG_FILE%
    pause
    exit /b 1
) else (
    echo [成功] 找到了C++编译器。 >> "%LOG_FILE%"
    where cl.exe >> "%LOG_FILE%"
)

REM 运行CarlaSetup.bat
echo 正在运行CarlaSetup.bat... >> "%LOG_FILE%"
echo 正在运行CarlaSetup.bat...
call "%~dp0CarlaSetup.bat" >> "%LOG_FILE%" 2>&1
if %errorLevel% neq 0 (
    echo [错误] CarlaSetup.bat执行失败！错误代码：%errorLevel% >> "%LOG_FILE%"
    echo CarlaSetup.bat执行失败！
    echo 详细信息请查看日志文件: %LOG_FILE%
    pause
    exit /b 1
) else (
    echo [成功] CarlaSetup.bat执行成功！ >> "%LOG_FILE%"
    echo CarlaSetup.bat执行成功！
    echo 详细信息请查看日志文件: %LOG_FILE%
)

REM 添加结束时间戳
echo. >> "%LOG_FILE%"
echo ====================================================== >> "%LOG_FILE%"
echo 脚本执行完成时间: >> "%LOG_FILE%"
date /t >> "%LOG_FILE%"
time /t >> "%LOG_FILE%"
echo ====================================================== >> "%LOG_FILE%"

pause