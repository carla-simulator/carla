"""Restore the tire-force application point used by the legacy two-wheelers.

Run with UnrealEditor-Cmd <project> -run=pythonscript -script=<this file>
-nullrhi -unattended -nosound. Set FIX_TWO_WHEELER_DRY_RUN=1 to inspect only.

These assets balance on four virtual outrigger wheels with a lowered center
of mass. PhysX applied tire forces at wheel height; Chaos defaults to the
ground contact point. Moving the lateral force below the center of mass
reverses its roll moment, making the vehicle lean out of a turn. Preserve
the original force geometry for these seven assets only.
"""

import os

import unreal


VEHICLES = (
    "CrossBike", "LeisureBike", "RoadBike", "Harley",
    "KawasakiNinja", "Vespa", "Yamaha",
)
PROPERTY = "legacy_wheel_friction_position"


def main():
    dry_run = os.environ.get("FIX_TWO_WHEELER_DRY_RUN") == "1"
    pending = []
    # Resolve every asset before changing any of them.
    for name in VEHICLES:
        path = f"/Game/Carla/Blueprints/Vehicles/2Wheeled/{name}/BP_{name}"
        cls = unreal.EditorAssetLibrary.load_blueprint_class(path)
        if cls is None:
            raise RuntimeError(f"Missing two-wheeler Blueprint: {path}")
        movement = unreal.get_default_object(cls).get_component_by_class(
            unreal.ChaosWheeledVehicleMovementComponent)
        if movement is None:
            raise RuntimeError(f"Missing Chaos movement component: {path}")
        current = movement.get_editor_property(PROPERTY)
        unreal.log(f"Two-wheeler lean: {path}: {PROPERTY}={current}")
        if not current:
            pending.append((path, movement))

    for path, movement in pending:
        if not dry_run:
            movement.set_editor_property(PROPERTY, True)
            if not unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False):
                raise RuntimeError(f"Failed to save {path}")
    unreal.log(f"Two-wheeler lean: {'would update' if dry_run else 'updated'} "
               f"{len(pending)} Blueprint(s)")


if __name__ == "__main__":
    main()
