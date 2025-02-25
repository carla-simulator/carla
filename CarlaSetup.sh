#! /bin/bash

timestamp=$(date +%F_%T)
setup_utils_folder=%cd%/Util/SetupUtils/
setup_logs_folder=%SetupUtilsFolder%/SetupLogs/

if not exist "%SetupLogsFolder%" (
    mkdir "%SetupLogsFolder%"
)

$setup_utils_folder/SetupMain.bat>$setup_logs_folder$timestamp.log
