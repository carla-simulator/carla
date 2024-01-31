from randomizer import main 
import os 
import git
from config import get_config

# get all folders in dir
repo_dir = git.Repo('.', search_parent_directories=True).working_tree_dir

scenario_directory = os.path.join(repo_dir, "runnerTool/scenarios/test") # Change this to the directory of your scenarios
scenario_folders = [f.path for f in os.scandir(scenario_directory) if f.is_dir()]
print(scenario_folders)

# run randomizer for multiple scenarios each with the same config
for folder in scenario_folders:
    directory = os.path.join(scenario_directory, folder)
    config = get_config()
    config["scenarioFolder"] = folder
    files = [file for file in os.listdir(directory) if file.endswith('.xosc')]
    
    if len(files) > 0:
        config["xosc_file_name"] = files[0]
        print(config["xosc_file_name"])
        main(config = config)