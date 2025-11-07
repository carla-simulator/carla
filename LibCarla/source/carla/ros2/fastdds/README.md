To update the types within this folder one has to:

  
  * Checkout the github.com/carla-simulator/ros-carla-msgs repository
    ```git clone https://github.com/carla-simulator/ros-carla-msgs```
  * install ROS2 on the system and all message dependencies of the carla_msgs (see ros-carla-msgs docu)
  * in case the carla msg files are changed:
    - build the ROS2 package of the carla_msgs
    - copy the idl files from the build folder into the respective carla_msgs folder
    - revert the removal of "#pragma once" line within the overridden idls
    - add "#pragma once" directive to newly created idls
  * To have all relevant files beeing placed in the correct subfolders by the code generator it is best practice to copy the carla_msgs folder
    in parallel to the other folders of your ROS2 system first and execute the generator from the respective ROS2 folder e.g.
     ```
     sudo cp -r carla_msgs /opt/ros/<distname>/share
     Fast-DDS-GEN/scripts/fastddsgen -d <home>/output-code -I /opt/ros/<distname>/share/ -typeros2 carla_msgs/msg/*.idl
     ```
     In case you get errors in some of the idl files: add "#pragma once" directive to those idls to ensure they are only included once by the generator.
  * In some cases you will have to rename variables because of name clashes within different sub-namespaces which the fastddsgen generator is not able to   
    distiguish. Easiest workaround for variables is placing a "_" in front of the name, so the output will be the same as expected. On class files append e.g. "BLABLA" and later perform a search and replace. Alternatively wait until the generator is fixed and works properly
