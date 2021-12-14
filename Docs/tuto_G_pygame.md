# Pygame for vehicle control 

[__PyGame__](https://www.pygame.org/news) is a cross-platform set of Python modules useful for writing video games. It provides a useful way of rendering real-time visual output from CARLA in order to monitor sensor output, such as cameras. PyGame can also capture keyboard events, so it is a good way to control actors such as vehicles.

In this tutorial, we will learn to set up a simple PyGame interface that allows us to monitor autonomous traffic driving around a map controlled by the Traffic Manager (TM) and then take manual control over any vehicle using the keyboard.

## Setting up the simulator and initialising traffic manager

First, we will initialise the TM and create some traffic randomly distributed around the city.

```py
import carla
import random
import pygame
import numpy as np

# Connect to the client and retrieve the world object
client = carla.Client('localhost', 2000)
world = client.get_world()

# Set up the simulator in synchronous mode
settings = world.get_settings()
settings.synchronous_mode = True # Enables synchronous mode
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

# Set up the TM in synchronous mode
traffic_manager = client.get_trafficmanager()
traffic_manager.set_synchronous_mode(True)

# Set a seed so behaviour can be repeated if necessary
traffic_manager.set_random_device_seed(0)
random.seed(0)

# We will aslo set up the spectator so we can see what we do
spectator = world.get_spectator()

```

## Spawning vehicles

We want to create a collection of vehicles spawned throughout the city and give the TM control over them.

```py
# Retrieve the map's spawn points
spawn_points = world.get_map().get_spawn_points()

# Select some models from the blueprint library
models = ['dodge', 'audi', 'model3', 'mini', 'mustang', 'lincoln', 'prius', 'nissan', 'crown', 'impala']
blueprints = []
for vehicle in world.get_blueprint_library().filter('*vehicle*'):
    if any(model in vehicle.id for model in models):
        blueprints.append(vehicle)

# Set a max number of vehicles and prepare a list for those we spawn
max_vehicles = 50
max_vehicles = min([max_vehicles, len(spawn_points)])
vehicles = []

# Take a random sample of the spawn points and spawn some vehicles
for i, spawn_point in enumerate(random.sample(spawn_points, max_vehicles)):
    temp = world.try_spawn_actor(random.choice(blueprints), spawn_point)
    if temp is not None:
        vehicles.append(temp)

# Parse the list of spawned vehicles and give control to the TM through set_autopilot()
for vehicle in vehicles:
    vehicle.set_autopilot(True)
    # Randomly set the probability that a vehicle will ignore traffic lights
    traffic_manager.ignore_lights_percentage(vehicle, random.randint(0,50))


```

## Rendering camera output and controlling vehicles with PyGame

Now we have a city populated with traffic, we can set up a camera to follow one of the cars and set up a control interface to take over it with keyboard input.

Firstly, we need to define a callback function for `camera.listen(...)` to render the pixel data to the PyGame interface. PyGame renders data to surfaces, then the screen, so in the callback, we populate a surface stored in an object passed to the callback function.

```py
# Render object to keep and pass the PyGame surface
class RenderObject(object):
    def __init__(self, width, height):
        init_image = np.random.randint(0,255,(height,width,3),dtype='uint8')
        self.surface = pygame.surfarray.make_surface(init_image.swapaxes(0,1))

# Camera sensor callback, reshapes raw data from camera into 2D RGB and applies to PyGame surface
def pygame_callback(data, obj):
    img = np.reshape(np.copy(data.raw_data), (data.height, data.width, 4))
    img = img[:,:,:3]
    img = img[:, :, ::-1]
    obj.surface = pygame.surfarray.make_surface(img.swapaxes(0,1))
```

Now we will create an object to handle the control logic. This can often need some tuning to meet specific needs, but this outlines a basic interface. When in control of a vehicle, this control interface allows control through the arrow keys on any standard keyboard. Forward arrow accelerates, backward arrow brakes and the left and right arrows turn the vehicle. If the down arrow is held when the vehicle is stationary or comes to a stop, it will engage reverse and start to move backwards.

```py

# Control object to manage vehicle controls
class ControlObject(object):
    def __init__(self, veh):
        
        # Conrol parameters to store the control state
        self._vehicle = veh
        self._steer = 0
        self._throttle = False
        self._brake = False
        self._steer = None
        self._steer_cache = 0
        # A carla.VehicleControl object is needed to alter the 
        # vehicle's control state
        self._control = carla.VehicleControl()
    
    # Check for key press events in the PyGame window
    # and define the control state
    def parse_control(self, event):
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_RETURN:
                self._vehicle.set_autopilot(False)
            if event.key == pygame.K_UP:
                self._throttle = True
            if event.key == pygame.K_DOWN:
                self._brake = True
            if event.key == pygame.K_RIGHT:
                self._steer = 1
            if event.key == pygame.K_LEFT:
                self._steer = -1
        if event.type == pygame.KEYUP:
            if event.key == pygame.K_UP:
                self._throttle = False
            if event.key == pygame.K_DOWN:
                self._brake = False
                self._control.reverse = False
            if event.key == pygame.K_RIGHT:
                self._steer = None
            if event.key == pygame.K_LEFT:
                self._steer = None
    
    # Process the current control state, change the control parameter
    # if the key remains pressed
    def process_control(self):
        
        if self._throttle: 
            self._control.throttle = min(self._control.throttle + 0.01, 1)
            self._control.gear = 1
            self._control.brake = False
        elif not self._brake:
            self._control.throttle = 0.0
        
        if self._brake:
            # If the down arrow is held down when the car is stationary, switch to reverse
            if self._vehicle.get_velocity().length() < 0.01 and not self._control.reverse:
                self._control.brake = 0.0
                self._control.gear = 1
                self._control.reverse = True
                self._control.throttle = min(self._control.throttle + 0.1, 1)
            elif self._control.reverse:
                self._control.throttle = min(self._control.throttle + 0.1, 1)
            else:
                self._control.throttle = 0.0
                self._control.brake = min(self._control.brake + 0.3, 1)
        else:
            self._control.brake = 0.0
            
        if self._steer is not None:
            if self._steer == 1:
                self._steer_cache += 0.03
            if self._steer == -1:
                self._steer_cache -= 0.03
            min(0.7, max(-0.7, self._steer_cache))
            self._control.steer = round(self._steer_cache,1)
        else:
            if self._steer_cache > 0.0:
                self._steer_cache *= 0.2
            if self._steer_cache < 0.0:
                self._steer_cache *= 0.2
            if 0.01 > self._steer_cache > -0.01:
                self._steer_cache = 0.0
            self._control.steer = round(self._steer_cache,1)
            
        # Ápply the control parameters to the ego vehicle
        self._vehicle.apply_control(self._control)
  
```

Now we will initialise the vehicle and the camera.

```py

# Randomly select a vehicle to follow with the camera
ego_vehicle = random.choice(vehicles)

# Initialise the camera floating behind the vehicle
camera_init_trans = carla.Transform(carla.Location(x=-5, z=3), carla.Rotation(pitch=-20))
camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=ego_vehicle)

# Start camera with PyGame callback
camera.listen(lambda image: pygame_callback(image, renderObject))

# Get camera dimensions
image_w = camera_bp.get_attribute("image_size_x").as_int()
image_h = camera_bp.get_attribute("image_size_y").as_int()

# Instantiate objects for rendering and vehicle control
renderObject = RenderObject(image_w, image_h)
controlObject = ControlObject(ego_vehicle)

```

Initialise the PyGame interface. This will call up a new window for PyGame.

```py

# Initialise the display
pygame.init()
gameDisplay = pygame.display.set_mode((image_w,image_h), pygame.HWSURFACE | pygame.DOUBLEBUF)
# Draw black to the display
gameDisplay.fill((0,0,0))
gameDisplay.blit(renderObject.surface, (0,0))
pygame.display.flip()

```

Now we can start the game loop. The view can cycle randomly through different vehicles in the map and visualize their journey through the traffic while controlled by the TM. Pressing the TAB key switches to a randomly chosen new vehicle and pressing the RETURN key enables manual control of the vehicle through the arrow keys on the keyboard. This kind of setup could be useful for example if needing to challenge an agent with erratic driving behaviour. The selection logic could be tweaked to select a vehicle close to the one driven by the agent. 


```py

# Game loop
crashed = False

while not crashed:
    # Advance the simulation time
    world.tick()
    # Update the display
    gameDisplay.blit(renderObject.surface, (0,0))
    pygame.display.flip()
    # Process the current control state
    controlObject.process_control()
    # Collect key press events
    for event in pygame.event.get():
        # If the window is closed, break the while loop
        if event.type == pygame.QUIT:
            crashed = True
        
        # Parse effect of key press event on control state
        controlObject.parse_control(event)
        if event.type == pygame.KEYUP:
            # TAB key switches vehicle
            if event.key == pygame.K_TAB:
                ego_vehicle.set_autopilot(True)
                ego_vehicle = random.choice(vehicles)
                # Ensure vehicle is still alive (might have been destroyed)
                if ego_vehicle.is_alive:
                    # Stop and remove the camera
                    camera.stop()
                    camera.destroy()

                    # Spawn new camera and attach to new vehicle
                    controlObject = ControlObject(ego_vehicle)
                    camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=ego_vehicle)
                    camera.listen(lambda image: pygame_callback(image, renderObject))

                    # Update PyGame window
                    gameDisplay.fill((0,0,0))               
                    gameDisplay.blit(renderObject.surface, (0,0))
                    pygame.display.flip()

# Stop camera and quit PyGame after exiting game loop
camera.stop()
pygame.quit()

```

![manual_control](../img/tuto_G_pygame/manual_control.gif)
