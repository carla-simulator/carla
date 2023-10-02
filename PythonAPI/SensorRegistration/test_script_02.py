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
ego_vehicle_blueprint = world.get_blueprint_library().find('vehicle.audi.a2')


spawn_points = world.get_map().get_spawn_points()
number_of_spawn_points = len(spawn_points)

if 0 < number_of_spawn_points:
    random.shuffle(spawn_points)
    ego_transform = spawn_points[0]
    ego_vehicle = world.spawn_actor(ego_vehicle_blueprint,ego_transform)
    print('\nEgo is spawned')
else: 
    logging.warning('Could not find any spawn points')

print(ego_vehicle.get_location())



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


def get_steering_angle(ego_vehicle):
    """
    steering_angle < 0: driving left
    steering_angle > 0: driving right
    """
    control = ego_vehicle.get_control()

    steering_angle = control.steer * 180 #angle in degree
    steering_angle = steering_angle * (math.pi / 360) #angle in radians
    return steering_angle

# Function to get the curvature of the road at the ego vehicle's current location in degrees
def get_curvature_at_location(vehicle_location):
    # Find the nearest waypoint corresponding to the vehicle's location
    waypoint = map.get_waypoint(vehicle_location)

    # Access the curvature information from the waypoint in radians per meter
    curvature_radians_per_meter = waypoint.lane_change if waypoint.lane_change is not None else 0.0

    # Convert curvature to degrees per meter
    curvature_degrees_per_meter = curvature_radians_per_meter # * (180.0 / math.pi)

    return curvature_degrees_per_meter




# Create an empty list to store the data as dictionaries
data_list = []

previous_speed = 0  # Store the previous speed value
previous_yaw = 0  # Store the previous yaw angle value

try:
    # Set the ego vehicle on autopilot
    ego_vehicle.set_autopilot(True)

    while True:
        follow_car(ego_vehicle, world)
         # Get the ego vehicle's current location
        ego_location = ego_vehicle.get_location()

        # Get the velocity of the ego vehicle
        velocity = ego_vehicle.get_velocity()
        speed = carla.Vector3D(velocity.x, velocity.y, velocity.z).length()

        #acceleration_vector = ego_vehicle.get_acceleration()
        #acceleration = carla.Vector3D(acceleration_vector.x, acceleration_vector.y, acceleration_vector.z).length()
        acceleration = (speed - previous_speed) / 1.0  # Change in speed over 1 second

        steering_angle = get_steering_angle(ego_vehicle)


        # Get the rotation (yaw) of the ego vehicle
        yaw = ego_vehicle.get_transform().rotation.yaw

        lateral_acceleration = (speed * math.radians(yaw - previous_yaw)) / 1.0  # Change in yaw over 1 second
        #print(lateral_acceleration)

        # Get and print the curvature at the ego vehicle's location in degrees per meter
        curvature_degrees_per_meter = get_curvature_at_location(ego_location)
        print(f'Curvature at Ego Vehicle Location (Degrees/m): {curvature_degrees_per_meter}')










        # Append the data as a dictionary to the list
        data_list.append({'Speed (m/s)': speed, 'Acceleration (m/s^2)': acceleration, 'Steering Angle' : steering_angle, 'Lateral Acceleration' : lateral_acceleration})

        previous_speed = speed  # Update the previous speed value
        previous_yaw = yaw  # Update the previous yaw angle value

        # Wait for 1 second before the next measurement
        time.sleep(1.0)

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

