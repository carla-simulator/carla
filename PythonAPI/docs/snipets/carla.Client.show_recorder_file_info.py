# ===================================================
# Show the information in a CARLA recording
# ===================================================

# This snipet makes use of a previously CARLA recording to display a summary of the data it contains per frame. 
#   To learn more about the CARLA recorder, read the docs available in: https://carla.readthedocs.io/en/latest/adv_recorder/


# To display the information we only need two things: the path to the log file, and a data display format. 
#   There are two displays: a summary of the most relevant events, or all the information available per frame. 
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '-l', '--log-path',
    help='Path to the .log file created by the CARLA recorder that is going to be replayed.')
argparser.add_argument(
    '-a', '--all',
    action='store_true',
    help='Use this flag to display all the information available per frame, instead of a summary.')
args = argparser.parse_args()

# If the path to a log was provided, start replaying. 
if args.log_path is not None:
    if os.path.exists(args.log_path):
        print("Starting the replay...")
        client.show_recorder_file_info(log_path,args.all)
    else: 
        print('No log found at %r.', % os.path.basename(log_path))
else: 
    print("You must provide with a recording using the --log-path argument.")


# Notice that this snipet does not stop the replay, so eventually the recording will end, and the simulation will continue where the recording ended. 