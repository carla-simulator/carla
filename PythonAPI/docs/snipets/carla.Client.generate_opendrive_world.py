# ===================================================
# Load an XODR map into CARLA 
# ===================================================

# In this snipet, a XODR file containing a map is loaded in CARLA. 
#   This snipet is based in the OpenDRIVE Standalone Mode feature: https://carla.readthedocs.io/en/latest/adv_opendrive/


# Define an argument for the script to provide with the path to a XODR file.
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '-x', '--xodr-path',
    metavar='XODR_FILE_PATH',
    help='The path to a XODR file that will be used as a CARLA map.')
args = argparser.parse_args()

# The path cannot be loaded directly into CARLA. First you have to load the file, and parse the XODR content as string. 
if args.xodr_path is not None:
    if os.path.exists(args.xodr_path):
        # Try to open the file, and send a message error in case it is not possible. 
        with open(args.xodr_path) as od_file:
            try:
                data = od_file.read()
            except OSError:
                print('The file could not be read.')
                sys.exit()
        print('Loading the OpenDRIVE map file at %r.' % os.path.basename(args.xodr_path))

        # The map generation allows for some parameterization. Read the docs on the OpenDRIVE Standalone Mode to find out more. 
        #   Remember that you could add these parameters as optional arguments and leave the following as default values. 
        vertex_distance = 2.0   # in meters
        max_road_length = 500.0 # in meters
        wall_height = 1.0       # in meters
        extra_width = 0.6       # in meters
        world = client.generate_opendrive_world(
            data, carla.OpendriveGenerationParameters(
                vertex_distance=vertex_distance,
                max_road_length=max_road_length,
                wall_height=wall_height,
                additional_width=extra_width,
                smooth_junctions=True,
                enable_mesh_visibility=True))
    else:
        print('No file found at %r.', % os.path.basename(xodr_path))

