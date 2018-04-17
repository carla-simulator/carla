@echo off

msg * "Sorry, this script is currently unavailable."
exit

rem echo Deleting intermediate folders...
rem FOR %%G IN (Binaries,Intermediate,Plugins\Carla\Binaries,Plugins\Carla\Intermediate) DO (if exist %%G ( rmdir /s/q %%G ))

rem echo Making CarlaServer...
rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
rem Visual Studio 2017 Enterprise:
rem call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" 
rem START /wait cmd.exe /k "cd Plugins\Carla & make clean default & pause & exit"



rem echo Launch editor...
rem start CarlaUE4.uproject
