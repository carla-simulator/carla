# Open-cast mine (Mine_01)

![mine01](img/catalogue/maps/mine01/mine_map.webp)

A model of an open-cast mining environment of the kind used for extracting minerals around the world. The mining area is connected to a parking area by a small network of access roads. The map was contributed by CARLA consortium members and sponsors [Synkrotron](https://www.synkrotron.ai/).

| | |
| --- | --- |
| __Map name__ | `Mine_01` |
| __In the default package__ | yes |
| __Surface__ | unpaved access roads and open pit, rather than the asphalt grid of the towns |
| __Map-specific assets__ | `vehicle.miningtruck.miningtruck`, the mining earth mover, is only spawnable while this map is loaded |

Load it like any other map:

```py
world = client.load_world('Mine_01')
```
