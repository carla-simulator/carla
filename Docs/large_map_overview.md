# Large maps

- [__Large maps overview__](#large-maps-overview)
- [__Tile streaming__](#tile-streaming)
- [__Dormant actors__](#dormant-actors)

---

## Large maps overview

The large map feature in CARLA allows users to perform simulations at huge scale. In CARLA, large maps are divided into square tiles no larger than 2kmx2km. Tiles will be streamed in and out of the server based on their proximity, or streaming distance, to the ego vehicle. Other actors on the map are also managed according to their streaming distance from the ego vehicle.

---

### Tile streaming

The ego vehicle is integral to the loading and unloading of map tiles. Tiles are streamed in and out of the server based on the value of the streaming distance from the ego vehicle.

To set the streaming distance so tiles will be loaded within a 2km radius of the ego vehicle:

```py
settings = world.get_settings()
settings.tile_stream_distance = 2
world.apply_settings(settings)
```

You can also set the streaming distance using `config.py`:

```sh
cd PythonAPI/util
python3 config.py --tile-stream-distance 2
```

!!! Note
    Large maps currently supports only one ego vehicle at a time.

---

### Dormant actors

The large map feature introduces the concept of dormant actors to CARLA. Dormant actors exist within the context of large maps only. Dormant actors are non-ego-vehicle actors in the simulation that are located outside of the __actor active distance__ of the ego vehicle, e.g., vehicles far from the ego vehicle. The actor active distance can be equal to or less than the streaming distance.

If an actor finds itself outside of the actor active distance of the ego vehicle, it will become dormant. The actor will still exist, but it will not be rendered. Physics will only be computed if [hybrid mode](#adv_traffic_manager.md#hybrid-physics-mode) is set on that actor. A dormant actor's [location](python_api.md#carla.Actor.set_location) and [transformation](python_api.md#carla.Actor.set_transform) can still be set, even if their physics is not being calculated. Once the dormant actor comes within actor active distance of the ego vehicle again, it will wake up, and its rendering and physics will resume as normal.

An actor will become dormant or wake up on a [`world.tick()`](python_api.md#carla.World.tick).

To set the actor active distance to a 2 km radius around the ego vehicle:

```py
settings = world.get_settings()
settings.actor_active_distance = 2
world.apply_settings(settings)
```

You can also set the actor active distance using `config.py`:

```sh
cd PythonAPI/util
python3 config.py --actor-active-distance 2
```

To check if an actor is dormant, you can use the Python API:

```py
actor.is_dormant
```

---





