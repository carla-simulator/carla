
# SUMO co-simulation

CARLA has developed a co-simulation feature with SUMO. This allows to distribute the tasks at will, and exploit the capabilities of each simulation in favour of the user.

*   [__Requisites__](#requisites)  
*   [__Run a custom co-simulation__](#run-a-custom-co-simulation)  
	*   [Create CARLA vtypes](#create-carla-vtypes)  
	*   [Create the SUMO net](#create-the-sumo-net)  
	*   [Run the synchronization](#run-the-synchronization)  
*   [__Spawn NPCs controlled by SUMO__](#spawn-npcs-controlled-by-sumo)  

---
## Requisites

First and foremost, it is necessary to [__install SUMO__](https://sumo.dlr.de/docs/Installing.html) to run the co-simulation. Building from source is recommended over a simple installation, as there are new features and fixes that will improve the co-simulation. 

Once that is done, set the SUMO environment variable.  
```sh
echo "export SUMO_HOME=/usr/share/sumo" >> ~/.bashrc && source ~/.bashrc
```

SUMO is ready to run the co-simulations. There are some examples in `Co-Simulation/Sumo/examples` for __Town01__, __Town04__, and __Town05__. These `.sumocfg` files describe the configuration of the simulation (e.g., net, routes, vehicle types...). Use one of these to test the co-simulation. The script has different options that are detailed [below](#run-the-synchronization). For the time being, let's run a simple example for __Town04__.  

Run a CARLA simulation with __Town04__.  
```sh
cd ~/carla
./CarlaUE4.sh
cd PythonAPI/util
python3 config.py --map Town04
```

Then, run the SUMO co-simulation example.  
```sh
cd ~/carla/Co-Simulation/Sumo
python3 run_synchronization.py examples/Town04.sumocfg  --sumo-gui
```
!!! Important
    

---
## Run a custom co-simulation

### Create carla vtypes

With the script `Co-Simulation/Sumo/util/create_sumo_vtypes.py` the user can create sumo *vtypes*, the equivalent to CARLA blueprints, based on the CARLA blueprint library.  

*   __`--carla-host`__ *(default: 127.0.0.1)* — IP of the carla host server.  
*   __`--carla-port`__ *(default: 2000)* — TCP port to listen to.  
*   __`--output-file`__ *(default: carlavtypes.rou.xml)* — The generated file containing the *vtypes*.  

This script uses the information stored in `data/vtypes.json` to create the SUMO *vtypes*. These can be modified by editing said file.  

!!! Warning
    A CARLA simulation must be running to execute the script.

### Create the SUMO net

The recommended way to create a SUMO net that synchronizes with CARLA is using the script `Co-Simulation/Sumo/util/netconvert_carla.py`. This will draw on the [netconvert](https://sumo.dlr.de/docs/NETCONVERT.html) tool provided by SUMO. In order to run the script, some arguments are needed.  

*   __`xodr_file`__ — OpenDRIVE file `.xodr`.
*   __`--output'`__ *(default:`net.net.xml`)* — output file `.net.xml`.
*   __`--guess-tls`__ *(default:false)* — SUMO can set traffic lights only for specific lanes in a road, but CARLA can't. If set to __True__, SUMO will not differenciate traffic lights for specific lanes, and these will be in sync with CARLA.  

The output of the script will be a `.net.xml` that can be edited using __[NETEDIT](https://sumo.dlr.de/docs/NETEDIT.html)__. Use it to edit the routes, add demand, and eventually, prepare a simulation that can be saved as `.sumocfg`.  

The examples provided may be helpful during this process. Take a look at `Co-Simulation/Sumo/examples`. For every `example.sumocfg` there are several related files under the same name. All of them comprise a co-simulation example.  

### Run the synchronization

Once a simulation is ready and saved as a `.sumocfg`, it is ready to run. There are some optional parameters to change the settings of the co-simulation. 

*   __`sumo_cfg_file`__ — The SUMO configuration file.  
*   __`--carla-host`__ *(default: 127.0.0.1)* — IP of the carla host server  
*   __`--carla-port`__ *(default: 2000)* — TCP port to listen to  
*   __`--sumo-host`__ *(default: 127.0.0.1)* — IP of the SUMO host server.  
*   __`--sumo-port`__ *(default: 8813)* — TCP port to listen to.  
*   __`--sumo-gui`__ — Open a window to visualize the gui version of SUMO.
*   __`--step-length`__ *(default: 0.05s)* — Set fixed delta seconds for the simulation time-step. 
*   __`--sync-vehicle-lights`__ *(default: False)* — Synchronize vehicle lights. 
*   __`--sync-vehicle-color`__ *(default: False)* — Synchronize vehicle color. 
*   __`--sync-vehicle-all`__ *(default: False)* — Synchronize all vehicle properties.  
*   __`--tls-manager`__ *(default: none)* — Choose which simulator should manage the traffic lights. The other will update those accordingly. The options are `carla`, `sumo`, and `none`. If `none` is chosen, traffic lights will not be synchronized. Each vehicle would only obey the traffic lights in the simulator that spawn it. 

```sh
python3 run_synchronization.py <SUMOCFG FILE> --tls-manager carla --sumo-gui
```

!!! Warning
    To stop the co-simulation, press `Ctrl+C` in the terminal that run the script.  

---
## Spawn NPCs controlled by SUMO

The co-simulation with SUMO makes for an additional feature. Vehicles can be spawned in CARLA through SUMO, and managed by the later as the Traffi Manager would do.  

The script `spawn_npc_sumo.py` is almost equivalent to the already-known `generate_traffic.py`. This script automatically generates a SUMO network in a temporal folder, based on the active town in CARLA. The script will create random routes and let the vehicles roam around.

As the script runs a synchronous simulation, and spawns vehicles in it, the arguments are the same that appear in `run_synchronization.py` and `generate_traffic.py`.

*   __`--host`__ *(default: 127.0.0.1)* — IP of the host server.  
*   __`--port`__ *(default: 2000)* — TCP port to listen to.  
*   __`-n,--number-of-vehicles`__ *(default: 10)* — Number of vehicles spawned.  
*   __`--safe`__ — Avoid spawning vehicles prone to accidents.  
*   __`--filterv`__ *(default: "vehicle.*")* — Filter the blueprint of the vehicles spawned.  
*   __`--sumo-gui`__ — Open a window to visualize SUMO.  
*   __`--step-length`__ *(default: 0.05s)* — Set fixed delta seconds for the simulation time-step.  
*   __`--sync-vehicle-lights`__ *(default: False)* — Synchronize vehicle lights state.  
*   __`--sync-vehicle-color`__ *(default: False)* — Synchronize vehicle color.  
*   __`--sync-vehicle-all`__ *(default: False)* — Synchronize all vehicle properties.  
*   __`--tls-manager`__ *(default: none)* — Choose which simulator will change the traffic lights' state. The other will update them accordingly. If `none`, traffic lights will not be synchronized.  

```sh
# Spawn 10 vehicles, that will be managed by SUMO instead of Traffic Manager.
# CARLA in charge of traffic lights.
# Open a window for SUMO visualization.
python3 spawn_sumo_npc.py -n 10 --tls-manager carla --sumo-gui
```

---

That is all there is so far, regarding for the SUMO co-simulation with CARLA. 

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
