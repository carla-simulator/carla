import xml.etree.ElementTree as ET
import carla
import git
import random
import os
from utils import *
from change import change
from config import config

# Load original Scenario
repo_dir = git.Repo('.', search_parent_directories=True).working_tree_dir
scenarioFolder = "runnerTool/scenarios/test"
xosc_file = os.path.join(repo_dir, scenarioFolder, "original.xosc")

# Parse XML
tree = ET.parse(xosc_file)
root = tree.getroot()

# Setup Carla
client = carla.Client("localhost", 2000)
world = client.get_world()
world_name = root.find(".//RoadNetwork/LogicFile").attrib["filepath"]
world = client.load_world(world_name)
world_map = world.get_map()

# Randomize
change(root, world_map, config=config)

# Save
tree.write(os.path.join(repo_dir, scenarioFolder, "modified.xosc"))