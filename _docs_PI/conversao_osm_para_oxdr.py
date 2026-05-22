import carla

# 1. Read your .osm file
osm_file_path = "mauazona.osm"
with open(osm_file_path, "r", encoding="utf-8") as f:
    osm_data = f.read()

# 2. Define the conversion settings
settings = carla.Osm2OdrSettings()
# Default lane width in meters
settings.default_lane_width = 3 
# Set to True if you want CARLA to guess where traffic lights go
settings.generate_traffic_lights = False 
settings.center_map = True
# Filter for specific road types (optional but recommended to avoid pedestrian paths)
settings.set_osm_way_types([
    "motorway", "motorway_link", "trunk", "trunk_link", 
    "primary", "primary_link", "secondary", "secondary_link", 
    "tertiary", "tertiary_link", "unclassified", "residential"
])

# 3. Convert the OSM data to an OpenDRIVE string
print("Converting .osm to .xodr...")
xodr_data = carla.Osm2Odr.convert(osm_data, settings)

# 4. Save the OpenDRIVE data to a new file
xodr_file_path = "mauazona.xodr"
with open(xodr_file_path, "w", encoding="utf-8") as f:
    f.write(xodr_data)

print(f"Success! Saved to {xodr_file_path}")