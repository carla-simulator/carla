# Scenarios

Learning an efficie...

* [__Create a new scenario__](#create-a-new-scenario)  
	* [Create an empty Python class](#create-an-empty-python-class)  
	* [Fill the Python class](#fill-the-python-class)  
	* [Add the scenario configuration](#add-the-scenario-configuration)  
* [__List of scenarios__](#list-of-scenarios)  
	* [ControlLoss](#controlloss)  
	* [DynamicObjectCrossing](#dynamicobjectcrossing)  
	* [FollowLeadingVehicle](#followleadingvehicle)  
	* [FollowLeadingVehicleWithObstacle](#followleadingvehiclewithobstacle)  
	* [ManeuverOppositeDirection](#maneuveroppositedirection)  
	* [NoSignalJunctionCrossing](#nosignaljunctioncrossing)  
	* [OppositeVehicleRunningRedLight](#oppositevehiclerunningredlight)  
	* [OtherLeadingVehicle](#otherleadingvehicle)  
	* [SignalizedJunctionLeftTurn](#signalizedjunctionleftturn)  
	* [SignalizedJunctionRightTurn](#signalizedjunctionrightturn)  
	* [StationaryObjectCrossing](#stationaryobjectcrossing)  
	* [VehicleTurningRight](#vehicleturningright)  
	* [VehicleTurningLeft](#vehicleturningleft)  


---
## Create a new scenario

This tutorial describes how you can create and run a new scenario using the
ScenarioRunner and the ScenarioManager suite.

Let us call the new scenario _NewScenario_. To create it, there are only few
steps required.

## Creating an empty Python class
Go to the Scenarios folder and create a new Python class with the name
_NewScenario_ in a new Python file (_new_scenario.py_). The class should be
derived from the _BasicScenario_ class. As a result, the class should look as
follows:

   ```
   class NewScenario(BasicScenario):
       """
       Some documentation on NewScenario
       :param world is the CARLA world
       :param ego_vehicles is a list of ego vehicles for this scenario
       :param config is the scenario configuration (ScenarioConfiguration)
       :param randomize can be used to select parameters randomly (optional, default=False)
       :param debug_mode can be used to provide more comprehensive console output (optional, default=False)
       :param criteria_enable can be used to disable/enable scenario evaluation based on test criteria (optional, default=True)
       :param timeout is the overall scenario timeout (optional, default=60 seconds)
       """

       # some ego vehicle parameters
       # some parameters for the other vehicles

       def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                    timeout=60):
           """
           Initialize all parameters required for NewScenario
           """

           # Call constructor of BasicScenario
           super(NewScenario, self).__init__(
             name="NewScenario",
             ego_vehicles,
             config,
             world,
             debug_mode,
             criteria_enable=criteria_enable))


       def create_behavior(self):
           """
           Setup the behavior for NewScenario
           """

       def create_test_criteria(self):
           """
           Setup the evaluation criteria for NewScenario
           """
   ```

## Filling the Python class

In the NewScenario class, you have to define the following methods mentioned in the code example.

### Initialize Method
The initialize method is intended to setup all parameters required for the scenario and all vehicles. This includes selecting the correct vehicles, spawning them at the correct location, etc. To simplify this, you may want to use the _setup_vehicle()_ function defined in basic_scenario.py

### CreateBehavior method
This method should setup the behavior tree that contains the behavior of all non-ego vehicles during the scenario. The behavior tree should use py_trees and the atomic behaviors defined in _atomic_scenario_behavior.py_

### CreateTestCriteria method
This method should setup a list with all evaluation criteria for the scenario.
The criteria should be based on the atomic criteria defined in _atomic_scenario_criteria.py_.

Note: From this list a parallel py_tree will be created automatically!

## Adding the scenario configuration
Finally the scenario configuration should be added to the examples/ folder. If you extend an already existing scenario module, you can simply extend the corresponding
XML, otherwise add a new XML file. In this case you can use any of the existing XML files as blueprint.

If you want to add multiple ego vehicles for a scenario, make sure that they use different role names, e.g.

```
    <scenario name="MultiEgoTown03" type="FreeRide" town="Town03">
        <ego_vehicle x="207" y="59" z="0" yaw="180" model="vehicle.lincoln.mkz2017" rolename="hero"/>
        <ego_vehicle x="237" y="-95.0754252474" z="0" yaw="90" model="vehicle.tesla.model3" rolename="hero2"/>
    </scenario>
```


---
## List of scenarios


### FollowLeadingVehicle
The scenario realizes a common driving behavior, in which the user-controlled ego vehicle follows a leading car driving down a given road in Town01. At some point the leading car slows down and finally stops. The ego vehicle has to react accordingly to avoid a collision. The scenario ends either via a timeout, or if the ego vehicle stopped close enough to the leading vehicle.

### FollowLeadingVehicleWithObstacle
This scenario is very similar to 'FollowLeadingVehicle'. The only difference is, that in front of the leading vehicle is a (hidden) obstacle that blocks the way.

### VehicleTurningRight
In this scenario the ego vehicle takes a right turn from an intersection where a cyclist suddenly drives into the way of the ego vehicle,which has to stop accordingly. After some time, the cyclist clears the road, such that ego vehicle can continue driving.

### VehicleTurningLeft
This scenario is similar to 'VehicleTurningRight'. The difference is that the ego vehicle takes a left turn from an intersection.

### OppositeVehicleRunningRedLight
In this scenario an illegal behavior at an intersection is tested. An other vehicle waits at an intersection, but illegally runs a red traffic light. The approaching ego vehicle has to handle this situation correctly, i.e. despite of a green traffic light, it has to stop and wait until the intersection is clear again. Afterwards, it should continue driving.

### StationaryObjectCrossing
In this scenario a cyclist is stationary waiting in the middle of the road and blocking the way for the ego vehicle. Hence, the ego vehicle has to stop in front of the cyclist.

### DynamicObjectCrossing
This is similar to 'StationaryObjectCrossing', but with the difference that the cyclist is dynamic. It suddenly drives into the way of the ego vehicle, which has to stop accordingly. After some time, the cyclist will clear the road, such that the ego vehicle can continue driving.

### NoSignalJunctionCrossing
This scenario tests negotiation between two vehicles crossing cross each other through a junction without signal.
The ego vehicle is passing through a junction without traffic lights And encounters another vehicle passing across the junction. The ego vehicle has
to avoid collision and navigate across the junction to succeed.

### ControlLoss
In this scenario control loss of a vehicle is tested due to bad road conditions, etc, and it checks whether the vehicle is regained its control and corrected its course. 

### ManeuverOppositeDirection
In this scenario vehicle is passing another vehicle in a rural area, in daylight, under clear weather conditions, at a non-junction and encroaches into another vehicle traveling in the opposite direction.

### OtherLeadingVehicle
The scenario realizes a common driving behavior, in which the user-controlled ego vehicle follows a leading car driving down a given road.
At some point the leading car has to decelerate. The ego vehicle has to react accordingly by changing lane to avoid a collision and follow the leading car in other lane. The scenario ends via timeout, or if the ego vehicle drives certain distance.

### SignalizedJunctionRightTurn
In this scenario right turn of hero actor without collision at signalized intersection is tested. Hero Vehicle is turning right in an urban area, at a signalized intersection and
turns into the same direction of another vehicle crossing straight initially from a lateral direction.

### SignalizedJunctionLeftTurn
In this scenario hero vehicle is turning left in an urban area, at a signalized intersection and cuts across the path of another vehicle coming straight crossing from an opposite direction.



---
That is a wrap on how to properly retrieve data from the simulation. Make sure to play around, change the conditions of the simulator, experiment with sensor settings. The possibilities are endless. 


Visit the forum to post any doubts or suggestions that have come to mind during this reading.  

<div text-align: center>
<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="CARLA forum">
CARLA forum</a>
</p>
</div>
</div>