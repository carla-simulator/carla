---
title: TraCI/Interfacing TraCI from Python
permalink: /TraCI/Interfacing_TraCI_from_Python/
---

The [TraCI](../TraCI.md#traci_commands) commands are split into the
13 domains gui, lane, poi, simulation, trafficlight, vehicletype, edge,
inductionloop, junction, multientryexit, polygon, route, person and
vehicle, which correspond to individual modules. For a detailed list of
available functions see the [pydoc generated documentation](https://sumo.dlr.de/pydoc/traci.html). The source code
can be found at
[\[1\]](https://github.com/eclipse/sumo/tree/master/tools/traci)

## importing **traci** in a script

To use the library, the {{SUMO}}/tools directory must be on the python load
path. This is typically done with a stanza like this:

```
 import os, sys
 if 'SUMO_HOME' in os.environ:
     tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
     sys.path.append(tools)
 else:
     sys.exit("please declare environment variable 'SUMO_HOME'")
```

This assumes that the [environment variable
**SUMO_HOME**](../Basics/Basic_Computer_Skills.md#additional_environment_variables)
is set before running the script. Alternatively, you can declare the
path to *sumo/tools* directly as in the line

```
sys.path.append(os.path.join('c:', os.sep, 'whatever', 'path', 'to', 'sumo', 'tools'))
```

## First Steps

In general it is very easy to interface with SUMO from Python (the
following example is a modification of
[tutorial/traci_tls](../Tutorials/TraCI4Traffic_Lights.md)):

First you compose the command line to start either
[sumo](../sumo.md) or [sumo-gui](../sumo-gui.md) (leaving out
the option  which was needed before 0.28.0):

```
sumoBinary = "/path/to/sumo-gui"
sumoCmd = [sumoBinary, "-c", "yourConfiguration.sumocfg"]
```

Then you start the simulation and connect to it with your script:

```
import traci
traci.start(sumoCmd)
step = 0
while step < 1000:
   traci.simulationStep()
   if traci.inductionloop.getLastStepVehicleNumber("0") > 0:
       traci.trafficlight.setRedYellowGreenState("0", "GrGr")
   step += 1

traci.close()
```

After connecting to the simulation, you can emit various commands and
execute simulation steps until you want to finish by closing the
connection. by default the close command will wait until the sumo
process really finishes. You can disable this by calling

```
traci.close(False)
```

## Subscriptions

Subscriptions can be thought of as a batch mode for retrieving
variables. Instead of asking for the same variables over and over again,
you can retrieve the values of interest automatically after each time
step. TraCI subscriptions are handled on a per module basis. That is you
can ask the module for the result of all current subscriptions after
each time step. In order to subscribe for variables you need to know
their variable ids which can be looked up in the traci/constants.py
file.

```
import traci
import traci.constants as tc

traci.start(["sumo", "-c", "my.sumocfg"])
traci.vehicle.subscribe(vehID, (tc.VAR_ROAD_ID, tc.VAR_LANEPOSITION))
print(traci.vehicle.getSubscriptionResults(vehID))
for step in range(3):
   print("step", step)
   traci.simulationStep()
   print(traci.vehicle.getSubscriptionResults(vehID))
traci.close()
```

The values retrieved are always the ones from the last time step, it is
not possible to retrieve older values.

## Context Subscriptions

Context subscriptions work like subscriptions in that they retrieve a
list of variables automatically for every simulation stop. However, the
do so by setting a reference object and a range and then retrieving
variables for all objects of a given type within range of the reference
object.

TraCI context subscriptions are handled on a per module basis. That is
you can ask the module for the result of all current subscriptions after
each time step. In order to subscribe for variables you need to the
domain id of the objects that shall be retrieved and the variable ids
which can be looked up in the traci/constants.py file. The domain id
always has the form CMD_GET_<DOMAIN\>_VARIABLE. The following code
retrieves all vehicle speeds and waiting times within range (42m) of a
junction (the vehicle ids are retrieved implicitly).

```
import traci
import traci.constants as tc

traci.start(["sumo", "-c", "my.sumocfg"])
traci.junction.subscribeContext(junctionID, tc.CMD_GET_VEHICLE_VARIABLE, 42, [tc.VAR_SPEED, tc.VAR_WAITING_TIME])
print(traci.junction.getContextSubscriptionResults(junctionID))
for step in range(3):
   print("step", step)
   traci.simulationStep()
   print(traci.junction.getContextSubscriptionResults(junctionID))
traci.close()
```

The values retrieved are always the ones from the last time step, it is
not possible to retrieve older values.

## Context Subscription Filters

For vehicle-to-vehicle context subscriptions (i.e., context
subscriptions, whose reference object is a vehicle and whose requested
context objects are vehicles as well) it is possible to request
additional filters to be applied already on the server side. The general
procedure is to equip a requested context subscription with the filter
directly after the call to `subscribeContext()` by a successive call to
`addSubscriptionFilter<FILTER_ID>()` as for instance in the following
snippet:

```
traci.vehicle.subscribeContext("ego", tc.CMD_GET_VEHICLE_VARIABLE, 0.0, [tc.VAR_SPEED])
traci.vehicle.addSubscriptionFilterLanes(lanes, noOpposite=True, downstreamDist=100, upstreamDist=50)
```

The first line requests a context subscription for the speed of vehicles
in the neighborhood of the reference vehicle with the ID `"ego"`. The
range of the context subscription (which refers to the radial context
region of the usual subscription mechanism) can be set equal to `0.0`,
since it is be overridden by the selective values of `downstreamDist`
and `upstreamDist`, respectively, given to the call of
`addSubscriptionFilterLanes()` in the second line. The call to
`addSubscriptionFilter<FILTER_NAME>()` automatically takes effect on
the last issued context subscription, which has to be of the
vehicle-to-vehicle form for a successful application.

The following filter types are available:

- Lanes: Return surrounding vehicles on lanes specified relatively to
  the reference vehicle
- CFManeuver: Return leader and follower on the reference vehicle's
  lane
- LCManeuver: Return leader and follower on the reference vehicle's
  lane and neighboring lane(s)
- Turn: Return conflicting vehicles on upcoming junctions along the
  vehicle's route
- VType: Only return vehicles of the specified vTypes
- VClass: Only return vehicles of the specified vClasses

See the [pydoc
documentation](http://sumo.dlr.de/daily/pydoc/traci._vehicle.html#VehicleDomain-addSubscriptionFilterCFManeuver)
for detailed specifications.

## Adding a StepListener

Often a function needs to be called each time when
traci.simulationStep() is called, to let this happen automatically
(always *after* each call to simulationStep()) it is possible to add a
StepListener object 'listener' (more precisely an instance of a subclass
of traci.StepListener) i.e.

```
 class ExampleListener(traci.StepListener):
    def step(self, t):
        # do something after every call to simulationStep
        print("ExampleListener called with parameter %s." % t)
        # indicate that the step listener should stay active in the next step
        return True
       
 listener = ExampleListener()
 traci.addStepListener(listener)
```
Please note that the listener is not activated for every simulation step
but for every call to simulationStep (which may perform multiple steps up to the given time *t*).
Furthermore the parameter *t* is not the current simulation time but exactly
the (optional) parameter passed to the simulationStep call (which is 0 by default).

!!! caution
    A TraCI StepListener cannot be used in the case that one TraCI client controls several SUMO-instances.

## Controlling parallel simulations from the same TraCI script

The TraCI python library can be used to control multiple simulations at
the same time with a single script. The function *traci.start()* has an
optional label argument which allows you to call it multiple times with
different simulation instances and labels. The function *traci.switch()*
can then be used to switch to any of the initialized labels:

```
 traci.start(["sumo", "-c", "sim1.sumocfg"], label="sim1")
 traci.start(["sumo", "-c", "sim2.sumocfg"], label="sim2")
 traci.switch("sim1")
 traci.simulationStep() # run 1 step for sim1
 traci.switch("sim2")
 traci.simulationStep() # run 1 step for sim2
```

If you prefer a more object oriented approach you can also use
connection objects to communicate with the simulation. They have the
same interface as the static *traci.* calls but you will still need to
start the simulation manually for them:

```
 traci.start(["sumo", "-c", "sim1.sumocfg"], label="sim1")
 traci.start(["sumo", "-c", "sim2.sumocfg"], label="sim2")
 conn1 = traci.getConnection("sim1")
 conn2 = traci.getConnection("sim2")
 conn1.simulationStep() # run 1 step for sim1
 conn2.simulationStep() # run 1 step for sim2
```

## Controlling the same simulation from multiple clients

To connect with multiple clients, the number of clients must be known in
advance and specified with sumo option **--num-clients** {{DT_INT}}. Also, the connection port must
be known to all clients. After deciding on a port it can be made
available to the clients via arguments or configuration files. A free
port can be obtained by

```
from sumolib.miscutils import getFreeSocketPort
port = sumolib.miscutils.getFreeSocketPort()
```

One client may use method *traci.start()* to start the simulation and
connect to it at the same time while the other client only needs to
connect. After establishing client order, each client must continuously
call *simulationStep* to allow the simulation to advance:

```
#client1
# PORT = int(sys.argv[1]) # example
traci.start(["sumo", "-c", "sim.sumocfg", "--num-clients", "2"], port=PORT)
traci.setOrder(1) # number can be anything
while traci.simulation.getMinExpectedNumber() > 0:
   traci.simulationStep()
   # more traci commands
traci.close(
```

```
# client2
# PORT = int(sys.argv[1]) # example
traci.init(PORT)
traci.setOrder(2) # number can be anything as long as each client gets its own number
while traci.simulation.getMinExpectedNumber() > 0:
   traci.simulationStep()
   # more traci commands
traci.close()
```

## Embedded Python

This feature has been removed Please use [libsumo](../Libsumo.md) instead.

## Additional Functions

When using TraCI there are some common tasks which are not covered by
the traci library such as

- Analyzing the road network
- Parsing simulation outputs

For this functionality it is recommended to use
[Tools/Sumolib](../Tools/Sumolib.md)

## Pitfalls and Solutions

- Note that strings, if exchanged, have to be ASCII-encoded.
- If you start sumo from within your python script using
  subprocess.Popen, be sure to call wait() on the resulting process
  object before quitting your script. You might loose output
  otherwise.
  
### Determine the traci library being loaded
When working with different sumo versions it may happen that the call `import traci` loads the wrong library.
The easiest way to debug this is to add the following lines after the import
```
import traci
print("LOADPATH:", '\n'.join(sys.path))                                                                                                                                      
print("TRACIPATH:", traci.__file__) 
sys.exit()
```
Make sure that the TRACIPATH corresponds to the sumo version that you wish to use. 
If it does not, then the order of directories in LOADPATH (sys.path) must be changed or the SUMO installation must be removed from any directories that come before the wanted directory.

### Debugging a TraCI session on Linux

Sometimes SUMO may crash while running a simulation with TraCI. The
below steps make it simple to run sumo with traci in a debugger:

1\) Add the option *--save-configuration* to your traci script:

```
traci.start([sumoBinary, '-c', 'run.sumocfg', '--save-configuration', 'debug.sumocfg'])
```

2\) Run your traci script. Instead of starting sumo it will just write
the configuration with the chosen port but it will still try to connect
repeatedly.

3\) Run

```
gdb --args sumoD -c debug.sumocfg
```

(where sumoD is sumo [compiled in debug
mode](../Installing/Linux_Build.md#building_the_sumo_binaries_with_cmake_recommended))

## Usage Examples

### Run a simulation until all vehicles have arrived

```
  while traci.simulation.getMinExpectedNumber() > 0:
      traci.simulationStep()
```

### Add trips (incomplete routes) dynamically

Define a route that consists of the start and destination edge:

```
   traci.route.add("trip", ["startEdge", "endEdge"])
```

Then add the vehicle with that route

```
   traci.vehicle.add("newVeh", "trip", typeID="reroutingType")
```

This will cause the vehicle to compute a new route from startEdge to
endEdge according to the estimated travel times in the network at the
time of departure. For details of this mechanism see
[Demand/Automatic_Routing](../Demand/Automatic_Routing.md).

### coordinate transformations

```
 x, y = traci.vehicle.getPosition(vehID)
 lon, lat = traci.simulation.convertGeo(x, y)
 x2, y2 = traci.simulation.convertGeo(lon, lat, fromGeo=True)
```

```
 edgeID, lanePosition, laneIndex = traci.simulation.convertRoad(x3, y3)
 edgeID, lanePosition, laneIndex = traci.simulation.convertRoad(lon2, lat2, True)
```

### Retrieve the timeLoss for all vehicles currently in the network

import traci import traci.constants as tc junctionID = '...'

1.  subscribe around an aribtrary junction with a sufficiently large
    radius to retrieve the speeds of all vehicles in every step

traci.junction.subscribeContext(junctionID,
tc.CMD_GET_VEHICLE_VARIABLE, 1000000, \[tc.VAR_SPEED,
tc.VAR_ALLOWED_SPEED\]) stepLength = traci.simulation.getDeltaT()
while traci.simulation.getMinExpectedNumber() \> 0:

```
   traci.simulationStep()
   scResults = traci.junction.getContextSubscriptionResults(junctionID)
   halting = 0
   if scResults:
       relSpeeds = [d[tc.VAR_SPEED] / d[tc.VAR_ALLOWED_SPEED] for d in scResults.values()]
       # compute values corresponding to summary-output
       running = len(relSpeeds)
       halting = len([1 for d in scResults.values() if d[tc.VAR_SPEED] < 0.1])
       meanSpeedRelative = sum(relSpeeds) / running
       timeLoss = (1 - meanSpeedRelative) * running * stepLength
   print(traci.simulation.getTime(), timeLoss, halting)
```

traci.close()

## Further Resources

- The module [Simpla](../Simpla.md) provides a library for
  platooning functions that can be integrated with user client
  scripts.
