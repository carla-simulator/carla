@echo off
REM 测试日志文件创建功能

set TEST_LOG=%~dp0TestLog.txt

echo 正在创建测试日志文件: %TEST_LOG%
echo 这是一条测试日志记录 >>"%TEST_LOG%"
echo %date% %time% >>"%TEST_LOG%"
echo 测试日志文件创建完成！
dir "%TEST_LOG%"

pause