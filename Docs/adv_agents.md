# CARLA Agents

An agent is a Python class that drives a vehicle. You build one around a vehicle you have already spawned, and from then on it decides how that vehicle moves: every step it returns a [`carla.VehicleControl`](python_api.md#carla.VehicleControl), which you apply to the vehicle. It plans the route, stops for traffic lights and reacts to what is in the way.

Agents run entirely on the client, in the [`agents`](https://github.com/carla-simulator/carla/tree/ue58-dev/PythonAPI/carla/agents) package under `PythonAPI/carla`. Nothing is added to the server and the vehicle is an ordinary actor, so an agent can be swapped for autopilot, or for your own control, at any point.

Three agents ship with CARLA.

`BasicAgent` is the base class and is usable on its own: it plans a route, follows it, and stops for traffic lights and for whatever is in its way.

`BehaviorAgent` and `ConstantVelocityAgent` both subclass it for a particular purpose. `BehaviorAgent` drives to a chosen profile — how close it follows, when it overtakes, when it tailgates. `ConstantVelocityAgent` holds a fixed speed whatever the traffic does, so a run can be repeated exactly.

Any of the three can be configured through its constructor and methods, or subclassed into one of your own.

- [__Use the provided agents__](#use-the-provided-agents)
- [__Behavior types__](#behavior-types)
    - [Create your own behavior type](#create-your-own-behavior-type)
- [__Overview of agent scripts__](#overview-of-agent-scripts)
    - [Planning and control](#planning-and-control)
    - [Agent behaviors](#agent-behaviors)
- [__Write a custom agent__](#write-a-custom-agent)

---

## Use the provided agents

__1.__ Import the agent class you want to use. The agents live in `PythonAPI/carla/agents`, which the `carla` wheel does not install, so put it on the path first — this is what `automatic_control.py` does. They also need `networkx` and `shapely`.

```py
import sys
sys.path.append('<CARLA>/PythonAPI/carla')

# To import a basic agent
from agents.navigation.basic_agent import BasicAgent

# To import a behavior agent
from agents.navigation.behavior_agent import BehaviorAgent

# To import a constant velocity agent
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent
```

__2.__ Any vehicle can be turned into an agent. [Spawn a vehicle](core_actors.md#spawning) and pass it as an argument to the agent class to instantiate it:

```py
# To start a basic agent
agent = BasicAgent(vehicle)

# To start a behavior agent with an aggressive profile
agent = BehaviorAgent(vehicle, behavior='aggressive')

# To start a constant velocity agent at 20 km/h
agent = ConstantVelocityAgent(vehicle, 20)
```

Read more about behavior types and how to configure your own in the section [__behavior types__](#behavior-types).

__3.__ You can set a destination for the agent to travel to. If you don't set a destination for the agent, it will roam endlessly around the map. To set the destination, provide the agent with a [location](python_api.md#carla.Location):

```py
destination = random.choice(spawn_points).location
agent.set_destination(destination)
```

__4.__ Vehicle controls and behaviors are applied during a navigation step. During each step, the __Basic Agent__ will apply a vehicle control and react to any vehicles or traffic lights by performing an emergency stop. The __Behavior Agent__ will react to traffic lights, avoid pedestrians, follow cars and navigate intersections according to the behavior type you applied:

```py
while True:
    world.tick()                 # world.wait_for_tick() in asynchronous mode
    vehicle.apply_control(agent.run_step())
```

__5.__ You can check if the agent has finished its trajectory and perform an action when that happens. The following snippet will end the simulation once your vehicle has reached its destination:

```py
while True:
    world.tick()
    if agent.done():
        print("The target has been reached, stopping the simulation")
        break

    vehicle.apply_control(agent.run_step())
```

__6.__ Instead of finishing the simulation when an agent has reached its target destination, a new, random route can be generated for the agent to follow:

```py
while True:
    world.tick()
    if agent.done():
        agent.set_destination(random.choice(spawn_points).location)
        print("The target has been reached, searching for another target")

    vehicle.apply_control(agent.run_step())
```

The __Basic Agent__ provides a few methods to manipulate agent behavior or program routes to follow:

- __`set_target_speed(speed)`:__ Set the target speed in km/h
- __`follow_speed_limits(value=True)`:__ Sets the agent to follow speed limits.
- __`set_destination(end_location, start_location=None, clean_queue=True)`:__ The agent will travel from a specific start location to an end location via the shortest route possible. If no start location is provided, it will use the current agent location. `clean_queue=False` appends the new route to the one already queued instead of replacing it.
- __`set_global_plan(plan, stop_waypoint_creation=True, clean_queue=True)`:__ Adds a specific plan for the agent to follow. The plan argument should consist of a list of `[carla.Waypoint, RoadOption]` that will be the path the agent needs to take. `stop_waypoint_creation` will prevent waypoints from being automatically created once the path has run its course. `clean_queue` will reset the agent's current plan.
- __`trace_route(start_waypoint, end_waypoint)`:__ Gets the shortest distance between two waypoints from the Global Route Planner and returns the path as a list of `[carla.Waypoint, RoadOption]` for the agent to follow.
- __`ignore_traffic_lights(active=True)`:__ Set the agent to ignore or obey traffic lights.
- __`ignore_stop_signs(active=True)`:__ Set the agent to ignore or obey stop signs.
- __`ignore_vehicles(active=True)`:__ Set the agent to ignore or react to other vehicles.

The [`automatic_control.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/examples/automatic_control.py) script, found in `PythonAPI/examples`, is an example of the Basic and Behavior Agents in action. To try the script, navigate to the example directory and run the following command:

```sh
# To run with a basic agent
python3 automatic_control.py --agent=Basic

# To run with a behavior agent
python3 automatic_control.py --agent=Behavior --behavior=aggressive

# To run with a constant velocity agent
python3 automatic_control.py --agent=Constant
```

---

## Behavior types

Behavior types for the behavior agent are defined in [`behavior_types.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/behavior_types.py). The three pre-configured profiles are __'cautious'__, __'normal'__ and __'aggressive'__; use them as they are, modify them, or write your own.

| Parameter | Cautious | Normal | Aggressive |
| --- | --- | --- | --- |
| `max_speed` | 40 | 50 | 70 |
| `speed_lim_dist` | 6 | 3 | 1 |
| `speed_decrease` | 12 | 10 | 8 |
| `safety_time` | 3 | 3 | 3 |
| `min_proximity_threshold` | 12 | 10 | 8 |
| `braking_distance` | 6 | 5 | 4 |
| `tailgate_counter` | 0 | 0 | -1 |

What each one does:

- __`max_speed`__: The maximum speed in km/h your vehicle will be able to reach.
- __`speed_lim_dist`__: Value in km/h that defines how far your vehicle's target speed will be from the current speed limit (e.g., if the speed limit is 30km/h and `speed_lim_dist` is 10km/h, then the target speed will be 20km/h)
- __`speed_decrease`__: km/h subtracted from the target speed when the time to collision with the vehicle ahead drops below `safety_time`.
- __`safety_time`__: Time-to-collision; an approximation of the time it will take for your vehicle to collide with one in front if it brakes suddenly.
- __`min_proximity_threshold`__: The minimum distance in meters from another vehicle or pedestrian before your vehicle performs a maneuver such as avoidance, or tailgating.
- __`braking_distance`__: The distance from a pedestrian or vehicle at which your vehicle will perform an emergency stop.
- __`tailgate_counter`__: Steps to wait before tailgating again. It counts down each step, and tailgating is only considered once it reaches zero.

### Create your own behavior type

To create your own behavior type:

__1.__ Create the class for your behavior type in [`behavior_types.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/behavior_types.py):

```py
class ProfileName:
    """Class for a ProfileName agent."""
    max_speed = 50
    speed_lim_dist = 3
    speed_decrease = 10
    safety_time = 3
    min_proximity_threshold = 10
    braking_distance = 5
    tailgate_counter = 0
```

__2.__ Define and instantiate your behavior type in the [`behavior_agent.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/behavior_agent.py) script:

```py
# Parameters for agent behavior
if behavior == 'cautious':
    self._behavior = Cautious()

elif behavior == 'normal':
    self._behavior = Normal()

elif behavior == 'aggressive':
    self._behavior = Aggressive()

elif behavior == 'profile_name':
    self._behavior = ProfileName()
```

---

## Overview of agent scripts

The main scripts involved in the CARLA Agents are found in [`PythonAPI/carla/agents/navigation`](https://github.com/carla-simulator/carla/tree/ue58-dev/PythonAPI/carla/agents/navigation). They fall into two categories; __planning and control__ and __agent behaviors__.

### Planning and control

- __[`controller.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/controller.py):__ Combines longitudinal and lateral PID controllers into a single class, __VehiclePIDController__, used for low-level control of vehicles from the client side of CARLA.
- __[`global_route_planner.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/global_route_planner.py):__ Gets detailed topology from the CARLA server to build a graph representation of the world map, providing waypoint and road option information for the __Local Planner__.
- __[`local_planner.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/local_planner.py):__ Follows waypoints based on control inputs from the __VehiclePIDController__. Waypoints can either be provided by the __Global Route Planner__ or be calculated dynamically, choosing random paths at junctions, similar to the [Traffic Manager](adv_traffic_manager.md).

### Agent behaviors

- __[`basic_agent.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/basic_agent.py):__ Contains the base class that implements a __Basic Agent__: it roams around the map or reaches a target destination by the shortest route, avoiding other vehicles and responding to traffic lights, but ignoring stop signs.
- __[`behavior_agent.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/behavior_agent.py):__ Contains a class that implements a more complex __Behavior Agent__ that reaches a target destination by the shortest route, following traffic lights and speed limits, overtaking and tailgating other vehicles. Three predefined types condition how it behaves. It does not handle stop signs: `BasicAgent.ignore_stop_signs` exists but has no effect, because no agent reads stop signs in the first place.
- __[`behavior_types.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/behavior_types.py):__ Contains the parameters for the behavior types that condition the __Behavior Agent__: Cautious, Normal and Aggressive.
- __[`constant_velocity_agent.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/constant_velocity_agent.py):__ A __Basic Agent__ that holds a constant speed, for runs where the ego's motion has to be repeatable. `automatic_control.py --agent=Constant` drives with it.
- __[`agents/tools/hints.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/tools/hints.py):__ The named tuples the detection helpers return: `ObstacleDetectionResult(obstacle_was_found, obstacle, distance)` and `TrafficLightDetectionResult(traffic_light_was_found, traffic_light)`. A custom agent reads the fields by name rather than unpacking a plain tuple.

The agent and planner modules carry type hints throughout, so an editor or a type checker can follow what a method takes and returns while you subclass them.

---

## Write a custom agent

The agents that ship with CARLA are examples; subclass the __Basic Agent__ to write your own. Only two pieces are required, __the initialization__ and the __run step__.

Find an example of a minimal layout of a custom agent below:

```py
import carla

from agents.navigation.basic_agent import BasicAgent

class CustomAgent(BasicAgent):
    def __init__(self, vehicle, target_speed=20, opt_dict={}):
        """
        :param vehicle: actor to apply the local planner logic onto
        :param target_speed: speed (in Km/h) at which the vehicle will move
        :param opt_dict: dictionary of options passed to BasicAgent
        """
        super().__init__(vehicle, target_speed, opt_dict)

    def run_step(self):
        """
        Execute one step of navigation.
        :return: carla.VehicleControl
        """
        # Actions to take during each simulation step
        control = carla.VehicleControl()
        return control
```

Check out the [`basic_agent.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/basic_agent.py) and [`behavior_agent.py`](https://github.com/carla-simulator/carla/blob/ue58-dev/PythonAPI/carla/agents/navigation/behavior_agent.py) scripts to explore their structure and functions for more ideas on how to create your own.

---

You can explore the provided agent scripts, expand upon them or use them as a baseline to create your own. If you have any questions about the agents, feel free to post in the [GitHub Discussions](https://github.com/carla-simulator/carla/discussions).
