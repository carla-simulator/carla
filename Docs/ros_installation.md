# ROS bridge installation

  * [__Requirements__](#requirements)  
  * [__Bridge installation__](#bridge-installation)  
	* a) using apt repository  
	* b) using source repository  
  * [__Run the ROS bridge__](#run-the-ros-bridge)  
	* Solving ImportError: No module named CARLA  
  * [__Setting CARLA__](#setting-carla)  
 
The ROS bridge enables two-way communication between ROS and CARLA.  
In order to do so, the information from the CARLA server is translated to ROS topics. For example, the information retrieved by sensors is structured to fit ROS messages.  
In the same way, the messages sent between nodes in ROS get translated to commands to be applied in CARLA. This is commonly used to update the state of a vehicle and apply controllers.  

---
## Requirements
#### ROS melodic

  * __ROS Kinetic/Melodic:__ follow the official documentation to [install ROS](http://wiki.ros.org/melodic/Installation/Ubuntu). Some ROS packages may be required, depending on the user needs, such as [rviz](https://wiki.ros.org/ainstein_radar_rviz_plugins) to visualize ROS data.  
  * __CARLA 0.9.7:__ previous versions are not compatible with the ROS bridge. Follow the [quick start installation](../getting_started/quickstart) or make the build for the corresponding platform. 

!!! Important
    Make sure that both CARLA and ROS work properly before continuing with the installation. 

---
## Bridge installation 

#### a) Using apt repository

First add the apt repository: 

* __Bridge for ROS Melodic:__
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 81061A1A042F527D &&
sudo add-apt-repository "deb [trusted=yes] http://dist.carla.org/carla-ros-bridge-melodic/ bionic main"
```

* __Bridge for ROS Kinetic:__
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 9BE2A0CDC0161D6C &&
sudo add-apt-repository "deb [trusted=yes] http://dist.carla.org/carla-ros-bridge-kinetic xenial main"
```

Then simply install the ROS bridge:
```sh
sudo apt update &&
sudo apt install carla-ros-bridge-<melodic or kinetic>
```

#### b) Using source repository

In order to use the ROS bridge, a catkin workspace is needed. It should be cloned and built in there.  
The following fragment creates a new workspace and clones the repository in there.  
```sh
#setup folder structure
mkdir -p ~/carla-ros-bridge/catkin_ws/src
cd ~/carla-ros-bridge
git clone https://github.com/carla-simulator/ros-bridge.git
cd catkin_ws/src
ln -s ../../ros-bridge
source /opt/ros/kinetic/setup.bash #Watch out, this sets ROS Kinetic. 
cd ..

#install required ros-dependencies
rosdep update
rosdep install --from-paths src --ignore-src -r

#build
catkin_make
```

---
## Run the ROS bridge

__1) run CARLA:__ the way to do so will depend on the the CARLA installation chosen, so here is a brief summary:  

* __Quick start/release package:__ run `./CarlaUE4.sh` in `/carla/`. 
* __apt installation:__ run `./CarlaUE4.sh` in `/opt/carla/bin/`. 
* __Build installation:__ run `make launch` in `/carla/`. 

__2) Add the source path:__ the source path for the workspace should be added so that the ROS bridge can be used from a terminal:  

* __Source for apt ROS bridge:__
```sh
source /opt/carla-ros-bridge/<melodic or kinetic>/setup.bash
```

* __Source for ROS bridge repository download:__
```sh
source ~/carla-ros-bridge/catkin_ws/devel/setup.bash
```

!!! Important
    The source path can be added to the environment to be set permanently, but it will cause conflict when working with another workspace.  

__3) start the ROS bridge:__ use any of the different launch files available that will serve to check if the bridge is running properly. Here are some suggestions:  

```sh
# Option 1: start the ros bridge
roslaunch carla_ros_bridge carla_ros_bridge.launch

# Option 2: start the ros bridge together with RVIZ
roslaunch carla_ros_bridge carla_ros_bridge_with_rviz.launch

# Option 3: start the ros bridge together with an example ego vehicle
roslaunch carla_ros_bridge carla_ros_bridge_with_example_ego_vehicle.launch
```

#### Solving ImportError: no module named CARLA

The path to CARLA Python is missing. The apt installation does this automatically, but it may be missing for other installations. Execute the following command with the complete path to the _.egg_ file (included). Use the one that, is supported by the Python version installed:

    export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/<your_egg_file>

!!! Note
    _.egg_ files may be either in `/PythonAPI/` or `/PythonAPI/dist/` depending on the CARLA installation.

To check the installation, import CARLA from Python and wait for a sucess message:
```sh
python -c 'import carla;print("Success")'
```

---
## Setting CARLA

Settings can be changed, in order to modify the way CARLA works along with the ROS bridge, by editing the file: [`share/carla_ros_bridge/config/settings.yaml`](https://github.com/carla-simulator/ros-bridge/blob/master/carla_ros_bridge/config/settings.yaml).

The parameters available refer to:  

* __Host/port:__ the network settings to connect to CARLA using a Python client.  
* __Synchronous mode:__ 
	* __If false (default):__ data is published on every `world.on_tick()` and every `sensor.listen()` callbacks.  
	* __If true:__ the bridge waits for all the sensor messages expected before the next tick. This might slow down the overall simulation but ensures reproducible results.  
* __Wait for vehicle command:__ in synchronous mode, pauses the tick until a vehicle control is completed. 
* __Simulation time-step:__ simulation time (delta seconds) between simulation steps. __It must be lower than 0.1__. Take a look at the [documentation](../simulation_time_and_synchrony) to learn more about this.  
* __Role names for the Ego vehicles:__ role names to identify ego vehicles. These will be controllable from ROS and thus, relevant topics will be created.  


!!! Warning
    In synchronous mode only the ros-bridge is allowed to tick. Other CARLA clients must passively wait.

#### Synchronous mode

The following topic allows to control the step update when in synchronous mode:  

| Topic            | Message type            |
| ---------------- | ----------------------- |
| `/carla/control` | [carla_msgs.CarlaControl](../ros_msgs#carlacontrolmsg) |

The message contains a constant named `command` that allows to:  

* Pause/Play the simulation. 
* Execute a single step. 

The [Control rqt plugin](https://github.com/carla-simulator/ros-bridge/blob/master/rqt_carla_control/README.md) launches a new window with a simple interface to manage these steps and publish in the corresponding topic.  
Simply run the following when CARLA is in synchronous mode:  
```sh
rqt --standalone rqt_carla_control
```
