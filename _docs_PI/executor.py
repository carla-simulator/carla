import carla

# Connect to the simulator
client = carla.Client('127.0.0.1', 2000)
client.set_timeout(60.0)

# Read the .xodr file you just generated
with open("arrumando1_mauazona.xodr", "r", encoding="utf-8") as f:
    xodr_data = f.read()

# Define how the 3D mesh should be generated
parameters = carla.OpendriveGenerationParameters(
    vertex_distance=2.0,  # Distance between vertices of the generated mesh
    max_road_length=50.0, # Max length of a single road mesh
    wall_height=0.1,      # Height of the walls on the edge of the road
    additional_width=0.6, # Extra width added to the road margins
    smooth_junctions=True,
    enable_mesh_visibility=True # Crucial: set to True so you can actually see the roads
)

# Tell CARLA to build the world
print("Generating 3D world in CARLA...")
world = client.generate_opendrive_world(xodr_data, parameters)
print("Map loaded successfully!")