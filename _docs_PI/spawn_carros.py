import carla
import random
import time

# 1. Connect to the simulator (make sure a.py has already loaded the map!)
client = carla.Client('127.0.0.1', 2000)
client.set_timeout(10.0)
world = client.get_world()

# 2. Get the blueprint library (this holds all the 3D models)
blueprint_library = world.get_blueprint_library()

# 3. Choose a car (Let's use a Tesla Model 3, but you can change this)
vehicle_bp = blueprint_library.filter('vehicle.tesla.model3')[0]

# 4. Get the mathematical map layout
carla_map = world.get_map()

# 5. Generate a list of valid driving locations (Waypoints) every 2 meters
print("Calculating road waypoints...")
waypoints = carla_map.generate_waypoints(distance=2.0)

# Pick a random spot on the road
valid_waypoint = random.choice(waypoints)
spawn_transform = valid_waypoint.transform

# CRITICAL FIX: Lift the car 0.5 meters into the air before spawning!
# If you don't do this, the tires might spawn inside the asphalt and the car will explode into the sky.
spawn_transform.location.z += 0.5 

# 6. Attempt to spawn the car
print("Attempting to drop the car into the map...")
vehicle = world.try_spawn_actor(vehicle_bp, spawn_transform)

if vehicle is not None:
    print("Success! The car is on the road.")
    
    # Turn over control to CARLA's internal AI traffic manager
    vehicle.set_autopilot(True)
    print("Autopilot engaged. The car should be driving!")
    
    # Keep the script running so the car exists
    try:
        print("Press Ctrl+C in this terminal to stop the script and delete the car.")
        while True:
            # We add a tiny delay so the while loop doesn't fry your CPU
            time.sleep(1) 
    except KeyboardInterrupt:
        print("\nCleaning up... deleting the car.")
        vehicle.destroy()
        print("Done.")
else:
    print("Failed to spawn the car! The spot might be blocked or geometrically invalid. Run the script again.")