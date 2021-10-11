""" Convert OpenStreetMap file to OpenDRIVE file. """
import carla

# Read the .osm data
with open("path/to/osm/file/map.osm", mode="r", encoding="utf-8") as osmFile:
    osm_data = osmFile.read()

# Define the desired settings. In this case, default values.
settings = carla.Osm2OdrSettings()

# Set OSM road types to export to OpenDRIVE
settings.set_osm_way_types([
    "motorway",
    "motorway_link",
    "trunk",
    "trunk_link",
    "primary",
    "primary_link",
    "secondary",
    "secondary_link",
    "tertiary",
    "tertiary_link",
    "unclassified",
    "residential"
])

# set width of each lane by meters
settings.default_lane_width = 6.0

# enable traffic light generation from OSM data
settings.generate_traffic_lights = True

# ignore OSM traffic lights and set lights for all junctions
settings.all_junctions_with_traffic_lights = True

# set center of map to the origin coordinates
settings.center_map = True

# Convert to .xodr
xodr_data = carla.Osm2Odr.convert(osm_data, settings)

# save opendrive file
with open("path/to/create/xodr/file/map.xodr", "w", encoding="utf-8") as xodrFile:
    xodrFile.write(xodr_data)
