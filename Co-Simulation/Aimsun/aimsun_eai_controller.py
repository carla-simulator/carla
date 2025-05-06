import glob

# Import math Library
import math
import os
import random
import socket
import struct
import sys
import pygame

import ExternalAgentsConnector_pb2 as EAI

# import carla
try:
    sys.path.append(
        glob.glob(
            "../carla/dist/carla-*%d.%d-%s.egg"
            % (
                sys.version_info.major,
                sys.version_info.minor,
                "win-amd64" if os.name == "nt" else "linux-x86_64",
            )
        )[0]
    )
except IndexError:
    pass

import carla


def send_message(my_socket, message):
    packed_len = struct.pack("!I", message.ByteSize())
    my_socket.sendall(packed_len)
    my_socket.sendall(message.SerializeToString())


def read_message(my_socket):
    # Find message size
    header_size = struct.Struct("!I").size
    header_buffer = my_socket.recv(header_size)  # Byte-form
    msg_size = struct.unpack("!I", header_buffer)[0]  # Int
    # Read message
    reader = EAI.Aimsun()
    message_buffer = my_socket.recv(msg_size)  # Byte-form
    reader.ParseFromString(message_buffer)  # String
    return reader


def external_agent_message_generator(x, y, h):
    writer = EAI.External()
    in_message = getattr(writer, "in")
    agent = in_message.agents.add()

    bubble = in_message.bubbles.add()
    bubble.id = 239
    bubble.x = x
    bubble.y = y
    bubble.radius = 1000

    agent.aimsun_id = -1
    agent.external_id = "1"

    pos = agent.position.add()
    pos.x = x
    pos.y = y
    pos.z = 0.0
    pos.h = h
    pos.length = 4.0
    pos.width = 2.0

    agent.owner = EAI.Agent.EXTERNAL
    agent.speed = 5.0
    agent.brakeLight = False
    agent.leftTurnIndicator = False
    agent.rightTurnIndicator = False
    agent.min_Upstream_TimeGap_For_LaneChange = 5.0
    agent.agent = EAI.Agent.CAR
    return writer


def do_load(my_socket):
    writer = EAI.External()
    load = getattr(writer, "load")
    load.file = "try_Eai.ang"
    load.rep = 0
    load.inputCoordinates = load.REAR_AXLE
    return writer


def do_close_result(my_socket):
    writer = EAI.External()
    close_result = getattr(writer, "close_result")
    close_result.state.state = EAI.ErrorState.OK
    return writer


# --- Connection --- aimsun
address = ("localhost", 1541)  # IP:Port
my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
my_socket.connect(address)
print("Connected to Aimsun Next through External Agent Interface.")

# --- Load Message at Startup ---
load_message = do_load(my_socket)
print("Sending Load Message...")
send_message(my_socket, load_message)
load_answer = read_message(my_socket)

# The port is the "bridge" through which data from different applications communicate
# (this python script with carla and this python script with aimsun)


# CARLA functionality: Replace this with whatever you need your simulation to do -

# Connect to carla's instance
client = carla.Client("localhost", 2000)

# time out if we don't connect in 10 seconds
client.set_timeout(10.0)

# world name THIS IS WHERE YOU SET THE WORLD TO BE LOADED BY CARLA

# world = client.load_world('Town01')
world = client.generate_opendrive_world(open("XXXX/new19.xodr", "r").read())

# set spectator camera location
spectator = world.get_spectator()
spec_location = carla.Location(x=30, y=0, z=100)
spec_rotation = carla.Rotation(pitch=270, yaw=270, roll=0)
transform = carla.Transform(spec_location, spec_rotation)
spectator.set_transform(transform)


# load the cars library
blueprint_library = world.get_blueprint_library()


# EXAMPLE: Spawn a vehicle and use Carla's autonomous control
vehicle_bp = blueprint_library.filter("vehicle.*")[0]  # Get the first vehicle blueprint
custom_spawn_point = carla.Transform(
    carla.Location(x=50.0, y=0.0, z=2.0),  # Position in the world
    carla.Rotation(pitch=0.0, yaw=0.0, roll=0.0),  # Orientation
)

map = world.get_map()
waypoints = map.generate_waypoints(2.0)
vehicle = world.spawn_actor(vehicle_bp, custom_spawn_point)
vehicle.set_autopilot(True)  # Simplistic autonomous driver in CARLA


# Set fixed time step
settings = world.get_settings()
settings.fixed_delta_seconds = 0.1
world.apply_settings(settings)


# Connect to the traffic manager
traffic_manager = client.get_trafficmanager()

# -----------------------------------------------------------------------------------

bp_dictionary = {
    0: "agent_not_defined",
    1: "vehicle.*audi.a2*",
    2: "vehicle.*gazelle*",
    3: "vehicle.*european_hgv*",
    4: "vehicle.*volkswagen.t2*",
    5: "walker.pedestrian.*",
    6: "vehicle.*kawasaki*",
}


def spawn_vehicle(x, y, agent_type):
    rotation = carla.Rotation(0, 0, 0)
    location = carla.Location(x, y, 2)
    print(f"location is {location}")
    transform = carla.Transform(location, rotation)

    bp_filter = blueprint_library.filter(bp_dictionary[agent_type])
    print(bp_dictionary[agent_type])
    print(bp_filter)
    bp = random.choice(bp_filter)

    if bp.has_attribute("color"):
        # random colour
        color = random.choice(bp.get_attribute("color").recommended_values)
        bp.set_attribute("color", color)
    # Spawn actor
    return world.spawn_actor(bp, transform)


vehicles = []
traffic_lights = set()


def vehicle_exists(id):
    for i in vehicles:
        if i.id_v == id:
            return i
    return None


def parser(aimsun_output):
    if aimsun_output.HasField("out"):
        for i in aimsun_output.out.lights_states:
            # for each traffic light in the output traffic light set,
            # a new traffic light is created if it does not exist before, and if it does exist,
            # a state change is performed (green/red/amber)
            s = TrafficLight(i.id, i.name, i.state)
            traffic_lights.discard(s)
            traffic_lights.add(s)

        for i in aimsun_output.out.agents:
            listapos = []
            for p in i.position:
                # Create a new position with the data extracted from aimsun
                pos = Position(p.x, p.y, p.h, p.length, p.width)
                listapos.append(pos)
            agent_type = i.agent
            # For each agent that enters, checked if it already exists
            v = vehicle_exists(i.aimsun_id)
            # If it does not exist, create it and add it to the list
            # if the vehicle already exists, update its speed
            if v == None:
                vehicles.append(
                    Agent(
                        i.aimsun_id,
                        listapos,
                        i.agent,
                        i.speed,
                        i.owner,
                        spawn_vehicle(listapos[0].x, listapos[0].y, agent_type),
                    )
                )
            else:
                v.change_position_carla(listapos[0].x, listapos[0].y, p.h)


class TrafficLight:
    id_s = 0
    nombre = 0
    state = ""

    # Calculates the hash (identifier component of the traffic light)
    def __hash__(self):
        return hash(self.id_s)

    # Initiate a traffic light
    def __init__(self, id_s, nombre, state):
        self.id_s = id_s

    # method to check if one traffic light is the same as another
    def __eq__(self, other):
        return self.__hash__() == other.__hash__()

    # Method to change state on this traffic light
    def change_state(self, new_state):
        print("I have changed to " + str(new_state))
        self.state = new_state


# Position class that stores all aimsun components
class Position:
    x = 0
    y = 0
    h = 0
    length = 0
    width = 0

    def __init__(self, x, y, h, length, width):
        self.x = x
        self.y = y
        self.h = h
        self.length = length
        self.width = width


class Agent:
    id_v = 0
    velocidad = 0
    posiciones = []
    t_agent = ""
    owner = ""
    vehiculo_carla = None

    def __init__(self, id_v, posiciones, t_agent, velocidad, owner, vehiculo_carla):
        self.id_v = id_v
        self.posiciones = posiciones
        self.t_agent = t_agent
        self.velocidad = velocidad
        self.owner = owner
        self.vehiculo_carla = vehiculo_carla
        self.vehiculo_carla.set_simulate_physics(False)

    # Updates the position of the object but not the rotation since
    # it is not able to obtain it from the proto
    def change_position_carla(self, x, y, h):
        location = carla.Location(x, y, 0)
        r = h * (180 / math.pi)
        rotation = carla.Rotation(0, r, 0)
        transform = carla.Transform(location, rotation)
        # applies it to the vehicle stored in the agent class
        self.vehiculo_carla.set_transform(transform)


if load_answer.HasField("load_result"):

    print("LoadResult Received.\n-----")
    # --- External Agent to send: Initial position ---

    close = False
    # program loop, while the connection is not closed this loop will be executed, t
    # the connection will be closed when aimsun stops giving outputs
    while not close:
        # Reading data from Aimsun Next
        aimsun_output = read_message(my_socket)
        # If you receive a close message, the connection ends
        close = aimsun_output.HasField("close")
        if close:
            print("Close request sent by Aimsun Next. Closing connection.")
            close_result = do_close_result(my_socket)
            send_message(my_socket, close_result)
        else:

            print("Data received from Aimsun:", aimsun_output, "\n-----")
            parser(aimsun_output)

        # this is for the Carla vehicle - replace this if you need more vehicles
        pos_x = vehicle.get_transform().location.x
        pos_y = -vehicle.get_transform().location.y
        r = -vehicle.get_transform().rotation.yaw
        h = r / (180 / math.pi)

        print("External Agent:", pos_x, pos_y)
        agent_message = external_agent_message_generator(pos_x, pos_y, h)
        send_message(my_socket, agent_message)
        print("External Agent Sent.\n-----")


else:
    print("Load Message was unsuccessful. Closing connection.")

my_socket.close()
