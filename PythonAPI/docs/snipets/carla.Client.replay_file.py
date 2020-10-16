# ===================================================
# Replay a CARLA recording
# ===================================================

# This snipet shows how to replay a recording of a CARLA simulation. 
#   To learn more about the CARLA recorder, read the docs available in: https://carla.readthedocs.io/en/latest/adv_recorder/


# Define some arguments for the replay. Only the path to the log file is compulsory. 
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '-l', '--log-path',
    help='Path to the .log file created by the CARLA recorder that is going to be replayed.')
argparser.add_argument(
    '-s', '--start',
    metavar='S',
    default=0,
    type=float,
    help='Time in seconds where the replay starts. Negative values will be considered starting from the end of the recording. Default is 0, the beginning of the recording.')
argparser.add_argument(
    '-d', '--duration',
    metavar='D',
    default=60,
    type=float,
    help='Duration of the replay. The simulation continues when the duration is finished or the recording ends. Default is 60 seconds.')
argparser.add_argument(
    '-a', '--actor',
    metavar='A',
    default=0,
    type=float,
    help='ID of the actor that the spectator camera will follow during the replay. Default is 0, which is none.')
args = argparser.parse_args()

# If the path to a log was provided, start replaying. 
if args.log_path is not None:
    if os.path.exists(args.log_path):
        print("Starting the replay...")
        client.replay_file(log_path,args.start,args.duration,args.actor)
    else: 
        print('No log found at %r.', % os.path.basename(log_path))
else: 
    print("You must provide with a recording using the --log-path argument.")


# Notice that this snipet does not stop the replay, so eventually the recording will end, and the simulation will continue where the recording ended. 