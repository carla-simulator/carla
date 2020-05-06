# OpenSCENARIO support

Learning an ef...

* [__Overview__](#overview)  
* [__Set the simulation__](#set-the-simulation)  
	* [Map setting](#map-setting)  
	* [Weather setting](#weather-setting)  


---
## Running scenarios using the OpenSCENARIO format
To run a scenario, which is based on the OpenSCENARIO format, please run the ScenarioRunner as follows:
```
python scenario_runner.py --openscenario <path/to/xosc-file>
```
Please note that the OpenSCENARIO support and the OpenSCENARIO format itself are still work in progress.
More information you can find in [OpenSCENARIO support](openscenario_support.md)

---
## OpenSCENARIO Support

The scenario_runner provides support for the [OpenSCENARIO](http://www.openscenario.org/) 1.0 standard.
The current implementation covers initial support for maneuver Actions, Conditions, Stories and the Storyboard.
If you would like to use evaluation criteria for a scenario to evaluate pass/fail results, these can be implemented
as StopTriggers (see below). However, not all features for these elements are yet available. If in doubt, please see the
module documentation in srunner/tools/openscenario_parser.py

An example for a supported scenario based on OpenSCENARIO is available [here](../srunner/examples/FollowLeadingVehicle.xosc)

In addition, it is recommended to take a look into the official documentation available [here](https://releases.asam.net/OpenSCENARIO/1.0.0/Model-Documentation/index.html) and [here](https://releases.asam.net/OpenSCENARIO/1.0.0/ASAM_OpenSCENARIO_BS-1-2_User-Guide_V1-0-0.html#_foreword).

>>>> Migrating OpenSCENARIO 0.9.x to 1.0

The easiest way to convert old OpenSCENARIO samples to the official standard 1.0 is to use _xsltproc_ and the migration scheme located in the openscenario folder.
Example:

```bash
xsltproc -o newScenario.xosc migration0_9_1to1_0.xslt oldScenario.xosc
```

In the following the OpenSCENARIO attributes are listed with their current support status.

### General OpenSCENARIO setup

This covers all part that are defined outside the OpenSCENARIO Storyboard

<table class ="defTable">
<thead>
<th>Attribute</th>
<th>Support</th>
<th>Notes</th>
</thead>
<tbody>
<td>
<code><i>FileHeader</i></code></td>
<td>&#9989;</td>
<td>Use "CARLA:" at the beginning of the description to use the CARLA coordinate system.</td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>ControllerCatalog</code></td>
<td>&#9989;</td>
<td>While the catalog is supported, the reference/usage may not work. </td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>EnvironmentCatalog</code></td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>ManeuverCatalog</code></td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>MiscObjectCatalog</code></td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>RouteCatalog</code></td>
<td>&#9989;</td>
<td>While the catalog is supported, the reference/usage may not work. </td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>TrajectoryCatalog</code></td>
<td>&#9989;</td>
<td>While the catalog is supported, the reference/usage may not work. </td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>PedestrianCatalog</code></td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>CatalogLocations</code></small><br><code>VehicleCatalog</code></td>
<td>&#9989;</td>
<td></td>
<tr>
<td><code>ParameterDeclarations</code></td>
<td>&#9989;</td>
<td>Parameters can currently only be defined at the beginning (i.e. globally).</td>
<tr>
<td><small><code>RoadNetwork</code></small><br><code>LogicFile</code></td>
<td>&#9989;</td>
<td>The CARLA level can be used directly (e.g. LogicFile=Town01). Also any OpenDRIVE path can be provided.</td>
<tr>
<td><small><code>RoadNetwork</code></small><br><code>SceneGraphFile</code></td>
<td>&#10060;</td>
<td>The provided information is not used.</td>
<tr>
<td><small><code>RoadNetwork</code></small><br><code>TafficSignals</code></td>
<td>&#10060;</td>
<td>The provided information is not used.</td>
<tr>
<td><small><code>Entities</code></small><br><code>EntitySelection</code></td>
<td>&#10060;</td>
<td>The provided information is not used.</td>
<tr>
<td><small><code>Entities</code><code>ScenarioObject</code></small><br><code>ObjectController</code></td>
<td>&#10060;</td>
<td>The provided information is not used.</td>
<tr>
<td><small><code>Entities</code><code>ScenarioObject</code></small><br><code>CatalogReference</code></td>
<td>&#9989;</td>
<td>The provided information is not used.</td>
<tr>
<td><small><code>Entities</code><code>ScenarioObject</code></small><br><code>MiscObject</code></td>
<td>&#9989;</td>
<td>The name should match a CARLA vehicle model, otherwise a default vehicle based on the vehicleCategory is used. BoundingBox entries are ignored.</td>
<tr>
<td><small><code>Entities</code><code>ScenarioObject</code></small><br><code>Pedestrian</code></td>
<td>&#9989;</td>
<td>The name should match a CARLA vehicle model, otherwise a default vehicle based on the vehicleCategory is used. BoundingBox entries are ignored.</td>
<tr>
<td><small><code>Entities</code><code>ScenarioObject</code></small><br><code>Vehicle</code></td>
<td>&#9989;</td>
<td>The name should match a CARLA vehicle model, otherwise a default vehicle based on the vehicleCategory is used. The color can be set via properties ('Property name="color" value="0,0,255"'). Axles, Performance, BoundingBox entries are ignored.</td>
</tbody>
</table>
<br>

### OpenSCENARIO Storyboard - Actions

The OpenSCENARIO Actions can be used for two different purposes. First, Actions can be used to define the initial behavior of something, e.g. a traffic participant. Therefore, Actions can be used within the OpenSCENARIO Init. In addition, Actions are also used within the OpenSCENARIO story. In the following, the support status for both application areas is listed. If an action contains of submodules, which are not listed, the support status applies to all submodules.

#### GlobalAction

<table class ="defTable">
<thead>
<th>Attribute</th>
<th>Init <br> support</th>
<th>Story <br> support</th>
<th>Notes</th>
</thead>
<tbody>
<td><code>EnvironmentAction</code></td>
<td>&#9989;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><code>EntityAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><code>ParameterAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><small><code>InfrastructureAction</code><code>TrafficSignalAction</code></small><br><code>TrafficSignalControllerAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><small><code>InfrastructureAction</code><code>TrafficSignalAction</code></small><br><code>TrafficSignalStateAction</code></td>
<td>&#10060;</td>
<td>&#9989;</td>
<td>Setting a traffic light state in CARLA works by providing the position of the relevant traffic light (Example: TrafficSignalStateAction name="pos=x,y" state="green").</td>
<tr>
<td><small><code>InfrastructureAction</code><code>TrafficSignalAction</code></small><br><code>TrafficAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
</tbody>
</table>
<br>


#### UserDefinedAction

<table class ="defTable">
<thead>
<th>Attribute</th>
<th>Init <br> support</th>
<th>Story <br> support</th>
<th>Notes</th>
</thead>
<tbody>
<td><code>CustomCommandAction</code></td>
<td>&#10060;</td>
<td>&#9989;</td>
<td>This action is currently used to trigger the execution of an additional script. Example: type="python /path/to/script args".</td>
</tbody>
</table>
<br>

#### PrivateAction

<table class ="defTable">
<thead>
<th>Attribute</th>
<th>Init <br> support</th>
<th>Story <br> support</th>
<th>Notes</th>
</thead>
<tbody>
<td><small><code>LongitudinalAction</code></small><br><code>SpeedAction</code></td>
<td>&#9989;</td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>LongitudinalAction</code></small><br><code>LongitudinalDistanceAction</code></td>
<td>&#10060;</td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>LateralAction</code></small><br><code>LaneChangeAction</code></td>
<td>&#10060;</td>
<td>&#9989;</td>
<td>Currently only lane change by one lane to the left or right is supported (RelativeTargetLane).</td>
<tr>
<td><small><code>LateralAction</code></small><br><code>LaneOffsetAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><small><code>LateralAction</code></small><br><code>LateralDistanceAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><code>VisibilityAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><code>SynchronizeAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><code>ActivateControllerAction</code></td>
<td>&#10060;</td>
<td>&#9989;</td>
<td>Only supports the autopilot at the moment</td>
<tr>
<td><code>ControllerAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><code>TeleportAction</code></td>
<td>&#9989;</td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>RoutingAction</code></small><br><code>AssignRouteAction</code></td>
<td>&#10060;</td>
<td>&#9989;</td>
<td></td>
<tr>
<td><small><code>RoutingAction</code></small><br><code>FollowTrajectoryAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
<tr>
<td><small><code>RoutingAction</code></small><br><code>AcquirePositionAction</code></td>
<td>&#10060;</td>
<td>&#10060;</td>
<td></td>
</tbody>
</table>
<br>


### OpenSCENARIO Storybord - Conditions

Conditions in OpenSCENARIO can be defined either as ByEntityCondition or as ByValueCondition.
Both can be used for StartTrigger and StopTrigger conditions.
The following two tables list the support status for each.

#### ByEntityCondition

|Condition  | Support Status | Notes / Remarks |
|-----------|:--------------:|-----------------|
|EndOfRoadCondition       | No  | |
|CollisionCondition       | Yes | |
|OffroadCondition         | No  | |
|TimeHeadwayCondition     | No  | |
|TimeToCollisionCondition | Yes | |
|AccelerationCondition    | No  | |
|StandStillCondition      | Yes | |
|SpeedCondition           | Yes | |
|RelativeSpeedCondition   | No  | |
|TraveledDistanceCondition| Yes | |
|ReachPositionCondition   | Yes | |
|DistanceCondition        | Yes | |
|RelativeDistanceCondition| Yes | |

#### ByValueCondition

|Condition  | Support Status | Notes / Remarks |
|-----------|:--------------:|-----------------|
|ParameterCondition               | Yes*| The level of support depends on the parameter. It is recommended to use other conditions if possible. Please also consider the note below. |
|TimeOfDayCondition               | No  | |
|SimulationTimeCondition          | Yes | |
|StoryboardElementStateCondition  | Yes*| startTransition, stopTransition, endTransition and completeState are currently supported|
|UserDefinedValueCondition        | No  | |
|TrafficSignalCondition           | No  | |
|TrafficSignalControllerCondition | No  | |

!!! Note
     In the OpenSCENARIO 1.0 standard, a definition of test / evaluation criteria is not
     defined. For this purpose, you can re-use StopTrigger conditions with CARLA. The following
     StopTrigger conditions for evaluation criteria are supported through ParameterConditions by
     providing the criteria name for the condition:

     * criteria_RunningStopTest
     * criteria_RunningRedLightTest
     * criteria_WrongLaneTest
     * criteria_OnSideWalkTest
     * criteria_KeepLaneTest
     * criteria_CollisionTest
     * criteria_DrivenDistanceTest

### OpenSCENARIO Storyboard - Positions

There are several ways of defining positions in OpenSCENARIO. In the following we list the
current support status for each definition format.

|Position  | Support Status | Notes / Remarks |
|----------|:--------------:|-----------------|
|WorldPosition          | Yes | |
|RelativeWorldPosition  | Yes | |
|RelativeObjectPosition | Yes | |
|RoadPosition           | No  | |
|RelativeRoadPosition   | No  | |
|LanePosition           | Yes | |
|RelativeLanePosition   | Yes | |
|RoutePosition          | No  | |

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