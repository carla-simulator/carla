import logging
import math
import random
import carla
import time
import numpy as np
import pandas as pd

# Initialize CARLA client and connect to the simulator
client = carla.Client('localhost', 2000)
client.set_timeout(2.0)

# Load the CARLA world and map
world = client.get_world()
map = world.get_map()

# Create an ego vehicle 
ego_vehicle_bp = world.get_blueprint_library().find('vehicle.audi.a2')


spawn_points = world.get_map().get_spawn_points()
number_of_spawn_points = len(spawn_points)

if 0 < number_of_spawn_points:
    random.shuffle(spawn_points)
    ego_transform = spawn_points[0]
    ego_vehicle = world.spawn_actor(ego_vehicle_bp,ego_transform)
    print('\nEgo is spawned')
else: 
    logging.warning('Could not find any spawn points')

print(ego_vehicle.get_location())

 # Create a camera sensor and attach it to the ego vehicle
camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
camera_bp.set_attribute('image_size_x', '1920')
camera_bp.set_attribute('image_size_y', '1080')
#camera_location = carla.Location(x=0, y=0, z=2.5)  # Adjust the position as needed
#camera_sensor = world.spawn_actor(camera_bp, camera_location, attach_to=ego_vehicle)

camera_transform = carla.Transform(carla.Location(x=0, y=0, z=2.5))  # Adjust the position as needed
camera_sensor = world.spawn_actor(camera_bp, camera_transform, attach_to=ego_vehicle)




def follow_car(ego_vehicle, world):
    """
    Set the spectator's view to follow the ego vehicle.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle that the spectator will follow.
        world (World): The game world where the spectator view is set.

    Description:
        This function calculates the desired spectator transform by positioning the spectator
        10 meters behind the ego vehicle and 5 meters above it. The spectator's view will follow
        the ego vehicle from this transformed position.

    Note:
        To face the vehicle from behind, uncomment the line 'spectator_transform.rotation.yaw += 180'.

    Returns:
        None: The function does not return any value.
    """
    # Calculate the desired spectator transform
    vehicle_transform = ego_vehicle.get_transform()
    spectator_transform = vehicle_transform
    spectator_transform.location -= (
        vehicle_transform.get_forward_vector() * 10
    )  # Move 10 meters behind the vehicle
    spectator_transform.location += (
        vehicle_transform.get_up_vector() * 5
    )  # Move 5 meters above the vehicle
    # spectator_transform.rotation.yaw += 180 # Face the vehicle from behind

    # Set the spectator's transform in the world
    world.get_spectator().set_transform(spectator_transform)


def get_speed(ego_vehicle):
    # Get the velocity of the ego vehicle
    velocity = ego_vehicle.get_velocity()
    speed = velocity.length()
    return speed #in m/s

def get_acceleration(speed, previous_speed, current_time, previous_time):
    time_interval = (current_time - previous_time) / 1_000_000_000
    acceleration = (speed - previous_speed) / time_interval  # Change in speed during one time interval
    return acceleration

def get_steering_angle(ego_vehicle):
    """
    steering_angle < 0: driving left
    steering_angle > 0: driving right
    """
    control = ego_vehicle.get_control()

    steering_angle = control.steer * 180 #angle in degree, control.steer in [-1.0, 1.0]
    #steering_angle = steering_angle * (math.pi / 360) #angle in radians
    return steering_angle

def get_lateral_acceleration(ego_vehicle, current_time, previous_time):
    time_interval = (current_time - previous_time) / 1_000_000_000
    # Get the lateral rotation (yaw) of the ego vehicle
    current_yaw = ego_vehicle.get_transform().rotation.yaw
    delta_yaw = math.radians(current_yaw - previous_yaw)

    lateral_acceleration = (speed * delta_yaw) / time_interval

    return lateral_acceleration, current_yaw

def get_dist_to_lane_center(ego_vehicle):
    ego_location = ego_vehicle.get_location()
    waypoint = world.get_map().get_waypoint(ego_location)
    lane_location = waypoint.transform.location
    dist_to_lane_center = math.sqrt((lane_location.x - ego_location.x) **2 + (lane_location.y - ego_location.y) ** 2)

    return dist_to_lane_center

def get_speed_of_vehicle_ahead(ego_vehicle):
    #camera_data = camera_sensor.listen()
    
    #TODO

    if leading_vehicle:
        leading_vehicle_speed = get_speed(leading_vehicle)
        return leading_vehicle_speed

    return None

# Function to get the curvature of the road at the ego vehicle's current location in degrees
def get_curvature_at_location(vehicle_location):
    # Find the nearest waypoint corresponding to the vehicle's location
    waypoint = map.get_waypoint(vehicle_location)
    # Get the curvature at the waypoint
    

    

    return curvature_degrees_per_meter




# Create an empty list to store the data as dictionaries
data_list = []

previous_speed = 0  # Store the previous speed value in m/s
previous_yaw = 0  # Store the previous yaw angle value
leading_vehicle = None  # Variable to store the leading vehicle reference
previous_time = time.time_ns()

time_interval = 1.0 #for acceleration, in s

try:
    # Set the ego vehicle on autopilot
    ego_vehicle.set_autopilot(True)

    while True:
        follow_car(ego_vehicle, world)

        #returns the current speed of the ego vehicle in m/s
        speed = get_speed(ego_vehicle)
        current_time = time.time_ns()

        acceleration = get_acceleration(speed, previous_speed, current_time, previous_time)

        steering_angle = get_steering_angle(ego_vehicle) #TODO angle in degree or radians?

        lateral_acceleration, current_yaw = get_lateral_acceleration(ego_vehicle, current_time, previous_time)

        dist_to_lane_center = get_dist_to_lane_center(ego_vehicle) #in m #TODO needs to be tested with manual car

        speed_vehicle_ahead = get_speed_of_vehicle_ahead(ego_vehicle) #TODO

        # Get and print the curvature at the ego vehicle's location in degrees per meter
        curvature_degrees_per_meter = get_curvature_at_location(ego_vehicle.get_location()) #TODO

        #print({'Speed (m/s)': speed, '\nAcceleration (m/s^2)': acceleration, '\nSteering Angle' : steering_angle, '\nLateral Acceleration' : lateral_acceleration})
        #print('\nCurvature at Ego Vehicle Location (Degrees/m): ' + str(curvature_degrees_per_meter))
     
        print("\n")


        # Append the data as a dictionary to the list
        data_list.append({'Speed (m/s)\t\t': speed, 'Acceleration (m/s^2)\t\t': acceleration, 'Steering Angle\t\t' : steering_angle,\
                          'Lateral Acceleration (m/s^2)\t\t' : lateral_acceleration, 'Distance to lane center (m)\t\t' : dist_to_lane_center,\
                            'Speed of vehicle ahead (m/s)\t\t' : speed_vehicle_ahead,\
                            'Road Curvature at Ego Vehicle Location (Degrees/m)\t\t' : curvature_degrees_per_meter})
        

        previous_speed = speed  # Update the previous speed value
        previous_time = current_time
        previous_yaw = current_yaw  # Update the previous yaw angle value

        time.sleep(time_interval) #TODO ticks in CARLA?
        world.tick()

except KeyboardInterrupt:
    pass

finally:
    print("It's working 03")

    # Clean up and close the file, turn off autopilot, and disconnect from CARLA
    ego_vehicle.set_autopilot(False)  # Disable autopilot
    ego_vehicle.destroy()
    #client.disconnect()

# Convert the list of dictionaries to a DataFrame
data_df = pd.DataFrame(data_list)

#Save data from the dataframe in a csv file
filename = 'test_data/' + time.strftime("%Y-%m-%d_%H-%M-%S") + '.csv'
data_df.to_csv(filename, index=False)

