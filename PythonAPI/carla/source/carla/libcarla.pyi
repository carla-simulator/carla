from enum import Enum
from typing import Callable, Iterable, Iterator, Union, overload


class AckermannControllerSettings():
    """Manages the settings of the Ackermann PID controller."""

    # region Instance Variables
    @property
    def speed_kp() -> float:
        """Proportional term of the speed PID controller."""
        ...

    @property
    def speed_ki() -> float:
        """Integral term of the speed PID controller."""
        ...

    @property
    def speed_kd() -> float:
        """Derivative term of the speed PID controller."""
        ...

    @property
    def accel_kp() -> float:
        """Proportional term of the acceleration PID controller."""
        ...

    @property
    def accel_ki() -> float:
        """Integral term of the acceleration PID controller."""
        ...

    @property
    def accel_kd() -> float:
        """Derivative term of the acceleration PID controller."""
        ...
    # endregion

    # region Methods
    def __init__(self,
                 speed_kp: float = 0.15,
                 speed_ki: float = 0.0,
                 speed_kd: float = 0.25,
                 accel_kp: float = 0.01,
                 accel_ki: float = 0.0,
                 accel_kd: float = 0.01) -> AckermannControllerSettings:
        """Manages the settings of the Ackermann PID controller.
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: AckermannControllerSettings) -> bool: ...

    def __ne__(self, __value: AckermannControllerSettings) -> bool: ...

    def __str__(self) -> str: ...
    # endregion


class Actor():
    """CARLA defines actors as anything that plays a role in the simulation or can be moved around.

    That includes: pedestrians, vehicles, sensors and traffic signs (considering traffic lights as part of these). Actors are spawned in the simulation by `carla.World` and they need for a `carla.ActorBlueprint` to be created. These blueprints belong into a library provided by CARLA, find more about them here.
    https://carla.readthedocs.io/en/latest/bp_library/
    """

    # region Instnce Variables
    @property
    def attributes() -> dict:
        """A dictionary containing the attributes of the blueprint this actor was based on.
        """
        ...

    @property
    def id() -> int:
        """Identifier for this actor. Unique during a given episode.
        """
        ...

    @property
    def type_id() -> str:
        """The identifier of the blueprint this actor was based on,

        e.g., `vehicle.ford.mustang.`"""
        ...

    @property
    def is_alive() -> bool:
        """Returns whether this object was destroyed using this actor handle."""
        ...

    @property
    def is_active() -> bool:
        """Returns whether this actor is active (`True`) or not (`False`)."""
        ...

    @property
    def is_dormant() -> bool:
        """Returns whether this actor is dormant (`True`) or not (`False`) - the opposite of `is_active`."""
        ...

    @property
    def parent() -> ["Actor"]:
        """Actors may be attached to a parent actor that they will follow around. This is said actor."""
        ...

    @property
    def semantic_tags() -> list[int]:
        """A list of semantic tags provided by the blueprint listing components for this actor.

        E.g. a traffic light could be tagged with `Pole` and `TrafficLight`. These tags are used by the semantic segmentation sensor. Find more about this and other sensors here
        https://carla.readthedocs.io/en/latest/ref_sensors/#semantic-segmentation-camera
        """

    @property
    def actor_state() -> ActorState:
        """Returns the carla.ActorState, which can identify if the actor is Active, Dormant or Invalid."""
        ...

    @property
    def bounding_box() -> BoundingBox:
        """Bounding box containing the geometry of the actor. Its location and rotation are relative to the actor it is attached to."""
    # endregion

    # region Methods
    def add_angular_impulse(self, angular_impulse: Vector3D):
        """Applies an angular impulse at the center of mass of the actor.

        This method should be used for instantaneous torques, usually applied once. Use `add_torque()` to apply rotation forces over a period of time.

        Args:
            `angular_impulse (Vector3D - degrees*s)`: Angular impulse vector in global coordinates.\n
        """
        ...

    def add_force(self, force: Vector3D):
        """Applies a force at the center of mass of the actor.

        This method should be used for forces that are applied over a certain period of time. Use `add_impulse()` to apply an impulse that only lasts an instant.

        Args:
            `force (Vector3D - N)`: Force vector in global coordinates.\n
        """
        ...

    def add_impulse(self, impulse: Vector3D):
        """Applies an impulse at the center of mass of the actor.

        This method should be used for instantaneous forces, usually applied once. Use `add_force()` to apply forces over a period of time.

        Args:
            `impulse (Vector3D - N*s)`: Impulse vector in global coordinates.\n
        """
        ...

    def add_torque(self, torque: Vector3D):
        """Applies a torque at the center of mass of the actor.

        This method should be used for torques that are applied over a certain period of time. Use `add_angular_impulse()` to apply a torque that only lasts an instant.

        Args:
            `torque (Vector3D - degrees)`: Torque vector in global coordinates.\n
        """
        ...

    def destroy(self) -> bool:
        """Tells the simulator to destroy this actor and returns True if it was successful.

        It has no effect if it was already destroyed.

        + Warning: This method blocks the script until the destruction is completed by the simulator.
        """
        ...

    def disable_constant_velocity(self):
        """Disables any constant velocity previously set for a `carla.Vehicle` actor.
        """
        ...

    def enable_constant_velocity(self, velocity: Vector3D):
        """Sets a vehicle's velocity vector to a constant value over time.

        The resulting velocity will be approximately the `velocity` being set, as with set_target_velocity().

        + Note: Only `carla.Vehicle` actors can use this method.

        + Warning: Enabling a constant velocity for a vehicle managed by the `Traffic Manager` may cause conflicts. This method overrides any changes in velocity by the TM.

        Args:
            `velocity (Vector3D - m/s)`:  Velocity vector in local space.\n
        """
        ...
    # endregion

    # region Getters
    def get_acceleration(self) -> Vector3D:
        """Returns the actor's 3D acceleration vector the client recieved during last tick. 

        The method does not call the simulator.

        Returns:
            `Vector3D`: m/s^2\n
        """
        ...

    def get_angular_velocity(self) -> Vector3D:
        """Returns the actor's angular velocity vector the client recieved during last tick.

        The method does not call the simulator.

        Returns:
            `Vector3D`: deg/s\n
        """
        ...

    def get_location(self) -> Location:
        """Returns the actor's location the client recieved during last tick. 

        The method does not call the simulator.

        + Setter: `carla.Actor.set_location`

        Returns:
            `Location`: - meters\n
        """
        ...

    def get_transform(self) -> Transform:
        """Returns the actor's transform (location and rotation) the client recieved during last tick.

        The method does not call the simulator.

        + Setter: `carla.Actor.set_transform`

        Returns:
            `Transform`\n
        """

    def get_velocity(self) -> Vector3D:
        """Returns the actor's velocity vector the client recieved during last tick.

        The method does not call the simulator.

        Returns:
            `Vector3D`:  m/s\n
        """
        ...

    def get_world(self) -> World:
        """Returns the world this actor belongs to.

        Returns:
            `World`\n
        """
        ...
    # endregion

    # region Setters
    def set_enable_gravity(self, enabled: bool):
        """Enables or disables gravity for the actor.
        Default is `True`.

        Args:
            `enabled (bool)`\n
        """
        ...

    def set_location(self, location: Location):
        """Teleports the actor to a given location.

        + Getter: `carla.Actor.get_location`

        Args:
            `location (Location)`: meters\n
        """
        ...

    def set_simulate_physics(self, enabled=True):
        """Enables or disables the simulation of physics on this actor.

        Args:
            `enabled (bool, optional)`: Defaults to True.\n
        """
        ...

    def set_target_angular_velocity(self, angular_velocity: Vector3D):
        """_summary_Sets the actor's angular velocity vector.

        This is applied before the physics step so the resulting angular velocity will be affected by external forces such as friction.

        Args:
            `angular_velocity (Vector3D)`: deg/s\n
        """
        ...

    def set_target_velocity(self, velocity: Vector3D):
        """Sets the actor's velocity vector.

        This is applied before the physics step so the resulting angular velocity will be affected by external forces such as friction.

        Args:
            `velocity (Vector3D)`\n
        """
        ...

    def set_transform(self, transform: Transform):
        """Teleports the actor to a given transform (location and rotation).

        + Getter: `carla.Actor.get_transform`

        Args:
            `transform (Transform)`\n
        """
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class ActorAttribute():
    """CARLA provides a library of blueprints for actors that can be accessed as `carla.BlueprintLibrary`. Each of these blueprints has a series of attributes defined internally. Some of these are modifiable, others are not. A list of recommended values is provided for those that can be set.
    """

    # region Instance Variables
    @property
    def id() -> str:
        """The attribute's name and identifier in the library.
        """
    @property
    def is_modifiable() -> bool:
        """It is True if the attribute's value can be modified."""
    @property
    def recommended_values() -> list[str]:
        """A list of values suggested by those who designed the blueprint."""
    @property
    def type() -> ActorAttributeType:
        """The attribute's parameter type."""
    # endregion

    # region Methods
    def as_bool(self) -> bool:
        """Reads the attribute as boolean value."""
        ...

    def as_color(self) -> Color:
        """Reads the attribute as `carla.Color`."""
        ...

    def as_float(self) -> float:
        """Reads the attribute as float."""
        ...

    def as_int(self) -> int:
        """Reads the attribute as int."""
        ...

    def as_str(self) -> str:
        """Reads the attribute as string."""
        ...
    # endregion

    # region Dunder Methods
    def __bool__(self) -> bool: ...
    def __eq__(self, __value: Union[bool, int, float, str, Color, ActorAttribute]) -> bool: ...
    def __float__(self) -> float: ...
    def __int__(self) -> int: ...
    def __ne__(self, __value: Union[bool, int, float, str, Color, ActorAttribute]) -> bool: ...
    def __nonzero__(self) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class ActorAttributeType(Enum):
    """CARLA provides a library of blueprints for actors in `carla.BlueprintLibrary` with different attributes each. This class defines the types those at `carla.ActorAttribute` can be as a series of enum. All this information is managed internally and listed here for a better comprehension of how CARLA works.
    """
    # region Instance Variables
    Bool = 0,
    Int = 1,
    Float = 2,
    String = 3,
    RGBColor = 4,
    # endregion


class ActorBlueprint():
    """CARLA provides a blueprint library for actors that can be consulted through `carla.BlueprintLibrary`. Each of these consists of an identifier for the blueprint and a series of attributes that may be modifiable or not. This class is the intermediate step between the library and the actor creation. Actors need an actor blueprint to be spawned. These store the information for said blueprint in an object with its attributes and some tags to categorize them. The user can then customize some attributes and eventually spawn the actors through `carla.World`.
    """

    # region Instance Variables
    @property
    def id() -> str:
        """The identifier of said blueprint inside the library.

        E.g. `walker.pedestrian.0001`."""

    @property
    def tags() -> list[str]:
        """A list of tags each blueprint has that helps describing them.

        E.g. `['0001', 'pedestrian', 'walker']`.
        """
        ...
    # endregion

    # region Methods
    def has_attribute(self, id: str) -> bool:
        """Returns `True` if the blueprint contains the attribute `id`.

        Args:
            `id (str)`: e.g. `gender` would return True for pedestrians' blueprints.\n

        Returns:
            `bool`\n
        """
        ...

    def has_tag(self, tag: str) -> bool:
        """Returns `True` if the blueprint has the specified `tag` listed.

        Args:
            `tag (str)`: e.g. `'walker'`\n

        Returns:
            `bool`\n
        """
        ...

    def match_tags(self, wildcard_pattern: str) -> bool:
        """Returns `True` if any of the tags listed for this blueprint matches `wildcard_pattern`.

        Matching follows `fnmatch` standard. https://docs.python.org/3.12/library/fnmatch.html

        Args:
            `wildcard_pattern (str)`\n

        Returns:
            `bool`\n
        """
    # endregion

    # region Getters
    def get_attribute(self, id: str) -> ActorAttribute:
        """Returns the actor's attribute with `id` as identifier if existing.

        + Setter: `carla.ActorBlueprint.set_attribute`

        Args:
            `id (str)`\n

        Returns:
            `ActorAttribute`\n
        """
        ...
    # endregion

    # region Setters
    def set_attribute(self, id: str, value: str):
        """If the `id` attribute is modifiable, changes its value to `value`.

        Args:
            `id (str)`: The identifier for the attribute that is intended to be changed.\n
            `value (str)`: The new value for said attribute.\n
        """
    # endregion

    # region Dunder Methods
    def __iter__(self):
        """Iterate over the `carla.ActorAttribute` that this blueprint has.
        """
        ...

    def __len__(self) -> int:
        """Returns the amount of attributes for this blueprint."""
        ...

    def __str__(self) -> str: ...
    # endregion


class ActorList():
    """A class that contains every actor present on the scene and provides access to them.

    The list is automatically created and updated by the server and it can be returned using `carla.World`.
    """

    # region Methods
    def filter(self, wildcard_pattern: str) -> list:
        """Filters a list of Actors matching wildcard_pattern against their variable `type_id` (which identifies the blueprint used to spawn them). Matching follows fnmatch standard.

        Args:
            `wildcard_pattern (str)`\n

        Returns:
            `list`\n
        """
        ...

    def find(self, actor_id: int) -> Actor:
        """Finds an actor using its identifier and returns it or None if it is not present.

        Args:
            `actor_id (int)`\n

        Returns:
            `Actor`\n
        """
        ...
    # endregion

    # region Dunder methods
    def __getitem__(self, pos: int) -> Actor:
        """Returns the actor corresponding to pos position in the list."""
        ...

    def __iter__(self):
        """Iterate over the `carla.Actor` contained in the list."""
        ...

    def __len__(self) -> int:
        """Returns the amount of actors listed."""
        ...

    def __str__(self) -> str:
        """Parses to the ID for every actor listed."""
        ...
    # endregion


class ActorSnapshot():
    """A class that comprises all the information for an actor at a certain moment in time. These objects are contained in a `carla.WorldSnapshot` and sent to the client once every tick.
    """

    # region Instance Variables
    @property
    def id() -> int:
        """An identifier for the snapshot itself."""
        ...
    # endregion

    # region Getters
    def get_acceleration(self) -> Vector3D:
        """Returns the acceleration vector registered for an actor in that tick.

        Returns:
            `Vector3D`: m/s^2\n
        """
        ...

    def get_angular_velocity(self) -> Vector3D:
        """Returns the angular velocity vector registered for an actor in that tick.

        Returns:
            `Vector3D`: rad/s\n
        """
        ...

    def get_transform(self) -> Transform:
        """Returns the actor's transform (location and rotation) for an actor in that tick.

        Returns:
            `Transform`\n
        """
        ...

    def get_velocity(self) -> Vector3D:
        """Returns the velocity vector registered for an actor in that tick.

        Returns:
            `Vector3D`: m/s\n
        """
        ...
    # endregion


class ActorState(Enum):
    """Class that defines the state of an actor."""
    # region Instance Variables
    Invalid = 0,
    """An actor is Invalid if a problem has occurred."""
    Active = 1,
    """An actor is Active when it visualized and can affect other actors."""
    Dormant = 2,
    """An actor is Dormant when it is not visualized and will not affect other actors through physics. For example, actors are dormant if they are on an unloaded tile in a large map."""
    # endregion


class AttachmentType(Enum):
    """Class that defines attachment options between an actor and its parent. When spawning actors, these can be attached to another actor so their position changes accordingly. This is specially useful for sensors. The snipet in `carla.World.spawn_actor` shows some sensors being attached to a car when spawned.

    + Note that the attachment type is declared as an enum within the class."""

    # region Instance Variables
    Rigid = 0,
    """With this fixed attachment the object follow its parent position strictly. This is the recommended attachment to retrieve precise data from the simulation."""
    SpringArm = 1,
    """An attachment that expands or retracts the position of the actor, depending on its parent. This attachment is only recommended to record videos from the simulation where a smooth movement is needed. SpringArms are an Unreal Engine component so check the UE docs to learn more about them.

    https://docs.unrealengine.com/5.3/en-US/using-spring-arm-components-in-unreal-engine/

    + Warning: The `SpringArm` attachment presents weird behaviors when an actor is spawned with a relative translation in the Z-axis (e.g. `child_location = Location(0,0,2)`).
        """

    SpringArmGhost = 2,
    """An attachment like the previous one but that does not make the collision test, and that means that it does not expands or retracts the position of the actor. The term ghost is because then the camera can cross walls and other geometries. This attachment is only recommended to record videos from the simulation where a smooth movement is needed. SpringArms are an Unreal Engine component so check the UE docs to learn more about them.

    https://docs.unrealengine.com/5.3/en-US/using-spring-arm-components-in-unreal-engine/

    + Warning: The `SpringArm` attachment presents weird behaviors when an actor is spawned with a relative translation in the Z-axis (e.g. `child_location = Location(0,0,2)`)."""
    # endregion


class BlueprintLibrary():
    """A class that contains the blueprints provided for actor spawning. Its main application is to return `carla.ActorBlueprint` objects needed to spawn actors. Each blueprint has an identifier and attributes that may or may not be modifiable. The library is automatically created by the server and can be accessed through `carla.World`.

    Here is a reference containing every available blueprint and its specifics.
    """

    # region Methods
    def filter(self, wildcard_pattern: str) -> BlueprintLibrary:
        """Filters a list of blueprints matching the `wildcard_pattern` against the id and tags of every blueprint contained in this library and returns the result as a new one. Matching follows fnmatch standard.

        Args:
            `wildcard_pattern (str)`\n

        Returns:
            `BlueprintLibrary`\n
        """

    def filter_by_attribute(self, name: str, value: str) -> BlueprintLibrary:
        """Filters a list of blueprints with a given attribute matching the `value` against every blueprint contained in this library and returns the result as a new one. Matching follows fnmatch standard.

        Args:
            `name (str)`\n
            `value (str)`\n

        Returns:
            `ActorBlueprint`\n
        """

    def find(self, id: str) -> ActorBlueprint:
        """Returns the blueprint corresponding to that identifier.

        Args:
            `id (str)`\n

        Returns:
            `ActorBlueprint`\n
        """
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos: int) -> ActorBlueprint:
        """Returns the blueprint stored in `pos` position inside the data structure containing them.
        """

    def __iter__(self):
        """Iterate over the `carla.ActorBlueprint` stored in the library."""

    def __len__(self):
        """Returns the amount of blueprints comprising the library."""

    def __str__(self) -> str:
        """Parses the identifiers for every blueprint to string."""
        ...
    # endregion


class BoundingBox():
    """Bounding boxes contain the geometry of an actor or an element in the scene. They can be used by `carla.DebugHelper` or a `carla.Client` to draw their shapes for debugging. Check out the snipet in `carla.DebugHelper.draw_box` where a snapshot of the world is used to draw bounding boxes for traffic lights.
    """

    # region Instance Variables
    @property
    def extent() -> Vector3D:
        """Vector from the center of the box to one vertex. The value in each axis equals half the size of the box for that axis. `extent.x * 2` would return the size of the box in the X-axis.

        Returns:
            `Vector3D`: meters\n
        """
    @property
    def location() -> Location:
        """The center of the bounding box.

        Returns:
            `Location`: meters\n
        """

    @property
    def rotation() -> Rotation:
        """The orientation of the bounding box."""
    # endregion

    # region Methods
    def __init__(self, location: Location, extent: Vector3D) -> BoundingBox:
        """Bounding boxes contain the geometry of an actor or an element in the scene. They can be used by `carla.DebugHelper` or a `carla.Client` to draw their shapes for debugging. Check out the snipet in `carla.DebugHelper.draw_box` where a snapshot of the world is used to draw bounding boxes for traffic lights.

        Args:
            `location (Location)`: Center of the box, relative to its parent.\n
            `extent (Vector3D)`: Vector containing half the size of the box for every axis.\n

        Returns:
            `BoundingBox`\n
        """
        ...

    def contains(self, world_point: Location, transform: Transform) -> bool:
        """Returns `True` if a point passed in world space is inside this bounding box.


        Args:
            `world_point (Location)`: meters. The point in world space to be checked.\n
            `transform (Transform)`: Contains location and rotation needed to convert this object's local space to world space.\n

        Returns:
            `bool`\n
        """
    # endregion

    # region Getters
    def get_local_vertices(self) -> list[Location]:
        """Returns a list containing the locations of this object's vertices in local space."""
        ...

    def get_world_vertices(self, transform: Transform) -> list[Location]:
        """Returns a list containing the locations of this object's vertices in world space.

        Args:
            `transform (Transform)`: Contains location and rotation needed to convert this object's local space to world space.\n
        """
        ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: BoundingBox) -> bool:
        """Returns true if both location and extent are equal for this and `other`."""

    def __ne__(self, __value: BoundingBox) -> bool:
        """Returns true if either location or extent are different for this and `other`."""

    def __str__(self) -> str:
        """Parses the location and extent of the bounding box to string."""
    # endregion


class CityObjectLabel(Enum):
    """Enum declaration that contains the different tags available to filter the bounding boxes returned by carla.World.get_level_bbs(). These values correspond to the semantic tag that the elements in the scene have."""
    None,
    Buildings = 1,
    Fences = 2,
    Other = 3,
    Pedestrians = 4,
    Poles = 5,
    RoadLines = 6,
    Roads = 7,
    Sidewalks = 8,
    TrafficSigns = 9,
    Vegetation = 10,
    Vehicles = 11,
    Walls = 12,
    Sky = 13,
    Ground = 14,
    Bridge = 15,
    RailTrack = 16,
    GuardRail = 17,
    TrafficLight = 18,
    Static = 19,
    Dynamic = 20,
    Water = 21,
    Terrain = 22,
    Any = 23


class Client():
    """The Client connects CARLA to the server which runs the simulation. Both server and client contain a CARLA library (libcarla) with some differences that allow communication between them. Many clients can be created and each of these will connect to the RPC server inside the simulation to send commands. The simulation runs server-side. Once the connection is established, the client will only receive data retrieved from the simulation. Walkers are the exception. The client is in charge of managing pedestrians so, if you are running a simulation with multiple clients, some issues may arise. For example, if you spawn walkers through different clients, collisions may happen, as each client is only aware of the ones it is in charge of.

    The client also has a recording feature that saves all the information of a simulation while running it. This allows the server to replay it at will to obtain information and experiment with it. Here is some information about how to use this recorder.

    https://carla.readthedocs.io/en/latest/adv_recorder/
    """
    # region Methods

    def __init__(self, host="127.0.0.1", port=2000, worker_threads=0) -> Client:
        """Client constructor.

        Args:
            `host (str, optional)`: IP address where a CARLA Simulator instance is running. Defaults to "127.0.0.1".\n
            `port (int, optional)`: TCP port where the CARLA Simulator instance is running. Defaults to 2000 and the subsequent 2001.\n
            `worker_threads (int, optional)`: Number of working threads used for background updates. If 0, use all available concurrency. Defaults to 0.\n

        Returns:
            `Client`: _description_\n
        """

    def apply_batch(self, commands: list[command]):
        """Executes a list of commands on a single simulation step and retrieves no information.

        If you need information about the response of each command, use the `apply_batch_sync()` method. Here is an example on how to delete the actors that appear in `carla.ActorList` all at once.

        https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/generate_traffic.py

        Args:
            `commands (list)`: A list of commands to execute in batch. Each command is different and has its own parameters. \n
        """

    def apply_batch_sync(self, commands: list[command], due_tick_cue=False) -> list[command.Response]:
        """Executes a list of commands on a single simulation step, blocks until the commands are linked, and returns a list of command.Response that can be used to determine whether a single command succeeded or not. Here is an example of it being used to spawn actors.

        https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/generate_traffic.py

        Args:
            `commands (list[command])`: A list of commands to execute in batch. The commands available are listed right above, in the method `apply_batch()`.\n
            `due_tick_cue (bool, optional)`: A boolean parameter to specify whether or not to perform a `carla.World.tick` after applying the batch in synchronous mode. Defaults to False.\n

        Returns:
            `list[command.Response]`\n
        """

    def generate_opendrive_world(self, opendrive: str, parameters: OpendriveGenerationParameters = (2.0, 50.0, 1.0, 0.6, True, True), reset_settings=True):
        """Loads a new world with a basic 3D topology generated from the content of an OpenDRIVE file. This content is passed as a `string` parameter. It is similar to `client.load_world(map_name)` but allows for custom OpenDRIVE maps in server side. Cars can drive around the map, but there are no graphics besides the road and sidewalks.

        Args:
            `opendrive (str)`: Content of an OpenDRIVE file as `string`, not the path to the `.xodr`.\n
            `parameters (OpendriveGenerationParameters, optional)`: Additional settings for the mesh generation. Defaults to (2.0, 50.0, 1.0, 0.6, True, True).\n
            `reset_settings (bool, optional)`: Option to reset the episode setting to default values, set to false to keep the current settings. This is useful to keep sync mode when changing map and to keep deterministic scenarios. Defaults to True.\n
        """

    def load_world(self, map_name: str, reset_settings=True, map_layers=MapLayer.All):
        """Creates a new world with default settings using `map_name` map. All actors in the current world will be destroyed.

        + Warning: `map_layers` are only available for "Opt" maps

        Args:
            `map_name (str)`: Name of the map to be used in this world. Accepts both full paths and map names,e.g. `'/Game/Carla/Maps/Town01'` or `'Town01'`. Remember that these paths are dynamic.\n
            `reset_settings (bool, optional)`: Option to reset the episode setting to default values, set to false to keep the current settings. This is useful to keep sync mode when changing map and to keep deterministic scenarios. Defaults to True.\n
            `map_layers (MapLayer, optional)`: Layers of the map that will be loaded. This parameter works like a flag mask. Defaults to MapLayer.All.\n
        """

    def reload_world(self, reset_settings=True):
        """Reload the current world, note that a new world is created with default settings using the same map. All actors present in the world will be destroyed, but traffic manager instances will stay alive.

        Args:
            `reset_settings (bool, optional)`: Option to reset the episode setting to default values, set to false to keep the current settings. This is useful to keep sync mode when changing map and to keep deterministic scenarios. Defaults to True.\n

        Raises:
            `RuntimeError` when corresponding.
        """

    def replay_file(self, name: str, start: float, duration: float, follow_id: int, replay_sensors: bool):
        """Load a new world with default settings using `map_name` map. All actors present in the current world will be destroyed, but traffic manager instances will stay alive.

        Args:
            `name (str)`: Name of the file containing the information of the simulation.\n
            `start (float - seconds)`: Time where to start playing the simulation. Negative is read as beginning from the end, being -10 just 10 seconds before the recording finished.\n
            `duration (float  - seconds)`: Time that will be reenacted using the information `name` file. If the end is reached, the simulation will continue.\n
            `follow_id (int)`: ID of the actor to follow. If this is 0 then camera is disabled.\n
            `replay_sensors (bool)`: Flag to enable or disable the spawn of sensors during playback.\n
        """

        def request_file(self, name: str):
            """Requests one of the required files returned by `carla.Client.get_required_files`.

            Args:
                `name (str)`: Name of the file you are requesting.\n
            """

        def show_recorder_actors_blocked(self, filename: str, min_time: float, min_distance: float) -> str:
            """The terminal will show the information registered for actors considered blocked. An actor is considered blocked when it does not move a minimum distance in a period of time, being these `min_distance` and `min_time`.

            Args:
                `filename (str)`: Name of the recorded file to load.\n
                `min_time (float - seconds)`: Minimum time the actor has to move a minimum distance before being considered blocked. Default is 60 seconds.\n
                `min_distance (float - centimeters)`: Minimum distance the actor has to move to not be considered blocked. Default is 100 centimeters.\n

            Returns:
                `str`\n
            """

        def show_recorder_collisions(self, filename: str, category1: str, category2: str) -> str:
            """The terminal will show the collisions registered by the recorder. These can be filtered by specifying the type of actor involved. The categories will be specified in `category1` and `category2` as follows: 'h' = Hero, the one vehicle that can be controlled manually or managed by the user. 'v' = Vehicle 'w' = Walker 't' = Traffic light 'o' = Other 'a' = Any If you want to see only collisions between a vehicles and a walkers, use for `category1` as 'v' and `category2` as 'w' or vice versa. If you want to see all the collisions (filter off) you can use 'a' for both parameters.

            Args:
                `filename (str)`: Name or absolute path of the file recorded, depending on your previous choice.\n
                `category1 (str)`: Character variable specifying a first type of actor involved in the collision.\n
                `category2 (str)`: Character variable specifying the second type of actor involved in the collision.\n

            Returns:
                `str`\n
            """

        def show_recorder_file_info(self, filename: str, show_all: bool) -> str:
            """The information saved by the recorder will be parsed and shown in your terminal as text (frames, times, events, state, positions...). The information shown can be specified by using the `show_all` parameter. Here is some more information about how to read the recorder file.

            https://carla.readthedocs.io/en/latest/ref_recorder_binary_file_format/

            Args:
                `filename (str)`: Name or absolute path of the file recorded, depending on your previous choice.\n
                `show_all (bool)`: If `True`, returns all the information stored for every frame (traffic light states, positions of all actors, orientation and animation data...). If `False`, returns a summary of key events and frames.\n

            Returns:
                `str`\n
            """

        def start_recorder(self, filename: str, additional_data=False):
            """Enables the recording feature, which will start saving every information possible needed by the server to replay the simulation.

            Args:
                `filename (str)`: Name of the file to write the recorded data. A simple name will save the recording in 'CarlaUE4/Saved/recording.log'. Otherwise, if some folder appears in the name, it will be considered an absolute path.\n
                `additional_data (bool, optional)`: Enables or disable recording non-essential data for reproducing the simulation (bounding box location, physics control parameters, etc). Defaults to False.\n
            """

        def stop_recorder(self):
            """Stops the recording in progress. If you specified a path in `filename`, the recording will be there. If not, look inside `CarlaUE4/Saved/`."""

        def stop_replayer(self, keep_actors: bool):
            """Stop current replayer.

            Args:
                `keep_actors (bool)`: `True` if you want autoremove all actors from the replayer, or `False` to keep them.\n
            """
    # endregion

    # region Getters
    def get_available_maps(self) -> list[str]:
        """Returns a list of strings containing the paths of the maps available on server. These paths are dynamic, they will be created during the simulation and so you will not find them when looking up in your files. One of the possible returns for this method would be: 

        >>> ['/Game/Carla/Maps/Town01', '/Game/Carla/Maps/Town02', '/Game/Carla/Maps/Town03', '/Game/Carla/Maps/Town04', '/Game/Carla/Maps/Town05', '/Game/Carla/Maps/Town06', '/Game/Carla/Maps/Town07']
        """

    def get_client_version(self) -> str:
        """Returns the client libcarla version by consulting it in the "Version.h" file. Both client and server can use different libcarla versions but some issues may arise regarding unexpected incompatibilities.
        """

    def get_required_files(self, folder: str, download=True):
        """Asks the server which files are required by the client to use the current map. Option to download files automatically if they are not already in the cache.

        Args:
            `folder (str)`: Folder where files required by the client will be downloaded to.\n
            `download (bool, optional)`: If `True`, downloads files that are not already in cache.. Defaults to True.\n
        """

    def get_server_version(self) -> str:
        """Returns the server libcarla version by consulting it in the "Version.h" file. Both client and server should use the same libcarla version.
        """

    def get_trafficmanager(self, client_connection=8000) -> TrafficManager:
        """Returns an instance of the traffic manager related to the specified port. If it does not exist, this will be created.

        Args:
            `client_connection (int, optional)`: Port that will be used by the traffic manager.. Defaults to 8000.\n
        """

    def get_world(self) -> World:
        """Returns the world object currently active in the simulation. This world will be later used for example to load maps.
        """
    # endregion

    # region Setters
    def set_files_base_folder(self, path: str):
        """Specifies the base folder where the local cache for required files will be placed.

        Args:
            `path (str)`: Specifies the base folder where the local cache for required files will be placed.\n
        """

    def set_replayer_ignore_hero(self, ignore_hero: bool):
        """Enables or disables playback of the hero vehicle during a playback of a recorded simulation.

        Args:
            `ignore_hero (bool)`: Enables or disables playback of the hero vehicle during a playback of a recorded simulation.\n
        """

    def set_replayer_ignore_spectator(self, ignore_spectator: bool):
        """Determines whether the recorded spectator movements will be replicated by the replayer.

        Args:
            `ignore_spectator (bool)`: Determines whether the recorded spectator movements will be replicated by the replayer.\n
        """

    def set_replayer_time_factor(self, time_factor=1.0):
        """When used, the time speed of the reenacted simulation is modified at will. It can be used several times while a playback is in curse.

        Args:
            `time_factor (float, optional)`: 1.0 means normal time speed. Greater than 1.0 means fast motion (2.0 would be double speed) and lesser means slow motion (0.5 would be half speed). Defaults to 1.0.\n
        """

    def set_timeout(self, second: float):
        """Sets the maximum time a network call is allowed before blocking it and raising a timeout exceeded error.

        Args:
            `second (float - seconds)`: New timeout value. Default is 5 seconds.\n
        """

    # endregion


class CollisionEvent(SensorData):
    """Class that defines a collision data for sensor.other.collision. The sensor creates one of these for every collision detected. Each collision sensor produces one collision event per collision per frame. Multiple collision events may be produced in a single frame by collisions with multiple other actors. Learn more about this here.

    https://carla.readthedocs.io/en/latest/ref_sensors/#collision-detector
    """
    # region Instance Variables
    @property
    def actor() -> Actor:
        """The actor the sensor is attached to, the one that measured the collision."""
    @property
    def other_actor() -> Actor:
        """The second actor involved in the collision."""
    @property
    def normal_impulse() -> Vector3D:
        """Normal impulse resulting of the collision.(N*s)"""
    # endregion


class Color():
    """Class that defines a 32-bit RGBA color."""

    # region Instance Variables
    @property
    def r() -> int:
        """Red color (0-255)."""
    @property
    def g() -> int:
        """Green color (0-255)."""
    @property
    def b() -> int:
        """Blue color (0-255)."""
    @property
    def a() -> int:
        """Alpha channel (0-255)."""
    # endregion

    # region Methods
    def __init__(self, r=0, g=0, b=0, a=255) -> Color:
        """Initializes a color, black by default.

        Args:
            `r (int, optional)`: Red color (0-255). Defaults to 0.\n
            `g (int, optional)`: Green color (0-255). Defaults to 0.\n
            `b (int, optional)`: Blue color (0-255). Defaults to 0.\n
            `a (int, optional)`: Alpha channel (0-255). Defaults to 255.\n
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: Color) -> bool: ...
    def __ne__(self, __value: Color) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class ColorConverter(Enum):
    """Class that defines conversion patterns that can be applied to a `carla.Image` in order to show information provided by `carla.Sensor`. Depth conversions cause a loss of accuracy, as sensors detect depth as float that is then converted to a grayscale value between 0 and 255. Take a look at the snipet in `carla.Sensor.listen` to see an example of how to create and save image data for `sensor.camera.semantic_segmentation`.
    """

    # region Instance Variables
    Raw = 0,
    """No changes applied to the image. Used by the `RGB camera`."""
    Depth = 1,
    """Converts the image to a linear depth map. Used by the `depth camera`."""
    LogarithmicDepth = 2,
    """Converts the image to a depth map using a logarithmic scale, leading to better precision for small distances at the expense of losing it when further away."""
    CityScapesPalette = 3,
    """Converts the image to a segmented map using tags provided by the blueprint library. Used by the `semantic segmentation camera`."""
    # endregion


class DVSEvent():
    """Class that defines a DVS event. An event is a quadruple, so a tuple of 4 elements, with `x`, `y` pixel coordinate location, timestamp `t` and polarity `pol` of the event. Learn more about them here.

    https://carla.readthedocs.io/en/latest/ref_sensors/
    """

    # region Instance Variables
    @property
    def x() -> int:
        """X pixel coordinate."""
    @property
    def x() -> int:
        """Y pixel coordinate."""
    @property
    def t() -> int:
        """Timestamp of the moment the event happened."""
    @property
    def pol() -> bool:
        """Polarity of the event. True for positive and False for negative."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class DVSEventArray():
    """Class that defines a stream of events in `carla.DVSEvent`. Such stream is an array of arbitrary size depending on the number of events. This class also stores the field of view, the height and width of the image and the timestamp from convenience. Learn more about them here.

    https://carla.readthedocs.io/en/latest/ref_sensors/
    """

    # region Instance Variables
    @property
    def fov() -> float:
        """Horizontal field of view of the image."""
    @property
    def height() -> int:
        """Image height in pixels."""
    @property
    def width() -> int:
        """Image width in pixels."""
    @property
    def raw_data() -> bytes: ...
    # endregion

    # region Methods
    def to_array(self) -> Iterable:
        """Converts the stream of events to an array of int values in the following order `[x, y, t, pol]`."""

    def to_array_pol(self) -> Iterable:
        """Returns an array with the polarity of all the events in the stream."""

    def to_array_t(self) -> Iterable:
        """Returns an array with the timestamp of all the events in the stream."""

    def to_array_x(self) -> Iterable:
        """Returns an array with X pixel coordinate of all the events in the stream."""

    def to_array_y(self) -> Iterable:
        """Returns an array with Y pixel coordinate of all the events in the stream."""

    def to_image(self) -> Image:
        """Converts the image following this pattern: blue indicates positive events, red indicates negative events."""
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos: int): ...

    def __iter__(self):
        """Iterate over the `carla.DVSEvent` retrieved as data."""

    def __len__(self): ...
    def __setitem(self, pos: int, color: Color): ...
    def __str__(self) -> str: ...
    # endregion


class DebugHelper():
    """Helper class part of `carla.World` that defines methods for creating debug shapes. By default, shapes last one second. They can be permanent, but take into account the resources needed to do so. Take a look at the snipets available for this class to learn how to debug easily in CARLA."""

    # region Methods
    def draw_arrow(self, begin: Location, end: Location, thickness=0.1, arrow_size=0.1, color: Color = (255, 0, 0), life_time=-1.0):
        """Draws an arrow from `begin` to `end` pointing in that direction.

        Args:
            begin (Location): Point in the coordinate system where the arrow starts (meters).
            end (Location): Point in the coordinate system where the arrow ends and points towards to (meters).
            thickness (float, optional): Density of the line (meters). Defaults to 0.1.
            arrow_size (float, optional): Size of the tip of the arrow (meters). Defaults to 0.1.
            color (Color, optional): RGB code to color the object. Defaults to (255,0,0).
            life_time (float, optional): Shape's lifespan. By default it only lasts one frame. Set this to `0` for permanent shapes (seconds). Defaults to -1.0.
        """
        ...

    def draw_box(self, box: BoundingBox, rotation: Rotation, thickness=0.1, color: Color = (255, 0, 0), life_time=-1.0):
        """Draws a box, ussually to act for object colliders.

        Args:
            box (BoundingBox): Object containing a location and the length of a box for every axis.
            rotation (Rotation): Orientation of the box according to Unreal Engine's axis system (degrees (pitch,yaw,roll)).
            thickness (float, optional): Density of the lines that define the box (meters). Defaults to 0.1.
            color (Color, optional): RGB code to color the object. Defaults to (255,0,0).
            life_time (float, optional): Shape's lifespan. By default it only lasts one frame. Set this to `0` for permanent shapes. Defaults to -1.0.
        """
        ...

    def draw_line(self, begin: Location, end: Location, thickness=0.1, color: Color = (255, 0, 0), life_time=-1.0):
        """Draws a line in between `begin` and `end`.

        Args:
            begin (Location): Point in the coordinate system where the line starts (meters).
            end (Location): Spot in the coordinate system where the line ends (meters).
            thickness (float, optional): Density of the line. Defaults to 0.1.
            color (Color, optional): RGB code to color the object. Defaults to (255,0,0).
            life_time (float, optional): Shape's lifespan. By default it only lasts one frame. Set this to `0` for permanent shapes. Defaults to -1.0.
        """
        ...

    def draw_point(self, location: Location, size=0.1, color=(255, 0, 0), life_time=-1.0):
        """Draws a point location.

        Args:
            location (Location): Spot in the coordinate system to center the object (meters).
            size (float, optional): Density of the point (meters). Defaults to 0.1.
            color (tuple, optional): RGB code to color the object. Defaults to (255,0,0).
            life_time (float, optional): Shape's lifespan. By default it only lasts one frame. Set this to 0 for permanent shapes (seconds). Defaults to -1.0.
        """
        ...

    def draw_string(self, location: Location, text: str, draw_shadow=False, color: Color = (255, 0, 0), life_time=-1.0):
        """Draws a string in a given location of the simulation which can only be seen server-side.

        Args:
            location (Location): Spot in the simulation where the text will be centered (meters).
            text (str): Text intended to be shown in the world.
            draw_shadow (bool, optional): Casts a shadow for the string that could help in visualization. It is disabled by default. Defaults to False.
            color (Color, optional): RGB code to color the string. Defaults to (255,0,0).
            life_time (float, optional): Shape's lifespan. By default it only lasts one frame. Set this to `0` for permanent shapes (seconds). Defaults to -1.0.
        """
        ...
    # endregion


class EnvironmentObject():
    """Class that represents a geometry in the level, this geometry could be part of an actor formed with other EnvironmentObjects (i.e.: buildings)."""

    # region Instance Variables
    @property
    def transform() -> Transform:
        """Contains the location and orientation of the EnvironmentObject in world space."""
        ...

    @property
    def bounding_box() -> BoundingBox:
        """Object containing a location, rotation and the length of a box for every axis in world space."""
        ...

    @property
    def id() -> int:
        """Unique ID to identify the object in the level."""
        ...

    @property
    def name() -> str:
        """Name of the EnvironmentObject."""
        ...

    @property
    def type() -> CityObjectLabel:
        """Semantic tag."""
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str:
        """Parses the EnvironmentObject to a string and shows them in command line."""
        ...
    # endregion


class FloatColor():
    """Class that defines a float RGBA color."""

    # region Instance Variables
    @property
    def r() -> float:
        """Red color."""
        ...

    @property
    def g() -> float:
        """Green color."""
        ...

    @property
    def b() -> float:
        """Blue color."""
        ...

    @property
    def a() -> float:
        """Alpha channel."""
        ...
    # endregion

    # region Methods
    def __init__(self, r=.0, g=.0, b=.0, a=1.0) -> FloatColor:
        """Initializes a color, black by default.

        Args:
            r (float, optional): Red color. Defaults to .0.
            g (float, optional): Green color. Defaults to .0.
            b (float, optional): Blue color. Defaults to .0.
            a (float, optional): Alpha channel. Defaults to 1.0.

        Returns:
            FloatColor
        """
        ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: FloatColor) -> bool: ...
    def __ne__(self, __value: FloatColor) -> bool: ...
    # endregion


class GBufferTextureID(Enum):
    """Defines the identifiers of each GBuffer texture (See the method `carla.Sensor.listen_to_gbuffer`)."""

    # region Instance Variables
    SceneColor = 0,
    """The texture "SceneColor" contains the final color of the image."""
    SceneDepth = 1,
    """The texture "SceneDepth" contains the depth buffer - linear in world units."""
    SceneStencil = 2,
    """The texture "SceneStencil" contains the stencil buffer."""
    GBufferA = 3,
    """The texture "GBufferA" contains the world-space normal vectors in the RGB channels. The alpha channel contains "per-object data"."""
    GBufferB = 4,
    """The texture "GBufferB" contains the metallic, specular and roughness in the RGB channels, respectively. The alpha channel contains a mask where the lower 4 bits indicate the shading model and the upper 4 bits contain the selective output mask."""
    GBufferC = 5,
    """The texture "GBufferC" contains the diffuse color in the RGB channels, with the indirect irradiance in the alpha channel.
    
    If static lightning is not allowed, the alpha channel will contain the ambient occlusion instead."""
    GBufferD = 6,
    """
    The contents of the "GBufferD" varies depending on the rendered object's material shading model (GBufferB):
        MSM_Subsurface (2), MSM_PreintegratedSkin (3), MSM_TwoSidedFoliage (6):
        RGB: Subsurface color.
        A: Opacity.
        MSM_ClearCoat (4):
        R: Clear coat.
        G: Roughness.
        MSM_SubsurfaceProfile (5):
        RGB: Subsurface profile.
        MSM_Hair (7):
        RG: World normal.
        B: Backlit value.
        MSM_Cloth (8):
        RGB: Subsurface color.
        A: Cloth value.
        MSM_Eye (9):
        RG: Eye tangent.
        B: Iris mask.
        A: Iris distance.
    """
    GBufferE = 7,
    """The texture "GBufferE" contains the precomputed shadow factors in the RGBA channels. This texture is unavailable if the selective output mask (GBufferB) does not have its 4th bit set."""
    GBufferF = 8,
    """The texture "GBufferF" contains the world-space tangent in the RGB channels and the anisotropy in the alpha channel. This texture is unavailable if the selective output mask (GBufferB) does not have its 5th bit set."""
    Velocity = 9,
    """The texture "Velocity" contains the screen-space velocity of the scene objects."""
    SSAO = 10,
    """The texture "SSAO" contains the screen-space ambient occlusion texture."""
    CustomDepth = 11,
    """The texture "CustomDepth" contains the Unreal Engine custom depth data."""
    CustomStencil = 12,
    """The texture "CustomStencil" contains the Unreal Engine custom stencil data."""
    # endregion


class GearPhysicsControl():
    """Class that provides access to vehicle transmission details by defining a gear and when to run on it. This will be later used by `carla.VehiclePhysicsControl` to help simulate physics.
    """

    # region Instance Variables
    def ratio() -> float:
        """The transmission ratio of the gear."""
        ...

    def down_ratio() -> float:
        """Quotient between current RPM and MaxRPM where the autonomous gear box should shift down."""
    def up_ratio() -> float:
        """Quotient between current RPM and MaxRPM where the autonomous gear box should shift up."""
    # endregion

    # region Methods
    def __init__(self, ratio=1.0, down_ratio=0.5, up_ratio=0.65) -> GearPhysicsControl:
        """Class that provides access to vehicle transmission details by defining a gear and when to run on it. This will be later used by `carla.VehiclePhysicsControl` to help simulate physics.

        Args:
            `ratio (float, optional)`: The transmission ratio of the gear. Defaults to 1.0.\n
            `down_ratio (float, optional)`: Quotient between current RPM and MaxRPM where the autonomous gear box should shift down. Defaults to 0.5.\n
            `up_ratio (float, optional)`: Quotient between current RPM and MaxRPM where the autonomous gear box should shift up. Defaults to 0.65.\n

        Returns:
            `GearPhysicsControl`\n
        """
        ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: GearPhysicsControl) -> bool: ...
    def __ne__(self, __value: GearPhysicsControl) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class GeoLocation():
    """Class that contains geographical coordinates simulated data. The `carla.Map` can convert simulation locations by using the tag in the OpenDRIVE file."""

    # region Instance Variables
    @property
    def latitude() -> float:
        """North/South value of a point on the map (degrees)."""
        ...

    @property
    def longitude() -> float:
        """West/East value of a point on the map (degrees)."""
        ...

    @property
    def altitude() -> float:
        """Height regarding ground level (meters)."""
        ...
    # endregion

    # region Methods
    def __init__(self, latitude=0.0, longitude=0.0, altitude=0.0) -> GeoLocation:
        """Class that contains geographical coordinates simulated data. The `carla.Map` can convert simulation locations by using the tag in the OpenDRIVE file.

        Args:
            `latitude (float, optional)`: North/South value of a point on the map (degrees). Defaults to 0.0.\n
            `longitude (float, optional)`: West/East value of a point on the map (degrees). Defaults to 0.0.\n
            `altitude (float, optional)`: Height regarding ground level (meters). Defaults to 0.0.\n
        """
        ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: GeoLocation) -> bool: ...
    def __ne__(self, __value: GeoLocation) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class GnssMeasurement(SensorData):
    """Class that defines the Gnss data registered by a `sensor.other.gnss`. It essentially reports its position with the position of the sensor and an OpenDRIVE geo-reference."""

    # region Instance Variables
    @property
    def altitude() -> float:
        """Height regarding ground level (meters)."""
        ...

    @property
    def latitude() -> float:
        """North/South value of a point on the map (degrees)."""
        ...

    @property
    def longitude() -> float:
        """West/East value of a point on the map (degrees)."""
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class IMUMeasurement(SensorData):
    """Class that defines the data registered by a `sensor.other.imu`, regarding the sensor's transformation according to the current `carla.World`. It essentially acts as accelerometer, gyroscope and compass."""

    # region Instance Variables
    @property
    def accelerometer() -> Vector3D:
        """Linear acceleration (m/s2)."""
        ...

    @property
    def compass() -> float:
        """Orientation with regard to the North ([0.0, -1.0, 0.0] in Unreal Engine) (radians)."""
        ...

    @property
    def gyroscope() -> Vector3D:
        """Angular velocity. (rad/s)"""
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class Image(SensorData):
    """Class that defines an image of 32-bit BGRA colors that will be used as initial data retrieved by camera sensors. There are different camera sensors (currently three, RGB, depth and semantic segmentation) and each of these makes different use for the images. Learn more about them here.

    https://carla.readthedocs.io/en/latest/ref_sensors/"""

    # region Instance Variables
    @property
    def fov() -> float:
        """Horizontal field of view of the image (degrees)."""
        ...

    @property
    def height() -> int:
        """Image height in pixels."""
        ...

    @property
    def width() -> int:
        """Image width in pixels."""
        ...

    @property
    def raw_data() -> bytes:
        """Flattened array of pixel data, use reshape to create an image array."""
        ...
    # endregion

    # region Methods
    def convert(self, color_converter: ColorConverter):
        """Converts the image following the `color_converter` pattern.

        Args:
            `color_converter (ColorConverter)`\n
        """

    def save_to_disk(self, path: str, color_converter=ColorConverter.Raw):
        """Saves the image to disk using a converter pattern stated as `color_converter`. The default conversion pattern is `Raw` that will make no changes to the image.

        Args:
            `path (str)`:Path that will contain the image.\n
            `color_converter (ColorConverter, optional)`: Default Raw will make no changes. Defaults to ColorConverter.Raw.\n
        """
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos=int) -> Color: ...

    def __iter__(self) -> Iterator[Color]:
        """terate over the `carla.Color` that form the image."""
        ...

    def __len__(self) -> int: ...
    def __setitem__(self, pos: int, color: Color): ...
    def __str__(self) -> str: ...
    # endregion


class Junction():
    """Class that embodies the intersections on the road described in the OpenDRIVE file according to OpenDRIVE 1.4 standards."""

    # region Instance Variables
    def id() -> int:
        """Identifier found in the OpenDRIVE file."""
        ...

    def bounding_box() -> BoundingBox:
        """Bounding box encapsulating the junction lanes."""
        ...
    # endregion

    # region Getters
    def get_waypoints(self, lane_type: LaneType) -> list[tuple[Waypoint, Waypoint]]:
        """Returns a list of pairs of waypoints. Every tuple on the list contains first an initial and then a final waypoint within the intersection boundaries that describe the beginning and the end of said lane along the junction. Lanes follow their OpenDRIVE definitions so there may be many different tuples with the same starting waypoint due to possible deviations, as this are considered different lanes.

        Args:
            lane_type (LaneType): Type of lanes to get the waypoints.

        Returns:
            list[tuple[Waypoint, Waypoint]]
        """
    # endregion


class LabelledPoint():
    """Class that represent a position in space with a semantic label."""

    # region Instance Variables
    # TODO 类型未定义
    @property
    def location() -> Location:
        """Position in 3D space."""
        ...

    @property
    def label():
        """Semantic tag of the point."""
        ...
    # endregion


class Landmark():
    """Class that defines any type of traffic landmark or sign affecting a road. These class mediates between the OpenDRIVE 1.4 standard definition of the landmarks and their representation in the simulation. This class retrieves all the information defining a landmark in OpenDRIVE and facilitates information about which lanes does it affect and when. Landmarks will be accessed by `carla.Waypoint objects` trying to retrieve the regulation of their lane. Therefore some attributes depend on the waypoint that is consulting the landmark and so, creating the object."""

    # region Instance Variables
    @property
    def road_id() -> int:
        """The OpenDRIVE ID of the road where this landmark is defined. Due to OpenDRIVE road definitions, this road may be different from the road the landmark is currently affecting. It is mostly the case in junctions where the road diverges in different routes.

        Example: a traffic light is defined in one of the divergent roads in a junction, but it affects all the possible routes."""
        ...

    @property
    def distance() -> float:
        """Distance between the landmark and the waypoint creating the object (querying `get_landmarks` or `get_landmarks_of_type`) (meters)."""
        ...

    @property
    def s() -> float:
        """Distance where the landmark is positioned along the geometry of the road `road_id` (meters)."""
        ...

    @property
    def t() -> float:
        """Lateral distance where the landmark is positioned from the edge of the road `road_id` (meters)."""
        ...

    @property
    def id() -> str:
        """Unique ID of the landmark in the OpenDRIVE file."""
        ...

    @property
    def name() -> str:
        """Name of the landmark in the in the OpenDRIVE file."""
        ...

    @property
    def is_dynamic() -> bool:
        """Indicates if the landmark has state changes over time such as traffic lights."""
        ...

    @property
    def orientation() -> LandmarkOrientation:
        """Indicates which lanes the landmark is facing towards to (degrees)."""
        ...

    @property
    def z_offset() -> float:
        """Height where the landmark is placed (meters)."""
        ...

    @property
    def country() -> str:
        """Country code where the landmark is defined (default to OpenDRIVE is Germany 2017)."""
        ...

    @property
    def type() -> str:
        """Type identifier of the landmark according to the country code."""
        ...

    @property
    def sub_type() -> str:
        """Subtype identifier of the landmark according to the country code."""

    @property
    def value() -> float:
        """Value printed in the signal (e.g. speed limit, maximum weight, etc)."""

    @property
    def unit() -> str:
        """Units of measurement for the attribute `value`."""
        ...

    @property
    def height() -> float:
        """Total height of the signal (meters)."""

    @property
    def width() -> float:
        """Total width of the signal (meters)."""

    @property
    def text() -> str:
        """Additional text in the signal."""

    @property
    def h_offset() -> float:
        """Orientation offset of the signal relative to the the definition of `road_id` at `s` in OpenDRIVE (meters)."""
        ...

    @property
    def pitch() -> float:
        """Pitch rotation of the signal (Y-axis in UE coordinates system) (meters)."""

    @property
    def roll() -> float:
        """Roll rotation of the signal (X-axis in UE coordinates system) (meters)."""

    @property
    def waypoint() -> Waypoint:
        """A waypoint placed in the lane of the one that made the query and at the `s` of the landmark. It is the first waypoint for which the landmark will be effective."""

    @property
    def transform() -> Transform:
        """The location and orientation of the landmark in the simulation."""
    # endregion

    # region Getters
    def get_lane_validities(self) -> list[tuple[int, int]]:
        """Returns which lanes the landmark is affecting to. As there may be specific lanes where the landmark is not effective, the return is a list of pairs containing ranges of the `lane_id` affected: 

        Example: In a road with 5 lanes, being 3 not affected: [(from_lane1,to_lane2),(from_lane4,to_lane5)]."""
    # endregion


class LandmarkOrientation(Enum):
    """Helper class to define the orientation of a landmark in the road. The definition is not directly translated from OpenDRIVE but converted for the sake of understanding."""

    # region Instance Variables
    Positive = 0,
    """The landmark faces towards vehicles going on the same direction as the road's geometry definition (lanes 0 and negative in OpenDRIVE)."""
    Negative = 1,
    """The landmark faces towards vehicles going on the opposite direction to the road's geometry definition (positive lanes in OpenDRIVE)."""
    Both = 2,
    """Affects vehicles going in both directions of the road."""
    # endregion


class LandmarkType(Enum):
    """Helper class containing a set of commonly used landmark types as defined by the default country code in the OpenDRIVE standard (Germany 2017). `carla.Landmark` does not reference this class. The landmark type is a string that varies greatly depending on the country code being used. This class only makes it easier to manage some of the most commonly used in the default set by describing them as an enum."""
    # region Instance Variables
    Danger = "101",
    LanesMerging = "121",
    CautionPedestrian = "133",
    CautionBicycle = "138"
    LevelCrossing = "150",
    StopSign = "206",
    YieldSign = "205",
    MandatoryTurnDirection = "209",
    MandatoryLeftRightDirection = "211",
    TwoChoiceTurnDirection = "214",
    Roundabout = "215",
    PassRightLeft = "222",
    AccessForbidden = "250",
    AccessForbiddenMotorvehicles = "251",
    AccessForbiddenTrucks = "253",
    AccessForbiddenBicycle = "254",
    AccessForbiddenWeight = "263",
    AccessForbiddenWidth = "264",
    AccessForbiddenHeight = "265",
    AccessForbiddenWrongDirection = "267",
    ForbiddenUTurn = "272",
    MaximumSpeed = "274",
    ForbiddenOvertakingMotorvehicles = "276",
    ForbiddenOvertakingTrucks = "277",
    AbsoluteNoStop = "283",
    RestrictedStop = "286",
    HasWayNextIntersection = "301",
    PriorityWay = "306",
    PriorityWayEnd = "307",
    CityBegin = "310",
    CityEnd = "311",
    Highway = "330",
    DeadEnd = "357",
    RecomendedSpeed = "380",
    RecomendedSpeedEnd = "381",
    # endregion


class LaneChange(Enum):
    """Class that defines the permission to turn either left, right, both or none (meaning only going straight is allowed). This information is stored for every `carla.Waypoint` according to the OpenDRIVE file. The snipet in `carla.Map.get_waypoint` shows how a waypoint can be used to learn which turns are permitted."""

    # region Instance Variables
    NONE = 0,
    """Traffic rules do not allow turning right or left, only going straight."""
    Right = 1,
    """Traffic rules allow turning right."""
    Left = 2,
    """Traffic rules allow turning left."""
    Both = 3,
    """Traffic rules allow turning either right or left."""
    # endregion


class LaneInvasionEvent(SensorData):
    """Class that defines lanes invasion for `sensor.other.lane_invasion`. It works only client-side and is dependant on OpenDRIVE to provide reliable information. The sensor creates one of this every time there is a lane invasion, which may be more than once per simulation step. Learn more about this here.

    https://carla.readthedocs.io/en/latest/ref_sensors/#lane-invasion-detector
    """

    # region Instance Variables
    def actor() -> Actor:
        """Gets the actor the sensor is attached to, the one that invaded another lane."""

    def crossed_lane_markings() -> list[LaneMarking]:
        """List of lane markings that have been crossed and detected by the sensor."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class LaneMarking():
    """Class that gathers all the information regarding a lane marking according to OpenDRIVE 1.4 standard standard."""

    # region Instance Variables
    @property
    def color() -> LaneMarkingColor:
        """Actual color of the marking."""

    @property
    def lane_change() -> LaneChange:
        """Permissions for said lane marking to be crossed."""

    @property
    def type() -> LaneMarkingType:
        """Lane marking type."""

    @property
    def width() -> float:
        """Horizontal lane marking thickness."""
    # endregion


class LaneMarkingColor(Enum):
    """Class that defines the lane marking colors according to OpenDRIVE 1.4."""

    # region Instance Variables
    White = 0,
    Standard = 0,
    """White by default."""
    Blue = 1,
    Green = 2,
    Red = 3,
    Yellow = 4,
    Other = 5,
    # endregion


class LaneMarkingType(Enum):
    """Class that defines the lane marking types accepted by OpenDRIVE 1.4. The snipet in `carla.Map.get_waypoint` shows how a waypoint can be used to retrieve the information about adjacent lane markings.

    + Note on double types: Lane markings are defined under the OpenDRIVE standard that determines whereas a line will be considered "BrokenSolid" or "SolidBroken". For each road there is a center lane marking, defined from left to right regarding the lane's directions. The rest of the lane markings are defined in order from the center lane to the closest outside of the road.
    """

    # region Instance Variables
    Other = 0,
    Broken = 1,
    Solid = 2,
    SolidSolid = 3,
    SolidBroken = 4,
    BrokenSolid = 5,
    BrokenBroken = 6,
    BottsDots = 7,
    Grass = 8,
    Curb = 9,
    NONE = 0,
    # endregion


class LaneType(Enum):
    """Class that defines the possible lane types accepted by OpenDRIVE 1.4. This standards define the road information. The snipet in `carla.Map.get_waypoint` makes use of a waypoint to get the current and adjacent lane types."""

    # region Instance Variables
    Any = -2,
    NONE = 1,
    """Every type except for `NONE`."""
    Driving = 2,
    Stop = 4,
    Shoulder = 8,
    Biking = 16,
    Sidewalk = 32,
    Border = 64,
    Restricted = 128,
    Parking = 256,
    Bidirectional = 512,
    Median = 1024,
    Special1 = 2048,
    Special2 = 4096,
    Special3 = 8192,
    RoadWorks = 16384,
    Tram = 32768,
    Rail = 65536,
    Entry = 131072,
    Exit = 262144,
    OffRamp = 524288,
    OnRamp = 1048576,
    # endregion


class LidarDetection():
    """Data contained inside a `carla.LidarMeasurement`. Each of these represents one of the points in the cloud with its location and its associated intensity."""
    # region Instance Variables
    @property
    def point() -> Location:
        """Point in xyz coordinates (meters)."""
    @property
    def intensity() -> float:
        """Computed intensity for this point as a scalar value between [0.0 , 1.0]."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class LidarMeasurement(SensorData):
    """Class that defines the LIDAR data retrieved by a `sensor.lidar.ray_cast`. This essentially simulates a rotating LIDAR using ray-casting. Learn more about this here.

    https://carla.readthedocs.io/en/latest/ref_sensors/#lidar-raycast-sensor
    """
    # region Instance Variables
    @property
    def channels() -> int:
        """Number of lasers shot."""

    @property
    def horizontal_angle() -> float:
        """Horizontal angle the LIDAR is rotated at the time of the measurement (radians)."""

    @property
    def raw_data() -> bytes:
        """Received list of 4D points. Each point consists of [x,y,z] coordinates plus the intensity computed for that point."""
    # endregion

    # region Methods
    def save_to_disk(self, path: str):
        """Saves the point cloud to disk as a `.ply` file describing data from 3D scanners. The files generated are ready to be used within `MeshLab`, an open source system for processing said files. Just take into account that axis may differ from Unreal Engine and so, need to be reallocated.

        Args:
            path (str)
        """
    # endregion

    # region Getters
    def get_point_count(self, channel: int):
        """Retrieves the number of points sorted by channel that are generated by this measure. Sorting by channel allows to identify the original channel for every point.

        Args:
            channel (int)
        """
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos: int) -> LidarDetection: ...

    def __iter__(self) -> Iterator[LidarDetection]:
        """Iterate over the carla.LidarDetection retrieved as data."""
        ...

    def __len__(self): ...
    def __setitem__(self, pos: int, detection: LidarDetection): ...
    def __str__(self) -> str: ...
    # endregion


class Light():
    """This class exposes the lights that exist in the scene, except for vehicle lights. The properties of a light can be queried and changed at will. Lights are automatically turned on when the simulator enters night mode (sun altitude is below zero)."""

    # region Instance Variables
    @property
    def color() -> Color:
        """Color of the light."""
        ...

    @property
    def id() -> int:
        """Identifier of the light."""
        ...

    @property
    def intensity() -> float:
        """Intensity of the light. (lumens)"""

    @property
    def is_on() -> bool:
        """Switch of the light. It is `True` when the light is on. When the night mode starts, this is set to `True`."""
        ...

    @property
    def location() -> Location:
        """Position of the light (meters)."""
        ...

    @property
    def light_group() -> LightGroup:
        """Group the light belongs to."""

    @property
    def light_state() -> LightState:
        """State of the light. Summarizes its attributes, group, and if it is on/off."""
    # endregion

    # region Methods
    def turn_off(self):
        """Switches off the light."""
        ...

    def turn_on(self):
        """Switches on the light."""
        ...
    # endregion

    # region Setters
    def set_color(self, color: Color):
        """Changes the color of the light to color.

        Args:
            color (Color)
        """
        ...

    def set_intensity(self, intensity: float):
        """Changes the intensity of the light to `intensity`.

        Args:
            intensity (float): (lumens)
        """

    def set_light_group(self, light_group: LightGroup):
        """Changes the light to the group `light_group`.

        Args:
            light_group (LightGroup)
        """

    def set_light_state(self, light_state: LightState):
        """Changes the state of the light to `light_state`. This may change attributes, group and turn the light on/off all at once.


        Args:
            light_state (LightState)
        """
    # endregion


class LightGroup(Enum):
    """This class categorizes the lights on scene into different groups. These groups available are provided as a enum values that can be used as flags.

    + Note. So far, though there is a `vehicle` group, vehicle lights are not available as `carla.Light` objects. These have to be managed using `carla.Vehicle` and `carla.VehicleLightState`."""

    NONE = 0,
    """All lights."""
    Vehicle = 1,
    Street = 2,
    Building = 3,
    Other = 4,


class LightManager():
    """This class handles the lights in the scene. Its main use is to get and set the state of groups or lists of lights in one call. An instance of this class can be retrieved by the `carla.World.get_lightmanager()`.

    + Note. So far, though there is a `vehicle` group, vehicle lights are not available as `carla.Light` objects. These have to be managed using `carla.Vehicle` and `carla.VehicleLightState`.
    """

    # region Methods
    def is_active(self, lights: list[Light]) -> list[bool]:
        """Returns a list with booleans stating if the elements in `lights` are switched on/off.

        Args:
            `lights (list[Light])`: List of lights to be queried.\n

        Returns:
            `list[bool]`\n
        """

    def turn_off(self, lights: list[Light]):
        """Switches off all the lights in `lights`.


        Args:
            `lights (list[Light])`: List of lights to be switched off.\n
        """

    def turn_on(self, lights: list[Light]):
        """Switches on all the lights in `lights`.


        Args:
            `lights (list[Light])`: List of lights to be switched on.\n
        """
    # endregion

    # region Getters
    def get_all_lights(self, light_group: LightGroup = LightGroup.NONE) -> list[Light]:
        """Returns a list containing the lights in a certain group. By default, the group is None.

        Args:
            `light_group (LightGroup, optional)`: Group to filter the lights returned. Defaults to LightGroup.NONE.\n
        """

    def get_color(self, lights: list[Light]) -> list[Color]:
        """Returns a list with the colors of every element in `lights`.

        + Setter: `carla.LightManager.set_color`

        Args:
            `lights (list[Light])`: List of lights to be queried.\n

        Returns:
            `list[Color]`: _description_\n
        """

    def get_intensity(self, lights: list[Light]) -> list[float]:
        """Returns a list with the intensity of every element in `lights`.

        Args:
            `lights (list[Light])`: List of lights to be queried.\n

        Returns:
            `list[float]`: (lumens)\n
        """

    def get_light_group(self, lights: list[Light]) -> list[LightGroup]:
        """Returns a list with the group of every element in `lights`.

        + Setter: `carla.LightManager.set_light_group`

        Args:
            `lights (list[Light])`: List of lights to be queried.\n
        """

    def get_light_state(self, lights: list[Light]) -> list[LightState]:
        """Returns a list with the state of all the attributes of every element in `lights`.

        + Setter: `carla.LightManager.set_light_state`

        Args:
            `lights (list[Light])`: List of lights to be queried.\n
        """

    def get_turned_off_lights(self, light_group: LightGroup) -> list[Light]:
        """Returns a list containing lights switched off in the scene, filtered by group.

        Args:
            `light_group (LightGroup)`: List of lights to be queried.\n
        """

    def get_turned_on_lights(self, light_group: LightGroup) -> list[Light]:
        """Returns a list containing lights switched on in the scene, filtered by group.

        Args:
            `light_group (LightGroup)`: List of lights to be queried.\n
        """
    # endregion

    # region Setters
    def set_active(self, lights: list[Light], active: list[bool]):
        """Switches on/off the elements in lights.

        Args:
            `lights (list[Light])`: List of lights to be switched on/off.\n
            `active (list[bool])`: List of booleans to be applied.\n
        """

    def set_color(self, lights: list[Light], color: Color):
        """Changes the color of the elements in `lights` to `color`.

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `color (Color)`: Color to be applied.\n
        """

    def set_colors(self, lights: list[Light], colors: list[Color]):
        """Changes the color of each element in lights to the corresponding in colors.

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `colors (list[Color])`: List of colors to be applied.\n
        """

    def set_day_night_cycle(self, active: bool):
        """All scene lights have a day-night cycle, automatically turning on and off with the altitude of the sun. This interferes in cases where full control of the scene lights is required, so setting this to `False` deactivates it. It can reactivated by setting it to `True`.


        Args:
            `active (bool)`:  (De)activation of the day-night cycle.\n
        """

    def set_intensities(self, lights: list[Light], intensities: list[float]):
        """Changes the intensity of each element in lights to the corresponding in intensities.

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `intensities (list[float])`: Intensity to be applied (lumens).\n
        """

    def set_intensity(self, lights: list[Light], intensity: float):
        """Changes the intensity of every element in `lights` to `intensity`.

        + Getter: `carla.LightManager.get_intensity`

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `intensity (float)`: Intensity to be applied.\n
        """

    def set_light_group(self, lights: list[Light], light_group: LightGroup):
        """Changes the group of every element in `lights` to `light_group`.

        + Getter: carla.LightManager.get_light_group

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `light_group (LightGroup)`: Group to be applied.\n
        """

    def set_light_groups(self, lights: list[Light], light_groups: list[LightGroup]):
        """Changes the group of each element in `lights` to the corresponding in `light_groups`.

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `light_groups (list[LightGroup])`: List of groups to be applied.\n
        """

    def set_light_state(self, lights: list[Light], light_state: LightState):
        """Changes the state of the attributes of every element in lights to light_state.

        + Getter: `carla.LightManager.get_light_state`

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `light_state (LightState)`: State of the attributes to be applied.\n
        """

    def set_light_states(self, lights: list[Light], light_states: list[LightState]):
        """Changes the state of the attributes of each element in lights to the corresponding in light_states.

        Args:
            `lights (list[Light])`: List of lights to be changed.\n
            `light_states (list[LightState])`: List of state of the attributes to be applied.\n
        """
    # endregion


class LightState():
    """This class represents all the light variables except the identifier and the location, which are should to be static. Using this class allows to manage all the parametrization of the light in one call.
    """

    # region Instance Variables
    @property
    def intensity() -> float:
        """Intensity of a light."""

    @property
    def color() -> Color:
        """Color of a light."""

    @property
    def group() -> LightGroup:
        """Group a light belongs to."""

    @property
    def active() -> bool:
        """Switch of a light. It is `True` when the light is on."""
    # endregion

    # region Methods
    def __init__(self, intensity=0.0, color=Color, group=LightGroup.NONE, active=False) -> LightState:
        """This class represents all the light variables except the identifier and the location, which are should to be static. Using this class allows to manage all the parametrization of the light in one call.

        Args:
            `intensity (float, optional)`: Intensity of a light (lumens). Defaults to 0.0.\n
            `color (_type_, optional)`: Color of a light. Defaults to Color.\n
            `group (_type_, optional)`: Group a light belongs to. Defaults to LightGroup.NONE.\n
            `active (bool, optional)`: Switch of a light. It is True when the light is on. Defaults to False.\n
        """
    # endregion


class Location(Vector3D):
    """Represents a spot in the world."""
    # region Instance Variables
    @property
    def x() -> float:
        """Distance from origin to spot on X axis (meter)."""
    @property
    def y() -> float:
        """Distance from origin to spot on Y axis (meter)."""
    @property
    def z() -> float:
        """Distance from origin to spot on Z axis. (meter)"""
    # endregion

    # region Methods
    def __init__(self, x=0.0, y=0.0, z=0.0) -> Location:
        """Represents a spot in the world.

        Args:
            `x (float, optional)`: Distance from origin to spot on X axis (meter). Defaults to 0.0.\n
            `y (float, optional)`: Distance from origin to spot on Y axis (meter). Defaults to 0.0.\n
            `z (float, optional)`: Distance from origin to spot on Z axis (meter). Defaults to 0.0.\n

        Returns:
            `Location`\n
        """
        ...

    def distance(self, location: Location) -> float:
        """Returns Euclidean distance from this location to another one.

        Args:
            `location (Location)`: The other point to compute the distance with.\n

        Returns:
            `float`: (meters)\n
        """
    # endregion

    # region Dunder Methods
    def __abs__(self) -> Location:
        """Returns a Location with the absolute value of the components x, y and z."""

    def __eq__(self, __value: Location) -> bool:
        """Returns `True` if both locations are the same point in space."""
        ...

    def __ne__(self, __value: object) -> bool:
        """Returns `True` if both locations are different points in space."""
        ...

    def __str__(self) -> str:
        """Parses the axis' values to string."""
        ...
    # endregion
    pass


class Map():
    """Class containing the road information and waypoint managing. Data is retrieved from an OpenDRIVE file that describes the road. A query system is defined which works hand in hand with `carla.Waypoint` to translate geometrical information from the .xodr to natural world points. CARLA is currently working with OpenDRIVE 1.4 standard."""

    # region Instance Variables
    @property
    def name() -> str:
        """The name of the map. It corresponds to the .umap from Unreal Engine that is loaded from a CARLA server, which then references to the .xodr road description."""
    # endregion

    # region Methods
    def __init__(self, name: str, xodr_content: str) -> list[Transform]:
        """Constructor for this class. Though a map is automatically generated when initializing the world, using this method in no-rendering mode facilitates working with an .xodr without any CARLA server running.

        Args:
            `name (str)`: Name of the current map.\n
            `xodr_content (str)`: xodr content in string format.\n
        """
        ...

    def cook_in_memory_map(self, path: str):
        """Generates a binary file from the CARLA map containing information used by the Traffic Manager. This method is only used during the import process for maps.

        Args:
            `path (str)`: Path to the intended location of the stored binary map file.\n
        """

    def generate_waypoints(self, distance: float) -> list[Waypoint]:
        """Returns a list of waypoints with a certain distance between them for every lane and centered inside of it. Waypoints are not listed in any particular order. Remember that waypoints closer than 2cm within the same road, section and lane will have the same identificator.

        Args:
            `distance (float)`: Approximate distance between waypoints (meters).\n

        Returns:
            `list[Waypoint]`\n
        """

    def save_to_disk(self, path: str):
        """Saves the .xodr OpenDRIVE file of the current map to disk.

        Args:
            `path (str)`: Path where the file will be saved.\n
        """
        ...

    def to_opendrive(self) -> str:
        """Returns the .xodr OpenDRIVe file of the current map as string.

        Returns:
            `str`\n
        """

    def transform_to_geolocation(self, location: Location) -> GeoLocation:
        """Converts a given `location`, a point in the simulation, to a `carla.GeoLocation`, which represents world coordinates. The geographical location of the map is defined inside OpenDRIVE within the tag .


        Args:
            `location (Location)`\n

        Returns:
            `GeoLocation`\n
        """
        ...
    # endregion

    # region Getters
    def get_all_landmarks(self) -> list[Landmark]:
        """Returns all the landmarks in the map. Landmarks retrieved using this method have a `null` waypoint."""
        ...

    def get_all_landmarks_from_id(self, opendrive_id: str) -> list[Landmark]:
        """Returns the landmarks with a certain OpenDRIVE ID. Landmarks retrieved using this method have a `null` waypoint.

        Args:
            `opendrive_id (str)`: The OpenDRIVE ID of the landmarks.\n
        """
        ...

    def get_all_landmarks_of_type(self, type: Union[str, LandmarkType]) -> list[Landmark]:
        """Returns the landmarks of a specific type. Landmarks retrieved using this method have a null waypoint.

        Args:
            `type (str)`: The type of the landmarks.\n
        """
        ...

    def get_crosswalks(self) -> list[Location]:
        """Returns a list of locations with all crosswalk zones in the form of closed polygons. The first point is repeated, symbolizing where the polygon begins and ends.
        """
        ...

    def get_landmark_group(self, landmark: Landmark) -> list[Landmark]:
        """Returns the landmarks in the same group as the specified landmark (including itself). Returns an empty list if the landmark does not belong to any group.

        Args:
            `landmark (Landmark)`: A landmark that belongs to the group.\n
        """
        ...

    def get_spawn_points(self) -> list[Transform]:
        """Returns a list of recommendations made by the creators of the map to be used as spawning points for the vehicles. The list includes carla.Transform objects with certain location and orientation. Said locations are slightly on-air in order to avoid Z-collisions, so vehicles fall for a bit before starting their way.
        """
        ...

    def get_topology(self) -> list[tuple[Waypoint, Waypoint]]:
        """Returns a list of tuples describing a minimal graph of the topology of the OpenDRIVE file. The tuples contain pairs of waypoints located either at the point a road begins or ends. The first one is the origin and the second one represents another road end that can be reached. This graph can be loaded into NetworkX to work with. 

        Output could look like this: `[(w0, w1), (w0, w2), (w1, w3), (w2, w3), (w0, w4)]`.
        """
        ...

    def get_waypoint(self, location: Location, project_to_road=True, lane_type=LaneType.Driving) -> Waypoint:
        """Returns a waypoint that can be located in an exact location or translated to the center of the nearest lane. Said lane type can be defined using flags such as `LaneType.Driving & LaneType.Shoulder`.

        The method will return `None` if the waypoint is not found, which may happen only when trying to retrieve a waypoint for an exact location. That eases checking if a point is inside a certain road, as otherwise, it will return the corresponding waypoint.

        Args:
            `location (Location)`: Location used as reference for the carla.Waypoint (meters).\n
            `project_to_road (bool, optional)`: If `True`, the waypoint will be at the center of the closest lane. If `False`, the waypoint will be exactly in location. `None` means said location does not belong to a road. Defaults to True.\n
            `lane_type (_type_, optional)`: Limits the search for nearest lane to one or various lane types that can be flagged. Defaults to LaneType.Driving.\n
        """
        ...

    def get_waypoint_xodr(self, road_id: int, lane_id: int, s: float) -> Waypoint:
        """Returns a waypoint if all the parameters passed are correct. Otherwise, returns `None`.

        Args:
            `road_id (int)`: ID of the road to get the waypoint.\n
            `lane_id (int)`: ID of the lane to get the waypoint.\n
            `s (float)`: Specify the length from the road start (meters).\n
        """
        ...
    # endregion


class MapLayer(Enum):
    """Class that represents each manageable layer of the map. Can be used as flags.

    + WARNING: Only "Opt" maps are able to work with map layers."""
    NONE = 0,
    Buildings = 1,
    Decals = 2,
    Foliage = 4,
    Ground = 8,
    ParkedVehicles = 16,
    Particles = 32,
    Props = 64,
    StreetLights = 128,
    Walls = 256,
    All = 65535,
    """All layers selected."""


class MaterialParameter(Enum):
    """Class that represents material parameters. Not all objects in the scene contain all parameters."""

    # region Instance Variables
    Normal = 0,
    """The Normal map of the object. Present in all objects."""
    AO_Roughness_Metallic_Emissive = 1,
    """A texture where each color channel represent a property of the material (R: Ambien oclusion, G: Roughness, B: Metallic, A: Emissive/Height map in some objects)."""
    Diffuse = 2,
    """The Diffuse texture of the object. Present in all objects."""
    Emissive = 3,
    """Emissive texture. Present in a few objects."""
    # endregion


class ObstacleDetectionEvent(SensorData):
    """Class that defines the obstacle data for `sensor.other.obstacle`. Learn more about this here.

    https://carla.readthedocs.io/en/latest/ref_sensors/#obstacle-detector
    """
    # region Instance Variables
    @property
    def actor() -> Actor:
        """The actor the sensor is attached to."""
        ...

    @property
    def other_actor() -> Actor:
        """The actor or object considered to be an obstacle."""
        ...

    @property
    def distance() -> float:
        """Distance between actor and other (meters)."""
        ...
    # endregion

    # region Dunder methods
    def __str__(self) -> str: ...
    # endregion


class OpendriveGenerationParameters():
    """This class defines the parameters used when generating a world using an OpenDRIVE file."""
    # region Instance Variables
    @property
    def vertex_distance() -> float:
        """Distance between vertices of the mesh generated. Default is 2.0."""
        ...

    @property
    def max_road_length() -> float:
        """Max road length for a single mesh portion. The mesh of the map is divided into portions, in order to avoid propagating issues. Default is 50.0."""
        ...

    @property
    def wall_height() -> float:
        """Height of walls created on the boundaries of the road. These prevent vehicles from falling off the road. Default is 1.0."""
        ...

    @property
    def additional_width() -> float:
        """Additional with applied junction lanes. Complex situations tend to occur at junctions, and a little increase can prevent vehicles from falling off the road. Default is 0.6."""
        ...

    @property
    def smooth_junctions() -> bool:
        """If `True`, the mesh at junctions will be smoothed to prevent issues where roads blocked other roads. Default is `True`."""
        ...

    @property
    def enable_mesh_visibility() -> bool:
        """If `True`, the road mesh will be rendered. Setting this to False should reduce the rendering overhead. Default is True."""
        ...

    @property
    def enable_pedestrian_navigation() -> bool:
        """If `True`, Pedestrian navigation will be enabled using Recast tool. For very large maps it is recomended to disable this option. Default is `True`."""
        ...
    # endregion


class OpticalFlowImage(SensorData):
    """Class that defines an optical flow image of 2-Dimension float (32-bit) vectors representing the optical flow detected in the field of view. The components of the vector represents the displacement of an object in the image plane. Each component outputs values in the normalized range [-2,2] which scales to [-2 size, 2 size] with size being the total resolution in the corresponding component."""
    # region Instance Variables
    @property
    def fov() -> float:
        """Horizontal field of view of the image. (degrees)"""
        ...

    @property
    def height() -> int:
        """Image height in pixels."""
        ...

    @property
    def width() -> int:
        """Image width in pixels."""
        ...

    @property
    def raw_data() -> bytes:
        """Flattened array of pixel data, use reshape to create an image array."""
    # endregion

    # region Getters
    def get_color_coded_flow(self) -> Image:
        """Visualization helper. Converts the optical flow image to an RGB image."""
        ...
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos: int) -> OpticalFlowPixel: ...

    def __iter__(self) -> Iterator[OpticalFlowPixel]:
        """Iterate over the `carla.OpticalFlowPixel `that form the image."""

    def __len__(self) -> int: ...
    def __setitem__(self, pos: int, color: Color): ...
    def __str__(self) -> str: ...
    # endregion


class OpticalFlowPixel():
    """Class that defines a 2 dimensional vector representing an optical flow pixel."""

    # region Instance Variables
    @property
    def x() -> float:
        """Optical flow in the x component."""
        ...

    @property
    def y() -> float:
        """Optical flow in the y component."""
    # endregion

    # region Methods
    def __init__(self, x=.0, y=.0) -> OpticalFlowPixel:
        """Initializes the Optical Flow Pixel. Zero by default.

        Args:
            `x (float, optional)`: Optical flow in the x component. Defaults to .0.\n
            `y (float, optional)`: Optical flow in the y component. Defaults to .0.\n
        """
        ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: OpticalFlowPixel) -> bool: ...
    def __ne__(self, __value: OpticalFlowPixel) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class Osm2Odr():
    """Class that converts an OpenStreetMap map to OpenDRIVE format, so that it can be loaded in CARLA. Find out more about this feature in the docs.

    https://carla.readthedocs.io/en/latest/tuto_G_openstreetmap/
    """
    # region Methods
    @staticmethod
    def convert(osm_file: str, settings: Osm2OdrSettings) -> str:
        """Takes the content of an `.osm` file (OpenStreetMap format) and returns the content of the `.xodr` (OpenDRIVE format) describing said map.

        Args:
            `osm_file (str)`: The content of the input OpenStreetMap file parsed as string.\n
            `settings (Osm2OdrSettings)`: Parameterization for the conversion.\n

        Returns:
            `str`\n
        """
    # endregion


class Osm2OdrSettings():
    """Helper class that contains the parameterization that will be used by carla.Osm2Odr to convert an OpenStreetMap map to OpenDRIVE format. Find out more about this feature in the docs.

    https://carla.readthedocs.io/en/latest/tuto_G_openstreetmap/
    """

    # region Instance Variables
    @property
    def use_offsets() -> bool:
        """Enables the use of offset for the conversion. The offset will move the origin position of the map. Default value is False.
        """
        ...

    @property
    def offset_x() -> float:
        """Offset in the X axis. Default value is 0.0 (meters)."""
        ...

    @property
    def offset_y() -> float:
        """Offset in the Y axis. Default value is 0.0 (meters)."""
        ...

    @property
    def default_lane_width() -> float:
        """Width of the lanes described in the resulting XODR map. Default value is 4.0 (meter)."""
        ...

    @property
    def elevation_layer_height() -> float:
        """Defines the height separating two different OpenStreetMap layers. Default value is 0.0."""
        ...

    @property
    def center_map() -> bool:
        """When this option is enabled, the geometry of the map will be displaced so that the origin of coordinates matches the center of the bounding box of the entire road map."""
        ...

    @property
    def proj_string() -> str:
        """Defines the `proj4` string that will be used to compute the projection from geocoordinates to cartesian coordinates. This string will be written in the resulting OpenDRIVE unless the options `use_offsets` or `center_map` are enabled as these options override some of the definitions in the string."""
        ...

    @property
    def generate_traffic_lights() -> bool:
        """Indicates wether to generate traffic light data in the OpenDRIVE. Road types defined by `set_traffic_light_excluded_way_types(way_types)` will not generate traffic lights."""
        ...

    @property
    def all_junctions_with_traffic_lights() -> bool:
        """When disabled, the converter will generate traffic light data from the OpenStreetMaps data only. When enabled, all junctions will generate traffic lights."""
    # endregion

    # region Setters
    def set_osm_way_types(self, way_types: list[str]):
        """Defines the OpenStreetMaps road types that will be imported to OpenDRIVE. By default the road types imported are:
            `motorway`, `motorway_link`, `trunk`, `trunk_link`, `primary`, `primary_link`, `secondary`, `secondary_link`, `tertiary`, `tertiary_link`, `unclassified`, `residential`.
        For a full list of road types check here.

        https://wiki.openstreetmap.org/wiki/Main_Page

        Args:
            `way_types (list[str])`: The list of road types.\n
        """

    def set_traffic_light_excluded_way_types(self, way_types: list[str]):
        """Defines the OpenStreetMaps road types that will not generate traffic lights even if `generate_traffic_lights` is enabled. By default the road types excluded are
            `motorway_link`, `primary_link`, `secondary_link`, `tertiary_link`

        Args:
            `way_types (list[str])`: The list of road types.\n
        """
    # endregion


class RadarDetection():
    """Data contained inside a `carla.RadarMeasurement`. Each of these represents one of the points in the cloud that a `sensor.other.radar` registers and contains the distance, angle and velocity in relation to the radar."""

    # region Instance Variables
    @property
    def altitude() -> float:
        """Altitude angle of the detection (radians)."""
        ...

    @property
    def azimuth() -> float:
        """Azimuth angle of the detection (radians)."""
        ...

    @property
    def depth() -> float:
        """Distance from the sensor to the detection position (meters)."""
        ...

    @property
    def velocity() -> float:
        """The velocity of the detected object towards the sensor (m/s)."""
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class RadarMeasurement(SensorData):
    """Class that defines and gathers the measures registered by a `sensor.other.radar`, representing a wall of points in front of the sensor with a distance, angle and velocity in relation to it. The data consists of a `carla.RadarDetection` array. Learn more about this here.

    https://carla.readthedocs.io/en/latest/ref_sensors/#radar-sensor
    """

    # region Instance Variables
    def raw_data() -> bytes:
        """The complete information of the `carla.RadarDetection` the radar has registered.
        """
    # endregion

    # region Getters
    def get_detection_count(self) -> int:
        """Retrieves the number of entries generated, same as `__str__()`."""
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos: int) -> RadarDetection: ...

    def __iter__(self) -> Iterator[RadarDetection]:
        """Iterate over the `carla.RadarDetection` retrieved as data."""

    def __len__(self) -> int: ...
    def __setitem__(self, pos: int, detection: RadarDetection): ...
    def __str__(self) -> str: ...
    # endregion


class Rotation():
    """Class that represents a 3D rotation and therefore, an orientation in space. CARLA uses the Unreal Engine coordinates system. This is a Z-up left-handed system.

    The constructor method follows a specific order of declaration: `(pitch, yaw, roll)`, which corresponds to `(Y-rotation,Z-rotation,X-rotation)`.
    """

    # region Instance Variables
    @property
    def pitch() -> float:
        """Y-axis rotation angle (degrees)."""
        ...

    @property
    def yaw() -> float:
        """Z-axis rotation angle (degrees)."""
        ...

    @property
    def roll() -> float:
        """X-axis rotation angle (degrees)."""
        ...
    # endregion

    # region Methods
    def __init__(self, pitch=.0, yaw=.0, roll=.0) -> Rotation:
        """+ Warning: The declaration order is different in CARLA (pitch,yaw,roll), and in the Unreal Engine Editor (roll,pitch,yaw). When working in a build from source, don't mix up the axes' rotations.

        Args:
            `pitch (float, optional)`: Y-axis rotation angle (degrees). Defaults to .0.\n
            `yaw (float, optional)`: Z-axis rotation angle (degrees). Defaults to .0.\n
            `roll (float, optional)`: X-axis rotation angle (degrees). Defaults to .0.\n
        """
        ...
    # endregion

    # region Getters
    def get_forward_vector(self) -> Vector3D:
        """Computes the vector pointing forward according to the rotation of the object."""
        ...

    def get_right_vector(self) -> Vector3D:
        """Computes the vector pointing to the right according to the rotation of the object."""
        ...

    def get_up_vector(self) -> Vector3D:
        """Computes the vector pointing upwards according to the rotation of the object."""
        ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: Rotation) -> bool:
        """Returns `True` if both rotations represent the same orientation for every axis."""
        ...

    def __ne__(self, __value: Rotation) -> bool:
        ...

    def __str__(self) -> str: ...
    # endregion


class SemanticLidarDetection():
    """Data contained inside a `carla.SemanticLidarMeasurement`. Each of these represents one of the points in the cloud with its location, the cosine of the incident angle, index of the object hit, and its semantic tag."""

    # region Instance Variables
    @property
    def point() -> Location:
        """[x,y,z] coordinates of the point (meters)."""
    @property
    def cos_inc_angle() -> float:
        """Cosine of the incident angle between the ray, and the normal of the hit object."""
    @property
    def object_idx() -> int:
        """ID of the actor hit by the ray."""
    @property
    def object_tag() -> int:
        """`Semantic tag` of the component hit by the ray."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class SemanticLidarMeasurement(SensorData):
    """Class that defines the semantic LIDAR data retrieved by a `sensor.lidar.ray_cast_semantic`. This essentially simulates a rotating LIDAR using ray-casting. Learn more about this here.

    https://carla.readthedocs.io/en/latest/ref_sensors/#semanticlidar-raycast-sensor
    """

    # region Instance Variables
    @property
    def channels() -> int:
        """Number of lasers shot."""
    @property
    def horizontal_angle() -> float:
        """Horizontal angle the LIDAR is rotated at the time of the measurement (radians)."""
    @property
    def raw_data() -> bytes:
        """Received list of raw detection points. Each point consists of [x,y,z] coordinates plus the cosine of the incident angle, the index of the hit actor, and its semantic tag."""
    # endregion

    # region Methods
    def save_to_disk(self, path: str):
        """Saves the point cloud to disk as a `.ply` file describing data from 3D scanners. The files generated are ready to be used within `MeshLab`, an open-source system for processing said files. Just take into account that axis may differ from Unreal Engine and so, need to be reallocated.

        Args:
            `path (str)`\n
        """
    # endregion

    # region Getters
    def get_point_count(self, channel: int):
        """Retrieves the number of points sorted by channel that are generated by this measure. Sorting by channel allows to identify the original channel for every point.

        Args:
            `channel (int)`\n
        """
    # endregion

    # region Dunder Methods
    def __getitem__(self, pos: int) -> SemanticLidarDetection: ...

    def __iter__(self) -> Iterator[SemanticLidarDetection]:
        """Iterate over the `carla.SemanticLidarDetection` retrieved as data."""
        ...

    def __len__(self): ...
    def __setitem__(self, pos: int, detection: SemanticLidarDetection): ...
    def __str__(self) -> str: ...
    # endregion


class Sensor(Actor):
    """Sensors compound a specific family of actors quite diverse and unique. They are normally spawned as attachment/sons of a vehicle (take a look at `carla.World` to learn about actor spawning). Sensors are thoroughly designed to retrieve different types of data that they are listening to. The data they receive is shaped as different subclasses inherited from `carla.SensorData` (depending on the sensor).

    Most sensors can be divided in two groups: those receiving data on every tick (cameras, point clouds and some specific sensors) and those who only receive under certain circumstances (trigger detectors). CARLA provides a specific set of sensors and their blueprint can be found in `carla.BlueprintLibrary`. All the information on their preferences and settlement can be found here, but the list of those available in CARLA so far goes as follow.

    https://carla.readthedocs.io/en/latest/ref_sensors/

    Receive data on every tick. 
        - Depth camera. 
        - Gnss sensor. 
        - IMU sensor. 
        - Lidar raycast. 
        - SemanticLidar raycast. 
        - Radar. 
        - RGB camera. 
        - RSS sensor. 
        - Semantic Segmentation camera.

    Only receive data when triggered.
        - Collision detector. 
        - Lane invasion detector.
        - Obstacle detector.    
    """

    # region Instance Variables
    @property
    def is_listening() -> bool:
        """When `True` the sensor will be waiting for data."""
    # endregion

    # region Methods
    def is_listening_gbuffer(self, gbuffer_id: GBufferTextureID) -> bool:
        """Returns whether the sensor is in a listening state for a specific GBuffer texture.

        Args:
            `gbuffer_id (GBufferTextureID)`: The ID of the target Unreal Engine GBuffer texture.\n
        """

    def listen(self, callback: Callable):
        """The function the sensor will be calling to every time a new measurement is received. This function needs for an argument containing an object type `carla.SensorData` to work with.

        Args:
            `callback (Callable)`: The called function with one argument containing the sensor data.\n
        """

    def listen_to_gbuffer(self, gbuffer_id: GBufferTextureID, callback: Callable):
        """The function the sensor will be calling to every time the desired GBuffer texture is received.

        This function needs for an argument containing an object type `carla.SensorData` to work with.

        Args:
            `gbuffer_id (GBufferTextureID)`: The ID of the target Unreal Engine GBuffer texture.\n
            `callback (Callable)`: The called function with one argument containing the received GBuffer texture.\n
        """

    def stop(self):
        """Commands the sensor to stop listening for data."""

    def stop_gbuffer(self, gbuffer_id: GBufferTextureID):
        """Commands the sensor to stop listening for the specified GBuffer texture.

        Args:
            `gbuffer_id (GBufferTextureID)`: The ID of the Unreal Engine GBuffer texture.\n
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class SensorData():
    """Base class for all the objects containing data generated by a `carla.Sensor`. This objects should be the argument of the function said sensor is listening to, in order to work with them. Each of these sensors needs for a specific type of sensor data. Hereunder is a list of the sensors and their corresponding data.
    - Cameras (RGB, depth and semantic segmentation): `carla.Image`.
    - Collision detector: `carla.CollisionEvent`.
    - GNSS sensor: `carla.GnssMeasurement`.
    - IMU sensor: `carla.IMUMeasurement`.
    - Lane invasion detector: `carla.LaneInvasionEvent`.
    - LIDAR sensor: `carla.LidarMeasurement`.
    - Obstacle detector: `carla.ObstacleDetectionEvent`.
    - Radar sensor: `carla.RadarMeasurement`.
    - RSS sensor: `carla.RssResponse`.
    - Semantic LIDAR sensor: `carla.SemanticLidarMeasurement`.
    """
    # region Instance Variables
    @property
    def frame() -> int:
        """Frame count when the data was generated."""
        ...

    @property
    def timestamp() -> float:
        """Simulation-time(seconds) when the data was generated."""
        ...

    @property
    def transform() -> Transform:
        """Sensor's transform when the data was generated."""
    # endregion


class TextureColor():
    """Class representing a texture object to be uploaded to the server. Pixel format is RGBA, uint8 per channel.
    """
    # region Instance Variables
    @property
    def width() -> int:
        """X-coordinate size of the texture."""
    @property
    def height() -> int:
        """Y-coordinate size of the texture."""
    # endregion

    # region Methods
    def __init__(self, width: int, height: int) -> TextureColor:
        """Initializes a the texture with a `(width, height)` size.

        Args:
            `width (int)`: X-coordinate size of the texture.\n
            `height (int)`: Y-coordinate size of the texture.\n
        """
        ...
    # endregion

    # region Getters
    def get(self, x: int, y: int) -> Color:
        """Get the `(x,y)` pixel data."""

    # endregion

    # region Setters
    def set(self, x: int, y: int, value: Color):
        """Sets the `(x,y)` pixel data with `value`."""

    def set_dimensions(self, width: int, height: int):
        """Resizes the texture to te specified dimensions."""
    # endregion


class TextureFloatColor():
    """Class representing a texture object to be uploaded to the server. Pixel format is RGBA, float per channel."""

    # region Instance Variables
    @property
    def width() -> int:
        """X-coordinate size of the texture."""
    @property
    def height() -> int:
        """Y-coordinate size of the texture."""
    # endregion

    # region Getters
    def get(self, x: int, y: int) -> FloatColor:
        """Get the `(x,y)` pixel data."""

    # endregion

    # region Setters
    def set(self, x: int, y: int, value: FloatColor):
        """Sets the `(x,y)` pixel data with `value`."""

    def set_dimensions(self, width: int, height: int):
        """Resizes the texture to te specified dimensions."""
    # endregion


class Timestamp():
    """Class that contains time information for simulated data. This information is automatically retrieved as part of the `carla.WorldSnapshot` the client gets on every frame, but might also be used in many other situations such as a `carla.Sensor` retrieveing data."""

    # region Instance Variables
    @property
    def frame() -> int:
        """The number of frames elapsed since the simulator was launched."""
    @property
    def elapsed_seconds() -> float:
        """Simulated seconds elapsed since the beginning of the current episode (seconds)."""
    @property
    def delta_seconds() -> float:
        """Simulated seconds elapsed since the previous frame (seconds)."""
    def platform_timestamp() -> float:
        """Time register of the frame at which this measurement was taken given by the OS in seconds (seconds)."""
    # endregion

    # region Methods
    def __init__(self, frame: int, elapsed_seconds: float, delta_seconds: float, platform_timestamp: float) -> Timestamp: ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: Timestamp) -> bool: ...
    def __ne__(self, __value: Timestamp) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class TrafficLight(TrafficSign):
    """ traffic light actor, considered a specific type of traffic sign. As traffic lights will mostly appear at junctions, they belong to a group which contains the different traffic lights in it. Inside the group, traffic lights are differenciated by their pole index.

    Within a group the state of traffic lights is changed in a cyclic pattern: one index is chosen and it spends a few seconds in green, yellow and eventually red. The rest of the traffic lights remain frozen in red this whole time, meaning that there is a gap in the last seconds of the cycle where all the traffic lights are red. However, the state of a traffic light can be changed manually."""

    # region Instance Variables
    @property
    def state() -> TrafficLightState:
        """Current state of the traffic light."""
    # endregion

    # region Methods
    def freeze(self, freeze: bool):
        """Stops all the traffic lights in the scene at their current state."""

    def is_frozen(self) -> bool:
        """The client returns `True` if a traffic light is frozen according to last tick. The method does not call the simulator."""

    def reset_group(self):
        """Resets the state of the traffic lights of the group to the initial state at the start of the simulation.

        + Note: This method calls the simulator.
        """
    # endregion

    # region Getters
    def get_affected_lane_waypoints(self) -> list[Waypoint]:
        """Returns a list of waypoints indicating the positions and lanes where the traffic light is having an effect."""

    def get_elapsed_time(self) -> float:
        """The client returns the time in seconds since current light state started according to last tick. The method does not call the simulator (seconds)."""

    def get_green_time(self) -> float:
        """The client returns the time set for the traffic light to be green, according to last tick. The method does not call the simulator (seconds).

        Setter: `carla.TrafficLight.set_green_time`
        """

    def get_group_traffic_lights(self) -> list[TrafficLight]:
        """Returns all traffic lights in the group this one belongs to.

        + Note: This method calls the simulator.
        """

    def get_light_boxes(self) -> list[BoundingBox]:
        """Returns a list of the bounding boxes encapsulating each light box of the traffic light."""

    def get_opendrive_id(self) -> str:
        """Returns the OpenDRIVE id of this traffic light."""

    def get_pole_index(self) -> int:
        """Returns the index of the pole that identifies it as part of the traffic light group of a junction."""

    def get_red_time(self) -> float:
        """The client returns the time set for the traffic light to be red, according to last tick. The method does not call the simulator (seconds).

        Setter: `carla.TrafficLight.set_red_time`
        """

    def get_state(self) -> TrafficLightState:
        """The client returns the state of the traffic light according to last tick. The method does not call the simulator.

        Setter: `carla.TrafficLight.set_state`
        """

    def get_stop_waypoints(self) -> list[Waypoint]:
        """Returns a list of waypoints indicating the stop position for the traffic light. These waypoints are computed from the trigger boxes of the traffic light that indicate where a vehicle should stop."""

    def get_yellow_time(self) -> float:
        """The client returns the time set for the traffic light to be yellow, according to last tick. The method does not call the simulator (seconds).

        Setter: `carla.TrafficLight.set_yellow_time`
        """
    # endregion

    # region Setters
    def set_green_time(self, green_time: float):
        """Sets a given time for the green light to be active.

        Getter: `carla.TrafficLight.get_green_time`

        Args:
            green_time (float): (seconds)
        """

    def set_red_time(self, red_time: float):
        """Sets a given time for the red state to be active.

        Getter: carla.TrafficLight.get_red_time

        Args:
            red_time (float): (seconds)
        """

    def set_state(self, state: TrafficLightState):
        """Sets a given state to a traffic light actor.

        Getter: `carla.TrafficLight.get_state`

        Args:
            state (TrafficLightState)
        """

    def set_yellow_time(self, yellow_time: float):
        """Sets a given time for the yellow light to be active.

        Getter: `carla.TrafficLight.get_yellow_time`

        Args:
            yellow_time (float): (seconds)
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class TrafficLightState(Enum):
    """All possible states for traffic lights. These can either change at a specific time step or be changed manually. The snipet in `carla.TrafficLight.set_state` changes the state of a traffic light on the fly."""
    Red = 0,
    Yellow = 1,
    Green = 2,
    Off = 3,
    Unknown = 4,


class TrafficManager():
    """The traffic manager is a module built on top of the CARLA API in C++. It handles any group of vehicles set to autopilot mode to populate the simulation with realistic urban traffic conditions and give the chance to user to customize some behaviours. The architecture of the traffic manager is divided in five different goal-oriented stages and a PID controller where the information flows until eventually, a `carla.VehicleControl` is applied to every vehicle registered in a traffic manager. In order to learn more, visit the documentation regarding this module.

    https://carla.readthedocs.io/en/latest/adv_traffic_manager/
    """

    # region Methods
    def auto_lane_change(self, actor: Actor, enable: bool):
        """Turns on or off lane changing behaviour for a vehicle.

        Args:
            actor (Actor): The vehicle whose settings are changed.
            enable (bool): `True` is default and enables lane changes. `False` will disable them.
        """

    def collision_detection(self, reference_actor: Actor, other_actor: Actor, detect_collision: bool):
        """Turns on/off collisions between a vehicle and another specific actor. In order to ignore all other vehicles, traffic lights or walkers, use the specific `ignore` methods described in this same section.

        Args:
            reference_actor (Actor): Vehicle that is going to ignore collisions.
            other_actor (Actor): The actor that `reference_actor` is going to ignore collisions with.
            detect_collision (bool): `True` is default and enables collisions. `False` will disable them.
        """

    def distance_to_leading_vehicle(self, actor: Actor, distance: float):
        """Sets the minimum distance in meters that a vehicle has to keep with the others. The distance is in meters and will affect the minimum moving distance. It is computed from front to back of the vehicle objects.

        Args:
            actor (Actor): Vehicle whose minimum distance is being changed.
            distance (float): Meters between both vehicles (meters).
        """

    def force_lane_change(self, actor: Actor, direction: bool):
        """Forces a vehicle to change either to the lane on its left or right, if existing, as indicated in `direction`. This method applies the lane change no matter what, disregarding possible collisions.

        Args:
            actor (Actor): Vehicle being forced to change lanes.
            direction (bool): Destination lane. `True` is the one on the right and `False` is the left one.
        """

    def global_lane_offset(self, offset: float):
        """Sets a global lane offset displacement from the center line. Positive values imply a right offset while negative ones mean a left one. Default is 0. Numbers high enough to cause the vehicle to drive through other lanes might break the controller.

        Args:
            offset (float): Lane offset displacement from the center line.
        """

    def global_percentage_speed_difference(self, percentage: float):
        """Sets the difference the vehicle's intended speed and its current speed limit. Speed limits can be exceeded by setting the `perc` to a negative value. Default is 30. Exceeding a speed limit can be done using negative percentages.

        Args:
            percentage (float): Percentage difference between intended speed and the current limit.
        """

    def ignore_lights_percentage(self, actor: Actor, perc: float):
        """During the traffic light stage, which runs every frame, this method sets the percent chance that traffic lights will be ignored for a vehicle.

        Args:
            actor (Actor): The actor that is going to ignore traffic lights.
            perc (float): Between 0 and 100. Amount of times traffic lights will be ignored.
        """

    def ignore_signs_percentage(self, actor: Actor, perc: float):
        """During the traffic light stage, which runs every frame, this method sets the percent chance that stop signs will be ignored for a vehicle.

        Args:
            actor (Actor): The actor that is going to ignore stop signs.
            perc (float): Between 0 and 100. Amount of times stop signs will be ignored.
        """

    def ignore_vehicles_percentage(self, actor: Actor, perc: float):
        """During the collision detection stage, which runs every frame, this method sets a percent chance that collisions with another vehicle will be ignored for a vehicle.

        Args:
            actor (Actor): The vehicle that is going to ignore other vehicles.
            perc (float): Between 0 and 100. Amount of times collisions will be ignored.
        """

    def ignore_walkers_percentage(self, actor: Actor, perc: float):
        """During the collision detection stage, which runs every frame, this method sets a percent chance that collisions with walkers will be ignored for a vehicle.

        Args:
            actor (Actor): The vehicle that is going to ignore walkers on scene.
            perc (float): Between 0 and 100. Amount of times collisions will be ignored.
        """

    def keep_right_rule_percentage(self, actor: Actor, perc: float):
        """During the localization stage, this method sets a percent chance that vehicle will follow the keep right rule, and stay in the right lane.

        Args:
            actor (Actor): Vehicle whose behaviour is being changed.
            perc (float): Between 0 and 100. Amount of times the vehicle will follow the keep right rule.
        """

    def random_left_lanechange_percentage(self, actor: Actor, percentage: float):
        """Adjust probability that in each timestep the actor will perform a left lane change, dependent on lane change availability.

        Args:
            actor (Actor): The actor that you wish to query.
            percentage (float): The probability of lane change in percentage units (between 0 and 100).
        """

    def random_right_lanechange_percentage(self, actor: Actor, percentage: float):
        """Adjust probability that in each timestep the actor will perform a right lane change, dependent on lane change availability.

        Args:
            actor (Actor): The actor that you wish to query.
            percentage (float): The probability of lane change in percentage units (between 0 and 100).
        """

    def shut_down(self):
        """Shuts down the traffic manager."""

    def update_vehicle_lights(self, actor: Actor, do_update: bool):
        """Sets if the Traffic Manager is responsible of updating the vehicle lights, or not. Default is False. The traffic manager will not change the vehicle light status of a vehicle, unless its auto_update_status is st to True.

        Args:
            actor (Actor): Vehicle whose lights status is being changed.
            do_update (bool): If `True` the traffic manager will manage the vehicle lights for the specified vehicle.
        """

    def vehicle_lane_offset(self, actor: Actor, offset: float):
        """Sets a lane offset displacement from the center line. Positive values imply a right offset while negative ones mean a left one. Default is 0. Numbers high enough to cause the vehicle to drive through other lanes might break the controller.


        Args:
            actor (Actor): Vehicle whose lane offset behaviour is being changed.
            offset (float): Lane offset displacement from the center line.
        """

    def vehicle_percentage_speed_difference(self, actor: Actor, percentage: float):
        """Sets the difference the vehicle's intended speed and its current speed limit. Speed limits can be exceeded by setting the `perc` to a negative value. Default is 30. Exceeding a speed limit can be done using negative percentages.


        Args:
            actor (Actor): Vehicle whose speed behaviour is being changed.
            percentage (float): Percentage difference between intended speed and the current limit.
        """
    # endregion

    # region Getters
    def get_all_actions(self, actor: Actor) -> list[Waypoint]:
        """Returns all known actions (i.e. road options and waypoints) that an actor controlled by the Traffic Manager will perform in its next steps.

        Args:
            actor (Actor): The actor that you wish to query.

        Returns:
            list[Waypoint]: list of lists with each element as follows - [Road option (string e.g. 'Left', 'Right', 'Straight'), Next waypoint (carla.Waypoint)]
        """

    def get_next_action(self, actor: Actor) -> list[Waypoint]:
        """Returns the next known road option and waypoint that an actor controlled by the Traffic Manager will follow.

        Args:
            actor (Actor): The actor that you wish to query.

        Returns:
            list[Waypoint]: list of two elements - [Road option (string e.g. 'Left', 'Right', 'Straight'), Next waypoint (carla.Waypoint)]
        """

    def get_port(self) -> int:
        """Returns the port where the Traffic Manager is connected. If the object is a TM-Client, it will return the port of its TM-Server. Read the documentation to learn the difference.

        https://carla.readthedocs.io/en/latest/python_api/#adv_traffic_manager.md#multiclient-and-multitm-management
        """
    # endregion

    # region Setters
    def set_boundaries_respawn_dormant_vehicles(self, lower_bound=25.0, upper_bound: float = WorldSettings.actor_active_distance):
        """Sets the upper and lower boundaries for dormant actors to be respawned near the hero vehicle.

        + Warning: The `upper_bound` cannot be higher than the `actor_active_distance`. The `lower_bound` cannot be less than `25`.

        Args:
            lower_bound (float, optional): The minimum distance in meters from the hero vehicle that a dormant actor will be respawned. Defaults to 25.0.
            upper_bound (float, optional): The maximum distance in meters from the hero vehicle that a dormant actor will be respawned. Defaults to WorldSettings.actor_active_distance.
        """

    def set_desired_speed(self, actor: Actor, speed: float):
        """Sets the speed of a vehicle to the specified value.

        Args:
            actor (Actor): Vehicle whose speed is being changed.
            speed (float): Desired speed at which the vehicle will move.
        """

    def set_global_distance_to_leading_vehicle(self, distance: float):
        """Sets the minimum distance in meters that vehicles have to keep with the rest. The distance is in meters and will affect the minimum moving distance. It is computed from center to center of the vehicle objects.

        Args:
            distance (float): Meters between vehicles (meters).
        """

    def set_hybrid_physics_mode(self, enabled=False):
        """Enables or disables the hybrid physics mode. In this mode, vehicle's farther than a certain radius from the ego vehicle will have their physics disabled. Computation cost will be reduced by not calculating vehicle dynamics. Vehicles will be teleported.

        Args:
            enabled (bool, optional): If `True`, enables the hybrid physics. Defaults to False.
        """

    def set_hybrid_physics_radius(self, r=50.0):
        """With hybrid physics on, changes the radius of the area of influence where physics are enabled.

        Args:
            r (float, optional): New radius where physics are enabled (meters). Defaults to 50.0.
        """

    def set_osm_mode(self, mode_switch=True):
        """Enables or disables the OSM mode. This mode allows the user to run TM in a map created with the OSM feature. These maps allow having dead-end streets. Normally, if vehicles cannot find the next waypoint, TM crashes. If OSM mode is enabled, it will show a warning, and destroy vehicles when necessary.

        Args:
            mode_switch (bool, optional): If True, the OSM mode is enabled. Defaults to True.
        """

    def set_path(self, actor: Actor, path: list[Location]):
        """Sets a list of locations for a vehicle to follow while controlled by the Traffic Manager.

        Args:
            actor (Actor): The actor that must follow the given path.
            path (list[Location]): The list of carla.Locations for the actor to follow.
        """

    def set_random_device_seed(self, value: int):
        """Sets a specific random seed for the Traffic Manager, thereby setting it to be deterministic.

        Args:
            value (int): Seed value for the random number generation of the Traffic Manager.
        """

    def set_respawn_dormant_vehicles(self, mode_switch=False):
        """If `True`, vehicles in large maps will respawn near the hero vehicle when they become dormant. Otherwise, they will stay dormant until they are within `actor_active_distance` of the hero vehicle again.


        Args:
            mode_switch (bool, optional) Defaults to False.
        """

    def set_route(self, actor: Actor, path: list[str]):
        """Sets a list of route instructions for a vehicle to follow while controlled by the Traffic Manager. The possible route instructions are 'Left', 'Right', 'Straight'.

        + Warning: Ensure that the lane topology doesn't impede the given route.


        Args:
            actor (Actor): The actor that must follow the given route instructions.
            path (list[str]): The list of route instructions (string) for the vehicle to follow.
        """

    def set_synchronous_mode(self, mode_switch=True):
        """Sets the Traffic Manager to synchronous mode. In a multiclient situation, only the TM-Server can tick. Similarly, in a multiTM situation, only one TM-Server must tick. Use this method in the client that does the world tick, and right after setting the world to synchronous mode, to set which TM will be the master while in sync.

        + Warning: If the server is set to synchronous mode, the TM must be set to synchronous mode too in the same client that does the tick.

        Args:
            mode_switch (bool, optional):  If True, the TM synchronous mode is enabled.. Defaults to True.
        """
    # endregion


class TrafficSign(Actor):
    """Traffic signs appearing in the simulation except for traffic lights. These have their own class inherited from this in `carla.TrafficLight`. Right now, speed signs, stops and yields are mainly the ones implemented, but many others are borne in mind.
    """

    # region Instance Variables
    @property
    def trigger_volume() -> BoundingBox:
        """A carla.BoundingBox situated near a traffic sign where the carla.Actor who is inside can know about it."""
    # endregion


class Transform():
    """Class that defines a transformation, a combination of location and rotation, without scaling.
    """

    # region Instance Variables
    @property
    def location() -> Location:
        """Describes a point in the coordinate system."""
    @property
    def rotation() -> Rotation:
        """Describes a rotation for an object according to Unreal Engine's axis system (degrees (pitch, yaw, roll))."""
    # endregion

    # region Methods
    def __init__(self, location: Location, rotation: Rotation) -> Transform: ...

    def transform(self, in_point: Location):
        """Translates a 3D point from local to global coordinates using the current transformation as frame of reference.

        Args:
            in_point (Location): Translates a 3D point from local to global coordinates using the current transformation as frame of reference.
        """

    def transform_vector(self, in_vector: Vector3D):
        """Rotates a vector using the current transformation as frame of reference, without applying translation. Use this to transform, for example, a velocity.

        Args:
            in_vector (Vector3D): Vector to which the transformation will be applied.
        """
    # endregion

    # region Getters
    def get_forward_vector(self) -> Vector3D:
        """Computes a forward vector using the rotation of the object."""

    def get_inverse_matrix(self) -> list[list[float]]:
        """Computes the 4-matrix representation of the inverse transformation."""

    def get_matrix(self) -> list[list[float]]:
        """Computes the 4-matrix representation of the transformation."""

    def get_right_vector(self) -> Vector3D:
        """Computes a right vector using the rotation of the object."""

    def get_up_vector(self) -> Vector3D:
        """Computes an up vector using the rotation of the object."""
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: Transform) -> bool:
        """Returns `True` if both `location` and `rotation` are equal for this and `other`."""

    def __ne__(self, __value: Transform) -> bool:
        """Returns `True` if any `location` and `rotation` are not equal for this and `other`."""

    def __str__(self) -> str:
        """Parses both `location` and `rotation` to string."""
    # endregion


class Vector2D():
    """Helper class to perform 2D operations."""

    # region Instance Variables
    @property
    def x() -> float:
        """X-axis value."""
    @property
    def y() -> float:
        """Y-axis value."""
    # endregion

    # region Methods
    def __init__(self, x=0.0, y=0.0) -> Vector2D: ...

    def length(self) -> float:
        """Computes the length of the vector."""

    def make_unit_vector(self) -> Vector3D:
        """Returns a vector with the same direction and unitary length."""

    def squared_length(self) -> float:
        """Computes the squared length of the vector."""
    # endregion

    # region Dunder Methods
    # TODO 返回值
    def __add__(self, other=Vector2D) -> Vector2D: ...

    def __eq__(self, other=Vector2D) -> bool:
        """Returns `True` if values for every axis are equal."""

    def __mul__(self, other=Vector2D) -> float: ...

    def __ne__(self, bool=Vector2D) -> bool:
        """Returns `True` if the value for any axis is different."""

    def __str__(self) -> str:
        """Returns the axis values for the vector parsed as string."""
        ...

    def __sub__(self, other=Vector2D) -> Vector2D: ...
    def __truediv__(self, other=Vector2D): ...

    # endregion


class Vector3D():
    """Helper class to perform 3D operations."""

    # region Instance Variables
    @property
    def x() -> float:
        """X-axis value."""
    @property
    def y() -> float:
        """Y-axis value."""
    @property
    def z() -> float:
        """Z-axis value."""
    # endregion

    # region Methods
    def __init__(self, x=0.0, y=0.0, z=0.0) -> Vector3D: ...

    def cross(self, vector: Vector3D) -> Vector3D:
        """Computes the cross product between two vectors.
"""

    def distance(self, vector: Vector3D) -> float:
        """Computes the distance between two vectors."""

    def distance_2d(self, vector: Vector3D) -> Vector3D:
        """Computes the 2-dimensional distance between two vectors."""

    def distance_squared(self, vector: Vector3D) -> float:
        """Computes the squared distance between two vectors."""

    def distance_squared_2d(self, vector: Vector3D) -> float:
        """Computes the 2-dimensional squared distance between two vectors."""

    def dot(self, vector: Vector3D) -> float:
        """Computes the dot product between two vectors."""

    def dot_2d(self, vector: Vector3D) -> float:
        """Computes the 2-dimensional dot product between two vectors."""

    def length(self) -> float:
        """Computes the length of the vector."""

    def make_unit_vector(self) -> Vector3D:
        """Returns a vector with the same direction and unitary length."""

    def squared_length(self) -> float:
        """Computes the squared length of the vector."""
    # endregion

    # region Getters
    def get_vector_angle(self, vector: Vector3D) -> float:
        """Computes the angle between a pair of 3D vectors in radians."""
    # endregion

    # region Dunder Methods
    def __abs__(self) -> Vector3D:
        """Returns a Vector3D with the absolute value of the components x, y and z."""

    def __add__(self, other=Vector3D) -> Vector3D: ...
    def __eq__(self, other=Vector3D) -> bool: ...
    def __mul__(self, other=Vector3D): ...
    def __ne__(self, other=Vector3D) -> bool: ...
    def __str__(self) -> str: ...
    def __sub__(self, other=Vector3D): ...
    def __truediv__(self, other=Vector3D): ...
    # endregion


class Vehicle(Actor):
    """One of the most important groups of actors in CARLA. These include any type of vehicle from cars to trucks, motorbikes, vans, bycicles and also official vehicles such as police cars. A wide set of these actors is provided in `carla.BlueprintLibrary` to facilitate differente requirements. Vehicles can be either manually controlled or set to an autopilot mode that will be conducted client-side by the `traffic manager`."""

    # region Instance Variables
    def bounding_box() -> BoundingBox:
        """Bounding box containing the geometry of the vehicle. Its location and rotation are relative to the vehicle it is attached to."""
    # endregion

    # region Methods
    def apply_ackermann_control(self, control: VehicleAckermannControl):
        """Applies an Ackermann control object on the next tick.

        Args:
            `control (VehicleAckermannControl)`\n
        """

    def apply_ackermann_controller_settings(self, settings: VehicleAckermannControl):
        """Applies a new Ackermann control settings to this vehicle in the next tick.

        + Warning: This method does call the simulator."""

    def apply_control(self, control: VehicleControl):
        """Applies a control object on the next tick, containing driving parameters such as throttle, steering or gear shifting."""

    def apply_physics_control(self, physics_control: VehiclePhysicsControl):
        """Applies a physics control object in the next tick containing the parameters that define the vehicle as a corporeal body. E.g.: moment of inertia, mass, drag coefficient and many more."""

    def close_door(self, door_idx: VehicleDoor):
        """Close the door `door_idx` if the vehicle has it. Use `carla.VehicleDoor.All` to close all available doors."""

    def enable_carsim(self, simfile_path: str):
        """Enables the CarSim physics solver for this particular vehicle. In order for this function to work, there needs to be a valid license manager running on the server side. The control inputs are redirected to CarSim which will provide the position and orientation of the vehicle for every frame.

        Args:
            `simfile_path (str)`: Path to the `.simfile` file with the parameters of the simulation.\n
        """

    def enable_chrono_physics(self, max_substeps: int, max_substep_delta_time: int, vehicle_json: str, powertrain_json: str, tire_json: str, base_json_path: str):
        """Enables Chrono physics on a spawned vehicle.

        + Note: Ensure that you have started the CARLA server with the ARGS="--chrono" flag. You will not be able to use Chrono physics without this flag set.
        + Warning: Collisions are not supported. When a collision is detected, physics will revert to the default CARLA physics.

        Args:
            `max_substeps (int)`: Max number of Chrono substeps.\n
            `max_substep_delta_time (int)`:  Max size of substep.\n
            `vehicle_json (str)`: Path to vehicle json file relative to `base_json_path`.\n
            `powertrain_json (str)`: Path to powertrain json file relative to `base_json_path`.\n
            `tire_json (str)`: Path to tire json file relative to `base_json_path`.\n
            `base_json_path (str)`: Path to `chrono/data/vehicle` folder. E.g., `/home/user/carla/Build/chrono-install/share/chrono/data/vehicle/` (the final / character is required).\n
        """

    def is_at_traffic_light(self) -> bool:
        """Vehicles will be affected by a traffic light when the light is red and the vehicle is inside its bounding box. The client returns whether a traffic light is affecting this vehicle according to last tick (it does not call the simulator)."""

    def open_door(self, door_idx: VehicleDoor):
        """Open the door `door_idx` if the vehicle has it. Use carla.VehicleDoor.All to open all available doors.

        Args:
            `door_idx (VehicleDoor)`: door index.\n
        """

    def show_debug_telemetry(self, enabled=True):
        """Enables or disables the telemetry on this vehicle. This shows information about the vehicles current state and forces applied to it in the spectator window. Only information for one vehicle can be shown so that, if you enable a second one, the previous will be automatically disabled.
        """

    def use_carsim_road(self, enabled: bool):
        """Enables or disables the usage of CarSim vs terrain file specified in the `.simfile`. By default this option is disabled and CarSim uses unreal engine methods to process the geometry of the scene."""
    # endregion

    # region Getters
    def get_ackermann_controller_settings(self) -> AckermannControllerSettings:
        """Returns the last Ackermann control settings applied to this vehicle.

        + Warning: This method does call the simulator to retrieve the value.
        """

    def get_control(self) -> VehicleControl:
        """The client returns the control applied in the last tick. The method does not call the simulator.
        """

    def get_failure_state(self) -> VehicleFailureState:
        """Vehicle have failure states, to indicate that it is incapable of continuing its route. This function returns the vehicle's specific failure state, or in other words, the cause that resulted in it."""

    def get_light_state(self) -> VehicleLightState:
        """Returns a flag representing the vehicle light state, this represents which lights are active or not.

        + Setter: `carla.Vehicle.set_light_state`
        """

    def get_physics_control(self) -> VehiclePhysicsControl:
        """The simulator returns the last physics control applied to this vehicle.

        + Warning: This method does call the simulator to retrieve the value.
        """

    def get_speed_limit(self) -> float:
        """The client returns the speed limit affecting this vehicle according to last tick (it does not call the simulator). The speed limit is updated when passing by a speed limit signal, so a vehicle might have none right after spawning.

        Returns:
            `float`: (km/h)\n
        """

    def get_traffic_light(self) -> TrafficLight:
        """Retrieves the traffic light actor affecting this vehicle (if any) according to last tick. The method does not call the simulator.
        """

    def get_traffic_light_state(self) -> TrafficLightState:
        """The client returns the state of the traffic light affecting this vehicle according to last tick. The method does not call the simulator. If no traffic light is currently affecting the vehicle, returns `green`.
        """

    def get_wheel_steer_angle(self, wheel_location: VehicleWheelLocation) -> float:
        """
        Returns the physics angle in degrees of a vehicle's wheel.

        + Note: Returns the angle based on the physics of the wheel, not the visual angle.
        """
    # endregion

    # region Setters
    def set_autopilot(self, enabled=True, port=8000):
        """Registers or deletes the vehicle from a Traffic Manager's list. When `True`, the Traffic Manager passed as parameter will move the vehicle around. The autopilot takes place client-side.

        Args:
            `enabled (bool, optional)`: Defaults to True.\n
            `port (int, optional)`: The port of the TM-Server where the vehicle is to be registered or unlisted. Defaults to 8000.\n
        """

    def set_light_state(self, light_state: VehicleLightState):
        """Sets the light state of a vehicle using a flag that represents the lights that are on and off.

        + Getter: `carla.Vehicle.get_light_state`
        """

    def set_wheel_steer_direction(self, wheel_location: VehicleWheelLocation, angle_in_deg: float):
        """Sets the angle of a vehicle's wheel visually.

        + Warning: Does not affect the physics of the vehicle.
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class VehicleAckermannControl():
    """Manages the basic movement of a vehicle using Ackermann driving controls."""

    # region Instance Variables
    @property
    def steer() -> float:
        """Desired steer (rad). Positive value is to the right. Default is 0.0."""
    @property
    def steer_speed() -> float:
        """Steering velocity (rad/s). Zero steering angle velocity means change the steering angle as quickly as possible. Default is 0.0."""

    @property
    def speed() -> float:
        """Desired speed (m/s). Default is 0.0."""

    @property
    def acceleration() -> float:
        """Desired acceleration (m/s2) Default is 0.0."""

    @property
    def jerk() -> float:
        """Desired jerk (m/s3). Default is 0.0."""
    # endregion

    # region Methods
    def __init__(self, steer=0.0, steer_speed=0.0, speed=0.0, acceleration=0.0, jerk=0.0) -> VehicleAckermannControl: ...
    # endregion

    # region Dunder Methods
    def __eq__(self, other=VehicleAckermannControl): ...
    def __ne__(self, __value: VehicleAckermannControl) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class VehicleControl():
    """Manages the basic movement of a vehicle using typical driving controls."""
    # region Instance Variables
    @property
    def throttle() -> float:
        """A scalar value to control the vehicle throttle [0.0, 1.0]. Default is 0.0."""
    @property
    def steer() -> float:
        """A scalar value to control the vehicle steering [-1.0, 1.0]. Default is 0.0."""
    @property
    def brake() -> float:
        """A scalar value to control the vehicle brake [0.0, 1.0]. Default is 0.0."""
    @property
    def hand_brake() -> bool:
        """Determines whether hand brake will be used. Default is `False`."""
    @property
    def reverse() -> bool:
        """Determines whether the vehicle will move backwards. Default is `False`."""
    @property
    def manual_gear_shift() -> bool:
        """Determines whether the vehicle will be controlled by changing gears manually. Default is `False`.
        """
    @property
    def gear() -> int:
        """States which gear is the vehicle running on."""
    # endregion

    # region Methods
    def __init__(self, throttle=0.0, steer=0.0, brake=0.0, hand_brake=False, reverse=False, manual_gear_shift=False, gear=0):
        """
        Args:
            `throttle (float, optional)`: Scalar value between [0.0,1.0]. Defaults to 0.0.\n
            `steer (float, optional)`: Scalar value between [0.0,1.0]. Defaults to 0.0.\n
            `brake (float, optional)`: Scalar value between [0.0,1.0]. Defaults to 0.0.\n
            `hand_brake (bool, optional)` Defaults to False.\n
            `reverse (bool, optional)` Defaults to False.\n
            `manual_gear_shift (bool, optional)` Defaults to False.\n
            `gear (int, optional)` Defaults to 0.\n
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: VehicleControl) -> bool: ...
    def __ne__(self, __value: VehicleControl) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class VehicleDoor(Enum):
    """Possible index representing the possible doors that can be open. Notice that not all possible doors are able to open in some vehicles."""
    FL = 0,
    """Front left door."""
    FR = 1,
    """Front right door."""
    RL = 2,
    """Back left door."""
    RR = 3,
    """Back right door."""
    All = 6,
    """Represents all doors."""


class VehicleFailureState(Enum):
    """Enum containing the different failure states of a vehicle, from which the it cannot recover. These are returned by get_failure_state() and only Rollover is currently implemented."""

    NONE = 0,
    Rollover = 1,
    Engine = 2,
    TirePuncture = 3,


class VehicleLightState(Enum):
    """Class that recaps the state of the lights of a vehicle, these can be used as a flags.

    E.g: `VehicleLightState.HighBeam & VehicleLightState.Brake` will return `True` when both are active.

    Lights are off by default in any situation and should be managed by the user via script. The blinkers blink automatically.

    + Warning: Right now, not all vehicles have been prepared to work with this functionality, this will be added to all of them in later updates."""
    NONE = 0,
    """All lights off."""
    Position = 1,
    LowBeam = 2,
    HighBeam = 4,
    Brake = 8,
    RightBlinker = 16,
    LeftBlinker = 32,
    Reverse = 64,
    Fog = 128,
    Interior = 256,
    Special1 = 512,
    """This is reserved for certain vehicles that can have special lights, like a siren."""
    Special2 = 1024,
    """This is reserved for certain vehicles that can have special lights, like a siren."""
    All = -1,
    """All lights on."""


class VehiclePhysicsControl():
    """Summarizes the parameters that will be used to simulate a carla.Vehicle as a physical object. The specific settings for the wheels though are stipulated using `carla.WheelPhysicsControl`."""

    # region Instance Variables
    @property
    def torque_curve() -> list[Vector2D]:
        """Curve that indicates the torque measured in Nm for a specific RPM of the vehicle's engine."""
    @property
    def max_rpm() -> float:
        """The maximum RPM of the vehicle's engine."""

    @property
    def moi() -> float:
        """The moment of inertia of the vehicle's engine. (kg*m^2)"""

    @property
    def damping_rate_full_throttle() -> float:
        """Damping ratio when the throttle is maximum."""

    @property
    def damping_rate_zero_throttle_clutch_engaged() -> float:
        """Damping ratio when the throttle is zero with clutch engaged."""

    @property
    def damping_rate_zero_throttle_clutch_disengaged() -> float:
        """Damping ratio when the throttle is zero with clutch disengaged."""

    @property
    def use_gear_autobox() -> bool:
        """If `True`, the vehicle will have an automatic transmission."""

    @property
    def gear_switch_time() -> float:
        """Switching time between gears. (seconds)"""

    @property
    def clutch_strength() -> float:
        """Clutch strength of the vehicle (kg*m^2/s)."""

    @property
    def final_ratio() -> float:
        """Fixed ratio from transmission to wheels."""

    @property
    def forward_gears() -> list[GearPhysicsControl]:
        """List of objects defining the vehicle's gears."""

    @property
    def mass() -> float:
        """Mass of the vehicle (kilograms)."""

    @property
    def drag_coefficient() -> float:
        """Drag coefficient of the vehicle's chassis."""

    @property
    def center_of_mass() -> Vector3D:
        """Center of mass of the vehicle (meters)."""

    @property
    def steering_curve() -> list[Vector2D]:
        """Curve that indicates the maximum steering for a specific forward speed."""

    @property
    def use_sweep_wheel_collision() -> bool:
        """Enable the use of sweep for wheel collision. By default, it is disabled and it uses a simple raycast from the axis to the floor for each wheel. This option provides a better collision model in which the full volume of the wheel is checked against collisions."""

    @property
    def wheels() -> list[WheelPhysicsControl]:
        """List of wheel physics objects. This list should have 4 elements, where index 0 corresponds to the front left wheel, index 1 corresponds to the front right wheel, index 2 corresponds to the back left wheel and index 3 corresponds to the back right wheel. For 2 wheeled vehicles, set the same values for both front and back wheels."""
    # endregion

    # region Methods
    def __init__(self,
                 torque_curve=[[0.0, 500.0], [5000.0, 500.0]],
                 max_rpm=5000.0,
                 moi=1.0,
                 damping_rate_full_throttle=0.15, damping_rate_zero_throttle_clutch_engaged=2.0, damping_rate_zero_throttle_clutch_disengaged=0.35, use_gear_autobox=True,
                 gear_switch_time=0.5,
                 clutch_strength=10.0,
                 final_ratio=4.0,
                 forward_gears=list(),
                 drag_coefficient=0.3,
                 center_of_mass=[0.0, 0.0, 0.0],
                 steering_curve=[[0.0, 1.0], [10.0, 0.5]],
                 wheels=list(),
                 use_sweep_wheel_collision=False,
                 mass=1000.0) -> VehiclePhysicsControl:
        """VehiclePhysicsControl constructor.

        Args:
            `torque_curve (list, optional)`. Defaults to [[0.0, 500.0], [5000.0, 500.0]].\n
            `max_rpm (float, optional)`. Defaults to 5000.0.\n
            `moi (float, optional)`: (kg*m^2). Defaults to 1.0.\n
            `damping_rate_full_throttle (float, optional)`. Defaults to 0.15.\n
            `damping_rate_zero_throttle_clutch_engaged (float, optional)`: Defaults to 2.0.\n
            `damping_rate_zero_throttle_clutch_disengaged (float, optional)`: Defaults to 0.35.\n
            `use_gear_autobox (bool, optional)`: Defaults to True.\n
            `gear_switch_time (float, optional)`: (seconds). Defaults to 0.5.\n
            `clutch_strength (float, optional)`: (kg*m^2/s). Defaults to 10.0.\n
            `final_ratio (float, optional)`: Defaults to 4.0.\n
            `forward_gears (list[GearPhysicsControl] optional)`: Defaults to list().\n
            `drag_coefficient (float, optional)`: Defaults to 0.3.\n
            `center_of_mass (list, optional)`: Defaults to [0.0, 0.0, 0.0].\n
            `steering_curve (list, optional)`: Defaults to [[0.0, 1.0], [10.0, 0.5]].\n
            `wheels (_type_, optional)`: Defaults to list().\n
            `use_sweep_wheel_collision (bool, optional)`: Defaults to False.\n
            `mass (float, optional)`: (kilograms). Defaults to 1000.0.\n
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, other=VehiclePhysicsControl) -> bool: ...
    def __ne__(self, other=VehiclePhysicsControl) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class VehicleWheelLocation(Enum):
    """enum representing the position of each wheel on a vehicle. Used to identify the target wheel when setting an angle in `carla.Vehicle.set_wheel_steer_direction` or `carla.Vehicle.get_wheel_steer_angle`."""
    FL_Wheel = 0,
    """Front left wheel of a 4 wheeled vehicle."""
    FR_Wheel = 1,
    """Front right wheel of a 4 wheeled vehicle."""
    BL_Wheel = 2,
    """Back left wheel of a 4 wheeled vehicle."""
    BR_Wheel = 3,
    """Back right wheel of a 4 wheeled vehicle."""
    Front_Wheel = 0,
    """Front wheel of a 2 wheeled vehicle."""
    Back_Wheel = 1,
    """Back wheel of a 2 wheeled vehicle."""


class Walker(Actor):
    """This class inherits from the `carla.Actor` and defines pedestrians in the simulation. Walkers are a special type of actor that can be controlled either by an AI (`carla.WalkerAIController`) or manually via script, using a series of `carla.WalkerControl` to move these and their skeletons.
    """

    # region Methods
    def apply_control(self, control: WalkerControl):
        """On the next tick, the control will move the walker in a certain direction with a certain speed. Jumps can be commanded too."""

    def blend_pose(self, blend_value: float):
        """Set the blending value of the custom pose with the animation. The values can be:

        + 0: will show only the animation
        + 1: will show only the custom pose (set by the user with set_bones())
        + any other: will interpolate all the bone positions between animation and the custom pose.

        Args:
            `blend_value (float)`: value from 0 to 1 with the blend percentage\n
        """

    def hide_pose(self):
        """Hide the custom pose and show the animation (same as calling `blend_pose(0)`).
        """

    def show_pose(self):
        """Show the custom pose and hide the animation (same as calling `blend_pose(1)`).
        """
    # endregion

    # region Getters
    def get_bones(self) -> WalkerBoneControlOut:
        """Return the structure with all the bone transformations from the actor. For each bone, we get the name and its transform in three different spaces:

        + name: bone name
        + world: transform in world coordinates
        + component: transform based on the pivot of the actor
        + relative: transform based on the bone parent.


        + Setter: `carla.Walker.set_bones`

        Returns:
            `WalkerBoneControlOut`: _description_\n
        """

    def get_control(self) -> WalkerControl:
        """The client returns the control applied to this walker during last tick. The method does not call the simulator.
        """

    def get_pose_from_animation(self):
        """Make a copy of the current animation frame as the custom pose. Initially the custom pose is the neutral pedestrian pose."""

    # endregion

    # region Setters
    def set_bones(self, bones: list[WalkerBoneControlIn]):
        """Set the bones of the actor. For each bone we want to set we use a relative transform. Only the bones in this list will be set. For each bone you need to setup this info:

        + name: bone name
        + relative: transform based on the bone parent.

        + Getter: `carla.Walker.get_bones`

        Args:
            `bones (WalkerBoneControlIn)`: list of pairs (bone_name, transform) for the bones that we want to set)\n
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class WalkerAIController(Actor):
    """Class that conducts AI control for a walker. The controllers are defined as actors, but they are quite different from the rest. They need to be attached to a parent actor during their creation, which is the walker they will be controlling (take a look at `carla.World` if you are yet to learn on how to spawn actors). They also need for a special blueprint (already defined in `carla.BlueprintLibrary` as "controller.ai.walker"). This is an empty blueprint, as the AI controller will be invisible in the simulation but will follow its parent around to dictate every step of the way.
    """

    # region Methods
    def go_to_location(self, destination: Location):
        """Sets the destination that the pedestrian will reach.

        Args:
            `destination (Location)`: (meters)\n
        """

    def start(self):
        """Enables AI control for its parent walker."""

    def stop(self):
        """Disables AI control for its parent walker."""
    # endregion

    # region Setters
    def set_max_speed(self, speed=1.4):
        """Sets a speed for the walker in meters per second.

        Args:
            `speed (float, optional)`: An easy walking speed is set by default (m/s). Defaults to 1.4.\n
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class WalkerBoneControlIn():
    """This class grants bone specific manipulation for walker. The skeletons of walkers have been unified for clarity and the transform applied to each bone are always relative to its parent. Take a look here to learn more on how to create a walker and define its movement.

    https://carla.readthedocs.io/en/latest/tuto_G_control_walker_skeletons/
    """

    # region Instance Variables
    @property
    def bone_transforms() -> list[tuple[str, Transform]]:
        """
        List with the data for each bone we want to set:
        + name: bone name
        + relative: transform based on the bone parent.
        """
    # endregion

    # region Methods
    def __init__(self, bone_transforms: list[tuple[str, Transform]]) -> WalkerBoneControlIn:
        """Initializes an object containing moves to be applied on tick. These are listed with the name of the bone and the transform that will be applied to it.

        Args:
            `bone_transforms (list[tuple[str,Transform]])`\n
        """
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class WalkerBoneControlOut():
    """This class is used to return all bone positions of a pedestrian. For each bone we get its name and its transform in three different spaces (world, actor and relative)."""

    # region Instance Variables
    def bone_transforms() -> list[tuple[str, Transform, Transform, Transform]]:
        """
        List of one entry per bone with this information:

        + name: bone name
        + world: transform in world coordinates
        + component: transform based on the pivot of the actor
        + relative: transform based on the bone parent.
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class WalkerControl():
    """This class defines specific directions that can be commanded to a `carla.Walker` to control it via script.

    AI control can be settled for walkers, but the control used to do so is `carla.WalkerAIController`.
    """

    # region Instance Variables
    @property
    def direction() -> Vector3D:
        """Vector using global coordinates that will correspond to the direction of the walker."""
    @property
    def speed() -> float:
        """A scalar value to control the walker's speed (m/s)."""
    @property
    def jump() -> bool:
        """If `True`, the walker will perform a jump."""
    # endregion

    # region Methods
    def __init__(self, direction=[1.0, 0.0, 0.0], speed=0.0, jump=False) -> WalkerControl:
        """This class defines specific directions that can be commanded to a `carla.Walker` to control it via script.

        Args:
            `direction (list, optional)`: Vector using global coordinates that will correspond to the direction of the walker.. Defaults to [1.0, 0.0, 0.0].\n
            `speed (float, optional)`: A scalar value to control the walker's speed (m/s). Defaults to 0.0.\n
            `jump (bool, optional)`: If `True`, the walker will perform a jump. Defaults to False.\n
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: WalkerControl) -> bool: ...
    def __ne__(self, __value: WalkerControl) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class Waypoint():
    """Waypoints in CARLA are described as 3D directed points. They have a `carla.Transform` which locates the waypoint in a road and orientates it according to the lane. They also store the road information belonging to said point regarding its lane and lane markings.

    All the information regarding waypoints and the waypoint API is retrieved as provided by the OpenDRIVE file. Once the client asks for the map object to the server, no longer communication will be needed.
    """
    # region Instance Variables
    @property
    def id() -> int:
        """The identifier is generated using a hash combination of the road, section, lane and s values that correspond to said point in the OpenDRIVE geometry. The s precision is set to 2 centimeters, so 2 waypoints closer than 2 centimeters in the same road, section and lane, will have the same identificator."""

    @property
    def transform() -> Transform:
        """Position and orientation of the waypoint according to the current lane information. This data is computed the first time it is accessed. It is not created right away in order to ease computing costs when lots of waypoints are created but their specific transform is not needed."""

    @property
    def road_id() -> int:
        """OpenDRIVE road's id."""

    @property
    def section_id() -> int:
        """OpenDRIVE section's id, based on the order that they are originally defined."""

    @property
    def is_junction() -> bool:
        """True if the current Waypoint is on a junction as defined by OpenDRIVE."""

    @property
    def junction_id() -> int:
        """OpenDRIVE junction's id. For more information refer to OpenDRIVE documentation.

        http://www.opendrive.org/docs/OpenDRIVEFormatSpecRev1.4H.pdf#page=20
        """

    @property
    def lane_id() -> int:
        """OpenDRIVE lane's id, this value can be positive or negative which represents the direction of the current lane with respect to the road. For more information refer to OpenDRIVE documentation."""

    @property
    def s() -> float:
        """OpenDRIVE s value of the current position."""

    @property
    def lane_width() -> float:
        """Horizontal size of the road at current `s`."""

    @property
    def lane_change() -> LaneChange:
        """Lane change definition of the current Waypoint's location, based on the traffic rules defined in the OpenDRIVE file. It states if a lane change can be done and in which direction."""

    @property
    def lane_type() -> LaneType:
        """The lane type of the current Waypoint, based on OpenDRIVE 1.4 standard."""

    @property
    def right_lane_marking() -> LaneMarking:
        """The right lane marking information based on the direction of the Waypoint."""

    @property
    def left_lane_marking() -> LaneMarking:
        """The left lane marking information based on the direction of the Waypoint."""
    # endregion

    # region Methods
    def next(self, distance: float) -> list[Waypoint]:
        """Returns a list of waypoints at a certain approximate `distance` from the current one. It takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option. The list may be empty if the lane is not connected to any other at the specified distance.

        Args:
            `distance (float)`: The approximate distance where to get the next waypoints (meters).\n

        Returns:
            `list[Waypoint]`\n
        """

    def next_until_lane_end(self, distance: float) -> list[Waypoint]:
        """This method does not return the waypoint previously visited by an actor, but a list of waypoints at an approximate `distance` but in the opposite direction of the lane. Similarly to `next()`, it takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option. The list may be empty if the lane is not connected to any other at the specified distance.


        Args:
            `distance (float)`: The approximate distance where to get the previous waypoints (meters).\n

        Returns:
            `list[Waypoint]`\n
        """

    def previous(self, distance: float) -> list[Waypoint]:
        """This method does not return the waypoint previously visited by an actor, but a list of waypoints at an approximate `distance` but in the opposite direction of the lane. Similarly to `next()`, it takes into account the road and its possible deviations without performing any lane change and returns one waypoint per option. The list may be empty if the lane is not connected to any other at the specified distance.

        Args:
            `distance (float)`: The approximate distance where to get the previous waypoints (meters).\n

        Returns:
            `list[Waypoint]`\n
        """

    def previous_until_lane_start(self, distance: float) -> list[Waypoint]:
        """Returns a list of waypoints from this to the start of the lane separated by a certain distance.

        Args:
            `distance (float)`: The approximate distance between waypoints (meters).\n

        Returns:
            `list[Waypoint]`\n
        """
    # endregion
    # region Getters

    def get_junction(self) -> Junction:
        """If the waypoint belongs to a junction this method returns the associated junction object. Otherwise returns `null`."""

    def get_landmarks(self, distance: float, stop_at_junction=False) -> list[Landmark]:
        """Returns a list of landmarks in the road from the current waypoint until the specified distance.

        Args:
            `distance (float)`: The maximum distance to search for landmarks from the current waypoint (meters).\n
            `stop_at_junction (bool, optional)`: Enables or disables the landmark search through junctions. Defaults to False.\n
        """

    def get_landmarks_of_type(self, distance: float, type: str, stop_at_junction=False) -> list[Landmark]:
        """Returns a list of landmarks in the road of a specified type from the current waypoint until the specified distance.

        Args:
            `distance (float)`: The maximum distance to search for landmarks from the current waypoint (meters).\n
            `type (str)`: The type of landmarks to search.\n
            `stop_at_junction (bool, optional)`: Enables or disables the landmark search through junctions. Defaults to False.\n
        """

    def get_left_lane(self) -> Waypoint:
        """Generates a Waypoint at the center of the left lane based on the direction of the current Waypoint, taking into account if the lane change is allowed in this location. Will return `None` if the lane does not exist.
        """

    def get_right_lane(self) -> Waypoint:
        """Generates a waypoint at the center of the right lane based on the direction of the current waypoint, taking into account if the lane change is allowed in this location. Will return None if the lane does not exist."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion


class WeatherParameters():
    """This class defines objects containing lighting and weather specifications that can later be applied in `carla.World`. So far, these conditions only intervene with `sensor.camera.rgb`. They neither affect the actor's physics nor other sensors.

    Each of these parameters acts indepently from the rest. Increasing the rainfall will not automatically create puddles nor change the road's humidity. That makes for a better customization but means that realistic conditions need to be scripted. However an example of dynamic weather conditions working realistically can be found here.

    https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/dynamic_weather.py
    """
    # region Instance Variables
    @property
    def cloudiness() -> float:
        """Values range from 0 to 100, being 0 a clear sky and 100 one completely covered with clouds."""
    @property
    def precipitation() -> float:
        """Rain intensity values range from 0 to 100, being 0 none at all and 100 a heavy rain."""
    @property
    def precipitation_deposits() -> float:
        """Determines the creation of puddles. Values range from 0 to 100, being 0 none at all and 100 a road completely capped with water. Puddles are created with static noise, meaning that they will always appear at the same locations."""
    @property
    def wind_intensity() -> float:
        """Controls the strenght of the wind with values from 0, no wind at all, to 100, a strong wind. The wind does affect rain direction and leaves from trees, so this value is restricted to avoid animation issues."""
    @property
    def sun_azimuth_angle() -> float:
        """The azimuth angle of the sun. Values range from 0 to 360. Zero is an origin point in a sphere determined by Unreal Engine (degrees)."""
    @property
    def sun_altitude_angle() -> float:
        """Altitude angle of the sun. Values range from -90 to 90 corresponding to midnight and midday each (degrees)."""
    @property
    def fog_density() -> float:
        """Fog concentration or thickness. It only affects the RGB camera sensor. Values range from 0 to 100."""
    @property
    def fog_distance() -> float:
        """Fog start distance. Values range from 0 to infinite (meters)."""
    @property
    def wetness() -> float:
        """Wetness intensity. It only affects the RGB camera sensor. Values range from 0 to 100."""
    @property
    def fog_falloff() -> float:
        """Density of the fog (as in specific mass) from 0 to infinity. The bigger the value, the more dense and heavy it will be, and the fog will reach smaller heights. Corresponds to `Fog Height Falloff` in the UE docs.

        If the value is 0, the fog will be lighter than air, and will cover the whole scene.
        A value of 1 is approximately as dense as the air, and reaches normal-sized buildings.

        For values greater than 5, the air will be so dense that it will be compressed on ground level.
        """
    @property
    def scattering_intensity() -> float:
        """Controls how much the light will contribute to volumetric fog. When set to 0, there is no contribution."""
    @property
    def mie_scattering_scale() -> float:
        """Controls interaction of light with large particles like pollen or air pollution resulting in a hazy sky with halos around the light sources. When set to 0, there is no contribution."""
    @property
    def rayleigh_scattering_scale() -> float:
        """Controls interaction of light with small particles like air molecules. Dependent on light wavelength, resulting in a blue sky in the day or red sky in the evening."""
    @property
    def dust_storm() -> float:
        """Determines the strength of the dust storm weather. Values range from 0 to 100."""
    # endregion

    # region Methods
    def __init__(self,
                 cloudiness=0.0,
                 precipitation=0.0,
                 precipitation_deposits=0.0,
                 wind_intensity=0.0,
                 sun_azimuth_angle=0.0,
                 sun_altitude_angle=0.0,
                 fog_density=0.0,
                 fog_distance=0.0,
                 wetness=0.0,
                 fog_falloff=0.0,
                 scattering_intensity=0.0,
                 mie_scattering_scale=0.0,
                 rayleigh_scattering_scale=0.0331) -> WeatherParameters:
        """Method to initialize an object defining weather conditions. This class has some presets for different noon and sunset conditions listed in a note below.

        + Note: ClearNoon, CloudyNoon, WetNoon, WetCloudyNoon, SoftRainNoon, MidRainyNoon, HardRainNoon, ClearSunset, CloudySunset, WetSunset, WetCloudySunset, SoftRainSunset, MidRainSunset, HardRainSunset.

        Args:
            `cloudiness (float, optional)`: 0 is a clear sky, 100 complete overcast. Defaults to 0.0.\n
            `precipitation (float, optional)`: 0 is no rain at all, 100 a heavy rain. Defaults to 0.0.\n
            `precipitation_deposits (float, optional)`: 0 means no puddles on the road, 100 means roads completely capped by rain. Defaults to 0.0.\n
            `wind_intensity (float, optional)`:  0 is calm, 100 a strong wind. Defaults to 0.0.\n
            `sun_azimuth_angle (float, optional)`: 0 is an arbitrary North, 180 its corresponding South. Defaults to 0.0.\n
            `sun_altitude_angle (float, optional)`: 90 is midday, -90 is midnight. Defaults to 0.0.\n
            `fog_density (float, optional)`: Concentration or thickness of the fog, from 0 to 100. Defaults to 0.0.\n
            `fog_distance (float, optional)`: Distance where the fog starts in meters. Defaults to 0.0.\n
            `wetness (float, optional)`: Humidity percentages of the road, from 0 to 100. Defaults to 0.0.\n
            `fog_falloff (float, optional)`: Density (specific mass) of the fog, from 0 to infinity. Defaults to 0.0.\n
            `scattering_intensity (float, optional)`: Controls how much the light will contribute to volumetric fog. When set to 0, there is no contribution. Defaults to 0.0.\n
            `mie_scattering_scale (float, optional)`: Controls interaction of light with large particles like pollen or air pollution resulting in a hazy sky with halos around the light sources. When set to 0, there is no contribution. Defaults to 0.0.\n
            `rayleigh_scattering_scale (float, optional)`: Controls interaction of light with small particles like air molecules. Dependent on light wavelength, resulting in a blue sky in the day or red sky in the evening. Defaults to 0.0331.\n
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: WeatherParameters) -> bool: ...
    def __ne__(self, __value: WeatherParameters) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class WheelPhysicsControl():
    """Class that defines specific physical parameters for wheel objects that will be part of a `carla.VehiclePhysicsControl` to simulate vehicle it as a material object."""

    # region Instance Variables
    @property
    def tire_friction() -> float:
        """A scalar value that indicates the friction of the wheel."""
    @property
    def damping_rate() -> float:
        """Damping rate of the wheel."""
    @property
    def max_steer_angle() -> float:
        """Maximum angle that the wheel can steer (degrees)."""
    @property
    def radius() -> float:
        """Radius of the wheel (centimeters)."""
    @property
    def max_brake_torque() -> float:
        """Maximum brake torque (N*m)."""
    @property
    def max_handbrake_torque() -> float:
        """Maximum handbrake torque."""
    @property
    def position() -> Vector3D:
        """World position of the wheel. This is a read-only parameter."""
    @property
    def long_stiff_value() -> float:
        """Tire longitudinal stiffness per unit gravitational acceleration. Each vehicle has a custom value (kg/rad)."""
    @property
    def lat_stiff_max_load() -> float:
        """Maximum normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire. Each vehicle has a custom value."""
    @property
    def lat_stiff_value() -> float:
        """Maximum stiffness per unit of lateral slip. Each vehicle has a custom value."""
    # endregion

    # region Methods
    def __init__(self,
                 tire_friction=2.0,
                 damping_rate=0.25,
                 max_steer_angle=70.0,
                 radius=30.0,
                 max_brake_torque=1500.0,
                 max_handbrake_torque=3000.0,
                 position: Vector3D = (0.0, 0.0, 0.0)) -> WheelPhysicsControl: ...
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: WheelPhysicsControl) -> bool: ...
    def __ne__(self, __value: WheelPhysicsControl) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class World():
    """World objects are created by the client to have a place for the simulation to happen. The world contains the map we can see, meaning the asset, not the navigation map. Navigation maps are part of the `carla.Map` class. It also manages the weather and actors present in it. There can only be one world per simulation, but it can be changed anytime."""

    # region Instance Variables
    @property
    def id() -> int:
        """The ID of the episode associated with this world. Episodes are different sessions of a simulation. These change everytime a world is disabled or reloaded. Keeping track is useful to avoid possible issues."""
    @property
    def debug() -> DebugHelper:
        """Responsible for creating different shapes for debugging. Take a look at its class to learn more about it."""
    # endregion

    # region Methods
    def apply_color_texture_to_object(self, object_name: str, material_parameter: MaterialParameter, texture: TextureColor):
        """Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to `object_name`."""

    def apply_color_texture_to_objects(self, objects_name_list: list[str], material_parameter: MaterialParameter, texture: TextureColor):
        """Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to all objects in `objects_name_list`."""

    def apply_float_color_texture_to_object(self, object_name: str, material_parameter: MaterialParameter, texture: TextureFloatColor):
        """Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to all objects in `objects_name_list`."""

    def apply_float_color_texture_to_objects(self, objects_name_list: list[str], material_parameter: MaterialParameter, texture: TextureFloatColor):
        """Applies a `texture` object in the field corresponfing to `material_parameter` (normal, diffuse, etc) to the object in the scene corresponding to all objects in `objects_name_list`."""

    def apply_settings(self, world_settings: WorldSettings) -> int:
        """This method applies settings contained in an object to the simulation running and returns the ID of the frame they were implemented.

        + Warning: If synchronous mode is enabled, and there is a Traffic Manager running, this must be set to sync mode too. Read this to learn how to do it.
        https://carla.readthedocs.io/en/latest/adv_traffic_manager/#synchronous-mode
        """

    def apply_textures_to_object(self, object_name: str, diffuse_texture: TextureColor, emissive_texture: TextureFloatColor, normal_texture: TextureFloatColor, ao_roughness_metallic_emissive_texture: TextureFloatColor):
        """Applies all texture fields in `carla.MaterialParameter` to the object `object_name`. Empty textures here will not be applied.
        """

    def apply_textures_to_objects(self, objects_name_list: list[str], diffuse_texture: TextureColor, emissive_texture: TextureFloatColor, normal_texture: TextureFloatColor, ao_roughness_metallic_emissive_texture: TextureFloatColor):
        """Applies all texture fields in `carla.MaterialParameter` to all objects in `objects_name_list`. Empty textures here will not be applied."""

    def cast_ray(self, initial_location: Location, final_location: Location) -> list[LabelledPoint]:
        """Casts a ray from the specified initial_location to final_location. The function then detects all geometries intersecting the ray and returns a list of `carla.LabelledPoint` in order.

        Args:
            initial_location (Location): The initial position of the ray.
            final_location (Location): The final position of the ray.
        """

    def enable_environment_objects(self, env_objects_ids: set[int], enable: bool):
        """Enable or disable a set of EnvironmentObject identified by their id. These objects will appear or disappear from the level.

        Args:
            env_objects_ids (set[int]): Set of EnvironmentObject ids to change.
            enable (bool): State to be applied to all the EnvironmentObject of the set.
        """

    def freeze_all_traffic_lights(self, frozen: bool):
        """Freezes or unfreezes all traffic lights in the scene. Frozen traffic lights can be modified by the user but the time will not update them until unfrozen."""

    def ground_projection(self, location: Location, search_distance: float) -> LabelledPoint:
        """Projects the specified point downwards in the scene. The functions casts a ray from location in the direction (0,0,-1) (downwards) and returns a `carla.LabelledPoint` object with the first geometry this ray intersects (usually the ground). If no geometry is found in the search_distance range the function returns `None`.

        Args:
            location (Location): The point to be projected.
            search_distance (float): The maximum distance to perform the projection.
        """

    def load_map_layer(self, map_layers: MapLayer):
        """Loads the selected layers to the level. If the layer is already loaded the call has no effect.

        + Warning: This only affects "Opt" maps. The minimum layout includes roads, sidewalks, traffic lights and traffic signs.

        Args:
            map_layers (MapLayer): Mask of level layers to be loaded.
        """

    def on_tick(self, callback: Callable) -> int:
        """This method is used in `asynchronous mode`. It starts callbacks from the client for the function defined as `callback`, and returns the ID of the callback. The function will be called everytime the server ticks. It requires a `carla.WorldSnapshot` as argument, which can be retrieved from `wait_for_tick()`. Use `remove_on_tick()` to stop the callbacks.

        Args:
            callback (Callable): Function with a `snapshot` as compulsory parameter that will be called when the client receives a tick.
        """

    def project_point(self, location: Location, direction: Vector3D, search_distance: float) -> LabelledPoint:
        """Projects the specified point to the desired direction in the scene. The functions casts a ray from location in a direction and returns a carla.Labelled object with the first geometry this ray intersects. If no geometry is found in the search_distance range the function returns `None`.

        Args:
            `location (Location)`: The point to be projected.\n
            `direction (Vector3D)`: The direction of projection.\n
            `search_distance (float)`: The maximum distance to perform the projection.\n

        Returns:
            `LabelledPoint`\n
        """

    def remove_on_tick(self, callback_id: int):
        """Stops the callback for callback_id started with `on_tick()`.

        Args:
            `callback_id (int)`: The callback to be removed. The ID is returned when creating the callback.\n
        """

    def reset_all_traffic_lights(self):
        """Resets the cycle of all traffic lights in the map to the initial state.
        """

    def spawn_actor(self, blueprint: ActorBlueprint, transform: Transform, attach_to: Actor = None, attachment=AttachmentType.Rigid) -> Actor:
        """The method will create, return and spawn an actor into the world. The actor will need an available blueprint to be created and a transform (location and rotation). It can also be attached to a parent with a certain attachment type.

        Args:
            `blueprint (ActorBlueprint)`: The reference from which the actor will be created.\n
            `transform (Transform)`: Contains the location and orientation the actor will be spawned with.\n
            `attach_to (Actor, optional)`: The parent object that the spawned actor will follow around. Defaults to None.\n
            `attachment (AttachmentType, optional)`: Determines how fixed and rigorous should be the changes in position according to its parent object. Defaults to AttachmentType.Rigid.\n

        Returns:
            `Actor`\n
        """

    def tick(self, seconds=10.0) -> int:
        """This method is used in synchronous mode, when the server waits for a client tick before computing the next frame. This method will send the tick, and give way to the server. It returns the ID of the new frame computed by the server.

        + Note: If no tick is received in synchronous mode, the simulation will freeze. Also, if many ticks are received from different clients, there may be synchronization issues. Please read the docs about synchronous mode to learn more.

        Args:
            `seconds (float, optional)`: Maximum time the server should wait for a tick (meters). Defaults to 10.0.\n

        Returns:
            `int`\n
        """

    def try_spawn_actor(self, blueprint: ActorBlueprint, transform: Transform, attach_to: Actor = None, attachment=AttachmentType.Rigid) -> Actor:
        """Same as `spawn_actor()` but returns `None` on failure instead of throwing an exception.

        Args:
            `blueprint (ActorBlueprint)`: The reference from which the actor will be created.\n
            `transform (Transform)`: Contains the location and orientation the actor will be spawned with.\n
            `attach_to (Actor, optional)`: The parent object that the spawned actor will follow around. Defaults to None.\n
            `attachment (AttachmentType, optional)`: Determines how fixed and rigorous should be the changes in position according to its parent object. Defaults to AttachmentType.Rigid.\n

        Returns:
            `Actor`\n
        """

    def unload_map_layer(self, map_layers: MapLayer):
        """Unloads the selected layers to the level. If the layer is already unloaded the call has no effect.

        + Warning: This only affects "Opt" maps. The minimum layout includes roads, sidewalks, traffic lights and traffic signs.

        Args:
            `map_layers (MapLayer)`: Mask of level layers to be unloaded.\n
        """

    def wait_for_tick(self, seconds=10.0) -> WorldSnapshot:
        """This method is used in asynchronous mode. It makes the client wait for a server tick. When the next frame is computed, the server will tick and return a snapshot describing the new state of the world.

        Args:
            `seconds (float, optional)`: Maximum time the server should wait for a tick (meters). Defaults to 10.0.\n

        Returns:
            `WorldSnapshot`\n
        """
    # endregion

    # region Getters
    def get_actor(self, actor_id: int) -> Actor:
        """Looks up for an actor by ID and returns `None` if not found."""

    def get_actors(self, actor_ids: list = None) -> ActorList:
        """Retrieves a list of `carla.Actor` elements, either using a list of IDs provided or just listing everyone on stage. If an ID does not correspond with any actor, it will be excluded from the list returned, meaning that both the list of IDs and the list of actors may have different lengths."""

    def get_blueprint_library(self) -> list[BlueprintLibrary]:
        """Returns a list of actor blueprints available to ease the spawn of these into the world."""

    def get_environment_objects(self, object_type: CityObjectLabel = CityObjectLabel.Any) -> list[EnvironmentObject]:
        """Returns a list of EnvironmentObject with the requested semantic tag. The method returns all the EnvironmentObjects in the level by default, but the query can be filtered by semantic tags with the argument `object_type`.

        Args:
            `object_type (CityObjectLabel, optional)`: Semantic tag of the EnvironmentObjects that are returned. Defaults to CityObjectLabel.Any.\n
        """

    def get_level_bbs(self, actor_type: CityObjectLabel = CityObjectLabel.Any) -> list[BoundingBox]:
        """Returns an array of bounding boxes with location and rotation in world space. The method returns all the bounding boxes in the level by default, but the query can be filtered by semantic tags with the argument `actor_type`.

        Args:
            `actor_type (CityObjectLabel, optional)`: Semantic tag of the elements contained in the bounding boxes that are returned. Defaults to CityObjectLabel.Any.\n

        Returns:
            `list[BoundingBox]`\n
        """

    def get_lightmanager(self) -> LightManager:
        """Returns an instance of carla.LightManager that can be used to handle the lights in the scene."""

    def get_map(self) -> Map:
        """Asks the server for the XODR containing the map file, and returns this parsed as a `carla.Map`.

        + Warning: This method does call the simulation. It is expensive, and should only be called once.

        """

    def get_names_of_all_objects(self) -> list[str]:
        """Returns a list of the names of all objects in the scene that can be painted with the apply texture functions."""

    def get_random_location_from_navigation(self) -> Location:
        """This can only be used with walkers. It retrieves a random location to be used as a destination using the `go_to_location()` method in `carla.WalkerAIController`. This location will be part of a sidewalk. Roads, crosswalks and grass zones are excluded. The method does not take into consideration locations of existing actors so if a collision happens when trying to spawn an actor, it will return an error. Take a look at `generate_traffic.py` for an example."""

    def get_settings() -> WorldSettings:
        """Returns an object containing some data about the simulation such as synchrony between client and server or rendering mode."""

    def get_snapshot(self) -> WorldSnapshot:
        """Returns a snapshot of the world at a certain moment comprising all the information about the actors."""

    def get_spectator(self) -> Actor:
        """Returns the spectator actor. The spectator is a special type of actor created by Unreal Engine, usually with ID=0, that acts as a camera and controls the view in the simulator window."""

    def get_traffic_light(self, landmark: Landmark) -> TrafficLight:
        """Provided a landmark, returns the traffic light object it describes.

        Args:
            `landmark (Landmark)`: The landmark object describing a traffic light.\n
        """

    def get_traffic_light_from_opendrive_id(self, traffic_light_id: str) -> TrafficLight:
        """Returns the traffic light actor corresponding to the indicated OpenDRIVE id.

        Args:
            `traffic_light_id (str)`: The OpenDRIVE id.\n
        """

    def get_traffic_lights_from_waypoint(self, waypoint: Waypoint, distance: float) -> list[TrafficLight]:
        """This function performs a search along the road in front of the specified waypoint and returns a list of traffic light actors found in the specified search distance.

        Args:
            `waypoint (Waypoint)`: The input waypoint.\n
            `distance (float)`: Search distance.\n
        """

    def get_traffic_lights_in_junction(self, junction_id: int) -> list[TrafficLight]:
        """Returns the list of traffic light actors affecting the junction indicated in `junction_id`.

        Args:
            `junction_id (int)`: The id of the junction.\n
        """

    def get_traffic_sign(self, landmark: Landmark) -> TrafficSign:
        """Provided a landmark, returns the traffic sign object it describes.

        Args:
            `landmark (Landmark)`: The landmark object describing a traffic sign.\n
        """

    def get_vehicles_light_states(self) -> dict[int, VehicleLightState]:
        """Returns a dict where the keys are `carla.Actor` IDs and the values are `carla.VehicleLightState` of that vehicle.
        """

    def get_weather(self) -> WeatherParameters:
        """Retrieves an object containing weather parameters currently active in the simulation, mainly cloudiness, precipitation, wind and sun position.

        + Setter: `carla.World.set_weather`
        """
    # endregion

    # region Setters
    def set_pedestrians_cross_factor(self, percentage: float):
        """Sets the percentage of pedestrians that can walk on the road or cross at any point on the road.

        + Note: Should be set before pedestrians are spawned.

        Args:
            `percentage (float)`: Value should be between 0.0 and 1.0. For example, a value of 0.1 would allow 10% of pedestrians to walk on the road.\n
        """

    def set_pedestrians_seed(self, seed: int):
        """ Sets the seed to use for any random number generated in relation to pedestrians.

        + Note: Should be set before pedestrians are spawned. If you want to repeat the same exact bodies (blueprint) for each pedestrian, then use the same seed in the Python code (where the blueprint is choosen randomly) and here, otherwise the pedestrians will repeat the same paths but the bodies will be different.
        """

    def set_weather(self, weather: WeatherParameters):
        """Changes the weather parameteres ruling the simulation to another ones defined in an object.

        + Getter: `carla.World.get_weather`
        Args:
            `weather (WeatherParameters)`: New conditions to be applied.\n
        """
    # endregion

    # region Dunder Methods
    def __str__(self) -> str:
        """The content of the world is parsed and printed as a brief report of its current state."""
    # endregion


class WorldSettings():
    """The simulation has some advanced configuration options that are contained in this class and can be managed using carla.World and its methods. These allow the user to choose between client-server synchrony/asynchrony, activation of "no rendering mode" and either if the simulation should run with a fixed or variable time-step. Check this out if you want to learn about it.

    https://carla.readthedocs.io/en/latest/adv_synchrony_timestep/
    """

    # region description
    @property
    def synchronous_mode() -> bool:
        """States the synchrony between client and server. When set to true, the server will wait for a client tick in order to move forward. It is `False` by default."""
    @property
    def no_rendering_mode() -> bool:
        """When enabled, the simulation will run no rendering at all. This is mainly used to avoid overhead during heavy traffic simulations. It is `False` by default."""
    @property
    def fixed_delta_seconds() -> float:
        """Ensures that the time elapsed between two steps of the simulation is fixed. Set this to `0.0` to work with a variable time-step, as happens by default."""
    @property
    def substepping() -> bool:
        """Enable the physics substepping. This option allows computing some physics substeps between two render frames. If synchronous mode is set, the number of substeps and its time interval are fixed and computed are so they fulfilled the requirements of `carla.WorldSettings.max_substep` and `carla.WorldSettings.max_substep_delta_time`. These last two parameters need to be compatible with c`arla.WorldSettings.fixed_delta_seconds`. Enabled by default."""
    @property
    def max_substep_delta_time() -> float:
        """Maximum delta time of the substeps. If the carla.`WorldSettingsmax_substep` is high enough, the substep delta time would be always below or equal to this value. By default, the value is set to 0.01."""
    @property
    def max_substeps() -> int:
        """The maximum number of physics substepping that are allowed. By default, the value is set to 10.
        """
    @property
    def max_culling_distance() -> float:
        """Configure the max draw distance for each mesh of the level."""
    @property
    def deterministic_ragdolls() -> bool:
        """Defines wether to use deterministic physics for pedestrian death animations or physical ragdoll simulation. When enabled, pedestrians have less realistic death animation but ensures determinism. When disabled, pedestrians are simulated as ragdolls with more realistic simulation and collision but no determinsm can be ensured."""
    @property
    def tile_stream_distance() -> bool:
        """Used for large maps only. Configures the maximum distance from the hero vehicle to stream tiled maps. Regions of the map within this range will be visible (and capable of simulating physics). Regions outside this region will not be loaded."""
    @property
    def actor_active_distance() -> float:
        """Used for large maps only. Configures the distance from the hero vehicle to convert actors to dormant. Actors within this range will be active, and actors outside will become dormant."""
        ...

    @property
    def spectator_as_ego() -> float:
        """Used for large maps only. Defines the influence of the spectator on tile loading in Large Maps. By default, the spectator will provoke loading of neighboring tiles in the absence of an ego actor. This might be inconvenient for applications that immediately spawn an ego actor."""
    # endregion

    # region Methods
    def __init__(self,
                 synchronous_mode=False,
                 no_rendering_mode=False,
                 fixed_delta_seconds=0.0,
                 max_culling_distance=0.0,
                 deterministic_ragdolls=False,
                 tile_stream_distance=3000,
                 actor_active_distance=2000,
                 spectator_as_ego=True) -> WorldSettings:
        """Creates an object containing desired settings that could later be applied through `carla.World` and its method `apply_settings()`.

        Args:
            `synchronous_mode (bool, optional)`: Set this to true to enable client-server synchrony. Defaults to False.\n
            `no_rendering_mode (bool, optional)`: Set this to true to completely disable rendering in the simulation. Defaults to False.\n
            `fixed_delta_seconds (float, optional)`: Set a fixed time-step in between frames. 0.0 means variable time-step (seconds). Defaults to 0.0.\n
            `max_culling_distance (float, optional)`: Configure the max draw distance for each mesh of the level (meters). Defaults to 0.0.\n
            `deterministic_ragdolls (bool, optional)`: Defines wether to use deterministic physics or ragdoll simulation for pedestrian deaths. Defaults to False.\n
            `tile_stream_distance (int, optional)`: Used for large maps only. Configures the maximum distance from the hero vehicle to stream tiled maps (meters). Defaults to 3000.\n
            `actor_active_distance (int, optional)`: Used for large maps only. Configures the distance from the hero vehicle to convert actors to dormant (meters). Defaults to 2000.\n
            `spectator_as_ego (bool, optional)`: Used for large maps only. Defines the influence of the spectator on tile loading in Large Maps. Defaults to True.\n

        Returns:
            `WorldSettings`: _description_\n
        """
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: WorldSettings) -> bool: ...
    def __ne__(self, __value: WorldSettings) -> bool: ...
    def __str__(self) -> str: ...
    # endregion


class WorldSnapshot():
    """This snapshot comprises all the information for every actor on scene at a certain moment of time. It creates and gives acces to a data structure containing a series of `carla.ActorSnapshot`. The client recieves a new snapshot on every tick that cannot be stored."""
    pass

    # region Instance Variables
    @property
    def id() -> int:
        """A value unique for every snapshot to differentiate them."""
    @property
    def frame() -> int:
        """Simulation frame in which the snapshot was taken."""
    @property
    def timestamp() -> Timestamp:
        """Precise moment in time when snapshot was taken. This class works in seconds as given by the operative system (seconds)."""
    # endregion

    # region Methods
    def find(self, actor_id: int) -> ActorSnapshot:
        """Given a certain actor ID, returns its corresponding snapshot or `None` if it is not found.
        """

    def has_actor(self, actor_id: int) -> bool:
        """Given a certain actor ID, checks if there is a snapshot corresponding it and so, if the actor was present at that moment."""
    # endregion

    # region Dunder Methods
    def __eq__(self, __value: WorldSnapshot) -> bool:
        """Returns True if both `timestamp` are the same."""

    def __iter__(self) -> Iterator[ActorSnapshot]:
        """Iterate over the `carla.ActorSnapshot` stored in the snapshot."""

    def __len__(self) -> int:
        """Returns the amount of `carla.ActorSnapshot` present in this snapshot."""

    def __ne__(self, __value: WorldSnapshot) -> bool:
        """Returns True if both `timestamp` are different."""
    # endregion


class command():

    class ApplyAngularImpulse():
        """Command adaptation of `add_angular_impulse()` in `carla.Actor`. Applies an angular impulse to an actor.
        """
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def impulse() -> Vector3D:
            """Angular impulse applied to the actor (degrees*s)."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], impulse: Vector3D) -> command.ApplyAngularImpulse:
            """Applies an angular impulse to an actor.

            Args:
                `actor (Union[Actor,int])`: Actor or its ID to whom the command will be applied to.\n
                `impulse (Vector3D)`: (degrees*s)\n
            """
        # endregion

    class ApplyForce():
        """Command adaptation of `add_force()` in `carla.Actor`. Applies a force to an actor."""
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def force() -> Vector3D:
            """Force applied to the actor over time (N)."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], force: Vector3D):
            """Applies a force to an actor.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `force (Vector3D)`: (N)\n
            """
        # endregion

    class ApplyImpulse():
        """Command adaptation of `add_impulse()` in `carla.Actor`. Applies an impulse to an actor."""
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def impulse() -> Vector3D:
            """Impulse applied to the actor (N*s)."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], impulse: Vector3D):
            """Applies an impulse to an actor.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `impulse (Vector3D)`: (N*s)\n
            """
        # endregion

    class ApplyTargetAngularVelocity():
        """Command adaptation of `set_target_angular_velocity()` in `carla.Actor`. Sets the actor's angular velocity vector.
        """

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def angular_velocity() -> Vector3D:
            """The 3D angular velocity that will be applied to the actor (deg/s)."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], angular_velocity: Vector3D):
            """Sets the actor's angular velocity vector.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `angular_velocity (Vector3D)`: Angular velocity vector applied to the actor.\n
            """
        # endregion

    class ApplyTargetVelocity():
        """Command adaptation of `set_target_velocity()` in `carla.Actor`. Sets the actor's target velocity vector.
        """
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def velocity() -> Vector3D:
            """The 3D velocity applied to the actor (m/s)."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], velocity: Vector3D):
            """Sets the actor's target velocity vector.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `velocity (Vector3D)`: Velocity vector applied to the actor.\n
            """
        # endregion

    class ApplyTorque():
        """Command adaptation of `set_transform()` in `carla.Actor`. Sets a new transform to an actor.
        """

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def transform() -> Transform:
            """Transformation to be applied."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], transform: Transform):
            """Sets a new transform to an actor.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `transform (Transform)`\n
            """
        # endregion

    class ApplyVehicleAckermannControl():
        """Command adaptation of `apply_ackermann_control()` in `carla.Vehicle`. Applies a certain akermann control to a vehicle.
        """
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def control() -> AckermannControllerSettings:
            """Vehicle ackermann control to be applied."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], control: AckermannControllerSettings):
            """Applies a certain akermann control to a vehicle.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `control (AckermannControllerSettings)`: Vehicle ackermann control to be applied.\n
            """
        # endregion

    class ApplyVehicleControl():
        """Command adaptation of `apply_control()` in `carla.Vehicle`. Applies a certain control to a vehicle."""

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def control() -> VehicleControl:
            """Vehicle control to be applied."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], control: VehicleControl):
            """Applies a certain control to a vehicle.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `control (VehicleControl)`: Vehicle control to be applied.\n
            """
        # endregion

    class ApplyVehiclePhysicsControl():
        """Command adaptation of `apply_physics_control()` in `carla.Vehicle`. Applies a new physics control to a vehicle, modifying its physical parameters."""

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        @property
        def control() -> VehiclePhysicsControl:
            """Physics control to be applied."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], control: VehicleControl):
            """Applies a new physics control to a vehicle, modifying its physical parameters.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `control (VehicleControl)`: Physics control to be applied.\n
            """
        # endregion

    class ApplyWalkerControl():
        """Command adaptation of `apply_control()` in `carla.Walker`. Applies a control to a walker."""

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Walker actor affected by the command."""
        @property
        def control() -> VehiclePhysicsControl:
            """Walker control to be applied."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], control: VehicleControl):
            """Applies a control to a walker.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `control (VehicleControl)`: Walker control to be applied.\n
            """
        # endregion

    class ApplyWalkerControl():
        """Apply a state to the walker actor. Specially useful to initialize an actor them with a specific location, orientation and speed."""

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Walker actor affected by the command."""
        @property
        def transform() -> Transform:
            """Transform to be applied."""
        @property
        def speed() -> float:
            """Speed to be applied (m/s)."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], transform: Transform, speed: float):
            """Apply a state to the walker actor. 

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `transform (Transform)`: Transform to be applied.\n
                `speed (float)`: Speed to be applied (m/s).\n
            """
        # endregion

    class DestroyActor():
        """Command adaptation of `destroy()` in `carla.Actor` that tells the simulator to destroy this actor. It has no effect if the actor was already destroyed. When executed with `apply_batch_sync()` in c`arla.Client` there will be a `command.Response` that will return a boolean stating whether the actor was successfully destroyed."""

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor affected by the command."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int]):
            """Command adaptation of `destroy()` in `carla.Actor` that tells the simulator to destroy this actor.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
            """
        # endregion

    class Response():
        """States the result of executing a command as either the ID of the actor to whom the command was applied to (when succeeded) or an error string (when failed). actor ID, depending on whether or not the command succeeded. The method `apply_batch_sync()` in c`arla.Client` returns a list of these to summarize the execution of a batch.
        """

        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor to whom the command was applied to. States that the command was successful."""
        @property
        def error() -> str:
            """A string stating the command has failed."""
        # endregion

        # region Methods
        def has_error(self) -> bool:
            """Returns `True` if the command execution fails, and `False` if it was successful."""
        # endregion

    class SetAutopilot():
        """Command adaptation of `set_autopilot()` in `carla.Vehicle`. Turns on/off the vehicle's autopilot mode."""

        # region Instance Methods
        @property
        def actor_id() -> int:
            """Actor that is affected by the command."""
        @property
        def enabled() -> bool:
            """If autopilot should be activated or not."""
        @property
        def port() -> int:
            """Port of the Traffic Manager where the vehicle is to be registered or unlisted."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], enabled: bool, port=8000):
            """Turns on/off the vehicle's autopilot mode.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `enabled (bool)`: _description_\n
                `port (int, optional)`:  The Traffic Manager port where the vehicle is to be registered or unlisted. If None is passed, it will consider a TM at default port 8000. Defaults to 8000.\n
            """
        # endregion

    class SetEnableGravity():
        """Command adaptation of `set_enable_gravity()` in `carla.Actor`. Enables or disables gravity on an actor."""
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor that is affected by the command."""
        @property
        def enabled() -> bool:
            """If gravity should be activated or not."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], enabled: bool):
            """Enables or disables gravity on an actor.

            Args:
                `actor (Union[Actor, int])`: Actor or Actor ID to which the command will be applied to.\n
                `enabled (bool)`: If gravity should be activated or not.\n
            """
        # endregion

    class SetSimulatePhysics():
        """Command adaptation of `set_simulate_physics()` in `carla.Actor`. Determines whether an actor will be affected by physics or not."""
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor that is affected by the command."""
        @property
        def enabled() -> bool:
            """If physics should be activated or not."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], enabled: bool):
            """Determines whether an actor will be affected by physics or not.

            Args:
                `actor (Union[Actor, int])`: Actor or Actor ID to which the command will be applied to.\n
                `enabled (bool)`: If physics should be activated or not.\n
            """
        # endregion

    class SetVehicleLightState():
        """Command adaptation of `set_light_state()` in `carla.Vehicle`. Sets the light state of a vehicle."""
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor that is affected by the command."""
        @property
        def light_state() -> VehicleLightState:
            """Recaps the state of the lights of a vehicle, these can be used as a flags."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], light_state: VehicleLightState):
            """Sets the light state of a vehicle.

            Args:
                `actor (Union[Actor, int])`: Actor or its ID to whom the command will be applied to.\n
                `light_state (VehicleLightState)`: Recaps the state of the lights of a vehicle, these can be used as a flags.\n
            """
        # endregion

    class ShowDebugTelemetry():
        """Command adaptation of `show_debug_telemetry()` in `carla.Actor`. Displays vehicle control telemetry data.
        """
        # region Instance Variables
        @property
        def actor_id() -> int:
            """Actor that is affected by the command."""
        @property
        def enabled() -> bool:
            """If debug should be activated or not."""
        # endregion

        # region Methods
        def __init__(self, actor: Union[Actor, int], enabled: bool):
            """Displays vehicle control telemetry data.

            Args:
                `actor (Union[Actor, int])`: Actor or Actor ID to which the command will be applied to.\n
                `enabled (bool)`: If debug should be activated or not.\n
            """
        # endregion

    class SpawnActor():
        """Command adaptation of `spawn_actor()` in `carla.World`. Spawns an actor into the world based on the blueprint provided and the transform. If a parent is provided, the actor is attached to it."""

        # region Instance Variables
        @property
        def transform() -> Transform:
            """Transform to be applied."""
        @property
        def parent_id() -> int:
            """Identificator of the parent actor."""
        # endregion

        # region Methods
        @overload
        def __init__(self): ...
        @overload
        def __init__(self, blueprint: ActorBlueprint, transform: Transform): ...
        @overload
        def __init__(self, blueprint: ActorBlueprint, transform: Transform, parent: Union[Actor, int]): ...

        def then(self, command: command):
            """Links another command to be executed right after. It allows to ease very common flows such as spawning a set of vehicles by command and then using this method to set them to autopilot automatically.

            Args:
                `command (command)`: a Carla command.\n
            """
        # endregion
