"""This is an artificial module that holds the classes that are added to libcarla if the RSS library is enabled."""
from typing import Callable

import ad
from libcarla import (
    Actor,
    Sensor,
    SensorData,
    Transform,
    VehicleControl,
    VehiclePhysicsControl,
    _CarlaEnum,
)

__all__ = [
    'RssActorConstellationData',
    'RssActorConstellationResult',
    'RssEgoDynamicsOnRoute',
    'RssLogLevel',
    'RssResponse',
    'RssRestrictor',
    'RssRoadBoundariesMode',
    'RssSensor',
]

class RssActorConstellationData:
    """Data structure that is provided within the callback registered by RssSensor.register_actor_constellation_callback()."""

    # region Instance Variables
    @property
    def ego_match_object(self) -> ad.map.match.Object:
        """The ego map matched information."""

    @property
    def ego_route(self) -> ad.map.route.FullRoute:
        """The ego route."""

    @property
    def ego_dynamics_on_route(self) -> RssEgoDynamicsOnRoute:
        """Current ego vehicle dynamics regarding the route."""

    @property
    def other_match_object(self) -> ad.map.match.Object:
        """The other object's map matched information. This is only valid if 'other_actor' is not 'None'."""

    @property
    def other_actor(self) -> Actor | None:
        """
        The other actor.
        This is 'None' in case of query of default parameters or artificial objects of kind [ad.rss.world.ObjectType.ArtificialObject](https://intel.github.io/ad-rss-lib/doxygen/ad_rss/namespacead_1_1rss_1_1world.html#a6432f1ef8d0657b4f21ed5966aca1625)
        with no dedicated 'carla.Actor' (as e.g. for the road boundaries at the moment; see RssSensor).
        """

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion

class RssActorConstellationResult:
    """
    Data structure that should be returned by the callback
    registered by RssSensor.register_actor_constellation_callback().
    """

    # region Instance Variables
    @property
    def rss_calculation_mode(self) -> ad.rss.map.RssMode:
        """The calculation mode to be applied with the actor."""
        ...
    @rss_calculation_mode.setter
    def rss_calculation_mode(self, value: ad.rss.map.RssMode) -> None:
        """Setter for rss_calculation_mode property."""
        ...

    @property
    def restrict_speed_limit_mode(self) -> ad.rss.map.RssSceneCreation.RestrictSpeedLimitMode:
        """The mode for restricting speed limit."""
        ...
    @restrict_speed_limit_mode.setter
    def restrict_speed_limit_mode(self, value: ad.rss.map.RssSceneCreation.RestrictSpeedLimitMode) -> None:
        """Setter for restrict_speed_limit_mode property."""
        ...

    @property
    def ego_vehicle_dynamics(self) -> ad.rss.world.RssDynamics:
        """The RSS dynamics to be applied for the ego vehicle."""
        ...
    @ego_vehicle_dynamics.setter
    def ego_vehicle_dynamics(self, value: ad.rss.world.RssDynamics) -> None:
        """Setter for ego_vehicle_dynamics property."""
        ...

    @property
    def actor_object_type(self) -> ad.rss.world.ObjectType:
        """The RSS object type to be used for the actor."""
        ...
    @actor_object_type.setter
    def actor_object_type(self, value: ad.rss.world.ObjectType) -> None:
        """Setter for actor_object_type property."""
        ...

    @property
    def actor_dynamics(self) -> ad.rss.world.RssDynamics:
        """The RSS dynamics to be applied for the actor."""
        ...
    @actor_dynamics.setter
    def actor_dynamics(self, value: ad.rss.world.RssDynamics) -> None:
        """Setter for actor_dynamics property."""
        ...
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion

class RssEgoDynamicsOnRoute:
    """
    Part of the data contained inside a carla.RssResponse describing the state of the vehicle.
    The parameters include its current dynamics, and how it is heading regarding the target route.
    """

    # region Instance Variables
    @property
    def ego_speed(self) -> ad.physics.Speed:
        """The ego vehicle's speed."""

    @property
    def min_stopping_distance(self) -> ad.physics.Distance:
        """The current minimum stopping distance."""

    @property
    def ego_center(self) -> ad.map.point.ENUPoint:
        """The considered enu position of the ego vehicle."""

    @property
    def ego_heading(self) -> ad.map.point.ENUHeading:
        """The considered heading of the ego vehicle."""

    @property
    def ego_center_within_route(self) -> bool:
        """States if the ego vehicle's center is within the route."""

    @property
    def crossing_border(self) -> bool:
        """States if the vehicle is already crossing one of the lane borders."""

    @property
    def route_heading(self) -> ad.map.point.ENUHeading:
        """The considered heading of the route."""

    @property
    def route_nominal_center(self) -> ad.map.point.ENUPoint:
        """The considered nominal center of the route."""

    @property
    def heading_diff(self) -> ad.map.point.ENUHeading:
        """The considered heading diff towards the route."""

    @property
    def route_speed_lat(self) -> ad.physics.Speed:
        """TThe ego vehicle's speed component lat regarding the route."""

    @property
    def route_speed_lon(self) -> ad.physics.Speed:
        """The ego vehicle's speed component lon regarding the route."""

    @property
    def route_accel_lat(self) -> ad.physics.Acceleration:
        """The ego vehicle's acceleration component lat regarding the route."""

    @property
    def route_accel_lon(self) -> ad.physics.Acceleration:
        """The ego vehicle's acceleration component lon regarding the route."""

    @property
    def avg_route_accel_lat(self) -> ad.physics.Acceleration:
        """The ego vehicle's acceleration component lat regarding the route smoothened by an average filter."""

    @property
    def avg_route_accel_lon(self) -> ad.physics.Acceleration:
        """The ego acceleration component lon regarding the route smoothened by an average filter."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion

class RssLogLevel(int, _CarlaEnum):
    """Enum declaration used in carla.RssSensor to set the log level."""

    trace = 0
    debug = 1
    info = 2
    warn = 3
    err = 4
    critical = 5
    off = 6

class RssResponse(SensorData):
    """
    Class that contains the output of a carla.RssSensor.
    This is the result of the RSS calculations performed for the parent vehicle of the sensor.

    A carla.RssRestrictor will use the data to modify the carla.VehicleControl of the vehicle.
    """

    # region Instance Variables
    @property
    def response_valid(self) -> bool:
        """
        States if the response is valid.
        It is False if calculations failed or an exception occurred.
        """

    @property
    def proper_response(self) -> ad.rss.state.ProperResponse:
        """The proper response that the RSS calculated for the vehicle."""

    @property
    def rss_state_snapshot(self) -> ad.rss.state.RssStateSnapshot:
        """Detailed RSS states at the current moment in time."""

    @property
    def ego_dynamics_on_route(self) -> RssEgoDynamicsOnRoute:
        """Current ego vehicle dynamics regarding the route."""

    @property
    def world_model(self) -> ad.rss.world.WorldModel:
        """World model used for calculations."""

    @property
    def situation_snapshot(self) -> ad.rss.situation.SituationSnapshot:
        """Detailed RSS situations extracted from the world model."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion

class RssRestrictor:
    """
    These objects apply restrictions to a carla.VehicleControl.
    It is part of the CARLA implementation of the C++ Library for Responsibility Sensitive Safety.
    This class works hand in hand with a rss sensor, which provides the data of the restrictions to be applied.
    """

    # region Methods
    def restrict_vehicle_control(self,
                                 vehicle_control: VehicleControl,
                                 proper_response: ad.rss.state.ProperResponse,
                                 ego_dynamics_on_route: RssEgoDynamicsOnRoute,
                                 vehicle_physics: VehiclePhysicsControl,
                                 ) -> VehicleControl:
        """
        Applies the safety restrictions given by a carla.RssSensor to a carla.VehicleCon

        Args:
            vehicle_control (VehicleControl):
                The input vehicle control to be restricted.
            proper_response (ad.rss.state.ProperResponse):
                Part of the response generated by the sensor. Contains restrictions to be applied to the acceleration of the vehicle.
            ego_dynamics_on_route (RssEgoDynamicsOnRoute):
                Tart of the response generated by the sensor. Contains dynamics and heading of the vehicle regarding its route.
            vehicle_physics (VehiclePhysicsControl):
                The current physics of the vehicle. Used to apply the restrictions properly.

        Returns:
            VehicleControl: The restricted vehicle control.

        """
    # endregion

    # region Setters
    def set_log_level(self, log_level: RssLogLevel):
        """Sets the log level."""
    # endregion

class RssRoadBoundariesMode(int, _CarlaEnum):
    """
    Enum declaration used in carla.RssSensor to enable or disable the stay on road feature.
    In summary, this feature considers the road boundaries as virtual objects.
    The minimum safety distance check is applied to these virtual walls,
    in order to make sure the vehicle does not drive off the road.
    """

    Off = 0
    On = 1

class RssSensor(Sensor):
    """
    This sensor works a bit differently than the rest. Take look at the [specific documentation](https://carla.readthedocs.io/en/latest/adv_rss/),
    and the [rss sensor reference](https://carla.readthedocs.io/en/latest/ref_sensors/#rss-sensor) to gain full understanding of it.

    The RSS sensor uses world information, and a [RSS library](https://github.com/intel/ad-rss-lib) to make safety checks on a vehicle.
    The output retrieved by the sensor is a `carla.RssResponse`. This will be used
    by a `carla.RssRestrictor` to modify a `carla.VehicleControl` before applying
    it to a vehicle.
    """

    # region Instance Variables

    @property
    def ego_vehicle_dynamics(self) -> ad.rss.world.RssDynamics:
        """
        States the [RSS parameters](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/) that the sensor will consider
        for the ego vehicle if no actor constellation callback is registered.
        """

    @property
    def other_vehicle_dynamics(self) -> ad.rss.world.RssDynamics:
        """
        States the [RSS parameters](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/) that the sensor will consider
        for the rest of vehicles if no actor constellation callback is registered.
        """

    @property
    def pedestrian_dynamics(self) -> ad.rss.world.RssDynamics:
        """
        States the [RSS parameters](https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/) that the sensor will consider
        for pedestrians if no actor constellation callback is registered.
        """

    @property
    def road_boundaries_mode(self) -> RssRoadBoundariesMode:
        """Switches the stay on road feature. By default is Off."""

    @road_boundaries_mode.setter
    def road_boundaries_mode(self, value: RssRoadBoundariesMode) -> None:
        ...

    @property
    def routing_targets(self) -> list[Transform]:  # declared as vector<carla.Transform>
        """The current list of targets considered to route the vehicle. If no routing targets are defined, a route is generated at random."""

    # region Methods
    def append_routing_target(self, routing_target: Transform):
        """
        Appends a new target position to the current route of the vehicle.

        Args:
            routing_target (Transform): New target point for the route. Choose these after the intersections to force the route to take the desired turn.
        """

    def drop_route(self):
        """
        Discards the current route.

        If there are targets remaining in `routing_targets`, creates a new route using those.
        Otherwise, a new route is created at random.
        """

    def register_actor_constellation_callback(self, callback: Callable[[RssActorConstellationData], RssActorConstellationResult]):
        """
        Register a callback to customize a `carla.RssActorConstellationResult`.
        By this callback the settings of RSS parameters are done per actor constellation
        and the settings (ego_vehicle_dynamics, other_vehicle_dynamics and pedestrian_dynamics) have no effect.

        Args:
            callback (Callable): The function to be called whenever a RSS situation is about to be calculated.
        """

    def reset_routing_targets(self):
        """Erases the targets that have been appended to the route."""

    def set_log_level(self, log_level: RssLogLevel | int):
        """Sets the log level."""
    def set_map_log_level(self, log_level: RssLogLevel | int):
        """Sets the map log level."""
    # endregion

    # region Dunder Methods
    def __str__(self) -> str: ...
    # endregion
