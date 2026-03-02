To update the types within this folder one has to:

  
  * Checkout the github.com/carla-simulator/ros-carla-msgs repository
    ```git clone https://github.com/carla-simulator/ros-carla-msgs```
  * install ROS2 on the system and all message dependencies of the carla_msgs (see ros-carla-msgs docu)
  * in case the carla msg files are changed:
    - build the ROS2 package of the carla_msgs
    - clean the idl files by calling the clean_idl_file.bash script on the files e.g.
    ```find install/carla_msgs -name "*.idl" -exec clean_idl_file.bash {} \;```
    - the cleaning adds "#pragma once" directive if not yet present to prevent from multiple including the files
  * To have all relevant files beeing placed in the correct subfolders by the code generator it is best practice to copy the carla_msgs folder
    in parallel to the other folders of your ROS2 system first and execute the generator from the respective ROS2 folder e.g.
     ```
     sudo cp -r carla_msgs /opt/ros/<distname>/share
     cd /opt/ros/<distname>/share
     Fast-DDS-GEN/scripts/fastddsgen -d <home>/output-code -I /opt/ros/<distname>/share/ -typeros2 ackermann_msgs/msg/AckermannDrive.idl nav_msgs/msg/Odometry.idl std_msgs/msg/Float32.idl sensor_msgs/msg/NavSatFix.idl sensor_msgs/msg/Imu.idl sensor_msgs/msg/PointCloud2.idl sensor_msgs/msg/Image.idl sensor_msgs/msg/CameraInfo.idl  tf2_msgs/msg/TFMessage.idl derived_object_msgs/msg/ObjectWithCovarianceArray.idl rosgraph_msgs/msg/Clock.idl derived_object_msgs/msg/ObjectArray.idl rosgraph_msgs/msg/Clock.idl carla_msgs/msg/*idl carla_msgs/srv/*idl
     ```
  * In some cases you will have to rename variables because of name clashes within different sub-namespaces which the fastddsgen generator is not able to   
    distiguish. Easiest workaround for variables is adding suffixes e.g. "BLABLA" to the class and/or variable names and later perform a search and replace.
    You might want to run the post_process_generated_file.sh within the generated code directory to take care on some issues observed with FastDDS generator 4.0.5 (including the BLABLA change).
    Finally, you might face some linker issues, because the clashes within the idl files transfer partially into the C++ files, where you might have to move some functions into proper namespaces(i.e register_double__36_type_identifier and register_double__9_type_identifier).
    Alternatively use the ros idl creation toolchain or wait until the generator is fixed and works properly.
  * When switching to a new version of FastCdr, one has to keep in mind, that the generated files of the image-type have been adapted to allow for other allocators (i.e. carla::sensor::data::SerializerVectorAllocator) for the vector type to support copyless passing the memory rendered within Unreal to the deserialization function. That also required to extend the fastcdr header files located in the fastcdr subdirectory. But that implementation effort seemed worth for the sake of speed on image transfer.
  



