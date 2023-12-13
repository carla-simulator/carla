import xml.etree.ElementTree as ET
import carla
import git
import os
from utils import *
from change import change
from config import get_config

# make main method
def main(config = None):
    # Configuration
    if config is None: 
        # run with config from config.py
        config = get_config()
    
    scenarioFolder = config["scenarioFolder"]
    xosc_file_name = config["xosc_file_name"]
    num_randomized_scenarios = config["num_randomized_scenarios"]

    # Load original Scenario
    repo_dir = git.Repo('.', search_parent_directories=True).working_tree_dir
    xosc_file = os.path.join(repo_dir, "runnerTool/scenarios/", scenarioFolder, xosc_file_name)


    # Setup Carla
    client = carla.Client("localhost", 2000)
    world = client.get_world()


    # Randomize
    for i in range(num_randomized_scenarios):
        tree = ET.parse(xosc_file)
        root = tree.getroot()
        world_name = root.find(".//RoadNetwork/LogicFile").attrib["filepath"]
        world = client.load_world(world_name)
        world_map = world.get_map()
        for intensity in config["intensity_levels"]:
            # Define the directory path
            dir_path = os.path.join(repo_dir, scenarioFolder, f"intensity_{str(intensity)}")

            # Check if the directory exists, if not create it
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)

            change(root, world_map, config=config, intensity=intensity)
            # Save
            tree.write(os.path.join(dir_path, f"modified_{i}.xosc"))

if __name__ == "__main__":
    main()