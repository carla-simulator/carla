
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

WARNING: The sync is using traci and sumolib. SUMO_HOME variable must be set!!!!!!!!

## Run the co-simulation

### Run your own SUMO simulation

#### Create carla vtypes

With the script util/create_sumo_vtypes.py the user can create sumo vtypes based on carla blueprints.

Argument of the script:

--carla-host
--carla-port
--output-file --> the generated vtypes will be written to this (default: carlavtypes.rou.xml)

This script uses the information stored in data/vtypes.json to create the sumo vtype. If the user wants to add custom properties to the sumo vtype it should be added in this file.

WARNING: A carla server must be running to execute the script!

#### Create sumo net

The recommended way to create a sumo net to synchronize with carla is with the script util/netconvert_carla.py. Internally it uses the netconvert tool provided by SUMO.

Arguments of the script:

xodr_file --> opendrive file (*.xodr')
--output', -o --> output file (*.net.xml)
--guess-tls --> guess traffic lights at intersections (default: False)')

At this point, the user can create their own routes for the different vehicles. At the end they have to generate the .sumocfg file. Some examples of complete sumo simulations are found in the examples folder. There are examples for Town01, Town04, Town05

#### Run the synchronization

To run the gerenated simulation run the following script:

python run_synchronization.py <SUMOCFG FILE> --tls-manager carla --sumo-gui

Arguments:
sumo_cfg_file --> help='sumo configuration file
--carla-host--> help='IP of the carla host server (default: 127.0.0.1)
--carla-port --> TCP port to listen to (default: 2000)
--sumo-host --> IP of the sumo host server (default: 127.0.0.1)
--sumo-port --> TCP port to liston to (default: 8813)
--sumo-gui --> run the gui version of sumo
--step-length --> set fixed delta seconds (default: 0.05s)
--sync-vehicle-lights --> synchronize vehicle lights state (default: False)
--sync-vehicle-color --> synchronize vehicle color (default: False)
--sync-vehicle-all --> synchronize all vehicle properties (default: False)
--tls-manager --> choices=['none', 'sumo', 'carla'], help="select traffic light manager (default: none)
                           
### Run spawn_npc_sumo.py

The script `spawn_npc_sumo.py` is almost equivalent to the already-known `spawn_npc.py`. The difference is that vehicles will be managed using SUMO instead of the Traffic Manager. This script automatically generates a SUMO network in a temporal folder, based on the active town in CARLA. The script will create random routes and let the vehicles roam around.

As the script runs a synchronous simulation, and spawns vehicles in it, the arguments are the same that appear in run_synchronization.py and spawn_npc.py.

```sh
# Spawn 10 vehicles, that will be managed by SUMO instead of Traffic Manager.
# CARLA in charge of traffic lights.
# Open a window for SUMO visualization.
python spawn_sumo_npc.py -n 10 --tls-manager carla --sumo-gui
```

Arguments:

    --host --> IP of the host server (default: 127.0.0.1)
    --port --> TCP port to listen to (default: 2000)
    -n,--number-of-vehicles-->number of vehicles (default: 10)
    -w,--number-of-walkers, --> number of walkers (default: 0)
    --safe --> avoid spawning vehicles prone to accidents
    --filterv --> vehicles filter (default: "vehicle.*")
    --filterw --> pedestrians filter (default: "walker.pedestrian.*")
    --sumo-gui --> run the gui version of sumo
    --step-length --> set fixed delta seconds (default: 0.05s)
    --sync-vehicle-lights --> synchronize vehicle lights state (default: False)
    --sync-vehicle-color --> synchronize vehicle color (default: False)
    --sync-vehicle-all --> synchronize all vehicle properties (default: False)
    --tls-manager --> choices=['none', 'sumo', 'carla'], "select traffic light manager (default: none)"

---

That is all there is so far, regarding for the SUMO co-simulation with CARLA. 

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
