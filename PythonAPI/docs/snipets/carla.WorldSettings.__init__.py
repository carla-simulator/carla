# ===================================================
# Define and apply world settings
# ===================================================

# This snipet defines and changes the world settings according to the ones passed as arguments. 
#   World settings affect the simulation in complex and broad ways. It is highly recommended to read the docs to understand what is going on. 
#   Read about the synchronous/asynchronous mode: https://carla.readthedocs.io/en/latest/adv_synchrony_timestep/
#   Read about the different rendering options: https://carla.readthedocs.io/en/latest/adv_rendering_options/ 

# Gather
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '-s', '--sync',
    action='store_true',
    help='Use this flag to enable synchronous mode.')
argparser.add_argument(
    '-r', '--no-render',
    action='store_true',
    help='Use this flag to enable no-rendering mode')
argparser.add_argument(
    '-d', '--delta',
    default=0.0,
    type=float,
    help='Define the delta for a fixed time-step. Default is 0.0, which is variable time-step.')
args = argparser.parse_args()

# Define the current and the new world settings.
current_settings = world.get_settings()
new_settings = carla.WorldSettings(sync,no_render,delta)

# If the settings want to be changed, apply these. 
if new_settings != current_settings:
    world.apply_settings(new_settings)



# ---
# This snipet is also available in carla.World.apply_settings()
