# ROS bridge installation

The ROS bridge enables two-way communication between ROS and CARLA. The information from the CARLA server is translated to ROS topics. In the same way, the messages sent between nodes in ROS get translated to commands to be applied in CARLA.

*   [__Requirements__](#requirements)  
	*   [Python version](#python-version)  
*   [__Bridge installation__](#bridge-installation)  
	*   [A. Using Debian repository](#a-using-debian-repository)  
	*   [B. Using source repository](#b-using-source-repository)  
*   [__Run the ROS bridge__](#run-the-ros-bridge)  
*   [__Setting CARLA__](#setting-carla)  

!!! Important
    ROS is still [experimental](http://wiki.ros.org/noetic/Installation) for Windows, so the ROS bridge has only been tested for Linux systems.  

---
## Requirements

Make sure that both requirements work properly before continuing with the installation.  

*  __ROS Kinetic/Melodic__ — Install the ROS version corresponding to your system. Additional ROS packages may be required, depending on the user needs. [rviz](http://wiki.ros.org/rviz) is highly recommended to visualize ROS data.  
	*   [__ROS Kinetic__](http://wiki.ros.org/kinetic/Installation) — For Ubuntu 16.04 (Xenial).  
	*   [__ROS Melodic__](http://wiki.ros.org/melodic/Installation/Ubuntu) — For Ubuntu 18.04 (Bionic).  
	*   [__ROS Noetic__](http://wiki.ros.org/noetic#Installation) — For Ubuntu 20.04 (Focal).
*  __CARLA 0.9.7 or later__ — Previous versions are not compatible with the ROS bridge. Follow the [quick start installation](start_quickstart.md) or make the build for [Linux](build_linux.md).

### Python version

The Python version needed to run the ROS bridge depends on the ROS version being used.  

*   __ROS Kinetic__ and __ROS Melodic__ — Python2.  
*   __ROS Noetic__ — Python3.  

CARLA provides different Python support depending on the installation method. Here is a summary. 

*   __CARLA release packages__ — Provide support for Python2 and Python3, so these can be used with any ROS version.  
*   __Windows build__ — Provides Support for the default Python installation in the system, so the ROS installation should match this.  
*   __Linux build__ — Provides support for Python3 by default (ROS Noetic). If Python2 is needed, the PythonAPI can be built for Python2 running the following command in the CARLA root directory. 
```sh
make PythonAPI ARGS="--python-version=2" # The numeric argument can be changed to build for any specific Python version
```

---
## Bridge installation 

!!! Important
    To install ROS bridge versions prior to 0.9.10, change to a previous version of the documentation using the pannel in the bottom right corner of the window, and follow the old instructions. ![docs_version_panel](img/docs_version_panel.jpg)

### A. Using Debian repository

Set up the Debian repository in the system.
```sh
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1AF1527DE64CB8D9
sudo add-apt-repository "deb [arch=amd64] http://dist.carla.org/carla $(lsb_release -sc) main"
```
Install the ROS bridge, and check for the installation in the `/opt/` folder.
```sh
sudo apt-get update # Update the Debian package index
sudo apt-get install carla-ros-bridge # Install the latest ROS bridge version, or update the current installation
```

This repository contains features from CARLA 0.9.10 and later versions. To install a specific version add the version tag to the installation command.  
```sh
sudo apt-get install carla-ros-bridge=0.9.10-1 # In this case, "0.9.10" refers to the ROS bridge version, and "1" to the Debian revision  
```

### B. Using source repository

A catkin workspace is needed to use the ROS bridge. It should be cloned and built in there. The following code creates a new workspace, and clones the repository in there.  

```sh
# Setup folder structure
mkdir -p ~/carla-ros-bridge/catkin_ws/src
cd ~/carla-ros-bridge
git clone https://github.com/carla-simulator/ros-bridge.git
cd ros-bridge
git submodule update --init
cd ../catkin_ws/src
ln -s ../../ros-bridge
source /opt/ros/kinetic/setup.bash # Watch out, this sets ROS Kinetic 
cd ..

# Install required ros-dependencies
rosdep update
rosdep install --from-paths src --ignore-src -r

# Build
catkin_make
```

---
## Run the ROS bridge

__1) Run CARLA.__ The way to do so depends on the CARLA installation.

*  __Quick start/release package.__ `./CarlaUE4.sh` in `carla/`. 
*  __Debian installation.__ `./CarlaUE4.sh` in `opt/carla-simulator/`. 
*  __Build installation.__ `make launch` in `carla/`. 

__2) Add the source path.__ The source path for the workspace has to be added, so that the ROS bridge can be used from a terminal.  

*  __Source for apt ROS bridge.__
```sh
source /opt/carla-ros-bridge/<kinetic or melodic>/setup.bash
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
<br><br>
<i><small><b>Note: </b>.egg files may be either in `/PythonAPI/` or `/PythonAPI/dist/` depending on the CARLA installation.</small></i>  

```sh
    export PYTHONPATH=$PYTHONPATH:<path/to/carla/>/PythonAPI/<your_egg_file>
```

Import CARLA from Python and wait for a sucess message to check the installation.
```sh
python3 -c 'import carla;print("Success")'
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
*  __Simulation time-step.__ Simulation time (delta seconds) between simulation steps. __It must be lower than 0.1__. Take a look at the [documentation](adv_synchrony_timestep.md) to learn more about this.  
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
