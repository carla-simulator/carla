# ROS bridge installation

*  [__Requirements__](#requirements)  
*  [__Bridge installation__](#bridge-installation)  
	* a) using apt repository  
	* b) using source repository  
*  [__Run the ROS bridge__](#run-the-ros-bridge)  
*  [__Setting CARLA__](#setting-carla)  
 
The ROS bridge enables two-way communication between ROS and CARLA. The information from the CARLA server is translated to ROS topics. In the same way, the messages sent between nodes in ROS get translated to commands to be applied in CARLA.

---
## Requirements
### ROS Kinetic/Melodic

*  __ROS Kinetic/Melodic.__ Install ROS [Melodic](http://wiki.ros.org/melodic/Installation/Ubuntu), for Ubuntu 18.04, or [Kinetic](http://wiki.ros.org/kinetic/Installation), for Ubuntu 16.04. ROS packages may be required, depending on the user needs. [rviz](http://wiki.ros.org/rviz) to visualize ROS data.  
*  __CARLA 0.9.7 or later.__ Previous versions are not compatible with the ROS bridge. Follow the [quick start installation](../getting_started/quickstart) or make the build for the corresponding platform. 

!!! Important
    Make sure that both CARLA and ROS work properly before continuing with the installation. 

---
## Bridge installation 

### a) Using apt repository

Add the apt repository.

*  __Bridge for ROS Melodic.__
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 81061A1A042F527D &&
sudo add-apt-repository "deb [arch=amd64 trusted=yes] http://dist.carla.org/carla-ros-bridge-melodic/ bionic main"
```

*  __Bridge for ROS Kinetic.__
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 9BE2A0CDC0161D6C &&
sudo add-apt-repository "deb [arch=amd64 trusted=yes] http://dist.carla.org/carla-ros-bridge-kinetic xenial main"
```

Install the ROS bridge.
```sh
sudo apt update &&
sudo apt install carla-ros-bridge-<melodic or kinetic>
```

### b) Using source repository

A catkin workspace is needed to use the ROS bridge. It should be cloned and built in there. The following code creates a new workspace, and clones the repository in there.  

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

__1) Run CARLA.__ The way to do so depends on the CARLA installation.

*  __Quick start/release package.__ `./CarlaUE4.sh` in `carla/`. 
*  __apt installation.__ `./CarlaUE4.sh` in `opt/carla/bin/`. 
*  __Build installation.__ `make launch` in `carla/`. 

__2) Add the source path.__ The source path for the workspace has to be added, so that the ROS bridge can be used from a terminal.  

*  __Source for apt ROS bridge.__
```sh
source /opt/carla-ros-bridge/<melodic or kinetic>/setup.bash
```

*  __Source for ROS bridge repository download.__
```sh
source ~/carla-ros-bridge/catkin_ws/devel/setup.bash
```

!!! Important
    The source path can be set permanently, but it will cause conflict when working with another workspace.  

__3) Start the ROS bridge.__ Use any of the different launch files available to check the installation. Here are some suggestions.  

```sh
# Option 1: start the ros bridge
roslaunch carla_ros_bridge carla_ros_bridge.launch

# Option 2: start the ros bridge together with RVIZ
roslaunch carla_ros_bridge carla_ros_bridge_with_rviz.launch

# Option 3: start the ros bridge together with an example ego vehicle
roslaunch carla_ros_bridge carla_ros_bridge_with_example_ego_vehicle.launch
```

<details>
   <summary>
    <h6>ImportError: no module named CARLA</h6>
   </summary>

The path to CARLA Python is missing. The apt installation does this automatically, but it may be missing for other installations. Execute the following command with the complete path to the <i>.egg</i> file (included). Use the one supported by the Python version installed.
<br>
<i><small><b>Note: </b>.egg files may be either in `/PythonAPI/` or `/PythonAPI/dist/` depending on the CARLA installation.</small></i>  

```sh
    export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/<your_egg_file>
```

Import CARLA from Python and wait for a sucess message to check the installation.
```sh
python -c 'import carla;print("Success")'
```
</details>

---
## Setting CARLA

To modify the way CARLA works along with the ROS bridge, edit [`share/carla_ros_bridge/config/settings.yaml`](https://github.com/carla-simulator/ros-bridge/blob/master/carla_ros_bridge/config/settings.yaml).

*  __Host/port.__ Network settings to connect to CARLA using a Python client.  
*  __Synchronous mode.__ 
	*  __If false (default).__ Data is published on every `world.on_tick()` and every `sensor.listen()` callbacks.  
	*  __If true__ The bridge waits for all the sensor messages expected before the next tick. This might slow down the overall simulation but ensures reproducible results.  
*  __Wait for vehicle command.__ In synchronous mode, pauses the tick until a vehicle control is completed. 
*  __Simulation time-step.__ Simulation time (delta seconds) between simulation steps. __It must be lower than 0.1__. Take a look at the [documentation](../simulation_time_and_synchrony) to learn more about this.  
*  __Role names for the Ego vehicles.__ Role names to identify ego vehicles. These will be controllable from ROS and thus, relevant topics will be created.  

!!! Warning
    In synchronous mode only the ros-bridge is allowed to tick. Other clients must passively wait.

### Synchronous mode

To control the step update when in synchronous mode, use the following topic. The message contains a constant named `command` that allows to __Pause/Play__ the simulation, and execute a __single step__.  

<table class ="defTable">
<thead>
<th>Topic</th>
<th>Message type</th>
</thead>
<tbody>
<td><code>/carla/control</code> </td>
<td><a href="../ros_msgs#carlacontrolmsg">carla_msgs.CarlaControl</a></td>
</tbody>
</table>
<br>

The [Control rqt plugin](https://github.com/carla-simulator/ros-bridge/blob/master/rqt_carla_control/README.md) launches a new window with a simple interface. It is used to manage the steps and publish in the corresponding topic. Simply run the following when CARLA in synchronous mode.  
```sh
rqt --standalone rqt_carla_control
```
