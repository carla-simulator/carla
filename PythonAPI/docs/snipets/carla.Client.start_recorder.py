# ===================================================
# Replay a CARLA recording
# ===================================================

# This snipet shows how to start recording a simulation. 
#   To learn more about the CARLA recorder, read the docs available in: https://carla.readthedocs.io/en/latest/adv_recorder/
#   To learn more about the log file and how is data stored, read the reference in: https://carla.readthedocs.io/en/latest/ref_recorder_binary_file_format/


# Define some arguments for the replay. Only the path to the log file is compulsory. 
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '-l', '--log-name',
    help='Name of the log file that will create the recorder. This can include a path, and if it does not, this will be considered to be the default CarlaUE4/Saved/recording.log.')
argparser.add_argument(
    '-d', '--extra-data',
    action='store_true',
    help='Use this flag to save additional data. Read the docs to find out more about this.')
args = argparser.parse_args()

# Start recording
if args.log_path is not None:
    client.start_recorder(args.log_name,extra_data)
else:
    print("You must provide with a name for the recording using the --log-name argument.")


# To stop recording, simply call for the corresponding method in this, or any other script. 
#   For the scope of this snipet, let's suppose we wanted a recording that lasted specifically 30 seconds. 
time.sleep(30)
client.stop_recorder()