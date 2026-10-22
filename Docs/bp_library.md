# Blueprint library

The blueprint library ([`carla.BlueprintLibrary`](python_api.md#carla.BlueprintLibrary)) lists every actor ([`carla.ActorBlueprint`](python_api.md#carla.ActorBlueprint)) that can be spawned, with the attributes ([`carla.ActorAttribute`](python_api.md#carla.ActorAttribute)) each one accepts. Start with the [introduction to blueprints](core_actors.md).

```py
blueprints = [bp for bp in world.get_blueprint_library().filter('*')]
for blueprint in blueprints:
   print(blueprint.id)
   for attr in blueprint:
       print('  - {}'.format(attr))
```

- [Vehicles](#vehicles) (25 blueprints)
- [Pedestrians](#pedestrians) (38 blueprints)
- [Controllers](#controllers) (1 blueprints)
- [Sensors](#sensors) (28 blueprints)
- [Props](#props) (84 blueprints)
- [Util](#util) (1 blueprints)

---

## Vehicles

Spawnable vehicles. See the [vehicle catalogue](catalogue_vehicles.md) for images and [Actors](core_actors.md) for how to drive them.

### vehicle.ambulance.ford

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `truck` |  | no |
| `color` | RGBColor | `255,255,255` | `255,255,255` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.ambulance.ford` | `vehicle.ambulance.ford` | yes |
| `ros_topic_name` | String | `vehicle.ambulance.ford` | `vehicle.ambulance.ford` | yes |
| `special_type` | String | `emergency` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.bh.crossbike

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `bicycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.bh.crossbike` | `vehicle.bh.crossbike` | yes |
| `ros_topic_name` | String | `vehicle.bh.crossbike` | `vehicle.bh.crossbike` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.carlacola.actors

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `truck` |  | no |
| `color` | RGBColor | `149,0,14` | `149,0,14` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.carlacola.actors` | `vehicle.carlacola.actors` | yes |
| `ros_topic_name` | String | `vehicle.carlacola.actors` | `vehicle.carlacola.actors` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.diamondback.century

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `bicycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.diamondback.century` | `vehicle.diamondback.century` | yes |
| `ros_topic_name` | String | `vehicle.diamondback.century` | `vehicle.diamondback.century` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.dodge.charger

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,0,0` | `0,0,0`, `71,71,71`, `220,218,217`, `78,17,17`, `142,45,22`, `221,50,13`, `72,119,172` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.dodge.charger` | `vehicle.dodge.charger` | yes |
| `ros_topic_name` | String | `vehicle.dodge.charger` | `vehicle.dodge.charger` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.dodgecop.charger

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `26,26,26` | `26,26,26` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.dodgecop.charger` | `vehicle.dodgecop.charger` | yes |
| `ros_topic_name` | String | `vehicle.dodgecop.charger` | `vehicle.dodgecop.charger` | yes |
| `special_type` | String | `emergency` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.firetruck.actors

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `truck` |  | no |
| `color` | RGBColor | `255,75,26` | `255,75,26` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.firetruck.actors` | `vehicle.firetruck.actors` | yes |
| `ros_topic_name` | String | `vehicle.firetruck.actors` | `vehicle.firetruck.actors` | yes |
| `special_type` | String | `emergency` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.fuso.mitsubishi

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `bus` |  | no |
| `color` | RGBColor | `250,252,246` | `250,252,246`, `212,229,246`, `167,44,58`, `0,84,166`, `142,142,142`, `255,238,170` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.fuso.mitsubishi` | `vehicle.fuso.mitsubishi` | yes |
| `ros_topic_name` | String | `vehicle.fuso.mitsubishi` | `vehicle.fuso.mitsubishi` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.gazelle.omafiets

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `bicycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.gazelle.omafiets` | `vehicle.gazelle.omafiets` | yes |
| `ros_topic_name` | String | `vehicle.gazelle.omafiets` | `vehicle.gazelle.omafiets` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.harley-davidson.low_rider

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `motorcycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.harley-davidson.low_rider` | `vehicle.harley-davidson.low_rider` | yes |
| `ros_topic_name` | String | `vehicle.harley-davidson.low_rider` | `vehicle.harley-davidson.low_rider` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.kawasaki.ninja

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `motorcycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.kawasaki.ninja` | `vehicle.kawasaki.ninja` | yes |
| `ros_topic_name` | String | `vehicle.kawasaki.ninja` | `vehicle.kawasaki.ninja` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.kia.niro

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,0,0` | `0,0,0`, `71,71,71`, `220,218,217`, `78,17,17`, `142,45,22`, `221,50,13`, `72,119,172` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.kia.niro` | `vehicle.kia.niro` | yes |
| `ros_topic_name` | String | `vehicle.kia.niro` | `vehicle.kia.niro` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.lincoln.mkz

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,0,0` | `0,0,0` | yes |
| `generation` | Int | `2` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.lincoln.mkz` | `vehicle.lincoln.mkz` | yes |
| `ros_topic_name` | String | `vehicle.lincoln.mkz` | `vehicle.lincoln.mkz` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.mini.cooper

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,0,0` | `0,0,0`, `104,4,8`, `27,54,118`, `26,62,29` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.mini.cooper` | `vehicle.mini.cooper` | yes |
| `ros_topic_name` | String | `vehicle.mini.cooper` | `vehicle.mini.cooper` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.nissan.patrol

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `2,35,54` | `2,35,54`, `83,83,83`, `172,172,172`, `5,5,5`, `122,107,100`, `159,65,47` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.nissan.patrol` | `vehicle.nissan.patrol` | yes |
| `ros_topic_name` | String | `vehicle.nissan.patrol` | `vehicle.nissan.patrol` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.sprinter.mercedes

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `van` |  | no |
| `color` | RGBColor | `233,234,236` | `233,234,236`, `135,151,160`, `78,80,77`, `69,22,25`, `28,51,83` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.sprinter.mercedes` | `vehicle.sprinter.mercedes` | yes |
| `ros_topic_name` | String | `vehicle.sprinter.mercedes` | `vehicle.sprinter.mercedes` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.taxi.ford

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `232,192,0` | `232,192,0` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.taxi.ford` | `vehicle.taxi.ford` | yes |
| `ros_topic_name` | String | `vehicle.taxi.ford` | `vehicle.taxi.ford` | yes |
| `special_type` | String | `taxi` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.tesla.model3

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `235,238,240` | `235,238,240`, `12,14,18`, `140,12,20` | yes |
| `generation` | Int | `3` |  | no |
| `has_dynamic_doors` | Bool | `true` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.tesla.model3` | `vehicle.tesla.model3` | yes |
| `ros_topic_name` | String | `vehicle.tesla.model3` | `vehicle.tesla.model3` | yes |
| `special_type` | String | `electric` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.ue4.audi.tt

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,0,0` | `0,0,0`, `27,54,118`, `104,4,8`, `230,216,205`, `108,108,108` | yes |
| `generation` | Int | `2` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.ue4.audi.tt` | `vehicle.ue4.audi.tt` | yes |
| `ros_topic_name` | String | `vehicle.ue4.audi.tt` | `vehicle.ue4.audi.tt` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.ue4.chevrolet.impala

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,146,255` | `0,146,255`, `27,54,118`, `104,4,8`, `230,216,205`, `108,108,108` | yes |
| `generation` | Int | `2` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.ue4.chevrolet.impala` | `vehicle.ue4.chevrolet.impala` | yes |
| `ros_topic_name` | String | `vehicle.ue4.chevrolet.impala` | `vehicle.ue4.chevrolet.impala` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.ue4.ford.crown

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `254,211,0` | `254,211,0` | yes |
| `generation` | Int | `2` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.ue4.ford.crown` | `vehicle.ue4.ford.crown` | yes |
| `ros_topic_name` | String | `vehicle.ue4.ford.crown` | `vehicle.ue4.ford.crown` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.ue4.ford.mustang

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `0,0,0` | `0,0,0`, `27,54,118`, `104,4,8`, `230,216,205`, `108,108,108` | yes |
| `generation` | Int | `2` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.ue4.ford.mustang` | `vehicle.ue4.ford.mustang` | yes |
| `ros_topic_name` | String | `vehicle.ue4.ford.mustang` | `vehicle.ue4.ford.mustang` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.ue4.mercedes.ccc

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `car` |  | no |
| `color` | RGBColor | `73,0,0` | `73,0,0`, `27,54,118`, `104,4,8`, `230,216,205` | yes |
| `generation` | Int | `2` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `4` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.ue4.mercedes.ccc` | `vehicle.ue4.mercedes.ccc` | yes |
| `ros_topic_name` | String | `vehicle.ue4.mercedes.ccc` | `vehicle.ue4.mercedes.ccc` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.vespa.zx125

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `motorcycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.vespa.zx125` | `vehicle.vespa.zx125` | yes |
| `ros_topic_name` | String | `vehicle.vespa.zx125` | `vehicle.vespa.zx125` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

### vehicle.yamaha.yzf

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `motorcycle` |  | no |
| `generation` | Int | `1` |  | no |
| `has_dynamic_doors` | Bool | `false` |  | no |
| `has_lights` | Bool | `true` |  | no |
| `number_of_wheels` | Int | `2` |  | no |
| `object_type` | String | `""` |  | no |
| `role_name` | String | `autopilot` | `autopilot`, `scenario`, `ego_vehicle` | yes |
| `ros2_ackermann_control` | Bool | `false` | `false` | yes |
| `ros2_autoware_control` | Bool | `false` | `false` | yes |
| `ros_name` | String | `vehicle.yamaha.yzf` | `vehicle.yamaha.yzf` | yes |
| `ros_topic_name` | String | `vehicle.yamaha.yzf` | `vehicle.yamaha.yzf` | yes |
| `special_type` | String | `""` |  | no |
| `sticky_control` | Bool | `true` | `true` | yes |
| `terramechanics` | Bool | `false` | `false` | yes |

---

## Pedestrians

Spawnable pedestrians. See the [pedestrian catalogue](catalogue_pedestrians.md); they are driven by a `controller.ai.walker`.

### walker.pedestrian.0015

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `true` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0015` | `walker.pedestrian.0015` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0015` | `walker.pedestrian.0015` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false`, `true` | yes |

### walker.pedestrian.0016

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `true` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0016` | `walker.pedestrian.0016` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0016` | `walker.pedestrian.0016` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false`, `true` | yes |

### walker.pedestrian.0017

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0017` | `walker.pedestrian.0017` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0017` | `walker.pedestrian.0017` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0018

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0018` | `walker.pedestrian.0018` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0018` | `walker.pedestrian.0018` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0019

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0019` | `walker.pedestrian.0019` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0019` | `walker.pedestrian.0019` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0020

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0020` | `walker.pedestrian.0020` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0020` | `walker.pedestrian.0020` | yes |
| `special_type` | String | `police` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0021

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0021` | `walker.pedestrian.0021` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0021` | `walker.pedestrian.0021` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0022

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0022` | `walker.pedestrian.0022` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0022` | `walker.pedestrian.0022` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0023

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0023` | `walker.pedestrian.0023` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0023` | `walker.pedestrian.0023` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0024

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0024` | `walker.pedestrian.0024` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0024` | `walker.pedestrian.0024` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0025

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `true` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0025` | `walker.pedestrian.0025` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0025` | `walker.pedestrian.0025` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false`, `true` | yes |

### walker.pedestrian.0026

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `true` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0026` | `walker.pedestrian.0026` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0026` | `walker.pedestrian.0026` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false`, `true` | yes |

### walker.pedestrian.0027

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0027` | `walker.pedestrian.0027` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0027` | `walker.pedestrian.0027` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0028

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `true` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0028` | `walker.pedestrian.0028` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0028` | `walker.pedestrian.0028` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false`, `true` | yes |

### walker.pedestrian.0029

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `true` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0029` | `walker.pedestrian.0029` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0029` | `walker.pedestrian.0029` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false`, `true` | yes |

### walker.pedestrian.0030

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0030` | `walker.pedestrian.0030` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0030` | `walker.pedestrian.0030` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0031

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0031` | `walker.pedestrian.0031` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0031` | `walker.pedestrian.0031` | yes |
| `special_type` | String | `police` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0032

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0032` | `walker.pedestrian.0032` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0032` | `walker.pedestrian.0032` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0033

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0033` | `walker.pedestrian.0033` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0033` | `walker.pedestrian.0033` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0034

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0034` | `walker.pedestrian.0034` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0034` | `walker.pedestrian.0034` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0035

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0035` | `walker.pedestrian.0035` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0035` | `walker.pedestrian.0035` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0036

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0036` | `walker.pedestrian.0036` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0036` | `walker.pedestrian.0036` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0037

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0037` | `walker.pedestrian.0037` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0037` | `walker.pedestrian.0037` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0038

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0038` | `walker.pedestrian.0038` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0038` | `walker.pedestrian.0038` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0039

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0039` | `walker.pedestrian.0039` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0039` | `walker.pedestrian.0039` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0040

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0040` | `walker.pedestrian.0040` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0040` | `walker.pedestrian.0040` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0041

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0041` | `walker.pedestrian.0041` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0041` | `walker.pedestrian.0041` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0042

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0042` | `walker.pedestrian.0042` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0042` | `walker.pedestrian.0042` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0043

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0043` | `walker.pedestrian.0043` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0043` | `walker.pedestrian.0043` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0044

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0044` | `walker.pedestrian.0044` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0044` | `walker.pedestrian.0044` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0045

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0045` | `walker.pedestrian.0045` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0045` | `walker.pedestrian.0045` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0046

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0046` | `walker.pedestrian.0046` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0046` | `walker.pedestrian.0046` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0047

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0047` | `walker.pedestrian.0047` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0047` | `walker.pedestrian.0047` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0048

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `adult` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0048` | `walker.pedestrian.0048` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0048` | `walker.pedestrian.0048` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.7`, `4.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0049

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `child` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0049` | `walker.pedestrian.0049` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0049` | `walker.pedestrian.0049` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.1`, `2.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0050

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `child` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `2` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0050` | `walker.pedestrian.0050` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0050` | `walker.pedestrian.0050` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.1`, `2.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0051

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `child` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `female` |  | no |
| `generation` | Int | `3` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0051` | `walker.pedestrian.0051` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0051` | `walker.pedestrian.0051` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `1.1`, `2.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

### walker.pedestrian.0052

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `age` | String | `child` |  | no |
| `can_use_wheelchair` | Bool | `false` |  | no |
| `gender` | String | `male` |  | no |
| `generation` | Int | `3` |  | no |
| `is_invincible` | Bool | `true` | `true` | yes |
| `role_name` | String | `pedestrian` | `pedestrian` | yes |
| `ros_name` | String | `walker.pedestrian.0052` | `walker.pedestrian.0052` | yes |
| `ros_topic_name` | String | `walker.pedestrian.0052` | `walker.pedestrian.0052` | yes |
| `special_type` | String | `""` |  | no |
| `speed` | Float | `0` | `0.0`, `0.85`, `2.0` | yes |
| `use_wheelchair` | Bool | `false` | `false` | yes |

---

## Controllers

Actors that drive another actor. `controller.ai.walker` steers a pedestrian towards a destination.

### controller.ai.walker

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `controller.ai.walker` | `controller.ai.walker` | yes |
| `ros_topic_name` | String | `controller.ai.walker` | `controller.ai.walker` | yes |

---

## Sensors

Sensors attachable to an actor. Each one is documented in [Sensors reference](ref_sensors.md).

### sensor.camera.depth

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `dlss_screen_percentage` | Float | `50` | `50.0` | yes |
| `enable_dlss` | Bool | `false` | `false` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.depth` | `sensor.camera.depth` | yes |
| `ros_topic_name` | String | `sensor.camera.depth` | `sensor.camera.depth` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.depth_fisheye

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `lut`, `ftheta` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `equirectangular` | Bool | `false` | `false` | yes |
| `face_size` | Int | `0` | `0` | yes |
| `focal_length` | Float | `0` | `0.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fov_fade_size` | Float | `0` | `0.0` | yes |
| `fov_mask` | Bool | `false` | `false` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `k0` | Float | `0.083092` | `0.083092` | yes |
| `k1` | Float | `0.011121` | `0.011121` | yes |
| `k2` | Float | `0.008587` | `0.008587` | yes |
| `k3` | Float | `0.000854` | `0.000854` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `longitude_offset` | Float | `0` | `0.0` | yes |
| `lut` | String | `""` | `` | yes |
| `perspective` | Bool | `false` | `false` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.depth_fisheye` | `sensor.camera.depth_fisheye` | yes |
| `ros_topic_name` | String | `sensor.camera.depth_fisheye` | `sensor.camera.depth_fisheye` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |

### sensor.camera.dvs

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `dlss_screen_percentage` | Float | `50` | `50.0` | yes |
| `enable_dlss` | Bool | `false` | `false` | yes |
| `enable_postprocess_effects` | Bool | `true` | `true` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `log_eps` | Float | `0.001` | `0.001` | yes |
| `negative_threshold` | Float | `0.3` | `0.3` | yes |
| `positive_threshold` | Float | `0.3` | `0.3` | yes |
| `post_process_profile` | String | `""` | `` | yes |
| `refractory_period_ns` | Int | `0` | `0` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.dvs` | `sensor.camera.dvs` | yes |
| `ros_topic_name` | String | `sensor.camera.dvs` | `sensor.camera.dvs` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `sigma_negative_threshold` | Float | `0` | `0.0` | yes |
| `sigma_positive_threshold` | Float | `0` | `0.0` | yes |
| `use_log` | Bool | `true` | `True` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.instance_segmentation

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `dlss_screen_percentage` | Float | `50` | `50.0` | yes |
| `enable_dlss` | Bool | `false` | `false` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.instance_segmentation` | `sensor.camera.instance_segmentation` | yes |
| `ros_topic_name` | String | `sensor.camera.instance_segmentation` | `sensor.camera.instance_segmentation` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.instance_segmentation_fisheye

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `lut`, `ftheta` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `equirectangular` | Bool | `false` | `false` | yes |
| `face_size` | Int | `0` | `0` | yes |
| `focal_length` | Float | `0` | `0.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fov_fade_size` | Float | `0` | `0.0` | yes |
| `fov_mask` | Bool | `false` | `false` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `k0` | Float | `0.083092` | `0.083092` | yes |
| `k1` | Float | `0.011121` | `0.011121` | yes |
| `k2` | Float | `0.008587` | `0.008587` | yes |
| `k3` | Float | `0.000854` | `0.000854` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `longitude_offset` | Float | `0` | `0.0` | yes |
| `lut` | String | `""` | `` | yes |
| `perspective` | Bool | `false` | `false` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.instance_segmentation_fisheye` | `sensor.camera.instance_segmentation_fisheye` | yes |
| `ros_topic_name` | String | `sensor.camera.instance_segmentation_fisheye` | `sensor.camera.instance_segmentation_fisheye` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |

### sensor.camera.normals

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.normals` | `sensor.camera.normals` | yes |
| `ros_topic_name` | String | `sensor.camera.normals` | `sensor.camera.normals` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.optical_flow

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `dlss_screen_percentage` | Float | `50` | `50.0` | yes |
| `enable_dlss` | Bool | `false` | `false` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.optical_flow` | `sensor.camera.optical_flow` | yes |
| `ros_topic_name` | String | `sensor.camera.optical_flow` | `sensor.camera.optical_flow` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.rgb

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `dlss_screen_percentage` | Float | `50` | `50.0` | yes |
| `enable_dlss` | Bool | `false` | `false` | yes |
| `enable_postprocess_effects` | Bool | `true` | `true` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `post_process_profile` | String | `""` | `` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.rgb` | `sensor.camera.rgb` | yes |
| `ros_topic_name` | String | `sensor.camera.rgb` | `sensor.camera.rgb` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.rgb_fisheye

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `black_clip` | Float | `0` | `0.0` | yes |
| `blade_count` | Int | `5` | `5` | yes |
| `bloom_intensity` | Float | `0.675` | `0.675` | yes |
| `blur_amount` | Float | `1` | `1.0` | yes |
| `blur_radius` | Float | `0` | `0.0` | yes |
| `calibration_constant` | Float | `16` | `16.0` | yes |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `lut`, `ftheta` | yes |
| `chromatic_aberration_intensity` | Float | `0` | `0.0` | yes |
| `chromatic_aberration_offset` | Float | `0` | `0.0` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `enable_postprocess_effects` | Bool | `true` | `true` | yes |
| `equirectangular` | Bool | `false` | `false` | yes |
| `exposure_compensation` | Float | `0` | `0.0` | yes |
| `exposure_max_bright` | Float | `12` | `12.0` | yes |
| `exposure_min_bright` | Float | `10` | `10.0` | yes |
| `exposure_mode` | String | `histogram` | `histogram`, `manual` | yes |
| `exposure_speed_down` | Float | `1` | `1.0` | yes |
| `exposure_speed_up` | Float | `3` | `3.0` | yes |
| `face_size` | Int | `0` | `0` | yes |
| `focal_distance` | Float | `1000` | `1000.0` | yes |
| `focal_length` | Float | `0` | `0.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fov_fade_size` | Float | `0` | `0.0` | yes |
| `fov_mask` | Bool | `false` | `false` | yes |
| `fstop` | Float | `1.4` | `1.4` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `gamma` | Float | `2.2` | `2.2` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `iso` | Float | `100` | `100.0` | yes |
| `k0` | Float | `0.083092` | `0.083092` | yes |
| `k1` | Float | `0.011121` | `0.011121` | yes |
| `k2` | Float | `0.008587` | `0.008587` | yes |
| `k3` | Float | `0.000854` | `0.000854` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_flare_intensity` | Float | `0.1` | `0.1` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `longitude_offset` | Float | `0` | `0.0` | yes |
| `lut` | String | `""` | `` | yes |
| `min_fstop` | Float | `1.2` | `1.2` | yes |
| `motion_blur_intensity` | Float | `0.45` | `0.45` | yes |
| `motion_blur_max_distortion` | Float | `0.35` | `0.35` | yes |
| `motion_blur_min_object_screen_size` | Float | `0.1` | `0.1` | yes |
| `perspective` | Bool | `false` | `false` | yes |
| `post_process_profile` | String | `""` | `` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.rgb_fisheye` | `sensor.camera.rgb_fisheye` | yes |
| `ros_topic_name` | String | `sensor.camera.rgb_fisheye` | `sensor.camera.rgb_fisheye` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `shoulder` | Float | `0.26` | `0.26` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `shutter_speed` | Float | `200` | `200.0` | yes |
| `slope` | Float | `0.88` | `0.88` | yes |
| `temp` | Float | `6500` | `6500.0` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |
| `tint` | Float | `0` | `0.0` | yes |
| `toe` | Float | `0.55` | `0.55` | yes |
| `white_clip` | Float | `0.04` | `0.04` | yes |

### sensor.camera.rt_lens

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `aperture_fstop` | Float | `0` | `0.0` | yes |
| `ca_shift_b` | Float | `1` | `1.0` | yes |
| `ca_shift_r` | Float | `1` | `1.0` | yes |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `brown_conrady`, `lut`, `ftheta` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `distortion_coeffs` | String | `""` | `` | yes |
| `enable_denoiser` | Bool | `true` | `true` | yes |
| `enable_postprocess_effects` | Bool | `true` | `true` | yes |
| `exposure_compensation` | Float | `0` | `0.0` | yes |
| `exposure_mode` | String | `auto` | `auto`, `manual` | yes |
| `focus_distance_m` | Float | `1000` | `1000.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lut` | String | `""` | `` | yes |
| `post_process_profile` | String | `""` | `` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.rt_lens` | `sensor.camera.rt_lens` | yes |
| `ros_topic_name` | String | `sensor.camera.rt_lens` | `sensor.camera.rt_lens` | yes |
| `samples_per_pixel` | Int | `16` | `16` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.rt_lens_distance

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `aperture_fstop` | Float | `0` | `0.0` | yes |
| `ca_shift_b` | Float | `1` | `1.0` | yes |
| `ca_shift_r` | Float | `1` | `1.0` | yes |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `brown_conrady`, `lut`, `ftheta` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `distortion_coeffs` | String | `""` | `` | yes |
| `enable_denoiser` | Bool | `false` | `false` | yes |
| `exposure_compensation` | Float | `0` | `0.0` | yes |
| `exposure_mode` | String | `auto` | `auto`, `manual` | yes |
| `focus_distance_m` | Float | `1000` | `1000.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lut` | String | `""` | `` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.rt_lens_distance` | `sensor.camera.rt_lens_distance` | yes |
| `ros_topic_name` | String | `sensor.camera.rt_lens_distance` | `sensor.camera.rt_lens_distance` | yes |
| `samples_per_pixel` | Int | `1` | `1` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.rt_lens_instance

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `aperture_fstop` | Float | `0` | `0.0` | yes |
| `ca_shift_b` | Float | `1` | `1.0` | yes |
| `ca_shift_r` | Float | `1` | `1.0` | yes |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `brown_conrady`, `lut`, `ftheta` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `distortion_coeffs` | String | `""` | `` | yes |
| `enable_denoiser` | Bool | `false` | `false` | yes |
| `exposure_compensation` | Float | `0` | `0.0` | yes |
| `exposure_mode` | String | `auto` | `auto`, `manual` | yes |
| `focus_distance_m` | Float | `1000` | `1000.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lut` | String | `""` | `` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.rt_lens_instance` | `sensor.camera.rt_lens_instance` | yes |
| `ros_topic_name` | String | `sensor.camera.rt_lens_instance` | `sensor.camera.rt_lens_instance` | yes |
| `samples_per_pixel` | Int | `1` | `1` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.semantic_segmentation

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `dlss_screen_percentage` | Float | `50` | `50.0` | yes |
| `enable_dlss` | Bool | `false` | `false` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.semantic_segmentation` | `sensor.camera.semantic_segmentation` | yes |
| `ros_topic_name` | String | `sensor.camera.semantic_segmentation` | `sensor.camera.semantic_segmentation` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `use_ray_tracing` | Bool | `true` | `true` | yes |

### sensor.camera.semantic_segmentation_fisheye

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `camera_model` | String | `perspective` | `perspective`, `stereographic`, `equidistant`, `equisolid`, `orthographic`, `kannala_brandt`, `lut`, `ftheta` | yes |
| `cx` | Float | `0.5` | `0.5` | yes |
| `cy` | Float | `0.5` | `0.5` | yes |
| `equirectangular` | Bool | `false` | `false` | yes |
| `face_size` | Int | `0` | `0` | yes |
| `focal_length` | Float | `0` | `0.0` | yes |
| `fov` | Float | `90` | `90.0` | yes |
| `fov_fade_size` | Float | `0` | `0.0` | yes |
| `fov_mask` | Bool | `false` | `false` | yes |
| `fx` | Float | `0` | `0.0` | yes |
| `fy` | Float | `0` | `0.0` | yes |
| `image_size_x` | Int | `800` | `800` | yes |
| `image_size_y` | Int | `600` | `600` | yes |
| `k0` | Float | `0.083092` | `0.083092` | yes |
| `k1` | Float | `0.011121` | `0.011121` | yes |
| `k2` | Float | `0.008587` | `0.008587` | yes |
| `k3` | Float | `0.000854` | `0.000854` | yes |
| `lens_circle_falloff` | Float | `5` | `5.0` | yes |
| `lens_circle_multiplier` | Float | `0` | `0.0` | yes |
| `lens_k` | Float | `-1` | `-1.0` | yes |
| `lens_kcube` | Float | `0` | `0.0` | yes |
| `lens_x_size` | Float | `0.08` | `0.08` | yes |
| `lens_y_size` | Float | `0.08` | `0.08` | yes |
| `longitude_offset` | Float | `0` | `0.0` | yes |
| `lut` | String | `""` | `` | yes |
| `perspective` | Bool | `false` | `false` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.camera.semantic_segmentation_fisheye` | `sensor.camera.semantic_segmentation_fisheye` | yes |
| `ros_topic_name` | String | `sensor.camera.semantic_segmentation_fisheye` | `sensor.camera.semantic_segmentation_fisheye` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `shadow_catcher_tags` | String | `""` | `` | yes |
| `show_only_actor_ids` | String | `""` | `` | yes |
| `show_only_tags` | String | `""` | `` | yes |
| `theta_max_deg` | Float | `0` | `0.0` | yes |

### sensor.lidar.hss_lidar

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `atmosphere_attenuation_rate` | Float | `0.004` | `0.004` | yes |
| `channels` | Int | `128` | `128` | yes |
| `dropoff_general_rate` | Float | `0.45` | `0.45` | yes |
| `dropoff_intensity_limit` | Float | `0.8` | `0.8` | yes |
| `dropoff_zero_intensity` | Float | `0.4` | `0.4` | yes |
| `horizontal_fov` | Float | `120` | `120.0` | yes |
| `horizontal_resolution` | Float | `0.1` | `0.1` | yes |
| `lower_fov` | Float | `-12.5` | `-12.5` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `noise_stddev` | Float | `0` | `0.0` | yes |
| `range` | Float | `200` | `200` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.lidar.hss_lidar` | `sensor.lidar.hss_lidar` | yes |
| `ros_topic_name` | String | `sensor.lidar.hss_lidar` | `sensor.lidar.hss_lidar` | yes |
| `rotation_frequency` | Float | `20` | `20` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `upper_fov` | Float | `12.9` | `12.9` | yes |

### sensor.lidar.ray_cast

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `atmosphere_attenuation_rate` | Float | `0.004` | `0.004` | yes |
| `channels` | Int | `64` | `64` | yes |
| `dropoff_general_rate` | Float | `0.45` | `0.45` | yes |
| `dropoff_intensity_limit` | Float | `0.8` | `0.8` | yes |
| `dropoff_zero_intensity` | Float | `0.4` | `0.4` | yes |
| `horizontal_fov` | Float | `360` | `360.0` | yes |
| `lower_fov` | Float | `-30` | `-30.0` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `noise_stddev` | Float | `0` | `0.0` | yes |
| `points_per_second` | Int | `600000` | `600000` | yes |
| `range` | Float | `50` | `50.0` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.lidar.ray_cast` | `sensor.lidar.ray_cast` | yes |
| `ros_topic_name` | String | `sensor.lidar.ray_cast` | `sensor.lidar.ray_cast` | yes |
| `rotation_frequency` | Float | `60` | `60.0` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `upper_fov` | Float | `10` | `10.0` | yes |

### sensor.lidar.ray_cast_semantic

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `channels` | Int | `64` | `64` | yes |
| `horizontal_fov` | Float | `360` | `360.0` | yes |
| `lower_fov` | Float | `-30` | `-30.0` | yes |
| `points_per_second` | Int | `600000` | `600000` | yes |
| `range` | Float | `50` | `50.0` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.lidar.ray_cast_semantic` | `sensor.lidar.ray_cast_semantic` | yes |
| `ros_topic_name` | String | `sensor.lidar.ray_cast_semantic` | `sensor.lidar.ray_cast_semantic` | yes |
| `rotation_frequency` | Float | `60` | `60.0` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `upper_fov` | Float | `10` | `10.0` | yes |

### sensor.other.autoware_gnss

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `noise_alt_bias` | Float | `0` | `0.0` | yes |
| `noise_alt_stddev` | Float | `0` | `0.0` | yes |
| `noise_lat_bias` | Float | `0` | `0.0` | yes |
| `noise_lat_stddev` | Float | `0` | `0.0` | yes |
| `noise_lon_bias` | Float | `0` | `0.0` | yes |
| `noise_lon_stddev` | Float | `0` | `0.0` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.autoware_gnss` | `sensor.other.autoware_gnss` | yes |
| `ros_topic_name` | String | `sensor.other.autoware_gnss` | `sensor.other.autoware_gnss` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |

### sensor.other.collision

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.other.collision` | `sensor.other.collision` | yes |
| `ros_topic_name` | String | `sensor.other.collision` | `sensor.other.collision` | yes |

### sensor.other.gnss

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `noise_alt_bias` | Float | `0` | `0.0` | yes |
| `noise_alt_stddev` | Float | `0` | `0.0` | yes |
| `noise_lat_bias` | Float | `0` | `0.0` | yes |
| `noise_lat_stddev` | Float | `0` | `0.0` | yes |
| `noise_lon_bias` | Float | `0` | `0.0` | yes |
| `noise_lon_stddev` | Float | `0` | `0.0` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.gnss` | `sensor.other.gnss` | yes |
| `ros_topic_name` | String | `sensor.other.gnss` | `sensor.other.gnss` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |

### sensor.other.imu

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `noise_accel_stddev_x` | Float | `0` | `0.0` | yes |
| `noise_accel_stddev_y` | Float | `0` | `0.0` | yes |
| `noise_accel_stddev_z` | Float | `0` | `0.0` | yes |
| `noise_gyro_bias_x` | Float | `0` | `0.0` | yes |
| `noise_gyro_bias_y` | Float | `0` | `0.0` | yes |
| `noise_gyro_bias_z` | Float | `0` | `0.0` | yes |
| `noise_gyro_stddev_x` | Float | `0` | `0.0` | yes |
| `noise_gyro_stddev_y` | Float | `0` | `0.0` | yes |
| `noise_gyro_stddev_z` | Float | `0` | `0.0` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.imu` | `sensor.other.imu` | yes |
| `ros_topic_name` | String | `sensor.other.imu` | `sensor.other.imu` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |

### sensor.other.lane_invasion

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.other.lane_invasion` | `sensor.other.lane_invasion` | yes |
| `ros_topic_name` | String | `sensor.other.lane_invasion` | `sensor.other.lane_invasion` | yes |

### sensor.other.obstacle

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `debug_linetrace` | Bool | `false` | `false` | yes |
| `distance` | Float | `5` | `5.0` | yes |
| `hit_radius` | Float | `0.5` | `0.5` | yes |
| `only_dynamics` | Bool | `false` | `false` | yes |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.other.obstacle` | `sensor.other.obstacle` | yes |
| `ros_topic_name` | String | `sensor.other.obstacle` | `sensor.other.obstacle` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |

### sensor.other.radar

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `horizontal_fov` | Float | `30` | `30` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `points_per_second` | Int | `1500` | `1500` | yes |
| `range` | Float | `100` | `100` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.radar` | `sensor.other.radar` | yes |
| `ros_topic_name` | String | `sensor.other.radar` | `sensor.other.radar` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `vertical_fov` | Float | `30` | `30` | yes |

### sensor.other.rss

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `role_name` | String | `front` | `front`, `back`, `left`, `right`, `front_left`, `front_right`, `back_left`, `back_right` | yes |
| `ros_name` | String | `sensor.other.rss` | `sensor.other.rss` | yes |
| `ros_topic_name` | String | `sensor.other.rss` | `sensor.other.rss` | yes |

### sensor.other.v2x

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `channel_id` | String | `Default` | `Default` | yes |
| `combined_antenna_gain` | Float | `10` | `10.0` | yes |
| `custom_fading_stddev` | Float | `0` | `0.0` | yes |
| `d_ref` | Float | `1` | `1.0` | yes |
| `filter_distance` | Float | `500` | `500.0` | yes |
| `fixed_rate` | Bool | `false` | `false` | yes |
| `frequency_ghz` | Float | `5.9` | `5.9` | yes |
| `gen_cam_max` | Float | `1` | `1.0` | yes |
| `gen_cam_min` | Float | `0.1` | `0.1` | yes |
| `noise_accel_stddev_x` | Float | `0` | `0.0` | yes |
| `noise_accel_stddev_y` | Float | `0` | `0.0` | yes |
| `noise_accel_stddev_z` | Float | `0` | `0.0` | yes |
| `noise_alt_bias` | Float | `0` | `0.0` | yes |
| `noise_alt_stddev` | Float | `0` | `0.0` | yes |
| `noise_head_bias` | Float | `0` | `0.0` | yes |
| `noise_head_stddev` | Float | `0` | `0.0` | yes |
| `noise_lat_bias` | Float | `0` | `0.0` | yes |
| `noise_lat_stddev` | Float | `0` | `0.0` | yes |
| `noise_lon_bias` | Float | `0` | `0.0` | yes |
| `noise_lon_stddev` | Float | `0` | `0.0` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `noise_vel_stddev_x` | Float | `0` | `0.0` | yes |
| `noise_yawrate_bias` | Float | `0` | `0.0` | yes |
| `noise_yawrate_stddev` | Float | `0` | `0.0` | yes |
| `path_loss_exponent` | Float | `2.7` | `2.7` | yes |
| `path_loss_model` | String | `winner` | `winner`, `geometric` | yes |
| `receiver_sensitivity` | Float | `-99` | `-99.0` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.v2x` | `sensor.other.v2x` | yes |
| `ros_topic_name` | String | `sensor.other.v2x` | `sensor.other.v2x` | yes |
| `scenario` | String | `highway` | `highway`, `rural`, `urban` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `transmit_power` | Float | `21.5` | `21.5` | yes |
| `use_etsi_fading` | Bool | `true` | `true` | yes |

### sensor.other.v2x_custom

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `channel_id` | String | `Default` | `Default` | yes |
| `combined_antenna_gain` | Float | `10` | `10.0` | yes |
| `custom_fading_stddev` | Float | `0` | `0.0` | yes |
| `d_ref` | Float | `1` | `1.0` | yes |
| `filter_distance` | Float | `500` | `500.0` | yes |
| `frequency_ghz` | Float | `5.9` | `5.9` | yes |
| `noise_seed` | Int | `0` | `0` | yes |
| `path_loss_exponent` | Float | `2.7` | `2.7` | yes |
| `path_loss_model` | String | `winner` | `winner`, `geometric` | yes |
| `receiver_sensitivity` | Float | `-99` | `-99.0` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.v2x_custom` | `sensor.other.v2x_custom` | yes |
| `ros_topic_name` | String | `sensor.other.v2x_custom` | `sensor.other.v2x_custom` | yes |
| `scenario` | String | `highway` | `highway`, `rural`, `urban` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `transmit_power` | Float | `21.5` | `21.5` | yes |
| `use_etsi_fading` | Bool | `true` | `true` | yes |

### sensor.other.vehicle_status

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `sensor.other.vehicle_status` | `sensor.other.vehicle_status` | yes |
| `ros_topic_name` | String | `sensor.other.vehicle_status` | `sensor.other.vehicle_status` | yes |
| `sensor_tick` | Float | `0` | `0.0` | yes |
| `speed_units` | String | `mps` |  | no |

---

## Props

Static meshes that can be spawned as actors. See the [prop catalogue](catalogue_props.md).

### static.prop.advertisement

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/Materials/Billboard/SM_Billboard2.SM_Billboard2` | `/Game/Carla/Static/Static/00_LegacyAssets/Materials/Billboard/SM_Billboard2.SM_Billboard2` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.advertisement` | `static.prop.advertisement` | yes |
| `ros_topic_name` | String | `static.prop.advertisement` | `static.prop.advertisement` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `signs` |  | no |

### static.prop.atm

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_Atm.SM_Atm` | `/Game/Carla/Static/Static/SM_Atm.SM_Atm` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.atm` | `static.prop.atm` | yes |
| `ros_topic_name` | String | `static.prop.atm` | `static.prop.atm` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `utilities` |  | no |

### static.prop.barbeque

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Barbecue.SM_Barbecue` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Barbecue.SM_Barbecue` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.barbeque` | `static.prop.barbeque` | yes |
| `ros_topic_name` | String | `static.prop.barbeque` | `static.prop.barbeque` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `other` |  | no |

### static.prop.barrel

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Barrel.SM_Barrel` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Barrel.SM_Barrel` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.barrel` | `static.prop.barrel` | yes |
| `ros_topic_name` | String | `static.prop.barrel` | `static.prop.barrel` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `barrels` |  | no |

### static.prop.bench01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_Bench01.SM_Bench01` | `/Game/Carla/Static/Static/SM_Bench01.SM_Bench01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.bench01` | `static.prop.bench01` | yes |
| `ros_topic_name` | String | `static.prop.bench01` | `static.prop.bench01` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `benches` |  | no |

### static.prop.bench02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_Bench02.SM_Bench02` | `/Game/Carla/Static/Static/SM_Bench02.SM_Bench02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.bench02` | `static.prop.bench02` | yes |
| `ros_topic_name` | String | `static.prop.bench02` | `static.prop.bench02` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `benches` |  | no |

### static.prop.bikehelmet

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_bikehelmet.SM_bikehelmet` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_bikehelmet.SM_bikehelmet` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.bikehelmet` | `static.prop.bikehelmet` | yes |
| `ros_topic_name` | String | `static.prop.bikehelmet` | `static.prop.bikehelmet` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `clothing` |  | no |

### static.prop.briefcase

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_Briefcase.SM_Briefcase` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_Briefcase.SM_Briefcase` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.briefcase` | `static.prop.briefcase` | yes |
| `ros_topic_name` | String | `static.prop.briefcase` | `static.prop.briefcase` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `cases` |  | no |

### static.prop.brokentile01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile01.SM_BrokenTile01` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile01.SM_BrokenTile01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.brokentile01` | `static.prop.brokentile01` | yes |
| `ros_topic_name` | String | `static.prop.brokentile01` | `static.prop.brokentile01` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.brokentile02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile02.SM_BrokenTile02` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile02.SM_BrokenTile02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.brokentile02` | `static.prop.brokentile02` | yes |
| `ros_topic_name` | String | `static.prop.brokentile02` | `static.prop.brokentile02` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.brokentile03

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile03.SM_BrokenTile03` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile03.SM_BrokenTile03` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.brokentile03` | `static.prop.brokentile03` | yes |
| `ros_topic_name` | String | `static.prop.brokentile03` | `static.prop.brokentile03` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.brokentile04

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile04.SM_BrokenTile04` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_BrokenTile04.SM_BrokenTile04` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.brokentile04` | `static.prop.brokentile04` | yes |
| `ros_topic_name` | String | `static.prop.brokentile04` | `static.prop.brokentile04` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.busstop

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/Materials/BusStop/SM_BusStop.SM_BusStop` | `/Game/Carla/Static/Static/Materials/BusStop/SM_BusStop.SM_BusStop` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.busstop` | `static.prop.busstop` | yes |
| `ros_topic_name` | String | `static.prop.busstop` | `static.prop.busstop` | yes |
| `size` | String | `big` |  | no |
| `subtype` | String | `transport` |  | no |

### static.prop.calibrator

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `utility` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/Materials/Calibrator/SM_calibration.SM_calibration` | `/Game/Carla/Static/Static/Materials/Calibrator/SM_calibration.SM_calibration` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.calibrator` | `static.prop.calibrator` | yes |
| `ros_topic_name` | String | `static.prop.calibrator` | `static.prop.calibrator` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `calibration` |  | no |

### static.prop.chainbarrier

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_ChainBarrier.SM_ChainBarrier` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_ChainBarrier.SM_ChainBarrier` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.chainbarrier` | `static.prop.chainbarrier` | yes |
| `ros_topic_name` | String | `static.prop.chainbarrier` | `static.prop.chainbarrier` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `barriers` |  | no |

### static.prop.chainbarrierend

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_ChainBarrierEnd.SM_ChainBarrierEnd` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_ChainBarrierEnd.SM_ChainBarrierEnd` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.chainbarrierend` | `static.prop.chainbarrierend` | yes |
| `ros_topic_name` | String | `static.prop.chainbarrierend` | `static.prop.chainbarrierend` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `barriers` |  | no |

### static.prop.colacan

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_ColaCan.SM_ColaCan` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_ColaCan.SM_ColaCan` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.colacan` | `static.prop.colacan` | yes |
| `ros_topic_name` | String | `static.prop.colacan` | `static.prop.colacan` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.constructioncone

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/Construction/SM_ConstructionCone.SM_ConstructionCone` | `/Game/Carla/Static/Dynamic/Construction/SM_ConstructionCone.SM_ConstructionCone` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.constructioncone` | `static.prop.constructioncone` | yes |
| `ros_topic_name` | String | `static.prop.constructioncone` | `static.prop.constructioncone` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `cones` |  | no |

### static.prop.container

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/Construction/SM_DebrisContainer.SM_DebrisContainer` | `/Game/Carla/Static/Dynamic/Construction/SM_DebrisContainer.SM_DebrisContainer` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.container` | `static.prop.container` | yes |
| `ros_topic_name` | String | `static.prop.container` | `static.prop.container` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.creasedbox01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_CreasedBox01.SM_CreasedBox01` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_CreasedBox01.SM_CreasedBox01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.creasedbox01` | `static.prop.creasedbox01` | yes |
| `ros_topic_name` | String | `static.prop.creasedbox01` | `static.prop.creasedbox01` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `boxes` |  | no |

### static.prop.dirtdebris01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Other/SM_DirtDebris01.SM_DirtDebris01` | `/Game/Carla/Static/Other/SM_DirtDebris01.SM_DirtDebris01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.dirtdebris01` | `static.prop.dirtdebris01` | yes |
| `ros_topic_name` | String | `static.prop.dirtdebris01` | `static.prop.dirtdebris01` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.dirtdebris02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Other/SM_DirtDebris02.SM_DirtDebris02` | `/Game/Carla/Static/Other/SM_DirtDebris02.SM_DirtDebris02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.dirtdebris02` | `static.prop.dirtdebris02` | yes |
| `ros_topic_name` | String | `static.prop.dirtdebris02` | `static.prop.dirtdebris02` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.dirtdebris03

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Other/SM_DirtDebris03.SM_DirtDebris03` | `/Game/Carla/Static/Other/SM_DirtDebris03.SM_DirtDebris03` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.dirtdebris03` | `static.prop.dirtdebris03` | yes |
| `ros_topic_name` | String | `static.prop.dirtdebris03` | `static.prop.dirtdebris03` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `debris` |  | no |

### static.prop.doghouse

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_DogHouse.SM_DogHouse` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_DogHouse.SM_DogHouse` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.doghouse` | `static.prop.doghouse` | yes |
| `ros_topic_name` | String | `static.prop.doghouse` | `static.prop.doghouse` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `other` |  | no |

### static.prop.dumpster

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Dumpster.SM_Dumpster` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Dumpster.SM_Dumpster` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.dumpster` | `static.prop.dumpster` | yes |
| `ros_topic_name` | String | `static.prop.dumpster` | `static.prop.dumpster` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.dumpster02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Dumpster02.SM_Dumpster02` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Dumpster02.SM_Dumpster02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.dumpster02` | `static.prop.dumpster02` | yes |
| `ros_topic_name` | String | `static.prop.dumpster02` | `static.prop.dumpster02` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.fountain

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_Fountain.SM_Fountain` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_Fountain.SM_Fountain` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.fountain` | `static.prop.fountain` | yes |
| `ros_topic_name` | String | `static.prop.fountain` | `static.prop.fountain` | yes |
| `size` | String | `big` |  | no |
| `subtype` | String | `monuments` |  | no |

### static.prop.garbage01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage01.SM_Garbage01` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage01.SM_Garbage01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.garbage01` | `static.prop.garbage01` | yes |
| `ros_topic_name` | String | `static.prop.garbage01` | `static.prop.garbage01` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.garbage02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage02.SM_Garbage02` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage02.SM_Garbage02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.garbage02` | `static.prop.garbage02` | yes |
| `ros_topic_name` | String | `static.prop.garbage02` | `static.prop.garbage02` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.garbage03

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage03.SM_Garbage03` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage03.SM_Garbage03` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.garbage03` | `static.prop.garbage03` | yes |
| `ros_topic_name` | String | `static.prop.garbage03` | `static.prop.garbage03` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.garbage04

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage04.SM_Garbage04` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage04.SM_Garbage04` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.garbage04` | `static.prop.garbage04` | yes |
| `ros_topic_name` | String | `static.prop.garbage04` | `static.prop.garbage04` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.garbage05

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage05.SM_Garbage05` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage05.SM_Garbage05` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.garbage05` | `static.prop.garbage05` | yes |
| `ros_topic_name` | String | `static.prop.garbage05` | `static.prop.garbage05` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.garbage06

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage06.SM_Garbage06` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/Wastes/SM_Garbage06.SM_Garbage06` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.garbage06` | `static.prop.garbage06` | yes |
| `ros_topic_name` | String | `static.prop.garbage06` | `static.prop.garbage06` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `garbage` |  | no |

### static.prop.gnome

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Gnome.SM_Gnome` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Gnome.SM_Gnome` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.gnome` | `static.prop.gnome` | yes |
| `ros_topic_name` | String | `static.prop.gnome` | `static.prop.gnome` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `other` |  | no |

### static.prop.guitarcase

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_GuitarCase.SM_GuitarCase` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_GuitarCase.SM_GuitarCase` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.guitarcase` | `static.prop.guitarcase` | yes |
| `ros_topic_name` | String | `static.prop.guitarcase` | `static.prop.guitarcase` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `cases` |  | no |

### static.prop.haybale

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/Farm/SM_hayBale.SM_hayBale` | `/Game/Carla/Static/Dynamic/Farm/SM_hayBale.SM_hayBale` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.haybale` | `static.prop.haybale` | yes |
| `ros_topic_name` | String | `static.prop.haybale` | `static.prop.haybale` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `other` |  | no |

### static.prop.kiosk_01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Building/SpecialBuildings/00_LegacyAssets/Kiosk/SM_Kiosk01.SM_Kiosk01` | `/Game/Carla/Static/Building/SpecialBuildings/00_LegacyAssets/Kiosk/SM_Kiosk01.SM_Kiosk01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.kiosk_01` | `static.prop.kiosk_01` | yes |
| `ros_topic_name` | String | `static.prop.kiosk_01` | `static.prop.kiosk_01` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `commercial` |  | no |

### static.prop.mailbox

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_MailBox.SM_MailBox` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_MailBox.SM_MailBox` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.mailbox` | `static.prop.mailbox` | yes |
| `ros_topic_name` | String | `static.prop.mailbox` | `static.prop.mailbox` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `utilities` |  | no |

### static.prop.maptable

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_MapTable.SM_MapTable` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_MapTable.SM_MapTable` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.maptable` | `static.prop.maptable` | yes |
| `ros_topic_name` | String | `static.prop.maptable` | `static.prop.maptable` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `monuments` |  | no |

### static.prop.mesh

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `mass` | Float | `0` | `` | yes |
| `mesh_path` | String | `""` | `` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `static.prop.mesh` | `static.prop.mesh` | yes |
| `ros_topic_name` | String | `static.prop.mesh` | `static.prop.mesh` | yes |
| `scale` | Float | `1` | `1.0f` | yes |

### static.prop.mobile

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_Mobile.SM_Mobile` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_Mobile.SM_Mobile` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.mobile` | `static.prop.mobile` | yes |
| `ros_topic_name` | String | `static.prop.mobile` | `static.prop.mobile` | yes |
| `size` | String | `tiny` |  | no |
| `subtype` | String | `personal_effects` |  | no |

### static.prop.motorhelmet

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/Sm_Motorhelmet.Sm_Motorhelmet` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/Sm_Motorhelmet.Sm_Motorhelmet` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.motorhelmet` | `static.prop.motorhelmet` | yes |
| `ros_topic_name` | String | `static.prop.motorhelmet` | `static.prop.motorhelmet` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `clothing` |  | no |

### static.prop.pergola

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Pergola.SM_Pergola` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Pergola.SM_Pergola` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.pergola` | `static.prop.pergola` | yes |
| `ros_topic_name` | String | `static.prop.pergola` | `static.prop.pergola` | yes |
| `size` | String | `big` |  | no |
| `subtype` | String | `furniture` |  | no |

### static.prop.plantpot01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_Plantpot01.SM_Plantpot01` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_Plantpot01.SM_Plantpot01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot01` | `static.prop.plantpot01` | yes |
| `ros_topic_name` | String | `static.prop.plantpot01` | `static.prop.plantpot01` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plantpot02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_PlantPot02.SM_PlantPot02` | `/Game/Carla/Static/Static/SM_PlantPot02.SM_PlantPot02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot02` | `static.prop.plantpot02` | yes |
| `ros_topic_name` | String | `static.prop.plantpot02` | `static.prop.plantpot02` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plantpot03

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_PlantPot03.SM_PlantPot03` | `/Game/Carla/Static/Static/SM_PlantPot03.SM_PlantPot03` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot03` | `static.prop.plantpot03` | yes |
| `ros_topic_name` | String | `static.prop.plantpot03` | `static.prop.plantpot03` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plantpot04

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_PlantPot04.SM_PlantPot04` | `/Game/Carla/Static/Static/SM_PlantPot04.SM_PlantPot04` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot04` | `static.prop.plantpot04` | yes |
| `ros_topic_name` | String | `static.prop.plantpot04` | `static.prop.plantpot04` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plantpot05

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlantPot05.SM_PlantPot05` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlantPot05.SM_PlantPot05` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot05` | `static.prop.plantpot05` | yes |
| `ros_topic_name` | String | `static.prop.plantpot05` | `static.prop.plantpot05` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plantpot06

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlantPot06.SM_PlantPot06` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlantPot06.SM_PlantPot06` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot06` | `static.prop.plantpot06` | yes |
| `ros_topic_name` | String | `static.prop.plantpot06` | `static.prop.plantpot06` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plantpot07

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlantPot07.SM_PlantPot07` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlantPot07.SM_PlantPot07` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plantpot07` | `static.prop.plantpot07` | yes |
| `ros_topic_name` | String | `static.prop.plantpot07` | `static.prop.plantpot07` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `plantpots` |  | no |

### static.prop.plasticbag

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_PlasticBag.SM_PlasticBag` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_PlasticBag.SM_PlasticBag` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plasticbag` | `static.prop.plasticbag` | yes |
| `ros_topic_name` | String | `static.prop.plasticbag` | `static.prop.plasticbag` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `bags` |  | no |

### static.prop.plasticchair

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_PlasticChair.SM_PlasticChair` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_PlasticChair.SM_PlasticChair` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plasticchair` | `static.prop.plasticchair` | yes |
| `ros_topic_name` | String | `static.prop.plasticchair` | `static.prop.plasticchair` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `furniture` |  | no |

### static.prop.plastictable

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_PlasticTable.SM_PlasticTable` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_PlasticTable.SM_PlasticTable` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.plastictable` | `static.prop.plastictable` | yes |
| `ros_topic_name` | String | `static.prop.plastictable` | `static.prop.plastictable` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `furniture` |  | no |

### static.prop.platformgarbage01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlatformGarbage01.SM_PlatformGarbage01` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_PlatformGarbage01.SM_PlatformGarbage01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.platformgarbage01` | `static.prop.platformgarbage01` | yes |
| `ros_topic_name` | String | `static.prop.platformgarbage01` | `static.prop.platformgarbage01` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `platforms` |  | no |

### static.prop.purse

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_Purse.SM_Purse` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_Purse.SM_Purse` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.purse` | `static.prop.purse` | yes |
| `ros_topic_name` | String | `static.prop.purse` | `static.prop.purse` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `personal_effects` |  | no |

### static.prop.recyclecardboard

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Recycle_Cardboard.SM_Recycle_Cardboard` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Recycle_Cardboard.SM_Recycle_Cardboard` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.recyclecardboard` | `static.prop.recyclecardboard` | yes |
| `ros_topic_name` | String | `static.prop.recyclecardboard` | `static.prop.recyclecardboard` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.recycleglass

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_RecycleGlass.SM_RecycleGlass` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_RecycleGlass.SM_RecycleGlass` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.recycleglass` | `static.prop.recycleglass` | yes |
| `ros_topic_name` | String | `static.prop.recycleglass` | `static.prop.recycleglass` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.recycleorganic

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Recycle_Organic.SM_Recycle_Organic` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Recycle_Organic.SM_Recycle_Organic` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.recycleorganic` | `static.prop.recycleorganic` | yes |
| `ros_topic_name` | String | `static.prop.recycleorganic` | `static.prop.recycleorganic` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.recycleplastic

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `containers` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Recycle_Plastic.SM_Recycle_Plastic` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_Recycle_Plastic.SM_Recycle_Plastic` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.recycleplastic` | `static.prop.recycleplastic` | yes |
| `ros_topic_name` | String | `static.prop.recycleplastic` | `static.prop.recycleplastic` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `bins` |  | no |

### static.prop.shoppingbag

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_ShoppingBag.SM_ShoppingBag` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_ShoppingBag.SM_ShoppingBag` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.shoppingbag` | `static.prop.shoppingbag` | yes |
| `ros_topic_name` | String | `static.prop.shoppingbag` | `static.prop.shoppingbag` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `bags` |  | no |

### static.prop.shoppingcart

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_ShoppinCart_Opt.SM_ShoppinCart_Opt` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_ShoppinCart_Opt.SM_ShoppinCart_Opt` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.shoppingcart` | `static.prop.shoppingcart` | yes |
| `ros_topic_name` | String | `static.prop.shoppingcart` | `static.prop.shoppingcart` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `carts` |  | no |

### static.prop.shoppingtrolley

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_ShoppingTrolley.SM_ShoppingTrolley` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_ShoppingTrolley.SM_ShoppingTrolley` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.shoppingtrolley` | `static.prop.shoppingtrolley` | yes |
| `ros_topic_name` | String | `static.prop.shoppingtrolley` | `static.prop.shoppingtrolley` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `carts` |  | no |

### static.prop.streetbarrier

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_StreetBarrier.SM_StreetBarrier` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_StreetBarrier.SM_StreetBarrier` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.streetbarrier` | `static.prop.streetbarrier` | yes |
| `ros_topic_name` | String | `static.prop.streetbarrier` | `static.prop.streetbarrier` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `barriers` |  | no |

### static.prop.streetfountain

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetFountain.SM_StreetFountain` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetFountain.SM_StreetFountain` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.streetfountain` | `static.prop.streetfountain` | yes |
| `ros_topic_name` | String | `static.prop.streetfountain` | `static.prop.streetfountain` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `utilities` |  | no |

### static.prop.streetsign

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetAD02.SM_StreetAD02` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetAD02.SM_StreetAD02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.streetsign` | `static.prop.streetsign` | yes |
| `ros_topic_name` | String | `static.prop.streetsign` | `static.prop.streetsign` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `signs` |  | no |

### static.prop.streetsign01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetAD01.SM_StreetAD01` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetAD01.SM_StreetAD01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.streetsign01` | `static.prop.streetsign01` | yes |
| `ros_topic_name` | String | `static.prop.streetsign01` | `static.prop.streetsign01` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `signs` |  | no |

### static.prop.streetsign04

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetAD04.SM_StreetAD04` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_StreetAD04.SM_StreetAD04` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.streetsign04` | `static.prop.streetsign04` | yes |
| `ros_topic_name` | String | `static.prop.streetsign04` | `static.prop.streetsign04` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `signs` |  | no |

### static.prop.swing

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_Swing.SM_Swing` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_Swing.SM_Swing` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.swing` | `static.prop.swing` | yes |
| `ros_topic_name` | String | `static.prop.swing` | `static.prop.swing` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `furniture` |  | no |

### static.prop.swingcouch

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_SwingCouch.SM_SwingCouch` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_SwingCouch.SM_SwingCouch` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.swingcouch` | `static.prop.swingcouch` | yes |
| `ros_topic_name` | String | `static.prop.swingcouch` | `static.prop.swingcouch` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `furniture` |  | no |

### static.prop.trafficcone01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_TrafficCones_01.SM_TrafficCones_01` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_TrafficCones_01.SM_TrafficCones_01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trafficcone01` | `static.prop.trafficcone01` | yes |
| `ros_topic_name` | String | `static.prop.trafficcone01` | `static.prop.trafficcone01` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `cones` |  | no |

### static.prop.trafficcone02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_TrafficCones_02.SM_TrafficCones_02` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_TrafficCones_02.SM_TrafficCones_02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trafficcone02` | `static.prop.trafficcone02` | yes |
| `ros_topic_name` | String | `static.prop.trafficcone02` | `static.prop.trafficcone02` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `cones` |  | no |

### static.prop.trafficwarning

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_TrafficCones_03.SM_TrafficCones_03` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_TrafficCones_03.SM_TrafficCones_03` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trafficwarning` | `static.prop.trafficwarning` | yes |
| `ros_topic_name` | String | `static.prop.trafficwarning` | `static.prop.trafficwarning` | yes |
| `size` | String | `big` |  | no |
| `subtype` | String | `signs` |  | no |

### static.prop.trampoline

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `park_garden` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Trampoline.SM_Trampoline` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Garden/SM_Trampoline.SM_Trampoline` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trampoline` | `static.prop.trampoline` | yes |
| `ros_topic_name` | String | `static.prop.trampoline` | `static.prop.trampoline` | yes |
| `size` | String | `big` |  | no |
| `subtype` | String | `furniture` |  | no |

### static.prop.trashbag

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_TrasdhBag.SM_TrasdhBag` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Trash/SM_TrasdhBag.SM_TrasdhBag` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trashbag` | `static.prop.trashbag` | yes |
| `ros_topic_name` | String | `static.prop.trashbag` | `static.prop.trashbag` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `bags` |  | no |

### static.prop.trashcan01

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_TrashCan01.SM_TrashCan01` | `/Game/Carla/Static/Static/SM_TrashCan01.SM_TrashCan01` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trashcan01` | `static.prop.trashcan01` | yes |
| `ros_topic_name` | String | `static.prop.trashcan01` | `static.prop.trashcan01` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `trash_cans` |  | no |

### static.prop.trashcan02

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Other/SM_TrashCan02.SM_TrashCan02` | `/Game/Carla/Static/Other/SM_TrashCan02.SM_TrashCan02` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trashcan02` | `static.prop.trashcan02` | yes |
| `ros_topic_name` | String | `static.prop.trashcan02` | `static.prop.trashcan02` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `trash_cans` |  | no |

### static.prop.trashcan03

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_TrashCan03.SM_TrashCan03` | `/Game/Carla/Static/Static/SM_TrashCan03.SM_TrashCan03` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trashcan03` | `static.prop.trashcan03` | yes |
| `ros_topic_name` | String | `static.prop.trashcan03` | `static.prop.trashcan03` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `trash_cans` |  | no |

### static.prop.trashcan04

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Other/SM_trashCan04.SM_trashCan04` | `/Game/Carla/Static/Other/SM_trashCan04.SM_trashCan04` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trashcan04` | `static.prop.trashcan04` | yes |
| `ros_topic_name` | String | `static.prop.trashcan04` | `static.prop.trashcan04` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `trash_cans` |  | no |

### static.prop.trashcan05

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `garbage` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/SM_Trashcan05.SM_Trashcan05` | `/Game/Carla/Static/Static/SM_Trashcan05.SM_Trashcan05` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.trashcan05` | `static.prop.trashcan05` | yes |
| `ros_topic_name` | String | `static.prop.trashcan05` | `static.prop.trashcan05` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `trash_cans` |  | no |

### static.prop.travelcase

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `personal` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_TravelCase.SM_TravelCase` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/PedestrianProps/SM_TravelCase.SM_TravelCase` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.travelcase` | `static.prop.travelcase` | yes |
| `ros_topic_name` | String | `static.prop.travelcase` | `static.prop.travelcase` | yes |
| `size` | String | `small` |  | no |
| `subtype` | String | `cases` |  | no |

### static.prop.vendingmachine

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `sidewalk` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Static/00_LegacyAssets/SM_CarlaCola.SM_CarlaCola` | `/Game/Carla/Static/Static/00_LegacyAssets/SM_CarlaCola.SM_CarlaCola` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.vendingmachine` | `static.prop.vendingmachine` | yes |
| `ros_topic_name` | String | `static.prop.vendingmachine` | `static.prop.vendingmachine` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `utilities` |  | no |

### static.prop.warningaccident

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_WarningAccident.SM_WarningAccident` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_WarningAccident.SM_WarningAccident` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.warningaccident` | `static.prop.warningaccident` | yes |
| `ros_topic_name` | String | `static.prop.warningaccident` | `static.prop.warningaccident` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `signs` |  | no |

### static.prop.warningconstruction

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `base_type` | String | `road` |  | no |
| `mesh_path` | String | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_WarningConstruction.SM_WarningConstruction` | `/Game/Carla/Static/Dynamic/00_LegacyAssets/Construction/SM_WarningConstruction.SM_WarningConstruction` | yes |
| `role_name` | String | `prop` | `prop` | yes |
| `ros_name` | String | `static.prop.warningconstruction` | `static.prop.warningconstruction` | yes |
| `ros_topic_name` | String | `static.prop.warningconstruction` | `static.prop.warningconstruction` | yes |
| `size` | String | `medium` |  | no |
| `subtype` | String | `construction` |  | no |

### static.trigger.friction

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `extent_x` | Float | `1` | `1.0f` | yes |
| `extent_y` | Float | `1` | `1.0f` | yes |
| `extent_z` | Float | `1` | `1.0f` | yes |
| `friction` | Float | `3.5` | `3.5f` | yes |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `static.trigger.friction` | `static.trigger.friction` | yes |
| `ros_topic_name` | String | `static.trigger.friction` | `static.trigger.friction` | yes |

---

## Util

### util.actor.empty

| Attribute | Type | Default | Values | Modifiable |
| --- | --- | --- | --- | --- |
| `role_name` | String | `default` | `default` | yes |
| `ros_name` | String | `util.actor.empty` | `util.actor.empty` | yes |
| `ros_topic_name` | String | `util.actor.empty` | `util.actor.empty` | yes |

