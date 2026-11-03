# The client

The client is how a script reaches a running simulation: it connects to the server, asks it for the world, and sends it commands. Everything a CARLA script does starts here.

- [__Client creation__](#client-creation)
- [__World connection__](#world-connection)
- [__Using commands__](#using-commands)
- [__Other client utilities__](#other-client-utilities)

Clients are one of the main elements in the CARLA architecture. They connect to the server, retrieve information, and command changes. That is done via scripts. The client identifies itself, and connects to the world to then operate with the simulation.

Besides that, clients are able to access advanced CARLA modules, features, and apply command batches. Only command batches will be covered in this section. These are useful for basic things such as spawning lots of actors. The rest are covered in their own pages.

Take a look at [__carla.Client__](python_api.md#carla.Client) in the Python API reference to learn on specific methods and variables of the class.


## Client creation

Two things are needed: the __IP__ address of the server and the __RPC port__ it listens on, 2000 by default. The server also opens the next two ports, 2001 for sensor streaming and 2002 for a secondary server, but the client is given only the first one. An optional third parameter sets the number of working threads, all of them (`0`) by default. [carla.Client](python_api.md#carla.Client.__init__) in the Python API reference has a snippet that parses these as script arguments.

```py
client = carla.Client('localhost', 2000)
```
By default, CARLA uses local host IP, and port 2000 to connect but these can be changed at will. The second port will always be `n+1`, 2001 in this case.

Once the client is created, set its __time-out__. This limits all networking operations so that these don't block the client forever. An error will be returned if connection fails.

```py
client.set_timeout(10.0) # seconds
```

It is possible to have many clients connected, as it is common to have more than one script running at a time. Working in a multiclient scheme with advanced CARLA features, such as the traffic manager, is bound to make communication more complex.

!!! Note
    Client and server have different `libcarla` modules. If the versions differ, issues may arise. This can be checked using the `get_client_version()` and `get_server_version()` methods.

## World connection

A client can connect and retrieve the current world fairly easily.

```py
world = client.get_world()
```

The client can also get a list of available maps to change the current one. This will destroy the current world and create a new one.
```py
print(client.get_available_maps())
...
world = client.load_world('Town01_Opt')
# client.reload_world() creates a new instance of the world with the same map. 
```

Every world object has an `id` or episode. Everytime the client calls for `load_world()` or `reload_world()` the previous one is destroyed. A new one is created from scratch with a new episode. Unreal Engine is not rebooted in the process.

## Using commands

__Commands__ are adaptations of some of the most-common CARLA methods, that can be applied in batches. For instance, the [command.SetAutopilot](python_api.md#command.SetAutopilot) is equivalent to [Vehicle.set_autopilot()](python_api.md#carla.Vehicle.set_autopilot), enables the autopilot for a vehicle. However, using the methods [Client.apply_batch](python_api.md#carla.Client.apply_batch) or [Client.apply_batch_sync()](python_api.md#carla.Client.apply_batch_sync), a list of commands can be applied in one single simulation step. This becomes extremely useful for methods that are usually applied to even hundreds of elements.

The following example uses a batch to destroy a list of vehicles all at once.

```py
client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])
```

Every available command is listed at the end of the [Python API reference](python_api.md#command.SpawnActor).

## Other client utilities

The main purpose of the client object is to get or change the world, and apply commands. However, it also provides access to some additional features.

*   __Traffic manager.__ This module is in charge of every vehicle set to autopilot to recreate urban traffic.  
*   __[Recorder](core_recorder.md).__ Allows to reenact a previous simulation. Uses [snapshots](core_world.md#world-snapshots) summarizing the simulation state per frame.

---

The world the client returns is the subject of [The world](core_world.md).
