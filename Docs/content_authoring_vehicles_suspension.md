# Content authoring - vehicle suspension

Every wheel of a CARLA vehicle is a `UChaosVehicleWheel` asset with its own suspension, so a four-wheeled vehicle has four independent suspensions. This page covers what the parameters mean and how to set them in the editor, which is where a vehicle's permanent values live. To change them on a running vehicle instead, see [Changing the suspension at runtime](tuto_G_control_vehicle_physics.md#changing-the-suspension-at-runtime).

- [__Wheel blueprints__](#wheel-blueprints)
- [__Suspension parameters__](#suspension-parameters)
- [__Editing a wheel blueprint__](#editing-a-wheel-blueprint)

---
## Wheel blueprints

CARLA's vehicles give their four wheels the same values unless the vehicle calls for otherwise. The wheel blueprints live next to the vehicle blueprint in `Content/Carla/Blueprints/Vehicles/<VehicleName>/` and are named `BP_<VehicleName>_<F|R><L|R>W`:

*   `F` or `R` — front or rear.
*   `L` or `R` — left or right.

![wheel_blueprints](img/content_authoring_vehicles_suspension/wheel_blueprints.webp)
<div class="img-caption">The Audi TT folder: the vehicle blueprint and its four wheel blueprints; the front left wheel is <code>BP_AudiTT_FLW</code>.</div>

---
## Suspension parameters

These are the properties under the __Suspension__ category of a wheel blueprint, with the defaults CARLA applies when a vehicle does not override them.

| Parameter | Default | Meaning |
| --- | --- | --- |
| `Suspension Axis` | `(0, 0, -1)` | Direction, in body space, along which the suspension force is applied. |
| `Suspension Force Offset` | `(0, 0, 0)` | Offset from the wheel at which that force is applied. |
| `Suspension Max Raise` | `10.0` | How far the wheel can travel above its resting position, in cm. |
| `Suspension Max Drop` | `10.0` | How far the wheel can drop below its resting position, in cm. |
| `Suspension Damping Ratio` | `0.5` | How fast the spring comes to rest, in `[0, 1]`. Higher damps the bouncing sooner. |
| `Wheel Load Ratio` | `0.5` | How much the load on the wheel affects its friction, in `[0, 1]`. `0` ignores the load, `1` is the realistic end; lower values cure lift-off oversteer. |
| `Spring Rate` | `250.0` | Spring force, in N/m. |
| `Spring Preload` | `50.0` | Spring preload, in N/m. |
| `Suspension Smoothing` | `0` | Smoothing of the suspension length, `0` off to `10` maximum. High values can make a wheel visibly sink into the ground for a frame. |
| `Rollbar Scaling` | `0.15` | Anti-roll bar effect, in `[0, 1]`. |
| `Sweep Shape` | `Raycast` | How the ground is found: `Raycast` (fastest), `Spherecast`, or `Shapecast` (uses the wheel's collision shape, slowest). |
| `Sweep Type` | `SimpleSweep` | Geometry the sweep tests against: `SimpleSweep` or `ComplexSweep`. |

CARLA's vehicles override several of these. The Audi TT's front left wheel, for example, uses `Suspension Max Raise` and `Max Drop` of `7.5`, a `Suspension Damping Ratio` of `1.0`, and a `Spherecast` with `ComplexSweep`.

!!! Note
    A stiffer vehicle wants smaller `Suspension Max Raise` and `Max Drop` with a higher `Spring Rate` and `Damping Ratio`; a soft, off-road one wants the opposite. Raise `Rollbar Scaling` to keep a tall vehicle from leaning in corners.

---
## Editing a wheel blueprint

__1.__ Open the vehicle's folder in the Unreal editor and double-click the wheel blueprint you want to change, for example `BP_AudiTT_FLW`.

__2.__ In the __Details__ panel, open the __Suspension__ category and set the values.

![wheel_suspension](img/content_authoring_vehicles_suspension/wheel_suspension.webp)
<div class="img-caption">The Suspension category of <code>BP_AudiTT_FLW</code>; the arrows on the right mark the values that differ from the defaults.</div>

__3.__ Repeat for the other three wheels, or set them all at once by selecting the four assets and editing the shared property.

__4.__ Compile, save, and drive the vehicle to check the result.

!!! Warning
    `Wheel Radius` has to keep the wheel resting on the road, neither hovering above it nor sunk into it. Changing the suspension travel without checking the radius is the usual cause of a floating vehicle.
