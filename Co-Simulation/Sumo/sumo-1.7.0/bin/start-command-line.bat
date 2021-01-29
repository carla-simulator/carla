@echo off
SET mypath=%~dp0

rem change the following line to customize the location of your sumo installation
SET sumo_home=%mypath:~0,-5%

rem change the following line to customize the location of your python installation
SET python_dir=C:\Python27

rem change the following line to set a new working dir:
set default_dir=%USERPROFILE%\Sumo
IF NOT EXIST %default_dir%\ mkdir %default_dir%

cmd /K "set PATH=%sumo_home%\bin;%python_dir%;%sumo_home%\tools;%PATH%& set PYTHONPATH=%PYTHONPATH%;%sumo_home%\tools& set SUMO_HOME=%sumo_home%& cd /d %default_dir%& echo info: added location of sumo, tools and python to the search path& echo info: variable SUMO_HOME is set to %SUMO_HOME%& echo.& echo use the 'cd /d' command to change directory& echo example usage:& echo cd /d c:\foo\bar
