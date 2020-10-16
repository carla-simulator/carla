# ===================================================
# Parse arguments when creating a client
# ===================================================

# This snipet shows how to parse some arguments when calling for a script, and how these can be used to create the client. 
#   The snipet can be found in any of the example scripts provided in PythonAPI/examples, just modified with a different set of arguments. 

# Create an argument parser and define the desired arguments
argparser = argparse.ArgumentParser(
    description=__doc__)
argparser.add_argument(
    '--host',
    metavar='H',
    default='127.0.0.1',
    help='IP of the host server (default: 127.0.0.1)')
argparser.add_argument(
    '-p', '--port',
    metavar='P',
    default=2000,
    type=int,
    help='TCP port to listen to (default: 2000)')
argparser.add_argument(
    '-s', '--speed',
    metavar='FACTOR',
    default=1.0,
    type=float,
    help='rate at which the weather changes (default: 1.0)')
args = argparser.parse_args()


# Use this arguments at will in your script
speed_factor = args.speed
update_freq = 0.1 / speed_factor

client = carla.Client(args.host, args.port)

