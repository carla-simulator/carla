# ===================================================
# Load a new CARLA map
# ===================================================

# This snipet changes the current map used for the simulation. All the actors in spawned will be destroyed. 
#   The CARLA maps available by default can be founde here: https://carla.readthedocs.io/en/latest/core_map/#carla-maps


# This function will return the names of the maps available. It will be called when the --list argument is used. 
def list_options(client):
    # Remove the paths, and leave only the names of the maps.
    maps = [m.replace('/Game/Carla/Maps/', '') for m in client.get_available_maps()]
    # Create a small function to format the output list. 
    indent = 4 * ' '
    def wrap(text):
        return '\n'.join(textwrap.wrap(text, initial_indent=indent, subsequent_indent=indent))
    # Print the list of map names. 
    print('available maps:\n')
    print(wrap(', '.join(sorted(maps))) + '.\n')


# Define the name of the map as an argument for the script. Optionally, we added an argument to display the names of the maps available in CARLA. 
#   The map can be loaded by name, or providing the path. By default, the CARLA maps are stored in Game/Carla/Maps/. 
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '-m', '--map',
    help='Load the map named as stated in this argument.')
argparser.add_argument(
    '-l', '--list',
    action='store_true',
    help='List the name of the maps available.')
args = argparser.parse_args()

# If the client requested a list of the maps available, provide with it.
if args.list:
    list_options(client)

# If a map name/path was provided, load the new map. 
if args.map is not None:
    print('load map %r.' % args.map)
    world = client.load_world(args.map)


