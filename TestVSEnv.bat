@echo off

REM 测试Visual Studio环境和C++编译器状态

REM 激活Visual Studio 2022开发环境
echo 正在激活Visual Studio 2022开发环境...
call "D:\VS2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorLevel% neq 0 (
    echo 激活Visual Studio环境失败！错误代码：%errorLevel%
    pause
    exit /b 1
)

REM 检查PATH中的MSVC路径
echo.
echo PATH中的MSVC路径:
for %%i in ("%PATH%;") do (
    echo %%i | findstr /i "MSVC" >nul
    if not errorlevel 1 (
        echo %%i
    )
)

REM 查找cl.exe编译器
echo.
echo 查找cl.exe编译器...
where cl.exe
if %errorLevel% neq 0 (
    echo 未找到cl.exe编译器！
    pause
    exit /b 1
)

REM 显示cl.exe版本信息
echo.
echo cl.exe版本信息:
cl.exe 2>&1 | findstr /i "Version"

pause