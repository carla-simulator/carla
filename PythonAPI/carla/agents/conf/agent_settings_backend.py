# DO NOT USE from __future__ import annotations ! This would break the dataclass interface.

from functools import partial
from dataclasses import dataclass, field
from typing import ClassVar, Dict, Optional, Tuple, Type, Union

import carla
from agents.navigation.local_planner import RoadOption

__all__ = ["AgentConfig", 
           "SimpleConfig", 
           "BasicAgentSettings", 
           "BehaviorAgentSettings", 
           "SimpleBasicAgentSettings", 
           "SimpleBehaviorAgentSettings"
        ]

MISSING = "???" 

# ---------------------
# Helper methods
# ---------------------

class class_or_instance_method:
    """Decorator to transform a method into both a regular and class method"""
    
    def __init__(self, call):
        self.__wrapped__ = call
        self._wrapper = lambda x : x # TODO: functools.partial and functools.wraps shadow the signature, this reveals it again.

    def __get__(self, instance : Union[None, "AgentConfig"], owner : Type["AgentConfig"]):
        if instance is None:  # called on class 
            return self._wrapper(partial(self.__wrapped__, owner))
        return self._wrapper(partial(self.__wrapped__, instance)) # called on instance


# ---------------------
# Base Classes
# ---------------------

class AgentConfig:
    """
    Base interface for the agent settings. 
    
    Handling the initialization from a nested dataclass and merges in the changes
    from the overwrites options.
    """
    overwrites: "Optional[Dict[str, Union[dict, AgentConfig]]]" = None
    
    @classmethod
    def get_defaults(cls) -> "AgentConfig":
        """Returns the global default options."""
        return cls
    
    def update(self, options : "Union[dict, AgentConfig]", clean=True):
        """Updates the options with a new dictionary."""
        if isinstance(options, AgentConfig):
            key_values = options.__dataclass_fields__.items()
        else:
            key_values = options.items()
        for k, v in key_values:
            if isinstance(getattr(self, k), AgentConfig):
                getattr(self, k).update(v)
            else:
                setattr(self, k, v)
        if clean:
            self._clean_options()

    def _clean_options(self):
        """Postprocessing of possibly wrong values"""
        NotImplemented

    def __post_init__(self):
        """
        Assures that if a dict is passed the values overwrite the defaults.
        
        # NOTE: Will be used for dataclass derived children
        """
        self._clean_options()
        if self.overwrites is None:
            return
        # Merge the overwrite dict into the correct ones.
        for key, value in self.overwrites.items():
            if key in self.__annotations__:
                if issubclass(self.__annotations__[key], AgentConfig):
                    getattr(self, key).update(value)
                else:
                    setattr(self, key, value)
            else:
                print(f"Warning: Key '{key}' not found in {self.__class__.__name__} default options. Consider updating or creating a new class to avoid this message.")


class SimpleConfig(object):
    """
    A class that allows a more simple way to initialize settings.
    Initializing an instance changes the type the the given base class, defined via `_base_settings`.
    
    :param _base_settings: The base class to port the settings to.
    
    TODO: NOTE: This class assumes that there are NO DUPLICATE keys in the underlying base
    """
    
    _base_settings: ClassVar["AgentConfig"] = None
    
    def __new__(cls, overwrites:Optional[Dict[str, Union[dict, AgentConfig]]]=None) -> "AgentConfig":
        """
        Transforms a SimpleConfig class into the given _base_settings
        
        :param overwrites: That allow overwrites during initialization.
        """
        if cls._base_settings is None:
            raise TypeError("{cls.__name__} must have a class set in the `_base_settings` to initialize to.")
        if cls is SimpleConfig:
            raise TypeError("SimpleConfig class may not be instantiated")
        simple_settings = {k:v for k,v  in cls.__dict__.items() if not k.startswith("_") } # Removes all private attributes
        if overwrites:
            simple_settings.update(overwrites) 
        return super().__new__(cls).to_nested_config(simple_settings) # call from a pseudo instance.
    
    @class_or_instance_method
    def to_nested_config(self, simple_overwrites:dict=None) -> AgentConfig:
        """
        Initializes the _base_settings with the given overwrites.
        
        Maps the keys of simple_overwrites to the base settings.
        
        More specifically builds a overwrites dict that is compatible with the nested configuration versions.
        
        NOTE: Assumes unique keys over all settings!
        # TODO: add a warning if a non-unique key is found in the overwrites.
        """
        if isinstance(self, type): # called on class
            return self()
        keys = set(simple_overwrites.keys())
        removed_keys = set() # to check for duplicated keys that cannot be set via SimpleConfig unambiguously
        overwrites = {}
        for name, base in self._base_settings.__annotations__.items():
            if not isinstance(base, type) or not issubclass(base, AgentConfig): # First out non AgentConfig attributes
                if name in keys:
                    overwrites[name] = simple_overwrites[name] # if updating a top level attribute
                    keys.remove(name)
                    removed_keys.add(name)
                continue
            matching = keys.intersection(base.__dataclass_fields__.keys()) # keys that match from the simple config to the real nested config
            if len(matching) > 0:
                if removed_keys.intersection(matching):
                    print("WARNING: Ambiguous key", removed_keys.intersection(matching), "in the SimpleConfig", str(self), "that occurs multiple times in its given base", self._base_settings.__name__+".", "Encountered at", name, base) # TODO: remove later, left here for testing.")
                overwrites[name] = {k: getattr(self, k) for k in matching}
                keys -= matching
                removed_keys.update(matching)
        if len(keys) != 0:
            overwrites.update({k: v for k,v in simple_overwrites.items() if k in keys}) # Add them to the top level
            print("Warning: Unmatched keys", keys, "in", self.__class__.__name__, "not contained in base", self._base_settings.__name__+".", "Adding them to the top-level of the settings.") # TODO: remove later, left here for testing.
        return self._base_settings(overwrites=overwrites)
        # TODO: could add new keys after post-processing.
        
        
# ---------------------

# ---------------------
# Live Info
# ---------------------

@dataclass
class LiveInfo(AgentConfig):
    current_speed : float = MISSING
    current_speed_limit : float = MISSING
    direction : RoadOption = MISSING
    velocity_vector : "carla.Vector3D" = MISSING
    
    # NOTE: Not ported to OmegaConf
    @property
    def speed(self):
        return self.current_speed

    @property
    def speed_limit(self):
        return self.current_speed_limit

# ---------------------
# Speed
# ---------------------    

@dataclass
class BasicAgentSpeedSettings(AgentConfig):
    target_speed: float = 20
    """desired cruise speed in Km/h; overwritten by SpeedLimit if follow_speed_limit is True"""
    
    follow_speed_limits: bool = False
    """If the agent should follow the speed limit. *NOTE:* SpeedLimit overwrites target_speed if True (local_planner.py)"""

@dataclass
class BehaviorAgentSpeedSettings(BasicAgentSpeedSettings):
    """
    The three situations they adjust their speed; # SEE: `behavior_agent.car_following_manager`
    
    Case A car in front and getting closer : slow down; slower than car in front
          Take minium from, speed decrease, speed limit adjustment and target_speed
          `target_speed` = min( other_vehicle_speed - self._behavior.speed_decrease, # <-- slow down BELOW the other car
                              self._behavior.max_speed # use target_speed instead
                              self._speed_limit - self._behavior.speed_lim_dist])
    Case B car in front but safe distance : match speed
          `target_speed` = min([
                    max(self._min_speed, other_vehicle_speed),  # <- match speed
                    self._behavior.max_speed,
                    self._speed_limit - self._behavior.speed_lim_dist])
    Case C front is clear
          `target_speed` = min([
                    self._behavior.max_speed,
                    self._speed_limit - self._behavior.speed_lim_dist])
    """
    # TODO:  deprecated max_speed use target_speed instead   # NOTE: Behavior agents are more flexible in their speed. 
    max_speed : float = 50 
    """The maximum speed in km/h your vehicle will be able to reach.
    From normal behavior. This supersedes the target_speed when following the BehaviorAgent logic."""
    
    # CASE A
    speed_decrease : float = 10
    """other_vehicle_speed"""
    
    safety_time : float = 3
    """Time in s before a collision at the same speed -> apply speed_decrease"""

    # CASE B
    min_speed : float = 5
    """Implement als variable, currently hard_coded"""

    # All Cases
    speed_lim_dist : float = 3
    """
    Difference to speed limit.
    NOTE: For negative values the car drives above speed limit
    """

    intersection_speed_decrease: float = 5.0
    """Reduction of the targeted_speed when approaching an intersection"""
    
    
@dataclass
class AutopilotSpeedSettings(AgentConfig):
    vehicle_percentage_speed_difference : float = 30 # in percent
    """
    Sets the difference the vehicle's intended speed and its current speed limit. 
    Speed limits can be exceeded by setting the percentage to a negative value. 
    Default is 30.
    
    Exceeding a speed limit can be done using negative percentages.
    """

# ---------------------
# Distance
# ---------------------


@dataclass
class BasicAgentDistanceSettings(AgentConfig):
    """
    Calculation of the minimum distance for # XXX
    min_distance = base_min_distance + distance_ratio * vehicle_speed 
    
    see local_planner.py `run_step`
    """
    
    base_min_distance : float = 3.0
    """
    Base value of the distance to keep
    """
    
    distance_ratio : float = 0.5
    """Increases minimum distance multiplied by speed"""
    

@dataclass
class BehaviorAgentDistanceSettings(BasicAgentDistanceSettings):
    """
    Collision Avoidance -----

    Distance in which for vehicles are checked
    max(min_proximity_threshold, self._speed_limit / (2 if LANE CHANGE else 3 ) )
    TODO: The secondary speed limit is hardcoded, make adjustable and optional
    automatic_proximity_threshold = {RoadOption.CHANGELANELEFT: 2, "same_lane" : 3, "right_lane" : 2}
    """
    
    min_proximity_threshold : float = 10
    """Range in which cars are detected. NOTE: Speed limit overwrites"""
    
    braking_distance : float = 5
    """Emergency Stop Distance Trigger"""
    

@dataclass
class AutopilotDistanceSettings(AgentConfig):
    distance_to_leading_vehicle : float = 5.0
    """
    Sets the minimum distance in meters that a vehicle has to keep with the others. 
    The distance is in meters and will affect the minimum moving distance. It is computed from front to back of the vehicle objects.
    """

# ---------------------
# Lane Change
# ---------------------

@dataclass
class BasicAgentLaneChangeSettings(AgentConfig):
    """
    Timings in seconds to finetune the lane change behavior.
    
    NOTE: see: `BasicAgent.lane_change` and `BasicAgent._generate_lane_change_path`
    """
    same_lane_time : float = 0.0
    other_lane_time : float = 0.0
    lane_change_time : float = 2.0

@dataclass
class BehaviorAgentLaneChangeSettings(BasicAgentLaneChangeSettings):
    pass

@dataclass
class AutopilotLaneChangeSettings(AgentConfig):
    auto_lane_change: bool = True
    """Turns on or off lane changing behavior for a vehicle."""
    
    random_left_lanechange_percentage: float = 0.1
    """
    Adjust probability that in each timestep the actor will perform a left/right lane change, 
    dependent on lane change availability.
    """
    random_right_lanechange_percentage : float = 0.1
    """
    Adjust probability that in each timestep the actor will perform a left/right lane change, 
    dependent on lane change availability.
    """

    keep_right_rule_percentage: float = 0.7
    """
    During the localization stage, this method sets a percent chance that vehicle will follow the keep right rule, 
    and stay in the right lane.
    """
    
# ---------------------
# Obstacles
# ---------------------

@dataclass
class BasicAgentObstacleDetectionAngles(AgentConfig):
    """
    Detection Angles for the BasicAgent used in the `BasicAgent._vehicle_obstacle_detected` method.
    
    The angle between the location and reference object.
    Being 0 a location in front and 180, one behind, i.e, the vector between has to satisfy: 
    low_angle_th < angle < up_angle_th.
    """
    
    walkers_lane_change : Tuple[float, float] = (0., 90.)
    """Detection angle of walkers when staying in the same lane"""
    
    walkers_same_lane : Tuple[float, float] = (0., 60.)
    """Detection angle of walkers when changing lanes"""
    
    cars_lane_change : Tuple[float, float] = (0., 180.)
    """Detection angle of cars when staying in the same lane"""
    
    cars_same_lane : Tuple[float, float] = (0., 30.)
    """Detection angle of cars when changing lanes"""
    
@dataclass
class BasicAgentObstacleSettings(AgentConfig):
    """
    --------------------------
    Agent Level
    see _affected_by_traffic_light and _affected_by_vehicle in basic_agent.py
    --------------------------
    Agents is aware of the vehicles and traffic lights within its distance parameters
    optionally can always ignore them.
    """
    
    ignore_vehicles : bool = False
    """Whether the agent should ignore vehicles"""
    
    ignore_traffic_lights : bool = False
    """Whether the agent should ignore traffic lights"""
    
    ignore_stop_signs : bool = False
    """
    Whether the agent should ignore stop signs
    
    NOTE: No usage implemented!
    """
    
    use_bbs_detection : bool = False
    """
    True: Whether to use a general approach to detect vehicles invading other lanes due to the offset.

    False: Simplified approach, using only the plan waypoints (similar to TM)
    
    See `BasicAgent._vehicle_obstacle_detected`
    """
    
    base_tlight_threshold : float = 5.0
    """
    Base distance to traffic lights to check if they affect the vehicle
        
    USAGE: max_vehicle_distance = base_vehicle_threshold + detection_speed_ratio * vehicle_speed
    USAGE: max_tlight_distance  = base_tlight_threshold  + detection_speed_ratio * vehicle_speed
    """
    
    base_vehicle_threshold : float = 5.0
    """
    Base distance to vehicles to check if they affect the vehicle
            
    USAGE: max_vehicle_distance = base_vehicle_threshold + detection_speed_ratio * vehicle_speed
    USAGE: max_tlight_distance  = base_tlight_threshold  + detection_speed_ratio * vehicle_speed
    """

    detection_speed_ratio : float = 1.0
    """
    Increases detection range based on speed
    
    USAGE: max_vehicle_distance = base_vehicle_threshold + detection_speed_ratio * vehicle_speed
    USAGE: max_tlight_distance  = base_tlight_threshold  + detection_speed_ratio * vehicle_speed
    """
    
    dynamic_threshold_by_speed : bool = True
    """
    Whether to add a dynamic threshold based on the vehicle speed to the base threshold.
    
    Usage: base_threshold + dynamic_threshold_by_speed * vehicle_speed
    """
    
    detection_angles : BasicAgentObstacleDetectionAngles = field(default_factory=BasicAgentObstacleDetectionAngles)
    """Defines detection angles used when checking for obstacles."""
    

@dataclass
class BehaviorAgentObstacleSettings(BasicAgentObstacleSettings):
    pass

@dataclass
class AutopilotObstacleSettings(AgentConfig):
    ignore_lights_percentage : float = 0.0
    """
    Percentage of time to ignore traffic lights
    """
    
    ignore_signs_percentage : float = 0.0
    """
    Percentage of time to ignore stop signs
    """
    
    ignore_walkers_percentage : float = 0.0
    """
    Percentage of time to ignore pedestrians
    """
    

# ---------------------
# ControllerSettings
# ---------------------

@dataclass
class BasicAgentControllerSettings(AgentConfig):
    """Limitations of the controls used one the PIDController Level"""
    
    max_brake : float = 0.5
    """
    Vehicle control how strong the brake is used, 
    
    NOTE: Also used in emergency stop
    """
    max_throttle : float = 0.75
    """maximum throttle applied to the vehicle"""
    max_steering : float = 0.8
    """maximum steering applied to the vehicle"""
    
    # Aliases used:
    #@property
    #def max_throt(self):
    #    return self.max_throttle

    #@property
    #def max_steer(self):
    #    return self.max_steering

@dataclass
class BehaviorAgentControllerSettings(BasicAgentControllerSettings):
    pass

@dataclass
class AutopilotControllerSettings(AgentConfig):
    vehicle_lane_offset : float = 0
    """
    Sets a lane offset displacement from the center line. Positive values imply a right offset while negative ones mean a left one.
    Default is 0. Numbers high enough to cause the vehicle to drive through other lanes might break the controller.
    """
    
# ---------------------
# PlannerSettings
# ---------------------

@dataclass
class PIDControllerDict:
    K_P : float 
    K_D : float 
    K_I : float = 0.05
    dt : float = 1.0 / 20.0
    """time differential in seconds"""

@dataclass
class BasicAgentPlannerSettings(AgentConfig):
    """
    PID controller using the following semantics:
            K_P -- Proportional term
            K_D -- Differential term
            K_I -- Integral term
            dt -- time differential in seconds
    offset: If different than zero, the vehicle will drive displaced from the center line.
    Positive values imply a right offset while negative ones mean a left one. 
    Numbers high enough to cause the vehicle to drive through other lanes might break the controller.
    
    Notes:
    `sampling_resolution` is used by the global planner to build a graph of road segments, also to get a path of waypoints from A to B

    `sampling_radius` is similar but only used only by the local_planner to compute the next waypoints forward. The distance of those is the sampling_radius.
    """
    
    dt : float = 1.0 / 20.0
    """time differential in seconds"""
    
    lateral_control_dict : PIDControllerDict = field(default_factory=lambda:PIDControllerDict(**{'K_P': 1.95, 'K_I': 0.05, 'K_D': 0.2}))
    """values of the lateral PID controller"""

    longitudinal_control_dict : PIDControllerDict  = field(default_factory=lambda:PIDControllerDict(**{'K_P': 1.0, 'K_I': 0.05, 'K_D': 0}))
    """values of the longitudinal PID controller"""
    
    offset : float = 0.0
    """
    If different than zero, the vehicle will drive displaced from the center line.
    
    Positive values imply a right offset while negative ones mean a left one. Numbers high enough
    to cause the vehicle to drive through other lanes might break the controller.
    """
    
    sampling_radius : float = 2.0
    """
    Distance between waypoints when planning a path in `local_planner._compute_next_waypoints`
    
    Used with Waypoint.next(sampling_radius)
    """
    
    sampling_resolution : float = 2.0
    """
    Distance between waypoints in `BasicAgent._generate_lane_change_path`
    Furthermore in the GlobalRoutePlanner to build the topology and for path planning.
    
    Used with the Waypoint.next(sampling_radius) and distance between waypoints.
    """


@dataclass
class BehaviorAgentPlannerSettings(BasicAgentPlannerSettings):
    sampling_resolution : float = 4.5
    """
    Distance between waypoints in `BasicAgent._generate_lane_change_path`
    and GlobalRoutePlanner to build the topology and path planning.
    
    Used with the Waypoint.next(sampling_radius)
    """
    
# ---------------------
# Emergency
# ---------------------
    
@dataclass
class BasicAgentEmergencySettings(AgentConfig):
    throttle : float = 0.0
    max_emergency_brake : float = 0.5
    hand_brake : bool = False
    
    
@dataclass
class BehaviorAgentEmergencySettings(BasicAgentEmergencySettings):
    pass


# ---------------------

@dataclass
class AutopilotBehavior(AgentConfig):
    """
    These are settings from the autopilot carla.TrafficManager which are not exposed or not used by the original carla agents.
    NOTE: That default values do not exist for most settings; we should set it to something reasonable.
    """

    auto_lane_change: bool = True
    """Turns on or off lane changing behavior for a vehicle."""
    
    vehicle_lane_offset : float = 0
    """
    Sets a lane offset displacement from the center line.
    
    Positive values imply a right offset while negative ones mean a left one. 
    Default is 0. 
    
    NOTE: Numbers high enough to cause the vehicle to drive through other lanes might break the controller.
    """

    random_left_lanechange_percentage: float = 0.1
    """
    Adjust probability that in each timestep the actor will perform a left/right lane change, 
    dependent on lane change availability.
    """
    random_right_lanechange_percentage : float = 0.1
    """
    Adjust probability that in each timestep the actor will perform a left/right lane change, 
    dependent on lane change availability.
    """

    keep_right_rule_percentage: float = 0.7
    """
    During the localization stage, this method sets a percent chance that vehicle will follow the keep right rule, 
    and stay in the right lane.
    """

    distance_to_leading_vehicle : float = 5.0
    """
    Sets the minimum distance in meters that a vehicle has to keep with the others. 
    The distance is in meters and will affect the minimum moving distance. It is computed from front to back of the vehicle objects. 
    """

    vehicle_percentage_speed_difference : float = 30
    """
    Sets the difference the vehicle's intended speed and its current speed limit. 
    Speed limits can be exceeded by setting the percentage to a negative value. 
    Exceeding a speed limit can be done using negative percentages.
    
    NOTE: unit is in percent.
    Default is 30. 
    """
    
    ignore_lights_percentage : float = 0.0
    ignore_signs_percentage : float = 0.0
    ignore_walkers_percentage : float = 0.0

    update_vehicle_lights : bool = False
    """Sets if the Traffic Manager is responsible of updating the vehicle lights, or not."""
    



# ---------------------
# Final Settings
# ---------------------

@dataclass
class BasicAgentSettings(AgentConfig):
    overwrites : Optional[Dict[str, dict]] = field(default_factory=dict, repr=False) # type: Optional[Dict[str, Union[dict|AgentConfig]]]
    live_info : LiveInfo = field(default_factory=LiveInfo, init=False)
    speed : BasicAgentSpeedSettings = field(default_factory=BasicAgentSpeedSettings, init=False)
    distance : BasicAgentDistanceSettings = field(default_factory=BasicAgentDistanceSettings, init=False)
    lane_change : BasicAgentLaneChangeSettings = field(default_factory=BasicAgentLaneChangeSettings, init=False)
    obstacles : BasicAgentObstacleSettings = field(default_factory=BasicAgentObstacleSettings, init=False)
    controls : BasicAgentControllerSettings = field(default_factory=BasicAgentControllerSettings, init=False)
    planner : BasicAgentPlannerSettings = field(default_factory=BasicAgentPlannerSettings, init=False)
    emergency : BasicAgentEmergencySettings = field(default_factory=BasicAgentEmergencySettings, init=False)
        
@dataclass
class BehaviorAgentSettings(AgentConfig):
    overwrites : Optional[Dict[str, dict]] = field(default_factory=dict, repr=False) # type: Optional[Dict[str, Union[dict|AgentConfig]]]
    live_info : LiveInfo = field(default_factory=LiveInfo, init=False)
    speed : BehaviorAgentSpeedSettings = field(default_factory=BehaviorAgentSpeedSettings, init=False)
    distance : BehaviorAgentDistanceSettings = field(default_factory=BehaviorAgentDistanceSettings, init=False)
    lane_change : BehaviorAgentLaneChangeSettings = field(default_factory=BehaviorAgentLaneChangeSettings, init=False)
    obstacles : BehaviorAgentObstacleSettings = field(default_factory=BehaviorAgentObstacleSettings, init=False)
    controls : BehaviorAgentControllerSettings = field(default_factory=BehaviorAgentControllerSettings, init=False)
    planner : BehaviorAgentPlannerSettings = field(default_factory=BehaviorAgentPlannerSettings, init=False)
    emergency : BehaviorAgentEmergencySettings = field(default_factory=BehaviorAgentEmergencySettings, init=False)
    avoid_tailgators : bool = True
 
@dataclass
class SimpleBasicAgentSettings(SimpleConfig, LiveInfo, BasicAgentSpeedSettings, BasicAgentDistanceSettings, BasicAgentLaneChangeSettings, BasicAgentObstacleSettings, BasicAgentControllerSettings, BasicAgentPlannerSettings, BasicAgentEmergencySettings):
    _base_settings :ClassVar[BasicAgentSettings] = BasicAgentSettings

@dataclass
class SimpleBehaviorAgentSettings(SimpleConfig, LiveInfo, BehaviorAgentSpeedSettings, BehaviorAgentDistanceSettings, BehaviorAgentLaneChangeSettings, BehaviorAgentObstacleSettings, BehaviorAgentControllerSettings, BehaviorAgentPlannerSettings, BehaviorAgentEmergencySettings):
    _base_settings :ClassVar[BehaviorAgentSettings] = BehaviorAgentSettings
    

