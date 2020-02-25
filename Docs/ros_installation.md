<h1>ROS bridge installation</h1>

  * [__Requirements__](#requirements)  
  * [__Bridge installation__](#bridge-installation)  
	* a) apt-get ROS bridge  
	* b) Repository download  
	* Import error: No module named CARLA  
  * [__Run the ROS bridge__](#run-the-ros-bridge)  
  * [__Settings__](#settings)  
 
The ROS bridge enables communication between ROS and CARLA so that both softwares can reach full potential by combining their features.  

---------------
##Requirements
<h4>ROS melodic</h4>

  * __ROS Kinetic/Melodic:__ follow the official documentation to [install ROS](http://wiki.ros.org/melodic/Installation/Ubuntu). Some ROS packages could be necessary, depending on the user needs, such as [rviz](https://wiki.ros.org/ainstein_radar_rviz_plugins) to visualize ROS data.  
  * __CARLA:__ only __CARLA 0.9.7__ and later versions are supported. Follow the [quick start installation](../getting_started/quickstart) or make the build for the corresponding platform. 

!!! Important
    Make sure that both CARLA and ROS work properly before continuing with the installation. 

---------------
##Bridge installation 

<h4>a) apt-get ROS bridge</h4>

First add the apt repository: 

* __For ROS Melodic:__
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 81061A1A042F527D &&
sudo add-apt-repository "deb [trusted=yes] http://34.227.255.250/carla-ros-bridge-melodic/ bionic main"
```

* __For ROS Kinetic:__
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 9BE2A0CDC0161D6C
sudo add-apt-repository "deb [trusted=yes] http://dist.carla.org/carla-ros-bridge-kinetic xenial main"
```

Then simply install the ROS bridge:
```sh
sudo apt-get update &&
sudo apt-get install carla-ros-bridge
```

<h4>b) Repository download</h4>

In order to work use the ROS bridge, first create a catkin workspace, instal the ROS bridge in there and build it to be available: 
```sh
#setup folder structure
mkdir -p ~/carla-ros-bridge/catkin_ws/src
cd ~/carla-ros-bridge
git clone https://github.com/carla-simulator/ros-bridge.git
cd catkin_ws/src
ln -s ../../ros-bridge
source /opt/ros/kinetic/setup.bash
cd ..

#install required ros-dependencies
rosdep update
rosdep install --from-paths src --ignore-src -r

#build
catkin_make
```

<h4>ImportError: no module named CARLA</h4>

The path to CARLA Python is missing. Please execute the following command with the complete path to the _.egg_ file (included). Use the one, that is supported by the Python version installed:

    export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/<your_egg_file>

!!! Note
    _.egg_ files may be either in `/PythonAPI/` or `/PythonAPI/dist/` depending on the CARLA installation.

To check the installation, import CARLA from Python and wait for a sucess message:
```sh
python -c 'import carla;print("Success")'
```


---------------
##Run the ROS bridge

__1) run CARLA:__ the way to do so will depend on the the CARLA installation chosen, so here is a brief summary:  

* __Quick start/release package:__ run `./CarlaUE4.sh` in `/carla/`. 
* __apt-get installation:__ run `./CarlaUE4.sh` in `/opt/carla/bin/`. 
* __Build installation:__ run `make launch` in `/carla/`. 

__2) __ The way to do so will depend on the the CARLA installation chosen, so here is a brief summary:  
```sh
export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/<your_egg_file>
source ~/carla-ros-bridge/catkin_ws/devel/setup.bash
```

__3) start the ROS bridge:__ use any of the different packages available that will serve to check if the bridge is running properly. Here are some suggestions:  

```sh
# Option 1: start the ros bridge
roslaunch carla_ros_bridge carla_ros_bridge.launch

# Option 2: start the ros bridge together with RVIZ
roslaunch carla_ros_bridge carla_ros_bridge_with_rviz.launch

# Option 3: start the ros bridge together with an example ego vehicle
roslaunch carla_ros_bridge carla_ros_bridge_with_example_ego_vehicle.launch
```

---------------
##Settings

There is some configuration for CARLA available from the ROS bridge. This can be setup by editing the file: [`carla_ros_bridge/config/settings.yaml`](https://github.com/carla-simulator/ros-bridge/blob/master/carla_ros_bridge/config/settings.yaml).

The parameters available refer to:  

* __Ego vehicle:__ the list in `/carla/ego_vehicle/rolename` sets role names to identify ego vehicles. These will be controllable from ROS and thus, relevant topics will be created.  
* __Host/port:__ the network settings to connect to CARLA using a Python client. 
* __Simulation time-step:__ simulation time (delta seconds) between simulation steps. __It must be lower than 0.1__. Take a look at the [documentation](../simulation_time_and_synchrony) to learn more about this. 
* __Synchronous mode:__ 
	* __If false (default):__ data is published on every `world.on_tick()` and every `sensor.listen()` callbacks.  
	* __If true:__ the bridge waits for all sensor data that is expected within the current frame. This might slow down the overall simulation but ensures reproducible results.  
* __Wait for vehicle command:__ in synchronous mode, this boolean determines if a vehicle command should be received before executing the next tick. 

!!! Warning
    In synchronous mode, only the ros-bridge is allowed to tick. Other CARLA clients must passively wait.

<h4>Synchronous mode</h4>

* Pause/Play  
* Execute single step

The following topic allows to control the stepping.  

| Topic            | Type                    |
| ---------------- | ----------------------- |
| `/carla/control` | carla_msgs.CarlaControl |

A CARLA [Control rqt plugin](https://github.com/carla-simulator/ros-bridge/blob/master/rqt_carla_control/README.md) is available to publish to the topic.
