# SUMO co-simulation

CARLA has developed a co-simulation feature with SUMO. This allows to distribute the tasks at will, and exploit the capabilities of each simulation in favour of the user.  

*   [__Requisites__](#requisites)  
	*   [Prepare a SUMO environment](#prepare-a-sumo-environment)
*   [__Run the co-simulation__](#run-the-co-simulation)  
	*   [Spawn vehicles](#spawn-vehicles)
*   [__Spawn NPCs with SUMO__](#spawn-npcs-with-sumo)  

---
## Requisites

First and foremost, it is necessary to [__install SUMO__](https://sumo.dlr.de/docs/Installing.html) to run the co-simulation. 

Once that is done, the SUMO environment has to be set to run the co-simulation. 

### Prepare a SUMO environment 

The script `util/netconvert_carla.py` generates a SUMO network from an OpenDRIVE file. This script runs the *netconvert* tool to generate the main road network without traffic lights. These are then created one by one, using the OpenDRIVE definition. That way, inaccuracies are avoided.  

```sh
python netconvert_carla.py <PATH_TO_XODR> --output Test.net.xml
```

Once the net file has been generated, this can be opened in SUMO. Use it to prepare the SUMO environment for the co-simulation.  
> What to know. Tips.  
> Can this be done without that?  
> How to store the environment.  

---
## Run the co-simulation

Everything related with this feature can be found in `Co-Simulation/Sumo`. Several examples are provided for some CARLA maps, specifically __Town01__, __Town04__, and __Town05__. These contain some basic SUMO environments that will ease the usage of the feature.  

Use one of this examples, or any other SUMO file, to start the co-simulation. There are some optional arguments that can be set.  

*   __`--tls-manager <string>`__ chooses which simulator will change the state of the traffic lights. The other will update them accordingly.  
	*   `carla` to put CARLA in charge of the traffic lights.  
	*   `sumo` to put SUMO in charge of the traffic lights. 
	*   `none` to disable the synchronization of traffic lights. Vehicles do not take them into consideration --> None of them, or only one? 

*   __`--sumo-gui`__ will create a window to visualize the SUMO simulation. By default SUMO runs off-screen. 

```sh
python run_synchronization.py examples/TestTLS.sumocfg --tls-manager carla --sumo-gui
```

Both simulations will run in synchrony. The actions or events happening in one simulator will propagate to the other, e.g. positions, spawning, vehicle lights, etc.

Traffic lights As long as they are defined in the OpenDRIVE road map, they will be generated in SUMO. Now the synchronization script has some additional arguments.
> Landmarks? 
> Pedestrians? 

!!! Important
    SUMO Traffic lights will not be generated in the released CARLA maps. They were added manually and cannot be retrieved from the OpenDRIVE.


### Spawn vehicles

There are some helper scripts that translate CARLA blueprints to SUMO vehicle types so that the vehicles’ specifications are the same in both simulators. If these are not used, CARLA will spawn a random vehicle based on the given SUMO vehicle type.


---
## SUMO Vs Traffic Manager

The script `spawn_npc_sumo.py` is almost equivalent to the already-known `spawn_npc.py`. The difference is that vehicles will be managed using SUMO instead of the Traffic Manager. This script automatically generates a SUMO network in a temporal folder, based on the active town in CARLA. The script will create random routes and let the vehicles roam around.

As the script runs a synchronous simulation, and spawns vehicles in it, the arguments are the same that appear in run_synchronization.py and spawn_npc.py.

```sh
# Spawn 10 vehicles, that will be managed by SUMO instead of Traffic Manager.
# CARLA in charge of traffic lights.
# Open a window for SUMO visualization.
python spawn_sumo_npc.py -n 10 --tls-manager carla --sumo-gui
```

---

That is all there is so far, regarding for the SUMO co-simulation with CARLA. 

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
