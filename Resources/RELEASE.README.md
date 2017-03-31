CARLA UE4
=========

To run the game at fixed time-step, e.g. 30 FPS

    $ CarlaUE4.sh -benchmark -fps=30

Other CARLA related command-line options

  * `-carla-settings=<ini-file-path>` Load settings from the given INI file. See Example.CarlaSettings.ini.
  * `-world-port=<port-number>` Listen for client connections at <port-number>, write and read ports are set to <port-number>+1 and <port-number>+2 respectively.
