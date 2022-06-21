# Generate maps with OpenStreetMap

In this guide you will learn:

- How to export a map from OpenStreetMaps.
- The different formats of map that can be used in CARLA and each format's limitations.
- How to convert the native `.osm` format to `.xodr`.
- How to include traffic light information in the `.xodr` file.
- How to run the final map in a CARLA simulation.

[OpenStreetMap](https://www.openstreetmap.org) is an open data map of the world developed by thousands of contributors and licensed under the [Open Data Commons Open Database License](https://opendatacommons.org/licenses/odbl/). Sections of the map can be exported to an XML formatted `.osm` file. CARLA can convert this file to an OpenDRIVE format and ingest it using the [OpenDRIVE Standalone Mode](#adv_opendrive.md).

- [__Export a map with OpenStreetMap__](#export-a-map-with-openstreetmap)
- [__Using OpenStreetMaps in CARLA__](#using-openstreetmaps-in-carla)
- [__Convert OpenStreetMap format to OpenDRIVE format__](#convert-openstreetmap-format-to-opendrive-format)
    - [Linux](#linux)
    - [Windows](#windows)
    - [Generate Traffic Lights](#generate-traffic-lights)
- [__Ingest into CARLA__](#ingest-into-carla)

---
## Export a map with OpenStreetMap

This section explains how to export your desired map information from Open Street Map:

__1.__ Navigate to the [Open Street Map website](https://www.openstreetmap.org). You will see the map view and a panel on the right side of the window where you can configure different map layers, query different features, toggle the legend, and more.

__2.__ Search for your desired location and zoom in to a specific area.

![openstreetmap_view](img/tuto_g_osm_web.jpg)

!!! Note
    If you would like to use a map of a large area, for example, Paris, you may consider using CARLA's [__Large Map__ feature](large_map_overview.md).

__3.__ Click on _Export_ on the upper left side of the window to open the _Export_ panel.

__4.__ Click on _Manually select a different area_ in the _Export_ panel.

__5.__ Select a custom area by dragging the corners of the square area in the viewport.

__6.__ Click the _Export_ button in the _Export_ panel and save the map information of the selected area as a `.osm` file.

![openstreetmap_area](img/tuto_g_osm_area.jpg)

---
## Using OpenStreetMaps in CARLA

Open Street Map data can be used in CARLA via three different methods. The method you use will depend on if the data is in the original `.osm` format or if you convert the file to `.xodr` using the conversion method explained in the following sections. Keeping the file in `.osm` is the most restrictive method as it does not allow for settings customization.

__Options available for `.xodr` format:__

- Generate the map in your own script. __This method allows parameterization.__
- Pass the file as a parameter to CARLA's `config.py`. __This method does not allow parameterization.__

__Options available for `.osm` format:__

- Pass the file as a parameter to CARLA's `config.py`. __This method does not allow parameterization.__

The following sections will provide more detail on the options listed above.

---

## Convert OpenStreetMap format to OpenDRIVE format

This section demonstrates how to use the Python API to convert the `.osm` file we exported in the previous section to `.xodr` format so that it is ready for use in CARLA.

The [carla.Osm2OdrSettings](python_api.md#carla.Osm2OdrSettings) class is used to configure conversion settings such as offset values, traffic light generation, origin coordinates, and more. The full list of configurable parameters is found in the Python API [documentation](python_api.md#carla.Osm2OdrSettings). The [carla.Osm2Odr](python_api.md#carla.Osm2Odr) class uses these settings to parse the `.osm` data and output it in `.xodr` format.

In Windows, the `.osm` file must be encoded to `UTF-8`. This is not necessary in Linux. Below are example code snippets that show how to perform the file conversion depending on your operating system:

##### Linux

```py
# Read the .osm data
f = open("path/to/osm/file", 'r')
osm_data = f.read()
f.close()

# Define the desired settings. In this case, default values.
settings = carla.Osm2OdrSettings()
# Set OSM road types to export to OpenDRIVE
settings.set_osm_way_types(["motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link", "secondary", "secondary_link", "tertiary", "tertiary_link", "unclassified", "residential"])
# Convert to .xodr
xodr_data = carla.Osm2Odr.convert(osm_data, settings)

# save opendrive file
f = open("path/to/output/file", 'w')
f.write(xodr_data)
f.close()
```

##### Windows

```py
import io

# Read the .osm data
f = io.open("test", mode="r", encoding="utf-8")
osm_data = f.read()
f.close()

# Define the desired settings. In this case, default values.
settings = carla.Osm2OdrSettings()
# Set OSM road types to export to OpenDRIVE
settings.set_osm_way_types(["motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link", "secondary", "secondary_link", "tertiary", "tertiary_link", "unclassified", "residential"])
# Convert to .xodr
xodr_data = carla.Osm2Odr.convert(osm_data, settings)

# save opendrive file
f = open("path/to/output/file", 'w')
f.write(xodr_data)
f.close()
```
<br>

---
### Generate Traffic Lights

Open Street Map data can define which junctions are controlled with traffic lights. To use this traffic light data in CARLA, you need to enable it in the OSM map settings via the Python API before converting the `.osm` file to `.xodr` format:

```py
# Define the desired settings. In this case, default values.
settings = carla.Osm2OdrSettings()
# enable traffic light generation from OSM data
settings.generate_traffic_lights = True
# Convert to .xodr
xodr_data = carla.Osm2Odr.convert(osm_data, settings)
```

Traffic light data quality can vary depending on the region from which you extract data. Some traffic light information may be missing completely. To work within these limitations, you can use the Python API to configure all junctions to be controlled with traffic lights:

```py
settings.all_junctions_with_traffic_lights = True
```

You can also exclude certain roads, e.g., motorway links, from generating traffic lights:

```
settings.set_traffic_light_excluded_way_types(["motorway_link"])
```

---
## Ingest into CARLA

This section explains how to use the different options available to ingest your Open Street Map information into CARLA using the [OpenDRIVE Standalone Mode](adv_opendrive.md).

There are three options available:

[__A)__](#a-use-your-own-script) Generate the map using a converted `.xodr` file in your own custom Python script. __This method allows parameterization.__  
[__B)__](#b-pass-xodr-to-configpy) Pass a converted `.xodr` file as a parameter to the CARLA `config.py` script. __This method does not allow parameterization.__  
[__C)__](#c-pass-osm-to-configpy) Pass the original `.osm` file as a parameter to the CARLA `config.py` script. __This method does not allow parameterization.__  

###### A) Use your own script

Generate the new map and block the simulation until it is ready by calling [`client.generate_opendrive_world()`](python_api.md#carla.Client.generate_opendrive_world). Use the [carla.OpendriveGenerationParameters](python_api.md#carla.OpendriveGenerationParameters) class to configure the mesh generation. See below for an example:

```py
vertex_distance = 2.0  # in meters
max_road_length = 500.0 # in meters
wall_height = 0.0      # in meters
extra_width = 0.6      # in meters
world = client.generate_opendrive_world(
    xodr_xml, carla.OpendriveGenerationParameters(
        vertex_distance=vertex_distance,
        max_road_length=max_road_length,
        wall_height=wall_height,
        additional_width=extra_width,
        smooth_junctions=True,
        enable_mesh_visibility=True))
```

!!! Note
    `wall_height = 0.0` is strongly recommended. OpenStreetMap defines lanes in opposing directions as different roads. If walls are generated, this will result in wall overlapping and undesired collisions.

###### B) Pass `.xodr` to `config.py`

After you have started a CARLA server, run the following command in a separate terminal to load your Open Street Map:

```sh
cd PythonAPI/util

python3 config.py -x=/path/to/xodr/file
```

[Default parameters](python_api.md#carla.OpendriveGenerationParameters) will be used.
###### C) Pass `.osm` to `config.py`

After you have started a CARLA server, run the following command in a separate terminal to load your Open Street Map:

```sh
cd PythonAPI/util

python3 config.py --osm-path=/path/to/OSM/file
```

[Default parameters](python_api.md#carla.OpendriveGenerationParameters) will be used.


Regardless of the method used, the map will be ingested into CARLA and the result should be similar to the image below:

![opendrive_meshissue](img/tuto_g_osm_carla.jpg)
<div style="text-align: right"><i>Outcome of the CARLA map generation using OpenStreetMap.</i></div>

<br>
!!! Warning
    The roads generated end abruptly at the borders of the map. This will cause the Traffic Manager to crash when vehicles are not able to find the next waypoint. To avoid this, the OSM mode in the Traffic Manager is set to __True__ by default ([`set_osm_mode()`](python_api.md#carlatrafficmanager)). This will show a warning and destroy vehicles when necessary.  

---

Any issues and doubts related with this topic can be posted in the CARLA forum.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>