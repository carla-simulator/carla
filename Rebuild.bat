@echo off

echo Deleting intermediate folders...
FOR %%G IN (Binaries,Intermediate,Plugins\Carla\Binaries,Plugins\Carla\Intermediate) DO (if exist %%G ( rmdir /s/q %%G ))

echo Making CarlaServer...
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
START /wait cmd.exe /k "cd Plugins\Carla & make clean default & pause & exit"

echo Launch editor...
start CarlaUE4.uproject
