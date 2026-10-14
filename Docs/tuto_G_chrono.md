# Chrono Integration

This guide outlines what Chrono is, how to use it in CARLA, and the limitations involved in the integration.

- [__Project Chrono__](#project-chrono)
- [__Using Chrono on CARLA__](#using-chrono-on-carla)
    - [Configuring the server](#configuring-the-server)
    - [Enabling Chrono physics](#enabling-chrono-physics)
- [__Limitations__](#limitations)

---

## Project Chrono

[Project Chrono](https://projectchrono.org/) is an open-source, multi-physics simulation engine that provides highly realistic vehicle dynamics using a template-based approach. The integration in CARLA allows users to utilize Chrono templates to simulate vehicle dynamics while navigating a map.

---

## Using Chrono with CARLA

To use the Chrono integration for vehicle Physics in CARLA, you must first build or launch the server with a build flag and then use the PythonAPI to enable Chrono for a vehicle spawned in the CARLA simulation.

!!! note
    The CARLA-Chrono integration is compatible with **Chrono version 6**. Ensure that your Chrono installation is compatible before starting. 

### Configuring the server

Chrono will only work if the CARLA server is compiled with the Chrono tag.

__In the build from source version of CARLA__, configure the project with `ENABLE_CHRONO` and then launch:

```sh
cmake --preset Release -DENABLE_CHRONO=ON
cmake --build Build/Release --target launch
```

The above pulls the source code for version 10.0.0 from the Project Chrono repository. If you wish to use your own Chrono checkout, point `CARLA_CHRONO_SOURCE_PATH` at it:

```sh
cmake --preset Release -DENABLE_CHRONO=ON -DCARLA_CHRONO_SOURCE_PATH=<PATH>
```

A different upstream revision can be selected with `-DCARLA_CHRONO_TAG=<TAG>`, but note that the
integration targets the Chrono 10 API and is not guaranteed to compile against other releases.

---

### Enabling Chrono physics

Chrono physics is enabled using the `enable_chrono_physics` method available through the [Actor](python_api.md#carlaactor) class. As well as values for substeps and substep delta time, it requires three template files and a base path to locate those files:

- __`base_path`:__ Path of the directory which contains the template files. This is necessary to ensure that auxiliary files referenced from the template files have a common base path from which to search.
- __`vehicle_json`:__ Path of the vehicle template file relative to the `base_path`.
- __`tire_json`:__ Path of the tire template file relative to the `base_path`. The tire model must take its forces from terrain queries, as TMeasy and Pacejka do. Chrono's rigid tire does not: it only produces forces through Chrono's own collision system, which never sees the CARLA terrain, so a vehicle on rigid tires falls through the ground.
- __`powertrain_json`:__ Path of the powertrain template file relative to the `base_path`. Since Chrono 8 a powertrain is an engine plus a transmission, so this file names the two templates to pair, using the same keys as the `Powertrain` block of a Chrono vehicle JSON:

```json
{
  "Name": "Sedan Simple Map Powertrain",
  "Type": "Powertrain",
  "Template": "PowertrainAssembly",
  "Engine Input File": "sedan/powertrain/Sedan_EngineSimpleMap.json",
  "Transmission Input File": "sedan/powertrain/Sedan_AutomaticTransmissionSimpleMap.json"
}
```

  Both paths are relative to `base_path`. The engine and transmission files themselves use Chrono's own `Engine` and `Transmission` template formats.

!!! Important
    Every template is checked before Chrono takes over the vehicle. A path that does not exist, a file that is not valid JSON, or a template of the wrong `Type` (an engine passed as the tire, say) makes `enable_chrono_physics` raise `RuntimeError` with the reason, and the vehicle keeps its current physics. Called without template arguments, it uses the sedan templates under `Co-Simulation/Chrono/Vehicles/`.

A default implementation for a sedan is provided in `Co-Simulation/Chrono/Vehicles/sedan/`. Only its JSON templates are needed: Unreal renders the vehicle, so CARLA does not load the meshes a Chrono template references. More example templates for other vehicles are available in `Build/Release/Chrono/install/share/chrono/data/vehicle`. Read the Project Chrono [documentation](https://api.projectchrono.org/manual_vehicle.html) to find out more about their vehicle examples and how to create templates.

See below for an example of how to enable Chrono physics:

```python
    # Spawn your vehicle
    vehicle = world.spawn_actor(bp, spawn_point)

    # Set the base path
    base_path = "/path/to/carla/Co-Simulation/Chrono/Vehicles/"

    # Set the template files

    vehicle_json = "sedan/vehicle/Sedan_Vehicle.json"
    powertrain_json = "sedan/powertrain/Sedan_SimpleMapPowertrain.json"
    tire_json = "sedan/tire/Sedan_TMeasyTire.json"

    # Enable Chrono physics

    vehicle.enable_chrono_physics(5000, 0.002, vehicle_json, powertrain_json, tire_json, base_path)
```

You can try the Chrono physics integration using the example script `manual_control_chrono.py` found in `PythonAPI/examples`. After running the script, press `Ctrl + o` to enable Chrono.

---

### Limitations

This integration does not support collisions. __When a collision occurs, the vehicle will revert to CARLA default physics.__
