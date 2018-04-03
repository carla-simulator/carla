
# Ros bridge for Carla simulator

This ros package aims at providing a simple ros bridge for carla simulator.

![rviz setup](./assets/rviz_carla_default.png "rviz")
![depthcloud](./assets/depth_cloud_and_lidar.png "depthcloud")

![short video](https://youtu.be/S_NoN2GBtdY)


# Features/Roadmap/TODO

- [x] Cameras (depth, segmentation, rgb) support
- [x] Add camera matrix
- [x] Lidar sensor support
- [x] Transform publications
- [x] Manual control using ackermann msg
- [x] Autopilot mode using rosparam 
- [x] Rosbag in the bridge (in order to avoid rosbag recoard -a small time errors)
- [x] Handle ros dependencies
- [x] Marker/bounding box messages for cars/pedestrian
- [ ] Better looking color map for segmentation
- [ ] Support dynamic change (restarting simulation using a topic/rosparam)


# Setup

## Clone the carla ros bridge branch

    git clone -b dev/add_ros_bridge https://github.com/laurent-george/carla.git

## Install carla python client in your workspace

    cd carla/PythonClient
    pip install -e .  --user --upgrade   

    now in a python2.7 terminal you should be able to do

    import carla

## Create a catkin workspace and install carla_ros_bridge package

    source /opt/ros/kinetic/setup.bash
    mkdir -p ~/catkin_ws/src 
    ln -s ~/carla/carla_ros_bridge/ ~/catkin_ws/src/
    cd ~/catkin_ws
    catkin_make 
    rosdep install --from-paths ~/catkin_ws/ 
    catkin_make 
    
    
# Start the ros bridge

First run the simulator (see carla documentation)

    ./CarlaUE4.sh /Game/Maps/Town01 -carla-server -benchmark -fps=15 -windowed -ResX=320 -ResY=240

Then start the ros bridge:

    source ~/catkin_ws/devel/setup.bash
    roslaunch carla_ros_bridge client.launch
    
To start the ros bridge with rviz use:

    roslaunch carla_ros_client client_with_rviz.launch
    
You can setup the wanted camera/sensors in config/settings.yaml.

# Autopilot control

To enable autopilot control set the ros param carla_autopilot to True

    rosparam set carla_autopilot True
    
# Manual control 

To enable manual control set the ros param carla_autopilot to False

    rosparam set carla_autopilot False
    

Then you can send command to the car using the /ackermann_cmd topic.

Example of forward movements, speed in in meters/sec.

     rostopic pub /cmd_vel ackermann_msgs/AckermannDrive "{steering_angle: 0.0, steering_angle_velocity: 0.0, speed: 10, acceleration: 0.0,
      jerk: 0.0}" -r 10
  
  
Example of forward with steering
  
     rostopic pub /cmd_vel ackermann_msgs/AckermannDrive "{steering_angle: 5.41, steering_angle_velocity: 0.0, speed: 10, acceleration: 0.0,
      jerk: 0.0}" -r 10
      
  Warning: the steering_angle is the driving angle (in radians) not the wheel angle, for now max wheel is set to 500 degrees.
  
  
Example for backward :

     rostopic pub /cmd_vel ackermann_msgs/AckermannDrive "{steering_angle: 0, steering_angle_velocity: 0.0, speed: -10, acceleration: 0.0,
      jerk: 0.0}" -r 10
