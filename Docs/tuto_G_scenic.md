# Scenic

This guide provides an overview of how to use Scenic with CARLA to generate multiple, diverse scenarios with a single scenario definition. It assumes that users have prior knowledge of the Scenic syntax. If you need to learn more about Scenic, then read their ["Getting Started with Scenic"](https://scenic-lang.readthedocs.io/en/latest/quickstart.html) guide and have a look at their tutorials for creating [static](https://scenic-lang.readthedocs.io/en/latest/tutorials/tutorial.html) and [dynamic](https://scenic-lang.readthedocs.io/en/latest/tutorials/dynamics.html) scenarios.

By the end of the guide you will know:

- The minimum requirements needed to run a Scenic script on CARLA.
- How to write a simple scenario definition to generate a multitude of scenario simulations.
- How to run a Scenic script on CARLA.
- Parameters used to configure Scenic simulations on CARLA.

---

- [__Before you begin__](#before-you-begin)
- [__Scenic domains__](#scenic-domains)
- [__Creating a Scenic scenario to use with CARLA__](#creating-a-scenic-scenario-to-use-with-carla)
- [__Run the scenario__](#run-the-scenario)
- [__Additional parameters__](#additional-parameters)

---

## Before you begin

Before using Scenic with CARLA, you will need to fulfill the following requirements:

- Install [Python 3.8](https://www.python.org/downloads/) or higher. 
- Install [Scenic](https://scenic-lang.readthedocs.io/en/latest/quickstart.html#installation).

---

## Scenic Domains

Scenic has a general driving domain which allows users to define scenarios that can be run on any driving simulator. In addition, it has other domains that are specific to each simulator. Check [here](https://scenic-lang.readthedocs.io/en/latest/libraries.html) for more information on Scenic domains.

Of particular importance within each domain are the behaviour and actions definitions. Check the links below for reference material to behaviours and actions from the Scenic driving domain and the CARLA domain:

- [Behaviours in the Scenic driving domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.behaviors.html)
- [Behaviours in the CARLA domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.behaviors.html)
- [Actions in the Scenic driving domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.actions.html)
- [Actions in the CARLA domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.actions.html#module-scenic.simulators.carla.actions)

---

## Creating a Scenic scenario to use with CARLA

This section walks through how to write a basic Scenic script in which a leading vehicle decelerates suddenly due to an obstacle in the road. An ego vehicle then needs to brake suddenly to avoid a collison with the leading vehicle. The [full script](https://github.com/BerkeleyLearnVerify/Scenic/blob/master/examples/carla/Carla_Challenge/carlaChallenge2.scenic) is found in the Scenic repository along with other examples involving more complex road networks. 

__1.__ Set the map parameters and declare the model to be used in the scenario:

- An `.xodr` file should be set as the value for the [`map`][scenic_map] parameter, this will be used later to generate road network information. 
- The parameter `carla_map` refers to the name of the CARLA map you would like to use in the simulation. If this is defined then Scenic will load all the assets of the map (buildings, trees, etc.), and if not, then the [OpenDRIVE standalone mode](adv_opendrive.md) will be used.
- The model includes all the utilities specific to running scenarios on CARLA. This should be defined in all the scripts you want to run on CARLA.

```scenic
## SET MAP AND MODEL
param map = localPath('../../../tests/formats/opendrive/maps/CARLA/Town01.xodr')
param carla_map = 'Town01'
model scenic.simulators.carla.model
```

[scenic_map]: https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.model.html?highlight=map#module-scenic.domains.driving.model

__2.__ Define the constants to be used in the scenario:

The scenario involves two vehicles, the leading vehicle and the ego vehicle. We will define the ego vehicle model, the speeds of both cars, the distance threshold for braking and the amount of brake to apply.

```scenic
## CONSTANTS
EGO_MODEL = "vehicle.lincoln.mkz_2017"
EGO_SPEED = 10
EGO_BRAKING_THRESHOLD = 12

LEAD_CAR_SPEED = 10
LEADCAR_BRAKING_THRESHOLD = 10

BRAKE_ACTION = 1.0
```

__3__. Define the scenario behaviours:

In this scenario we will use the Scenic [behaviour library](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.behaviors.html) to instruct the ego vehicle to follow the lane at the predefined speed and then brake hard when it gets within a certain distance of another vehicle. The leading vehicle will also follow the lane at the predefined speed and brake hard within a certain distance of any objects:

```scenic
## DEFINING BEHAVIORS
# EGO BEHAVIOR: Follow lane, and brake after passing a threshold distance to the leading car
behavior EgoBehavior(speed=10):
    try:
        do FollowLaneBehavior(speed)

    interrupt when withinDistanceToAnyCars(self, EGO_BRAKING_THRESHOLD):
        take SetBrakeAction(BRAKE_ACTION)

# LEAD CAR BEHAVIOR: Follow lane, and brake after passing a threshold distance to obstacle
behavior LeadingCarBehavior(speed=10):
    try: 
        do FollowLaneBehavior(speed)

    interrupt when withinDistanceToAnyObjs(self, LEADCAR_BRAKING_THRESHOLD):
        take SetBrakeAction(BRAKE_ACTION)
```

__4.__ Generate the road network:

The Scenic [roads library](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.roads.html) is used to generate the road network geometry and traffic information. The road network is represented by an instance of the [`Network`](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.domains.driving.roads.html#scenic.domains.driving.roads.Network) class and is generated from the `.xodr` file defined at the beginning of the script.

```scenic
## DEFINING SPATIAL RELATIONS
# make sure to put '*' to uniformly randomly select from all elements of the list, 'lanes'
lane = Uniform(*network.lanes)
```

__5.__ Set the scene:

We will now define the starting positions for the vehicles and placement of objects. 

- Place a trash can in the middle of the lane:

```scenic
obstacle = Trash on lane.centerline
```

- Place the leading car driving at the predefined speed along the road at a distance of between 50 and 30 meters behind the obstacle:

```scenic
leadCar = Car following roadDirection from obstacle for Range(-50, -30),
        with behavior LeadingCarBehavior(LEAD_CAR_SPEED)
```

- Place the ego vehicle driving at the predefined speed along the road at a distance of between 15 to 10 meters behind the leading vehicle:

```scenic
ego = Car following roadDirection from leadCar for Range(-15, -10),
        with blueprint EGO_MODEL,
        with behavior EgoBehavior(EGO_SPEED)
```

- Make it a requirement that the scene takes place more than 80 meters from an intersection:

```scenic
require (distance to intersection) > 80
```

__6.__ Set an end point so the script knows when the scene is finished:

The scenario will end when the speed of the ego vehicle goes below 0.1 meters per second and is situated less than 30 meters from the obstacle.

```scenic
terminate when ego.speed < 0.1 and (distance to obstacle) < 30
```

---

### Run the scenario

To run the Scenic scenario:

__1.__ Start the CARLA server.

__2.__ Run the following command:

```scenic
scenic path/to/scenic/script.scenic --simulate
```

A pygame window will appear and the scenario will play out repeatedly, each time generating a unique scenario within the bounds of the restrictions set in the script. To stop the scenario generation, press `ctrl + C` in the terminal. 

---

### Additional parameters

The CARLA model provides several global parameters than can be overridden in scenarios using the [`param` statement](https://scenic-lang.readthedocs.io/en/latest/syntax_details.html#param-identifier-value) or via the command line using the [`--param` option](https://scenic-lang.readthedocs.io/en/latest/options.html#cmdoption-p).

Below is a table of configurable parameters in the CARLA model:

| Name | Value | Description |
|------|-------|-------------|
| `carla_map` | `str` | Name of the CARLA map to use (e.g. 'Town01'). If set to ``None``, CARLA will attempt to create a world in the OpenDRIVE standalone mode using the `.xodr` file defined in the [`map`][scenic_map] parameter. |
| `timestep` | `float` | Timestep to use for simulations (how frequently Scenic interrupts CARLA to run behaviors, check requirements, etc.) in seconds. Default is 0.1 seconds. |
| `weather` | `str` or `dict` | Weather to use for the simulation. Can be either a string identifying one of the CARLA weather presets (e.g. 'ClearSunset') or a dictionary specifying all the [weather parameters](python_api.md#carla.WeatherParameters)). Default is a uniform distribution over all the weather presets. |
| `address` | `str` | IP address to connect to CARLA. Default is localhost (127.0.0.1).|
| `port` | `int` | Port to connect to CARLA. Default is 2000. |
| `timeout` | `float` | Maximum time in seconds to wait when attempting to connect to CARLA. Default is 10.|
| `render` | `int` | Whether or not to have CARLA create a window showing the simulations from the point of view of the ego object: `1` for yes, `0` for no. Default `1`. |
| `record` | `str` | If nonempty, folder in which to save CARLA record files for replaying simulations. |

<br>
