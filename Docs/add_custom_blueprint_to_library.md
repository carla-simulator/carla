# Adding custom blueprints to Carla blueprint library

This guide describes a process of adding custom blueprints created in Unreal Editor 4 to Carla's blueprint library, that makes it possible to spawn the blueprint using the Python API. 

In order for your blueprints to be correctly parsed by Carla, they must have the `Actor (AActor)` class as their parent or a class that inherits `Actor` in its hierarchy. 

Create a file named `BlueprintParameters.json` in `/CarlaUE4/Content/Carla/Config/` directory with following format:

```
{
  "Blueprints": [
    {
      "Name": "SomeBlueprintName",
      "Path": "/Game/Path/To/SomeBlueprintName"
    },
	{
      "Name": "SomeOtherBlueprintName",
      "Path": "/Game/Path/To/SomeOtherBlueprintName"
    }
  ]
}
```
If, for example, in your UE4 Content Browser you have a blueprint with path `"Content -> Blueprints -> SomeBlueprintName"`, then you would modify the `Path` in `BlueprintParameters.json` as `"Path": "/Game/Blueprints/SomeBlueprintName"`. Repeat for other blueprints. That's it! When you run the simulator and retrieve the blueprint library using the Python API, you should see your custom blueprints listed there. In order to retrieve your custom blueprint, you coould do the following:

```
world = client.get_world()
blueprint_library = world.get_blueprint_library()
blueprint_name = 'SomeBlueprintName'
bp = blueprint_library.find(blueprint_name)
```

You have now successfully retrieved your blueprint in Python API and can spawn it using:

```
spawn_point = random.choice(world.get_map().get_spawn_points())
actor = world.spawn_actor(bp, spawn_point)
```
