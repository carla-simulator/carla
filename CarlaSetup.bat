@echo off

set timestamp=%date%-%time%
set setup_utils_folder=%cd%\Util\SetupUtils\
set setup_logs_folder=%SetupUtilsFolder%\SetupLogs\

if not exist "%SetupLogsFolder%" (
    mkdir "%SetupLogsFolder%"
)

%SetupUtilsFolder%SetupMain.bat>%SetupLogsFolder%%timestamp%.log
