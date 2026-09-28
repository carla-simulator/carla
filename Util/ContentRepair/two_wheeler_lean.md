# Two-wheelers leaning away from turns

Verified against code `c83b98916` (`ue58-dev`) and content `8069f39db08`
(`ue58-dev-carla`) on 2026-09-10.

## Cause

The migrated vehicles use four invisible outrigger wheels and deliberately
lowered centers of mass. Their inward lean comes from the tire forces acting
above the center of mass, rather than an explicit lean animation.

UE4 PhysX applied tire forces at the suspension force application position
(wheel-center position plus the suspension force offset). Chaos defaults to
applying those forces at the ground contact point instead. On these assets,
that puts the lateral tire force below the lowered center of mass and reverses
its roll moment. The bikes consequently roll outward like a car.

For a right turn, the tire force points right. Applied above the center of
mass, it tips the top of the bike right; applied below it, it tips the top
left. The relevant relationship is `torque = (application point - COM) × force`.

The setting `VehicleMovementComp > Wheel Setup > Legacy Wheel Friction
Position` was false on all seven migrated vehicles. Setting it to true restores
the wheel-center application point used by their existing tuning.

The source paths explaining the difference, relative to the engine root:

- UE4: `Engine/Plugins/Runtime/PhysXVehicles/Source/PhysXVehicles/Private/WheeledVehicleMovementComponent.cpp`,
  `SetupWheelMassProperties_AssumesLocked`: `PTireForceAppCMOffset = PSuspForceAppCMOffset`.
- UE5: `Engine/Plugins/Experimental/ChaosVehiclesPlugin/Source/ChaosVehicles/Private/ChaosWheeledVehicleMovementComponent.cpp`,
  `ApplyWheelFrictionForces`: `bLegacyWheelFrictionPosition` selects
  `WheelState.WheelWorldLocation[WheelIdx]` instead of `HitResult.ImpactPoint`.

This explains why inspecting the Blueprint event/animation graphs did not
reveal a reversed lean calculation. `BP_Base2wheeledNew` reads steering for
the handlebars; `4WheeledBikeAnim` does not implement body lean. The reversal
happens in the physics force application geometry.

## Fix

Enable `bLegacyWheelFrictionPosition` on the movement component of
`BP_CrossBike`, `BP_LeisureBike`, `BP_RoadBike`, `BP_Harley`,
`BP_KawasakiNinja`, `BP_Vespa`, and `BP_Yamaha` in the content repository.
`fix_two_wheeler_lean.py` applies this exact change and supports a dry run.
It is idempotent; a separate editor process reloaded all seven saved assets
and reported true for every flag, with zero further updates needed.

No engine/CARLA runtime rebuild is required. Restart a running editor/server
to load the corrected assets; packaged builds need the updated content cooked.

## Runtime verification

Used a separate headless server on port 4654 and a flat, 200 m wide generated
road. Each vehicle starts fresh for each case, accelerates straight for 4 s,
then holds steering at -0.35, 0, or +0.35 for 5 s at approximately 20 km/h.
Values below are the mean body tilt over the final 2 s, measured from the
vehicle up vector against its horizontal right vector. Actual yaw rate
confirms the vehicle turns in the commanded direction.

| Vehicle | Before: outward left/right | After: inward left/right |
| --- | ---: | ---: |
| BH Crossbike | 9.89° / 9.39° | 2.32° / 2.78° |
| Diamondback Century | 11.53° / 11.72° | 3.32° / 3.88° |
| Gazelle Omafiets | 11.64° / 11.64° | 2.20° / 2.48° |
| Harley Low Rider | 12.67° / 12.37° | 0.82° / 1.04° |
| Kawasaki Ninja | 9.66° / 9.39° | 2.19° / 2.41° |
| Vespa ZX125 | 7.89° / 7.29° | 2.20° / 2.72° |
| Yamaha YZF | 12.64° / 12.33° | 0.81° / 1.05° |

All 14 turning cases reproduced outward lean before the change and inward
lean afterward. All seven straight cases stayed below 0.3° mean tilt.
The regression test passed **21/21** cases after the change:

```sh
python PythonAPI/test/test_two_wheeler_lean.py --port 4654 --output lean.json
```

Run against a dedicated server: the test replaces its map. It asserts motion,
turn direction, inward lean, and straight-line stability, and writes per-tick
measurements to JSON. The fix restores the legacy force geometry; these tests
do not claim physically accurate motorcycle lean angles or cover all speeds,
slopes, jumps, and collision conditions.
