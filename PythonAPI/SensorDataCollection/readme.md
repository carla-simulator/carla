# SensorDataCollection

## Description

This library encompasses functions to collect the following sensor data from a vehicle in Carla:
* [Speed](#speed)
* [Acceleration](#acceleration)
* [Lateral acceleration](#lateral-acceleration)
* [Steering angle](#steering-angle)
* [Distance to lane center](#distance-to-lane-center)
* [Curvature of road](#curvature-of-road)
* [Speed of vehicle ahead](#speed-of-vehicle-ahead)
* [Car detection matrix: street type detection + other car detection](#car-detection-matrix)


## Usage

### Quickstart with [example python notebook ](/PythonAPI/SensorDataCollection/car_detection_playground.ipynb): 
1. Package runterladen +  conda env python 3.8
2. carla library installieren: carla/Dist/CARLA_Shipping_f38eea6e-dirty/LinuxNoEditor/PythonAPI/carla/dist/carla-0.9.14-cp38-cp38-linux_x86_64.whl
3. Start Caral Server
4. Load map: python3 repos/carla/PythonAPI/util/config.py --map=Town04
5. Start manual Car: python3 repos/carla/PythonAPI/examples/manual_control.py

For more detailed installation guide refer to the [Carla ReadMe](https://github.com/carla-simulator/carla).
 

### General usage:
To use the data collection functions, simply import all functions from the utils.py script. Then, create a game loop and call the desired functions to get the sensor data for every tick. Some functions need to track additional information in each tick, which must be initialized in the beginning and updated in the end of the every iteration of the game loop.

See sub-sections for more details.

## Sensor Data Functions

### Speed

Function call: get_speed(ego_vehicle)
Return: 

### Acceleration

Function call: get_acceleration(speed, previous_speed, current_time, previous_time)
Return: 

### Lateral acceleration

Function call: get_lateral_acceleration(ego_vehicle, speed, previous_yaw, current_time, previous_time)
Return: 

### Steering angle

Function call: get_steering_angle(ego_vehicle) #TODO angle in degree or radians?
Return: 

### Distance to lane center

Function call: get_dist_to_lane_center(ego_vehicle, world) 
Return: 

### Curvature of road

Function call: get_curvature_at_location(ego_location, world) 
Return: 

### Speed of vehicle ahead

Function call: get_speed_of_vehicle_ahead(ego_waypoint, world) 
Return: 

### Car detection matrix
#### Parameters Description:
The following parameters are required for the function:

- **`params_car_detection`** (`Dictionary`):
  - **Purpose**: Stores parameters related to the car detection sensor.
  - **Details**:
    - This variable is imported alongside other utilities from `utils.py`.
    - It is an initialized dictionary, encompassing all required parameters specific to the car detection sensor.
    - The function updates and returns this dictionary after every call, allowing for the car detection matrix to be updated in the next tick based on the current state.

- **`ego_vehicle`** (`carla.Actor`):
  - **Purpose**: Represents the ego vehicle for which sensor data is collected.
  - **Details**:
    - It is an object of the ego vehicle, crucial for gathering relevant sensor data.

- **`ego_waypoint`** (`carla.Waypoint`):
  - **Purpose**: Waypoint object associated with the ego vehicle.
  - **Details**:
    - This object is used in the context of the ego vehicle for sensor data collection.

- **`ego_location`** (`carla.Location`):
  - **Purpose**: Location object of the ego vehicle.
  - **Details**:
    - Utilized for determining the current position of the ego vehicle in sensor data collection.

- **`world`** (`carla.World`):
  - **Purpose**: Represents the virtual simulator world.
  - **Details**:
    - This object encapsulates the entire virtual environment in which the ego vehicle operates.

#### Function Call:
Detailed information about the function call:

- **Function**: `get_car_detection_matrix`
- **Syntax**: get_car_detection_matrix(**params_car_detection, ego_vehicle=ego_vehicle, ego_waypoint=ego_waypoint, ego_location=ego_location, world=world)


#### Return Values

The function returns the following values:

- **Car Detection Matrix**:
    - **Type**: ordered Dictionary
    - **Description**: An ordered dictionary representing the new car detection matrix. The keys for existing lanes are the lane IDs in the format "road_id_lane_id". For non-existing lanes different placeholder exist, e.g.  left_outer_lane, left_inner_lane, No_4th_lane, No_opposing_direction. 
        - Base scenario: The matrix follows the ego vehicle such that the ego vehicle is always positioned in the middle column and the ego perspective is driving from left to right in the matrix. 
        - City Junction: The matrix has a static position covering the entire junction. Consequently the ego vehicle changes the column of the matrix when driving through the junction. 

    - **Format example**:
        ```
        {
            "left_outer_lane":  [3, 3, 3, 3, 3, 3, 3, 3],
            "left_inner_lane":  [3, 3, 3, 3, 3, 3, 3, 3],
            "1_2":              [0, 0, 0, 0, 0, 0, 2, 0],
            "1_1":              [0, 0, 0, 0, 0, 0, 0, 0],
            "1_-1":             [0, 0, 0, 1, 0, 0, 0, 0],
            "1_-2":             [0, 2, 0, 0, 0, 0, 0, 0],
            "right_inner_lane": [3, 3, 3, 0, 0, 3, 3, 3],
            "right_outer_lane": [3, 3, 3, 0, 2, 3, 3, 3]
        }

        The values indicate whether a vehicle is present:
        0 - No vehicle, 1 - Ego vehicle, 2 - other car, 3 - No road.
        ```

- **Updated Parameters for the Next Tick**:
    - **Type**: Dictionary
    - **Description**: This is an updated version of the `params_car_detection` dictionary. It includes modified values reflecting the current state, which will be used to update the car detection matrix in the next tick of the simulation.


