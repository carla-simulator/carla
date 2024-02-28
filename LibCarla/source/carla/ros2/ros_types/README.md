To update the types within this folder one has to:

  * ```git clone https://github.com/carla-simulator/ros-carla-msgs carla_msgs```
  * install ROS2 on the system and all message dependencies of the carla_msgs
  * in case the carla msg files are changed:
    - build the ROS2 package of the carla_msgs
    - copy the idl files from the build folder into the respective carla_msgs folder
    - revert the removal of "#pragma once" line within the overridden idls
    - add "#pragma once" directive to newly created idls
  * Then (install and) call fastddsgen outside the carla_msgs folder e.g.
    ```fastddsgen -d code -I . -I /opt/ros/humble/share/ -typeros2 carla_msgs/msg/*.idl```
  * Then copy all required code into the respecitve subfolders in here and put rework include directives where necessary 
    as the fastddsgen generator is unfortunately not supporting to replicate the correct subdirectory structure in all cases yet.


  

