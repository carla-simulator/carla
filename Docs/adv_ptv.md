# PTV-Vissim co-simulation

CARLA has developed a co-simulation feature with PTV-Vissim. This allows to distribute the tasks at will, and exploit the capabilities of each simulation in favour of the user.  

*   [__Requisites__](#requisites)  
*   [__Run a co-simulation__](#run-the-co-simulation)  
	*   [Create a new environment](#create-a-new-environment)
	
---
## Requisites

In order to run the co-simulation, two things are necessary.  

*   Buy a license for [__PTV-Vissim simulator__](https://www.ptvgroup.com/en/solutions/products/ptv-vissim/). It is necessary to acquire the Driving Simulator Interface add-on.  
*   In the PTV-Vissim installation folder, look for a `DrivingSimulatorProxy.dll`. Move it to `C:\Windows\System32`. 
Once that is done, the PTV-Vissim environment has to be set to run the co-simulation. 

---
## Run a co-simulation

Everything related with this feature can be found in `Co-Simulation/PTV-Vissim`. CARLA provides some examples that contain environments for __Town01__, and __Town03__.  

To run a co-simulation, use the script `PTV-Vissim/run_synchronization.py`. This has one mandatory argument containing the PTV-Vissim environment, and some other optional arguments.  

*   `vissim_network`— The vissim network file. This can be an example or a self-created PTV-Vissim environment.  
*   `--carla-host` *(default: 127.0.0.1)* — IP of the carla host server.  
*   `--carla-port` *(default: 2000)*  TCP port to listen to.  
*   `--vissim-version` *(default: 2020)* — PTV-Vissim version.  
*   `--step-length` *(default: 0.05s)* — set fixed delta seconds for the simulation time-step.  
*   `--simulator-vehicles` *(default: 1)* — number of vehicles that will be spawned in CARLA and passed to PTV-Vissim.  

```sh
python3 run_synchronization.py examples/Town03/Town03.inpx
```

Both simulations will run in synchrony. The actions or events happening in one simulator will propagate to the other. So far, the feature only includes vehicle movement, and spawning. The spawning is limited due to PTV-Vissim types.  
*   If a vehicle is spawned in CARLA, and in PTV-Vissim the setting is `Vehicle type: car`, the vehicle spawned will be a car. No matter if it as a motorbike in CARLA. In the examples provided, the vehicle type is set to `car`.  
*   If a vehicle is spawned in PTV-Vissim, CARLA will use a vehicle of the same type. The dimensions and characteristics will be similar, but not exactly the same.  

### Create a new environment 

In order for a new PTV-Vissim environment to run with CARLA, there are a few settings to be done.  

* __Activate the driving simulator__. Go to `Base Data/Network settings/Driving simulator` and enable the option.  
* __Specify the vehicle and pedestrian types__. These are the types that will be used in PTV-Vissim to sync with the spawnings done in CARLA. By default are empty.  
* __Export the environment as `.inpx`__. Create the environment, export it, and run the co-simulation with `run_synchronization.py`.  

![ptv_types](img/ptv_types.png)
<div style="text-align: right"><i>Any vehicle that is spawned in CARLA, will be spawned in PTV-Vissim using these types.</i></div>

!!! Warning
    The environment will crash if the pedestrian and vehicle types are left empty. 

---

That is all there is so far, regarding for the PTV-Vissim co-simulation with CARLA. 

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
