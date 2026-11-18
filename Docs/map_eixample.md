# Eixample

![eixample_slideshow](img/catalogue/maps/eixample/eixample_montage.webp)
<div class="img-caption">Eixample from above at an angle and from almost overhead, with its untextured building volumes, and EixampleDemo, the same district with façades, trees and traffic lights.</div>

Eixample is a [digital twin](content_authoring_maps_digital_twin.md) of part of the Eixample district of Barcelona: the street grid of the district, generated from map data, with the buildings as simple untextured volumes. It is a small [World Partition](core_large_maps.md) map.

| | |
| --- | --- |
| __Map name__ | `Eixample` |
| __In the default package__ | no: install a [content pack](map_content_packs.md) that contains it, such as AdditionalMaps, or use a source build |
| __Map type__ | a [World Partition](core_large_maps.md) map: the server streams the cells around the vehicle with `role_name=hero` |
| __Size__ | about 510 x 455 m of roads |
| __Spawn points__ | 274, from `world.get_map().get_spawn_points()` |
| __Junctions__ | 15, with 27 traffic lights |

Load it like any other map:

```py
world = client.load_world('Eixample')
```

Only the cells around the ego vehicle are loaded. Spawn the ego with `role_name=hero` before placing sensors, as described in [Large maps](core_large_maps.md).

!!! Note
    The buildings are volumes without textures or windows, so street-level camera images are mostly flat walls. The map is best suited to driving and planning tests on a real street layout.

!!! Note
    The content also holds `EixampleDemo`, the same district with procedural façades, vegetation and staged traffic signals. It references assets of the `CarlaDigitalTwinsTool` content, which CARLA does not distribute, so without that content its buildings load grey and untextured. See [Digital Twin Tool](content_authoring_maps_digital_twin.md) and [EixampleDemo](#eixampledemo) below.

---
## Road network

The streets follow the diagonal grid of the district: wide avenues of up to three lanes in each direction crossed by narrower streets.

![eixample_top](img/catalogue/maps/eixample/eixample_top.webp)
<div class="img-caption">Eixample from almost overhead: one of the wide avenues across the grid.</div>

![eixample_roads](img/catalogue/maps/eixample/eixample_roads.webp)
<div class="img-caption">The road network of Eixample from the Map API, with the index of each spawn point.</div>

---
## Buildings

Each block of the grid is filled with untextured building volumes of different heights around inner courtyards, in the layout of the real district.

![eixample_aerial](img/catalogue/maps/eixample/eixample_aerial.webp)
<div class="img-caption">The building volumes and inner courtyards of the blocks along a street of the grid.</div>

---
## EixampleDemo

`EixampleDemo` is the same district dressed by the [Digital Twin Tool](content_authoring_maps_digital_twin.md): procedural façades on every building, trees along the avenues and traffic lights at the junctions. The images below are taken with the `CarlaDigitalTwinsTool` content installed. Without it, the buildings of `EixampleDemo` load grey and untextured.

```py
world = client.load_world('EixampleDemo')
```

![eixample_demo_crossing](img/catalogue/maps/eixample/eixample_demo_crossing.webp)
<div class="img-caption">A junction of EixampleDemo with traffic lights and pedestrian crossings on every side.</div>

![eixample_demo_junction](img/catalogue/maps/eixample/eixample_demo_junction.webp)
<div class="img-caption">Traffic waiting at the stop line of another junction, with the avenue beyond.</div>

![eixample_demo_avenue](img/catalogue/maps/eixample/eixample_demo_avenue.webp)
<div class="img-caption">One of the avenues of EixampleDemo, with a row of trees between the road and the sidewalk.</div>

![eixample_demo_street](img/catalogue/maps/eixample/eixample_demo_street.webp)
<div class="img-caption">EixampleDemo at street level: the façades of the blocks and the traffic lights of the next junction.</div>
