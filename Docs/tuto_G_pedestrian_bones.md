# Retrieve pedestrian ground truth bones through API

To train autonomous vehicles, it is essential to make sure they recognise not only buildings, roads and cars, but also the pedestrians that occupy the sidewalks and cross the roads, to ensure the safety of all road users. The CARLA simulator provides AI controlled pedestrians to populate your simulation and training data with human forms. There are many computer vision applications in which pose estimation is an important factor including autonomous driving, but also in security, crowd control and multiple robotic applications. 

CARLA's API provides functionality to retrieve the ground truth skeleton from the pedestrian's in the simulation. The skeleton is composed as a set of bones, each with a root node or vertex and a vector defining the pose (or orientation) of the bone. These bones control the movement of the limbs of the simulated pedestrian. By collecting together the ensemble of individual bones, a model of the virtual human's pose can be built that can be used to compare against a pose model estimated by a neural network, or even used to train a neural network for pose estimation. 

In this tutorial, we will go through the steps of spawning a pedestrian in a map, setting up an AI controller to move the pedestrian and then retrieving the ground truth skeleton and projecting the bones onto a 2D camera capture.

## Setting up the simulator

First, launch the CARLA simulator as per your standard workflow, either in standalone mode or inside the Unreal Edtior. We will import several utility librarys to use for maths and plotting. To give us more control over the simulation, we will use [__synchronous mode__](adv_synchrony_timestep.md) in this tutorial. This means that our Python client controls the time progression of the simulator.

```py
import carla
import random
import matplotlib.pyplot as plt
import numpy as np
import time
import pyqtgraph as pg
import math
import queue

# Connect to the client and retrieve the world object
client = carla.Client('localhost', 2000)
world = client.get_world()

# Set up the simulator in synchronous mode
settings = world.get_settings()
settings.synchronous_mode = True # Enables synchronous mode
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

# We will aslo set up the spectator so we can see what we do
spectator = world.get_spectator()

```

## Spawning a pedestrian in the CARLA simulator

First, we want to spawn a pedestrian in the simulation. This can be done at a random location using `world.get_random_location_from_navigation()`, or it can be chosen using coordinates gathered from the Unreal Editor. In the unreal editor, add an empty actor to the location where you want to spawn your pedestrian, then use the inspector on the right hand side to query the coordinates.

![actor_location](../img/tuto_G_pedestrian_bones/actor_location.png)

!!! Note
    The Unreal Editor works in units of centimeters, while CARLA works in units of meters so the units must be converted. Ensure to divide the Unreal Editor coordinates by 10 before using in the CARLA simulator.


Once you have chosen your coordinates, you can then spawn the pedestrian. We will also spawn a camera to gather images. We also need a [`Queue`](#https://docs.python.org/3/library/queue.html) object to allow us easy access to the data from the camera (as the camera sensor output is multithreaded)


```py

# Get the pedestrian blueprint and spawn it
pedestrian_bp = random.choice(world.get_blueprint_library().filter('*walker.pedestrian*'))
transform = carla.Transform(carla.Location(x=-77,y=120.1,z=1.18))
pedestrian = world.try_spawn_actor(pedestrian_bp, transform)

# Spawn an RGB camera
camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, transform)

# Create a queue to store and retrieve the sensor data
image_queue = queue.Queue()
camera.listen(image_queue.put)

# Now we will rotate the camera to face the pedestrian
rot_offset = 2.5 # Rotation offset in radians
trans = pedestrian.get_transform()
x = math.cos(rot_offset) * -3
y = math.sin(rot_offset) * 3
trans.location.x += x
trans.location.y += y
trans.location.z = 2
trans.rotation.pitch = -16
trans.rotation.roll = 0
trans.rotation.yaw = -360 * (rot_offset/(math.pi*2))
spectator.set_transform(trans)
camera.set_transform(trans)

# Call tick to add actors to the scene
world.tick()

# Move the spectator to see the result
spectator.set_transform(camera.get_transform())

```



