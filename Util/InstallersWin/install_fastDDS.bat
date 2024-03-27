@echo off
setlocal

rem BAT script that downloads and installs Fast-DDS library
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================


set DEL_SRC=false
set BOOST_VERSION="unknown"
set FASTDDS_INSTALL_DIR=""
set GENERATOR=""

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )

    if "%1"=="--delete-src" (
        set DEL_SRC=true
    )

    if "%1"=="--boost-version" (
        set BOOST_VERSION=%2
        shift
    )

    if "%1"=="--install-dir" (
        set FASTDDS_INSTALL_DIR=%2
        shift
    )

    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
    )

    shift
    goto :arg-parse
)

if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"
echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
    set PLATFORM=-A x64
) || (
    set PLATFORM=
)
echo "%GENERATOR%" "%PLATFORM%"

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

set FASTDDS_SRC=fastDDS-src
set FASTDDS_SRC_DIR=%BUILD_DIR%%FASTDDS_SRC%\
if "%FASTDDS_INSTALL_DIR%" == "" (
    set FASTDDS_INSTALL=fastDDS-install
    set FASTDDS_INSTALL_DIR=%BUILD_DIR%%FASTDDS_INSTALL%\
)
set FASTDDS_BUILD_DIR=%FASTDDS_SRC_DIR%build\fastdds
set FASTDDS_BASENAME=%FASTDDS_SRC%
set FOONATHAN_MEMORY_VENDOR_BASENAME=foonathan-memory-vendor
set FOONATHAN_MEMORY_VENDOR_SOURCE_DIR=%FASTDDS_SRC_DIR%\thirdparty\foonathan-memory-vendor
set FOONATHAN_MEMORY_VENDOR_BUILD_DIR=%FASTDDS_SRC_DIR%build\foonathan-memory-vendor

if exist "%FASTDDS_INSTALL_DIR%" (
    goto already_build
)

if not exist "%FASTDDS_SRC_DIR%" (
    echo %FILE_N% Cloning "Fast-DDS"

    git clone --depth 1 --branch 2.11.2 https://github.com/eProsima/Fast-DDS.git "%FASTDDS_SRC_DIR:~0,-1%"
    if %errorlevel% neq 0 goto error_git
    git submodule init
    if %errorlevel% neq 0 goto error_git
    git submodule update
    if %errorlevel% neq 0 goto error_git
    git clone --depth 1 --branch master https://github.com/eProsima/foonathan_memory_vendor.git "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%"
) else (
    echo %FILE_N% Not cloning "Fast-DDS" because already exists a folder called "%FASTDDS_SRC%".
)

if not exist "%FOONATHAN_MEMORY_VENDOR_BUILD_DIR%" (
    echo %FILE_N% Creating "%FOONATHAN_MEMORY_VENDOR_BUILD_DIR%"
    mkdir "%FOONATHAN_MEMORY_VENDOR_BUILD_DIR%"
)
cd "%FOONATHAN_MEMORY_VENDOR_BUILD_DIR%"

echo %FILE_N% Generating build: foonathan memory vendor ...
cmake -G %GENERATOR% %PLATFORM%^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
    -DBUILD_STATIC_LIBS=ON^
    -DBUILD_SHARED_LIBS=OFF^
    -DCMAKE_CXX_FLAGS="/DBOOST_NO_EXCEPTIONS /DASIO_NO_EXCEPTIONS"^
    "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building foonathan memory vendor... 
cmake --build . --config Release --target install

if exist "%FASTDDS_SRC_DIR%\thirdparty\boost\include\boost" (
    echo %FILE_N% Preparing fastdds boost ... 
    @REM remove their boost includes, but keep their entry point
    rd /s /q "%FASTDDS_SRC_DIR%\thirdparty\boost\include\boost"
    @REM  ensure the find boost compiles without exceptions
    sed -i s/"CXX_STANDARD 11"/"CXX_STANDARD 11\n         COMPILE_DEFINITIONS \"-DBOOST_NO_EXCEPTIONS\""/ "%FASTDDS_SRC_DIR%\cmake\modules\FindThirdpartyBoost.cmake"
    sed -i s/"class ThirdpartyBoostCompileTest"/"#ifdef BOOST_NO_EXCEPTIONS\nnamespace boost {void throw_exception(std::exception const \& e) {}}\n#endif\nclass ThirdpartyBoostCompileTest"/ "%FASTDDS_SRC_DIR%\thirdparty\boost\test\ThirdpartyBoostCompile_test.cpp"
)

if exist "%FASTDDS_SRC_DIR%\src\cpp\utils\StringMatching.cpp" (
    echo %FILE_N% Patching fastdds ... 
    sed -i s/"defined(__cplusplus_winrt)"/"(1)"/ "%FASTDDS_SRC_DIR%\src\cpp\utils\StringMatching.cpp"
    sed -i s/"replace_all(pattern"/"replace_all(path"/ "%FASTDDS_SRC_DIR%\src\cpp\utils\StringMatching.cpp"
)

if not exist "%FASTDDS_BUILD_DIR%" (
    echo %FILE_N% Creating "%FASTDDS_BUILD_DIR%"
    mkdir "%FASTDDS_BUILD_DIR%"
)
cd "%FASTDDS_BUILD_DIR%"
echo %FILE_N% Generating build: fastdds ...

cmake -G %GENERATOR% %PLATFORM%^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
    -DCMAKE_MODULE_PATH="%FASTDDS_INSTALL_DIR:\=/%"^
    -DBUILD_STATIC_LIBS=ON^
    -DBUILD_SHARED_LIBS=OFF^
    -DBUILD_TESTING=OFF^
    -DCOMPILE_EXAMPLES=OFF^
    -DCOMPILE_TOOLS=OFF^
    -DTHIRDPARTY_Asio=FORCE^
    -DTHIRDPARTY_fastcdr=FORCE^
    -DTHIRDPARTY_TinyXML2=FORCE^
    -DTHIRDPARTY_BOOST_INCLUDE_DIR="%BUILD_DIR%boost-%BOOST_VERSION%-install\include;%FASTDDS_SRC_DIR%\thirdparty\boost\include"^
    -DCMAKE_CXX_FLAGS="/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING /DBOOST_NO_EXCEPTIONS /DASIO_NO_EXCEPTIONS"^
    "%FASTDDS_SRC_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building fastdds...
cmake --build . --config Release --target install

if %errorlevel%  neq 0 goto error_install

rem copy asio header files as they are not copied automatically by the above build, but we need those for handling the asio-exceptions
copy %FASTDDS_SRC_DIR%thirdparty\asio\asio\include\asio.hpp %INSTALLATION_DIR%fastDDS-install\include\ > NUL
xcopy /Y /S /I  %FASTDDS_SRC_DIR%thirdparty\asio\asio\include\asio\* %INSTALLATION_DIR%fastDDS-install\include\asio\* > NUL
if %errorlevel%  neq 0 goto error_install

rem Remove the downloaded Fast-DDS source because is no more needed
if %DEL_SRC% == true (
    rd /s /q "%FASTDDS_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Fast-DDS" has been successfully installed in "%FASTDDS_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A "Fast-DDS" installation already exists.
    echo %FILE_N% Delete "%FASTDDS_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_git
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure "git" is installed.
    echo %FILE_N%              - Make sure it is available on your Windows "path".
    goto bad_exit

:error_cmake
    echo.
    echo %FILE_N% [CMAKE ERROR] An error ocurred while executing the cmake.
    echo %FILE_N% [CMAKE ERROR] Possible causes:
    echo %FILE_N%                - Make sure "CMake" is installed.
    echo %FILE_N%                - Make sure it is available on your Windows "path".
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [Visual Studio 16 2019 Win64 ERROR] An error ocurred while installing using Visual Studio 16 2019 Win64.
    echo %FILE_N% [Visual Studio 16 2019 Win64 ERROR] Possible causes:
    echo %FILE_N%                - Make sure you have Visual Studio installed.
    echo %FILE_N%                - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                  For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                  or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal & set install_dds=%FASTDDS_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%FASTDDS_INSTALL_DIR%" rd /s /q "%FASTDDS_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
