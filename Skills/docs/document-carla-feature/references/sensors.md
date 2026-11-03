# Format: sensors

A sensor blueprint the server registers gets a section in `Docs/ref_sensors.md`. The [`audit-carla-docs`](../../audit-carla-docs/SKILL.md) report lists the ones that have none (`sensor`), each with the file that registers it.

## 1. Find out what it is, from the code

Three things decide the entry, and none of them can be guessed:

- **The blueprint attributes.** Read `MakeXxxDefinition` in `ActorBlueprintFunctionLibrary.cpp` for the ids, types and defaults. The table shows the default.
- **What `listen()` delivers.** The sensor class names its data type, but the only reliable answer is to spawn it and look. Some sensors deliver a bare `carla.SensorData` with no payload at all.
- **What the payload means.** A camera that encodes data in its pixels needs the encoding spelled out. Measure it rather than infer it from the material or shader name: point the camera at a surface whose answer you know and read a pixel back.

## 2. Write the section

Copy the shape of a neighbouring sensor; the page is consistent, and a new entry with its own layout stands out. The parts, in order:

```markdown
## Name of the sensor

*   __Blueprint:__ sensor.group.id
*   __Output:__ [carla.Type](python_api.md#carla.Type) per step (unless `sensor_tick` says otherwise).

One paragraph on what it measures and how to read it.

#### Name attributes
| Blueprint attribute | Type | Default | Description |

#### Output attributes
| Sensor data attribute | Type | Description |
```

- Add the sensor to the index list at the top of the page in the same pass; it is a separate list.
- Dots in a default are escaped (`0\.0`), and `carla.Transform` in the output table is linked as `[carla.Transform](<../python_api#carlatransform>)`.

## 3. Say what a Python client does not get

A sensor that exists for the native ROS 2 interface still appears in the blueprint library, so somebody will spawn it and wonder why the callback is empty. Say so in an `!!! Important` box, and point at the API that does carry the data.

## 4. Verify

Build the docs and check every link in the new section resolves: a link to a page that does not exist is a new broken link (`Docs/` has no Autoware page, for one). When the target came from the audit, re-run it with `--signal sensor --signal link`.
