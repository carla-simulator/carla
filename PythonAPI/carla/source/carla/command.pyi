"""
Submodule with commands that can be used with `carla.Client.apply_batch`
and `carla.Client.apply_batch_sync`.
"""
# needs change in API
# pylint: disable=too-many-locals,too-many-public-methods,too-many-arguments,too-many-public-methods,too-few-public-methods,too-many-lines
# Fixable low-priority
# pylint: disable=line-too-long,R0801# needs change in API
# pylint: disable=too-many-locals,too-many-public-methods,too-many-arguments,too-many-public-methods,too-few-public-methods,too-many-lines
# Fixable low-priority
# pylint: disable=line-too-long,R0801

from typing import (  # pylint: disable=no-name-in-module
    Protocol,
    overload,
    type_check_only,
)

from .libcarla import (
    AckermannControllerSettings,
    Actor,
    ActorBlueprint,
    Transform,
    Vector3D,
    VehicleControl,
    VehicleLightState,
    VehiclePhysicsControl,
)

__all__ = [
    "ApplyAngularImpulse",
    "ApplyForce",
    "ApplyImpulse",
    "ApplyTargetAngularVelocity",
    "ApplyTargetVelocity",
    "ApplyTorque",
    "ApplyTransform",
    "ApplyVehicleAckermannControl",
    "ApplyVehicleControl",
    "ApplyVehiclePhysicsControl",
    "ApplyWalkerControl",
    "ApplyWalkerState",
    "DestroyActor",
    "FutureActor",
    "Response",
    "SetAutopilot",
    "SetEnableGravity",
    "SetSimulatePhysics",
    "SetVehicleLightState",
    "ShowDebugTelemetry",
    "SpawnActor",
]

@type_check_only
class _IsCommand(Protocol):
    """
    Indicates a command from the command namespace.

    Non commands are `FutureActor` or `Response`.
    """

class ApplyAngularImpulse(_IsCommand):
    """Command adaptation of `add_angular_impulse()` in `carla.Actor`. Applies an angular impulse to an actor."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def impulse(self) -> Vector3D:
        """Angular impulse applied to the actor (degrees*s)."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, impulse: Vector3D) -> None:
        """Applies an angular impulse to an actor.

        Args:
            `actor (Union[Actor,int])`: Actor or its ID to whom the command will be applied to.\n
            `impulse (Vector3D)`: (degrees*s)
        """
    # endregion

class ApplyForce(_IsCommand):
    """Command adaptation of `add_force()` in `carla.Actor`. Applies a force to an actor."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def force(self) -> Vector3D:
        """Force applied to the actor over time (N)."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, force: Vector3D) -> None:
        """Applies a force to an actor.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `force (Vector3D)`: (N)
        """
    # endregion

class ApplyImpulse(_IsCommand):
    """Command adaptation of `add_impulse()` in `carla.Actor`. Applies an impulse to an actor."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def impulse(self) -> Vector3D:
        """Impulse applied to the actor (N*s)."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, impulse: Vector3D) -> None:
        """Applies an impulse to an actor.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `impulse (Vector3D)`: (N*s)
        """
    # endregion

class ApplyTargetAngularVelocity(_IsCommand):
    """Command adaptation of `set_target_angular_velocity()` in `carla.Actor`. Sets the actor's angular velocity vector."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def angular_velocity(self) -> Vector3D:
        """The 3D angular velocity that will be applied to the actor (deg/s)."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, angular_velocity: Vector3D) -> None:
        """Sets the actor's angular velocity vector.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `angular_velocity (Vector3D)`: Angular velocity vector applied to the actor.
        """
    # endregion

class ApplyTargetVelocity(_IsCommand):
    """Command adaptation of `set_target_velocity()` in `carla.Actor`. Sets the actor's target velocity vector."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def velocity(self) -> Vector3D:
        """The 3D velocity applied to the actor (m/s)."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, velocity: Vector3D) -> None:
        """Sets the actor's target velocity vector.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `velocity (Vector3D)`: Velocity vector applied to the actor.
        """
    # endregion

class ApplyTorque(_IsCommand):
    """Command adaptation of `add_torque()` in carla.Actor. Applies a torque to an actor."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""

    @property
    def transform(self) -> Vector3D:
        """Torque applied to the actor over time (degrees)."""

    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, torque: Vector3D) -> None:
        """Sets a new transform to an actor.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `torque (Vector3D)`: Torque vector in global coordinates (degrees).
        """
    # endregion

class ApplyTransform(_IsCommand):
    """Command adaptation of `set_transform()` in `carla.Actor`. Sets a new transform to an actor."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def transform(self) -> Transform:
        """Transformation to be applied."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, transform: Transform) -> None:
        """Sets a new transform to an actor.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `transform (Transform)`
        """
    # endregion

class ApplyVehicleAckermannControl(_IsCommand):
    """Command adaptation of `apply_ackermann_control()` in `carla.Vehicle`. Applies a certain akermann control to a vehicle."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def control(self) -> AckermannControllerSettings:
        """Vehicle ackermann control to be applied."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, control: AckermannControllerSettings) -> None:
        """Applies a certain akermann control to a vehicle.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `control (AckermannControllerSettings)`: Vehicle ackermann control to be applied.
        """
    # endregion

class ApplyVehicleControl(_IsCommand):
    """Command adaptation of `apply_control()` in `carla.Vehicle`. Applies a certain control to a vehicle."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def control(self) -> VehicleControl:
        """Vehicle control to be applied."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, control: VehicleControl) -> None:
        """Applies a certain control to a vehicle.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `control (VehicleControl)`: Vehicle control to be applied.
        """
    # endregion

class ApplyVehiclePhysicsControl(_IsCommand):
    """Command adaptation of `apply_physics_control()` in `carla.Vehicle`. Applies a new physics control to a vehicle, modifying its physical parameters."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    @property
    def control(self) -> VehiclePhysicsControl:
        """Physics control to be applied."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, control: VehicleControl) -> None:
        """Applies a new physics control to a vehicle, modifying its physical parameters.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `control (VehicleControl)`: Physics control to be applied.
        """
    # endregion

class ApplyWalkerControl(_IsCommand):
    """Command adaptation of `apply_control()` in `carla.Walker`. Applies a control to a walker."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Walker actor affected by the command."""
    @property
    def control(self) -> VehiclePhysicsControl:
        """Walker control to be applied."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, control: VehicleControl) -> None:
        """Applies a control to a walker.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `control (VehicleControl)`: Walker control to be applied.
        """
    # endregion

class ApplyWalkerState(_IsCommand):
    """Apply a state to the walker actor. Specially useful to initialize an actor them with a specific location, orientation and speed."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Walker actor affected by the command."""
    @property
    def transform(self) -> Transform:
        """Transform to be applied."""
    @property
    def speed(self) -> float:
        """Speed to be applied (m/s)."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, transform: Transform, speed: float) -> None:
        """Apply a state to the walker actor.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `transform (Transform)`: Transform to be applied.\n
            `speed (float)`: Speed to be applied (m/s).
        """
    # endregion

class DestroyActor(_IsCommand):
    """Command adaptation of `destroy()` in `carla.Actor` that tells the simulator to destroy this actor. It has no effect if the actor was already destroyed. When executed with `apply_batch_sync()` in c`arla.Client` there will be a `command.Response` that will return a boolean stating whether the actor was successfully destroyed."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor affected by the command."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int) -> None:
        """Command adaptation of `destroy()` in `carla.Actor` that tells the simulator to destroy this actor.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.
        """
    # endregion

class FutureActor:
    """A utility object used to reference an actor that will be created in the command in the previous step, it has no parameters or methods."""

class Response:
    """States the result of executing a command as either the ID of the actor to whom the command was applied to (when succeeded) or an error string (when failed). actor ID, depending on whether or not the command succeeded. The method `apply_batch_sync()` in c`arla.Client` returns a list of these to summarize the execution of a batch."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor to whom the command was applied to. States that the command was successful."""
    @property
    def error(self) -> str:
        """A string stating the command has failed."""
    # endregion

    # region Methods
    def has_error(self) -> bool:
        """Returns `True` if the command execution fails, and `False` if it was successful."""
    # endregion

class SetAutopilot(_IsCommand):
    """Command adaptation of `set_autopilot()` in `carla.Vehicle`. Turns on/off the vehicle's autopilot mode."""

    # region Instance Methods
    @property
    def actor_id(self) -> int:
        """Actor that is affected by the command."""
    @property
    def enabled(self) -> bool:
        """If autopilot should be activated or not."""
    @property
    def port(self) -> int:
        """Port of the Traffic Manager where the vehicle is to be registered or unlisted."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, enabled: bool, port=8000) -> None:
        """Turns on/off the vehicle's autopilot mode.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `enabled (bool)`: _description_\n
            `port (int, optional)`:  The Traffic Manager port where the vehicle is to be registered or unlisted. If None is passed, it will consider a TM at default port 8000. Defaults to 8000.
        """
    # endregion

class SetEnableGravity(_IsCommand):
    """Command adaptation of `set_enable_gravity()` in `carla.Actor`. Enables or disables gravity on an actor."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor that is affected by the command."""
    @property
    def enabled(self) -> bool:
        """If gravity should be activated or not."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, enabled: bool) -> None:
        """Enables or disables gravity on an actor.

        Args:
            `actor (Actor | int)`: Actor or Actor ID to which the command will be applied to.\n
            `enabled (bool)`: If gravity should be activated or not.
        """
    # endregion

class SetSimulatePhysics(_IsCommand):
    """Command adaptation of `set_simulate_physics()` in `carla.Actor`. Determines whether an actor will be affected by physics or not."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor that is affected by the command."""
    @property
    def enabled(self) -> bool:
        """If physics should be activated or not."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, enabled: bool) -> None:
        """Determines whether an actor will be affected by physics or not.

        Args:
            `actor (Actor | int)`: Actor or Actor ID to which the command will be applied to.\n
            `enabled (bool)`: If physics should be activated or not.
        """
    # endregion

class SetVehicleLightState(_IsCommand):
    """Command adaptation of `set_light_state()` in `carla.Vehicle`. Sets the light state of a vehicle."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor that is affected by the command."""
    @property
    def light_state(self) -> VehicleLightState:
        """Recaps the state of the lights of a vehicle, these can be used as a flags."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, light_state: VehicleLightState) -> None:
        """Sets the light state of a vehicle.

        Args:
            `actor (Actor | int)`: Actor or its ID to whom the command will be applied to.\n
            `light_state (VehicleLightState)`: Recaps the state of the lights of a vehicle, these can be used as a flags.
        """
    # endregion

class ShowDebugTelemetry(_IsCommand):
    """Command adaptation of `show_debug_telemetry()` in `carla.Actor`. Displays vehicle control telemetry data."""

    # region Instance Variables
    @property
    def actor_id(self) -> int:
        """Actor that is affected by the command."""
    @property
    def enabled(self) -> bool:
        """If debug should be activated or not."""
    # endregion

    # region Methods
    def __init__(self, actor: Actor | int, enabled: bool) -> None:
        """Displays vehicle control telemetry data.

        Args:
            `actor (Actor | int)`: Actor or Actor ID to which the command will be applied to.\n
            `enabled (bool)`: If debug should be activated or not.
        """
    # endregion

class SpawnActor(_IsCommand):
    """Command adaptation of `spawn_actor()` in `carla.World`. Spawns an actor into the world based on the blueprint provided and the transform. If a parent is provided, the actor is attached to it."""

    # region Instance Variables
    @property
    def transform(self) -> Transform:
        """Transform to be applied."""
    @property
    def parent_id(self) -> int:
        """Identificator of the parent actor."""
    # endregion

    # region Methods
    @overload
    def __init__(self) -> None: ...
    @overload
    def __init__(self, blueprint: ActorBlueprint, transform: Transform) -> None: ...
    @overload
    def __init__(self, blueprint: ActorBlueprint, transform: Transform, parent: Actor | int) -> None: ...

    def then(self, command: _IsCommand) -> _IsCommand:
        """Links another command to be executed right after. It allows to ease very common flows such as spawning a set of vehicles by command and then using this method to set them to autopilot automatically.

        Args:
            `command (command)`: a Carla command.
        """
    # endregion
