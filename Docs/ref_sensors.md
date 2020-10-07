# Sensors reference

*   [__Collision detector__](#collision-detector)
*   [__Depth camera__](#depth-camera)
*   [__GNSS sensor__](#gnss-sensor)
*   [__IMU sensor__](#imu-sensor)
*   [__Lane invasion detector__](#lane-invasion-detector)
*   [__LIDAR sensor__](#lidar-sensor)
*   [__Obstacle detector__](#obstacle-detector)
*   [__Radar sensor__](#radar-sensor)
*   [__RGB camera__](#rgb-camera)
*   [__RSS sensor__](#rss-sensor)
*   [__Semantic LIDAR sensor__](#semantic-lidar-sensor)
*   [__Semantic segmentation camera__](#semantic-segmentation-camera)
*   [__DVS camera__](#dvs-camera)

!!! Important
    All the sensors use the UE coordinate system (__x__-*forward*, __y__-*right*, __z__-*up*), and return coordinates in local space. When using any visualization software, pay attention to its coordinate system. Many invert the Y-axis, so visualizing the sensor data directly may result in mirrored outputs.  

---
## Collision detector

* __Blueprint:__ sensor.other.collision
* __Output:__ [carla.CollisionEvent](python_api.md#carla.CollisionEvent) per collision.

This sensor registers an event each time its parent actor collisions against something in the world. Several collisions may be detected during a single simulation step.
To ensure that collisions with any kind of object are detected, the server creates "fake" actors for elements such as buildings or bushes so the semantic tag can be retrieved to identify it.

Collision detectors do not have any configurable attribute.

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>actor</code></td>
<td><a href="../python_api#carlaactor">carla.Actor</a></td>
<td>Actor that measured the collision (sensor's parent).</td>
<tr>
<td><code>other_actor</code></td>
<td><a href="../python_api#carlaactor">carla.Actor</a></td>
<td>Actor against whom the parent collided.</td>
<tr>
<td><code>normal_impulse</code></td>
<td><a href="../python_api#carlavector3d">carla.Vector3D</a></td>
<td>Normal impulse result of the collision.</td>
</tbody>
</table>

---
## Depth camera

* __Blueprint:__ sensor.camera.depth
* __Output:__ [carla.Image](python_api.md#carla.Image) per step (unless `sensor_tick` says otherwise).

The camera provides a raw data of the scene codifying the distance of each pixel to the camera (also known as **depth buffer** or **z-buffer**) to create a depth map of the elements.

The image codifies depth value per pixel using 3 channels of the RGB color space, from less to more significant bytes: _R -> G -> B_. The actual distance in meters can be
decoded with:

```
normalized = (R + G * 256 + B * 256 * 256) / (256 * 256 * 256 - 1)
in_meters = 1000 * normalized
```

The output [carla.Image](python_api.md#carla.Image) should then be saved to disk using a [carla.colorConverter](python_api.md#carla.ColorConverter) that will turn the distance stored in RGB channels into a __[0,1]__ float containing the distance and then translate this to grayscale.
There are two options in [carla.colorConverter](python_api.md#carla.ColorConverter) to get a depth view: __Depth__ and __Logaritmic depth__. The precision is milimetric in both, but the logarithmic approach provides better results for closer objects.

```py
...
raw_image.save_to_disk("path/to/save/converted/image",carla.Depth)
```


![ImageDepth](img/ref_sensors_depth.jpg)


#### Basic camera attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>image_size_x</code> </td>
<td>int</td>
<td>800</td>
<td>Image width in pixels.</td>
<tr>
<td><code>image_size_y</code></td>
<td>int</td>
<td>600</td>
<td>Image height in pixels.</td>
<tr>
<td><code>fov</code></td>
<td>float</td>
<td>90.0</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
</tbody>
</table>
<br>

#### Camera lens distortion attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>lens_circle_falloff</code> </td>
<td>float</td>
<td>5.0</td>
<td>Range: [0.0, 10.0]</td>
<tr>
<td><code>lens_circle_multiplier</code></td>
<td>float</td>
<td>0.0</td>
<td>Range: [0.0, 10.0]</td>
<tr>
<td><code>lens_k</code></td>
<td>float</td>
<td>-1.0</td>
<td>Range: [-inf, inf]</td>
<tr>
<td><code>lens_kcube</code></td>
<td>float</td>
<td>0.0</td>
<td>Range: [-inf, inf]</td>
<tr>
<td><code>lens_x_size</code></td>
<td>float</td>
<td>0.08</td>
<td>Range: [0.0, 1.0]</td>
<tr>
<td><code>lens_y_size</code></td>
<td>float</td>
<td>0.08</td>
<td>Range: [0.0, 1.0]</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>width</code></td>
<td>int</td>
<td>Image width in pixels.</td>
<tr>
<td><code>height</code></td>
<td>int</td>
<td>Image height in pixels.</td>
<tr>
<td><code>fov</code></td>
<td>float</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>raw_data</code></td>
<td>bytes</td>
<td>Array of BGRA 32-bit pixels.</td>
</tbody>
</table>

---
## GNSS sensor

* __Blueprint:__ sensor.other.gnss
* __Output:__ [carla.GNSSMeasurement](python_api.md#carla.GnssMeasurement) per step (unless `sensor_tick` says otherwise).

Reports current [gnss position](https://www.gsa.europa.eu/european-gnss/what-gnss) of its parent object. This is calculated by adding the metric position to an initial geo reference location defined within the OpenDRIVE map definition.

#### GNSS attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>noise_alt_bias</code> </td>
<td>float</td>
<td>0.0</td>
<td>Mean parameter in the noise model for altitude.</td>
<tr>
<td><code>noise_alt_stddev</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for altitude.</td>
<tr>
<td><code>noise_lat_bias</code></td>
<td>float</td>
<td>0.0</td>
<td>Mean parameter in the noise model for latitude.</td>
<tr>
<td><code>noise_lat_stddev</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for latitude.</td>
<tr>
<td><code>noise_lon_bias</code></td>
<td>float</td>
<td>0.0</td>
<td>Mean parameter in the noise model for longitude.</td>
<tr>
<td><code>noise_lon_stddev</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for longitude.</td>
<tr>
<td><code>noise_seed</code></td>
<td>int</td>
<td>0</td>
<td>Initializer for a pseudorandom number generator.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>latitude</code></td>
<td>double</td>
<td>Latitude of the actor.</td>
<tr>
<td><code>longitude</code></td>
<td>double</td>
<td>Longitude of the actor.</td>
<tr>
<td><code>altitude</code></td>
<td>double</td>
<td>Altitude of the actor.</td>
</tbody>
</table>

---
## IMU sensor

* __Blueprint:__ sensor.other.imu
* __Output:__ [carla.IMUMeasurement](python_api.md#carla.IMUMeasurement) per step (unless `sensor_tick` says otherwise).

Provides measures that accelerometer, gyroscope and compass would retrieve for the parent object. The data is collected from the object's current state.

#### IMU attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>noise_accel_stddev_x</code> </td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for acceleration (X axis).</td>
<tr>
<td><code>noise_accel_stddev_y</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for acceleration (Y axis).</td>
<tr>
<td><code>noise_accel_stddev_z</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for acceleration (Z axis).</td>
<tr>
<td><code>noise_gyro_bias_x</code></td>
<td>float</td>
<td>0.0</td>
<td>Mean parameter in the noise model for the gyroscope (X axis).</td>
<tr>
<td><code>noise_gyro_bias_y</code></td>
<td>float</td>
<td>0.0</td>
<td>Mean parameter in the noise model for the gyroscope (Y axis).</td>
<tr>
<td><code>noise_gyro_bias_z</code></td>
<td>float</td>
<td>0.0</td>
<td>Mean parameter in the noise model for the gyroscope (Z axis).</td>
<tr>
<td><code>noise_gyro_stddev_x</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for the gyroscope (X axis).</td>
<tr>
<td><code>noise_gyro_stddev_y</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for the gyroscope (Y axis).</td>
<tr>
<td><code>noise_gyro_stddev_z</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation parameter in the noise model for the gyroscope (Z axis).</td>
<tr>
<td><code>noise_seed</code></td>
<td>int</td>
<td>0</td>
<td>Initializer for a pseudorandom number generator.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>accelerometer</code></td>
<td><a href="../python_api#carlavector3d">carla.Vector3D</a></td>
<td>Measures linear acceleration in <code>m/s^2</code>.</td>
<tr>
<td><code>gyroscope</code></td>
<td><a href="../python_api#carlavector3d">carla.Vector3D</a></td>
<td>Measures angular velocity in <code>rad/sec</code>.</td>
<tr>
<td><code>compass</code></td>
<td>float</td>
<td>Orientation in radians. North is <code>(0.0, -1.0, 0.0)</code> in UE.</td>
</tbody>
</table>

---
## Lane invasion detector

* __Blueprint:__ sensor.other.lane_invasion
* __Output:__ [carla.LaneInvasionEvent](python_api.md#carla.LaneInvasionEvent) per crossing.

Registers an event each time its parent crosses a lane marking.
The sensor uses road data provided by the OpenDRIVE description of the map to determine whether the parent vehicle is invading another lane by considering the space between wheels.
However there are some things to be taken into consideration:

* Discrepancies between the OpenDRIVE file and the map will create irregularities such as crossing lanes that are not visible in the map.
* The output retrieves a list of crossed lane markings: the computation is done in OpenDRIVE and considering the whole space between the four wheels as a whole. Thus, there may be more than one lane being crossed at the same time.

This sensor does not have any configurable attribute.

!!! Important
    This sensor works fully on the client-side.

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>actor</code></td>
<td><a href="../python_api#carlaactor">carla.Actor</a></td>
<td>Vehicle that invaded another lane (parent actor).</td>
<tr>
<td><code>crossed_lane_markings</code></td>
<td>list(<a href="../python_api#carlalanemarking">carla.LaneMarking</a>)</td>
<td>List of lane markings that have been crossed.</td>
</tbody>
</table>

---
## LIDAR sensor

* __Blueprint:__ sensor.lidar.ray_cast
* __Output:__ [carla.LidarMeasurement](python_api.md#carla.LidarMeasurement) per step (unless `sensor_tick` says otherwise).

This sensor simulates a rotating LIDAR implemented using ray-casting.
The points are computed by adding a laser for each channel distributed in the vertical FOV. The rotation is simulated computing the horizontal angle that the Lidar rotated in a frame. The point cloud is calculated by doing a ray-cast for each laser in every step.
`points_per_channel_each_step = points_per_second / (FPS * channels)`

A LIDAR measurement contains a package with all the points generated during a `1/FPS` interval. During this interval the physics are not updated so all the points in a measurement reflect the same "static picture" of the scene.

This output contains a cloud of simulation points and thus, it can be iterated to retrieve a list of their [`carla.Location`](python_api.md#carla.Location):

```py
for location in lidar_measurement:
    print(location)
```

The information of the LIDAR measurement is enconded 4D points. Being the first three, the space points in xyz coordinates and the last one intensity loss during the travel. This intensity is computed by the following formula.
<br>
![LidarIntensityComputation](img/lidar_intensity.jpg)

`a` — Attenuation coefficient. This may depend on the sensor's wavelenght, and the conditions of the atmosphere. It can be modified with the LIDAR attribute `atmosphere_attenuation_rate`.
`d` — Distance from the hit point to the sensor.

For a better realism, points in the cloud can be dropped off. This is an easy way to simulate loss due to external perturbations. This can done combining two different.

*   __General drop-off__ — Proportion of points that are dropped off randomly. This is done before the tracing, meaning the points being dropped are not calculated, and therefore improves the performance. If `dropoff_general_rate = 0.5`, half of the points will be dropped.
*   __Instensity-based drop-off__ — For each point detected, and extra drop-off is performed with a probability based in the computed intensity. This probability is determined by two parameters. `dropoff_zero_intensity` is the probability of points with zero intensity to be dropped. `dropoff_intensity_limit` is a threshold intensity above which no points will be dropped. The probability of a point within the range to be dropped is a linear proportion based on these two parameters.

Additionally, the `noise_stddev` attribute makes for a noise model to simulate unexpected deviations that appear in real-life sensors. For positive values, each point is randomly perturbed along the vector of the laser ray. The result is a LIDAR sensor with perfect angular positioning, but noisy distance measurement.

The rotation of the LIDAR can be tuned to cover a specific angle on every simulation step (using a [fixed time-step](adv_synchrony_timestep.md)). For example, to rotate once per step (full circle output, as in the picture below), the rotation frequency and the simulated FPS should be equal. <br> __1.__ Set the sensor's frequency `sensors_bp['lidar'][0].set_attribute('rotation_frequency','10')`. <br> __2.__ Run the simulation using `python3 config.py --fps=10`.

![LidarPointCloud](img/lidar_point_cloud.jpg)

#### Lidar attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>channels</code> </td>
<td>int</td>
<td>32</td>
<td>Number of lasers.</td>
<tr>
<td><code>range</code></td>
<td>float</td>
<td>10.0</td>
<td>Maximum distance to measure/raycast in meters (centimeters for CARLA 0.9.6 or previous).</td>
<tr>
<td><code>points_per_second</code></td>
<td>int</td>
<td>56000</td>
<td>Points generated by all lasers per second.</td>
<tr>
<td><code>rotation_frequency</code></td>
<td>float</td>
<td>10.0</td>
<td>LIDAR rotation frequency.</td>
<tr>
<td><code>upper_fov</code></td>
<td>float</td>
<td>10.0</td>
<td>Angle in degrees of the highest laser.</td>
<tr>
<td><code>lower_fov</code></td>
<td>float</td>
<td>-30.0</td>
<td>Angle in degrees of the lowest laser.</td>
<tr>
<td><code>atmosphere_attenuation_rate</code></td>
<td>float</td>
<td>0.004</td>
<td>Coefficient that measures the LIDAR instensity loss per meter. Check the intensity computation above.</td>
<tr>
<td><code>dropoff_general_rate</code></td>
<td>float</td>
<td>0.45</td>
<td>General proportion of points that are randomy dropped.</td>
<tr>
<td><code>dropoff_intensity_limit</code></td>
<td>float</td>
<td>0.8</td>
<td>For the intensity based drop-off, the threshold intensity value above which no points are dropped.</td>
<tr>
<td><code>dropoff_zero_intensity</code></td>
<td>float</td>
<td>0.4</td>
<td>For the intensity based drop-off, the probability of each point with zero intensity being dropped.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
<tr>
<td><code>noise_stddev</code></td>
<td>float</td>
<td>0.0</td>
<td>Standard deviation of the noise model to disturb each point along the vector of its raycast.</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>horizontal_angle</code></td>
<td>float</td>
<td>Angle (radians) in the XY plane of the LIDAR in the current frame.</td>
<tr>
<td><code>channels</code></td>
<td>int</td>
<td>Number of channels (lasers) of the LIDAR.</td>
<tr>
<td><code>get_point_count(channel)</code></td>
<td>int</td>
<td>Number of points per channel captured this frame.</td>
<tr>
<td><code>raw_data</code></td>
<td>bytes</td>
<td>Array of 32-bits floats (XYZI of each point).</td>
</tbody>
</table>

---
## Obstacle detector

* __Blueprint:__ sensor.other.obstacle
* __Output:__ [carla.ObstacleDetectionEvent](python_api.md#carla.ObstacleDetectionEvent) per obstacle (unless `sensor_tick` says otherwise).

Registers an event every time the parent actor has an obstacle ahead.
In order to anticipate obstacles, the sensor creates a capsular shape ahead of the parent vehicle and uses it to check for collisions.
To ensure that collisions with any kind of object are detected, the server creates "fake" actors for elements such as buildings or bushes so the semantic tag can be retrieved to identify it.

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>distance</code> </td>
<td>float</td>
<td>5</td>
<td>Distance to trace.</td>
<tr>
<td><code>hit_radius</code></td>
<td>float</td>
<td>0.5</td>
<td>Radius of the trace.</td>
<tr>
<td><code>only_dynamics</code></td>
<td>bool</td>
<td>False</td>
<td>If true, the trace will only consider dynamic objects.</td>
<tr>
<td><code>debug_linetrace</code></td>
<td>bool</td>
<td>False</td>
<td>If true, the trace will be visible.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>actor</code></td>
<td><a href="../python_api#carlaactor">carla.Actor</a></td>
<td>Actor that detected the obstacle (parent actor).</td>
<tr>
<td><code>other_actor</code></td>
<td><a href="../python_api#carlaactor">carla.Actor</a></td>
<td>Actor detected as an obstacle.</td>
<tr>
<td><code>distance</code></td>
<td>float</td>
<td>Distance from <code>actor</code> to <code>other_actor</code>.</td>
</tbody>
</table>

---
## Radar sensor

* __Blueprint:__ sensor.other.radar
* __Output:__ [carla.RadarMeasurement](python_api.md#carla.RadarMeasurement) per step (unless `sensor_tick` says otherwise).

The sensor creates a conic view that is translated to a 2D point map of the elements in sight and their speed regarding the sensor. This can be used to shape elements and evaluate their movement and direction. Due to the use of polar coordinates, the points will concentrate around the center of the view.

Points measured are contained in [carla.RadarMeasurement](python_api.md#carla.RadarMeasurement) as an array of [carla.RadarDetection](python_api.md#carla.RadarDetection), which specifies their polar coordinates, distance and velocity.
This raw data provided by the radar sensor can be easily converted to a format manageable by __numpy__:
```py
# To get a numpy [[vel, altitude, azimuth, depth],...[,,,]]:
points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
points = np.reshape(points, (len(radar_data), 4))
```

The provided script `manual_control.py` uses this sensor to show the points being detected and paint them white when static, red when moving towards the object and blue when moving away:

![ImageRadar](img/ref_sensors_radar.jpg)

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>horizontal_fov</code> </td>
<td>float</td>
<td>30.0</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>points_per_second</code></td>
<td>int</td>
<td>1500</td>
<td>Points generated by all lasers per second.</td>
<tr>
<td><code>range</code></td>
<td>float</td>
<td>100</td>
<td>Maximum distance to measure/raycast in meters.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
<tr>
<td><code>vertical_fov</code></td>
<td>float</td>
<td>30.0</td>
<td>Vertical field of view in degrees.</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>raw_data</code> </td>
<td><a href="../python_api#carlaradardetection">carla.RadarDetection</a></td>
<td>The list of points detected.</td>
</tbody>
</table>
<br>

<table class ="defTable">
<thead>
<th>RadarDetection attributes</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>altitude</code> </td>
<td>float</td>
<td>Altitude angle in radians.</td>
<tr>
<td><code>azimuth</code></td>
<td>float</td>
<td>Azimuth angle in radians.</td>
<tr>
<td><code>depth</code></td>
<td>float</td>
<td>Distance in meters.</td>
<tr>
<td><code>velocity</code></td>
<td>float</td>
<td>Velocity towards the sensor.</td>
</tbody>
</table>

---
## RGB camera

* __Blueprint:__ sensor.camera.rgb
* __Output:__ [carla.Image](python_api.md#carla.Image) per step (unless `sensor_tick` says otherwise)..

The "RGB" camera acts as a regular camera capturing images from the scene.
[carla.colorConverter](python_api.md#carla.ColorConverter)

If `enable_postprocess_effects` is enabled, a set of post-process effects is applied to the image for the sake of realism:

* __Vignette:__ darkens the border of the screen.
* __Grain jitter:__ adds some noise to the render.
* __Bloom:__ intense lights burn the area around them.
* __Auto exposure:__ modifies the image gamma to simulate the eye adaptation to darker or brighter areas.
* __Lens flares:__ simulates the reflection of bright objects on the lens.
* __Depth of field:__ blurs objects near or very far away of the camera.


The `sensor_tick` tells how fast we want the sensor to capture the data.
A value of 1.5 means that we want the sensor to capture data each second and a half. By default a value of 0.0 means as fast as possible.

![ImageRGB](img/ref_sensors_rgb.jpg)

#### Basic camera attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>bloom_intensity</code> </td>
<td>float</td>
<td>0.675</td>
<td>Intensity for the bloom post-process effect, <code>0.0</code> for disabling it.</td>
<tr>
<td><code>fov</code> </td>
<td>float</td>
<td>90.0</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>fstop</code></td>
<td>float</td>
<td>1.4</td>
<td>Opening of the camera lens. Aperture is <code>1/fstop</code> with typical lens going down to f/1.2 (larger opening). Larger numbers will reduce the Depth of Field effect.</td>
<tr>
<td><code>image_size_x</code></td>
<td>int</td>
<td>800</td>
<td>Image width in pixels.</td>
<tr>
<td><code>image_size_y</code></td>
<td>int</td>
<td>600</td>
<td>Image height in pixels.</td>
<tr>
<td><code>iso</code></td>
<td>float</td>
<td>100.0</td>
<td>The camera sensor sensitivity.</td>
<tr>
<td><code>gamma</code></td>
<td>float</td>
<td>2.2</td>
<td>Target gamma value of the camera.</td>
<tr>
<td><code>lens_flare_intensity</code></td>
<td>float</td>
<td>0.1</td>
<td>Intensity for the lens flare post-process effect, <code>0.0</code> for disabling it.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
<tr>
<td><code>shutter_speed</code></td>
<td>float</td>
<td>200.0</td>
<td>The camera shutter speed in seconds (1.0/s).</td>
</tbody>
</table>
<br>

#### Camera lens distortion attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>lens_circle_falloff</code> </td>
<td>float</td>
<td>5.0</td>
<td>Range: [0.0, 10.0]</td>
<tr>
<td><code>lens_circle_multiplier</code></td>
<td>float</td>
<td>0.0</td>
<td>Range: [0.0, 10.0]</td>
<tr>
<td><code>lens_k</code></td>
<td>float</td>
<td>-1.0</td>
<td>Range: [-inf, inf]</td>
<tr>
<td><code>lens_kcube</code></td>
<td>float</td>
<td>0.0</td>
<td>Range: [-inf, inf]</td>
<tr>
<td><code>lens_x_size</code></td>
<td>float</td>
<td>0.08</td>
<td>Range: [0.0, 1.0]</td>
<tr>
<td><code>lens_y_size</code></td>
<td>float</td>
<td>0.08</td>
<td>Range: [0.0, 1.0]</td>
</tbody>
</table>

#### Advanced camera attributes

Since these effects are provided by UE, please make sure to check their documentation:

  * [Automatic Exposure][AutomaticExposure.Docs]
  * [Cinematic Depth of Field Method][CinematicDOFMethod.Docs]
  * [Color Grading and Filmic Tonemapper][ColorGrading.Docs]

[AutomaticExposure.Docs]: https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/AutomaticExposure/index.html
[CinematicDOFMethod.Docs]: https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/DepthOfField/CinematicDOFMethods/index.html
[ColorGrading.Docs]: https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/ColorGrading/index.html

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td><code>min_fstop</code> </td>
<td>float</td>
<td>1.2</td>
<td>Maximum aperture.</td>
<tr>
<td><code>blade_count</code> </td>
<td>int</td>
<td>5</td>
<td>Number of blades that make up the diaphragm mechanism.</td>
<tr>
<td><code>exposure_mode</code> </td>
<td>str</td>
<td><code>histogram</code></td>
<td>Can be <code>manual</code> or <code>histogram</code>. More in <a href="https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/AutomaticExposure/index.html">UE4 docs</a>.</td>
<tr>
<td><code>exposure_compensation</code> </td>
<td>float</td>
<td><b>Linux:</b> -1.5<br><b>Windows:</b> 0.0</td>
<td>Logarithmic adjustment for the exposure. 0: no adjustment, -1:2x darker, -2:4 darker, 1:2x brighter, 2:4x brighter.</td>
<tr>
<td><code>exposure_min_bright</code> </td>
<td>float</td>
<td>7.0</td>
<td>In <code>exposure_mode: "histogram"</code>. Minimum brightness for auto exposure. The lowest the eye can adapt within. Must be greater than 0 and less than or equal to <code>exposure_max_bright</code>.</td>
<tr>
<td><code>exposure_max_bright</code> </td>
<td>float</td>
<td>9.0</td>
<td>In `exposure_mode: "histogram"`. Maximum brightness for auto exposure. The highestthe eye can adapt within. Must be greater than 0 and greater than or equal to `exposure_min_bright`.</td>
<tr>
<td><code>exposure_speed_up</code> </td>
<td>float</td>
<td>3.0</td>
<td>In <code>exposure_mode: "histogram"</code>. Speed at which the adaptation occurs from dark to bright environment.</td>
<tr>
<td><code>exposure_speed_down</code> </td>
<td>float</td>
<td>1.0</td>
<td>In <code>exposure_mode: "histogram"</code>. Speed at which the adaptation occurs from bright to dark environment.</td>
<tr>
<td><code>calibration_constant</code> </td>
<td>float</td>
<td>16.0</td>
<td>Calibration constant for 18% albedo.</td>
<tr>
<td><code>focal_distance</code> </td>
<td>float</td>
<td>1000.0</td>
<td>Distance at which the depth of field effect should be sharp. Measured in cm (UE units).</td>
<tr>
<td><code>blur_amount</code> </td>
<td>float</td>
<td>1.0</td>
<td>Strength/intensity of motion blur.</td>
<tr>
<td><code>blur_radius</code> </td>
<td>float</td>
<td>0.0</td>
<td>Radius in pixels at 1080p resolution to emulate atmospheric scattering according to distance from camera.</td>
<tr>
<td><code>motion_blur_intensity</code> </td>
<td>float</td>
<td>0.45</td>
<td>Strength of motion blur [0,1].</td>
<tr>
<td><code>motion_blur_max_distortion</code> </td>
<td>float</td>
<td>0.35</td>
<td>Max distortion caused by motion blur. Percentage of screen width.</td>
<tr>
<td><code>motion_blur_min_object_screen_size</code> </td>
<td>float</td>
<td>0.1</td>
<td>Percentage of screen width objects must have for motion blur, lower value means less draw calls.</td>
<tr>
<td><code>slope</code> </td>
<td>float</td>
<td>0.88</td>
<td>Steepness of the S-curve for the tonemapper. Larger values make the slope steeper (darker) [0.0, 1.0].</td>
<tr>
<td><code>toe</code> </td>
<td>float</td>
<td>0.55</td>
<td>Adjusts dark color in the tonemapper [0.0, 1.0].</td>
<tr>
<td><code>shoulder</code> </td>
<td>float</td>
<td>0.26</td>
<td>Adjusts bright color in the tonemapper [0.0, 1.0].</td>
<tr>
<td><code>black_clip</code> </td>
<td>float</td>
<td>0.0</td>
<td>This should NOT be adjusted. Sets where the crossover happens and black tones start to cut off their value [0.0, 1.0].</td>
<tr>
<td><code>white_clip</code> </td>
<td>float</td>
<td>0.04</td>
<td>Set where the crossover happens and white tones start to cut off their value. Subtle change in most cases [0.0, 1.0].</td>
<tr>
<td><code>temp</code></td>
<td>float</td>
<td>6500.0</td>
<td>White balance in relation to the temperature of the light in the scene. <b>White light:</b> when this matches light temperature. <b>Warm light:</b> When higher than the light in the scene, it is a yellowish color. <b>Cool light:</b> When lower than the light. Blueish color.</td>
<tr>
<td><code>tint</code> </td>
<td>float</td>
<td>0.0</td>
<td>White balance temperature tint. Adjusts cyan and magenta color ranges. This should be used along with the white balance Temp property to get accurate colors. Under some light temperatures, the colors may appear to be more yellow or blue. This can be used to balance the resulting color to look more natural.</td>
<tr>
<td><code>chromatic_aberration_intensity</code> </td>
<td>float</td>
<td>0.0</td>
<td>Scaling factor to control color shifting, more noticeable on the screen borders.</td>
<tr>
<td><code>chromatic_aberration_offset</code> </td>
<td>float</td>
<td>0.0</td>
<td>Normalized distance to the center of the image where the effect takes place.</td>
<tr>
<td><code>enable_postprocess_effects</code> </td>
<td>bool</td>
<td>True</td>
<td>Post-process effects activation.</td>
</tbody>
</table>
<br>

[AutomaticExposure.gamesetting]: https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/AutomaticExposure/index.html#gamesetting

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>width</code></td>
<td>int</td>
<td>Image width in pixels.</td>
<tr>
<td><code>height</code></td>
<td>int</td>
<td>Image height in pixels.</td>
<tr>
<td><code>fov</code></td>
<td>float</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>raw_data</code></td>
<td>bytes</td>
<td>Array of BGRA 32-bit pixels.</td>
</tbody>
</table>
<br>

---
## RSS sensor

*   __Blueprint:__ sensor.other.rss
*   __Output:__ [carla.RssResponse](python_api.md#carla.RssResponse) per step (unless `sensor_tick` says otherwise).

!!! Important
    It is highly recommended to read the specific [rss documentation](adv_rss.md) before reading this.

This sensor integrates the [C++ Library for Responsibility Sensitive Safety](https://github.com/intel/ad-rss-lib) in CARLA. It is disabled by default in CARLA, and it has to be explicitly built in order to be used.

The RSS sensor calculates the RSS state of a vehicle and retrieves the current RSS Response as sensor data. The [carla.RssRestrictor](python_api.md#carla.RssRestrictor) will use this data to adapt a [carla.VehicleControl](python_api.md#carla.VehicleControl) before applying it to a vehicle.

These controllers can be generated by an *Automated Driving* stack or user input. For instance, hereunder there is a fragment of code from `PythonAPI/examples/rss/manual_control_rss.py`, where the user input is modified using RSS when necessary.

__1.__ Checks if the __RssSensor__ generates a valid response containing restrictions.
__2.__ Gathers the current dynamics of the vehicle and the vehicle physics.
__3.__ Applies restrictions to the vehicle control using the response from the RssSensor, and the current dynamics and physicis of the vehicle.

```py
rss_proper_response = self._world.rss_sensor.proper_response if self._world.rss_sensor and self._world.rss_sensor.response_valid else None
if rss_proper_response:
...
        vehicle_control = self._restrictor.restrict_vehicle_control(
            vehicle_control, rss_proper_response, self._world.rss_sensor.ego_dynamics_on_route, self._vehicle_physics)
```


#### The carla.RssSensor class

The blueprint for this sensor has no modifiable attributes. However, the [carla.RssSensor](python_api.md#carla.RssSensor) object that it instantiates has attributes and methods that are detailed in the Python API reference. Here is a summary of them.

<table class ="defTable">
<thead>
<th><a href="../python_api#carlarsssensor">carla.RssSensor variables</a></th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>ego_vehicle_dynamics</code> </td>
<td><a href="https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/">ad.rss.world.RssDynamics</a></td>
<td>RSS parameters to be applied for the ego vehicle </td>
<tr>
<td><code>other_vehicle_dynamics</code> </td>
<td><a href="https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/">ad.rss.world.RssDynamics</a></td>
<td>RSS parameters to be applied for the other vehicles</td>
<tr>
<td><code>pedestrian_dynamics</code> </td>
<td><a href="https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/">ad.rss.world.RssDynamics</a></td>
<td>RSS parameters to be applied for pedestrians</td>
<tr>
<td><code>road_boundaries_mode</code></td>
<td><a href="../python_api#carlarssroadboundariesmode">carla.RssRoadBoundariesMode</a></td>
<td>Enables/Disables the <a href="https://intel.github.io/ad-rss-lib/ad_rss_map_integration/HandleRoadBoundaries">stay on road</a> feature. Default is <b>Off</b>.</td>
</table>
<br>

```py
# Fragment of rss_sensor.py
# The carla.RssSensor is updated when listening for a new carla.RssResponse
def _on_rss_response(weak_self, response):
...
        self.timestamp = response.timestamp
        self.response_valid = response.response_valid
        self.proper_response = response.proper_response
        self.ego_dynamics_on_route = response.ego_dynamics_on_route
        self.rss_state_snapshot = response.rss_state_snapshot
        self.situation_snapshot = response.situation_snapshot
        self.world_model = response.world_model
```

!!! Warning
    This sensor works fully on the client side. There is no blueprint in the server. Changes on the attributes will have effect __after__ the *listen()* has been called.

The methods available in this class are related to the routing of the vehicle. RSS calculations are always based on a route of the ego vehicle through the road network.

The sensor allows to control the considered route by providing some key points, which could be the [carla.Transform](python_api.md#carla.Transform) in a [carla.Waypoint](python_api.md#carla.Waypoint). These points are best selected after the intersections to force the route to take the desired turn.

<table class ="defTable">
<thead>
<th><a href="../python_api#carlarsssensor">carla.RssSensor methods</a></th>
<th>Description</th>
</thead>
<tbody>
<tr>
<td><code>routing_targets</code></td>
<td>Get the current list of routing targets used for route.</td>
<tr>
<td><code>append_routing_target</code></td>
<td>Append an additional position to the current routing targets.</td>
<tr>
<td><code>reset_routing_targets</code></td>
<td>Deletes the appended routing targets.</td>
<tr>
<td><code>drop_route</code></td>
<td>Discards the current route and creates a new one.</td>
<tr>
<td><code>register_actor_constellation_callback</code></td>
<td>Register a callback to customize the calculations.</td>
<tr>
<td><code>set_log_level</code></td>
<td>Sets the log level.</td>
<tr>
<td><code>set_map_log_level</code></td>
<td>Sets the log level used for map related logs.</td>
</table>
<br>

```py
# Update the current route
self.sensor.reset_routing_targets()
if routing_targets:
    for target in routing_targets:
        self.sensor.append_routing_target(target)
```

!!! Note
    If no routing targets are defined, a random route is created.

#### Output attributes

<table class ="defTable">
<thead>
<th><a href="../python_api#carlarssresponse">carla.RssResponse attributes</a></th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>response_valid</code></td>
<td>bool</td>
<td>Validity of the response data.</td>
<tr>
<td><code>proper_response</code> </td>
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1ProperResponse.html">ad.rss.state.ProperResponse</a></td>
<td>Proper response that the RSS calculated for the vehicle including acceleration restrictions.</td>
<tr>
<td><code>rss_state_snapshot</code></td>
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1RssStateSnapshot.html">ad.rss.state.RssStateSnapshot</a></td>
<td>RSS states at the current point in time. This is the detailed individual output of the RSS calclulations.</td>
<tr>
<td><code>situation_snapshot</code></td>
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1situation_1_1SituationSnapshot.html">ad.rss.situation.SituationSnapshot</a></td>
<td>RSS situation at the current point in time. This is the processed input data for the RSS calclulations.</td>
<tr>
<td><code>world_model</code></td>
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1WorldModel.html">ad.rss.world.WorldModel</a></td>
<td>RSS world model at the current point in time. This is the input data for the RSS calculations.</td>
<tr>
<td><code>ego_dynamics_on_route</code></td>
<td><a href="../python_api#carlarssegodynamicsonroute">carla.RssEgoDynamicsOnRoute</a></td>
<td>Current ego vehicle dynamics regarding the route.</td>
</tbody>
</table>

In case a actor_constellation_callback is registered, a call is triggered for:

1. default calculation (`actor_constellation_data.other_actor=None`)
2. per-actor calculation

```py
# Fragment of rss_sensor.py
# The function is registered as actor_constellation_callback
def _on_actor_constellation_request(self, actor_constellation_data):
    actor_constellation_result = carla.RssActorConstellationResult()
    actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.NotRelevant
    actor_constellation_result.restrict_speed_limit_mode = ad.rss.map.RssSceneCreation.RestrictSpeedLimitMode.IncreasedSpeedLimit10
    actor_constellation_result.ego_vehicle_dynamics = self.current_vehicle_parameters
    actor_constellation_result.actor_object_type = ad.rss.world.ObjectType.Invalid
    actor_constellation_result.actor_dynamics = self.current_vehicle_parameters

    actor_id = -1
    actor_type_id = "none"
    if actor_constellation_data.other_actor != None:
        # customize actor_constellation_result for specific actor
        ...
    else:
        # default
        ...
    return actor_constellation_result
```


---
## Semantic LIDAR sensor

* __Blueprint:__ sensor.lidar.ray_cast_semantic
* __Output:__ [carla.SemanticLidarMeasurement](python_api.md#carla.SemanticLidarMeasurement) per step (unless `sensor_tick` says otherwise).

This sensor simulates a rotating LIDAR implemented using ray-casting that exposes all the information about the raycast hit. Its behaviour is quite similar to the [LIDAR sensor](#lidar-sensor), but there are two main differences between them.

*   The raw data retrieved by the semantic LIDAR includes more data per point.
	*   Coordinates of the point (as the normal LIDAR does).
	*   The cosine between the angle of incidence and the normal of the surface hit.
	*   Instance and semantic ground-truth. Basically the index of the CARLA object hit, and its semantic tag.
*   The semantic LIDAR does not include neither intensity, drop-off nor noise model attributes.

The points are computed by adding a laser for each channel distributed in the vertical FOV. The rotation is simulated computing the horizontal angle that the LIDAR rotated in a frame. The point cloud is calculated by doing a ray-cast for each laser in every step.
```sh
points_per_channel_each_step = points_per_second / (FPS * channels)
```

A LIDAR measurement contains a package with all the points generated during a `1/FPS` interval. During this interval the physics are not updated so all the points in a measurement reflect the same "static picture" of the scene.

This output contains a cloud of lidar semantic detections and therefore, it can be iterated to retrieve a list of their [`carla.SemanticLidarDetection`](python_api.md#carla.SemanticLidarDetection):

```py
for detection in semantic_lidar_measurement:
    print(detection)
```

The rotation of the LIDAR can be tuned to cover a specific angle on every simulation step (using a [fixed time-step](adv_synchrony_timestep.md)). For example, to rotate once per step (full circle output, as in the picture below), the rotation frequency and the simulated FPS should be equal. <br>
__1.__ Set the sensor's frequency `sensors_bp['lidar'][0].set_attribute('rotation_frequency','10')`. <br>
__2.__ Run the simulation using `python3 config.py --fps=10`.

![LidarPointCloud](img/semantic_lidar_point_cloud.jpg)

#### SemanticLidar attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>channels</code> </td>
<td>int</td>
<td>32</td>
<td>Number of lasers.</td>
<tr>
<td><code>range</code></td>
<td>float</td>
<td>10.0</td>
<td>Maximum distance to measure/raycast in meters (centimeters for CARLA 0.9.6 or previous).</td>
<tr>
<td><code>points_per_second</code></td>
<td>int</td>
<td>56000</td>
<td>Points generated by all lasers per second.</td>
<tr>
<td><code>rotation_frequency</code></td>
<td>float</td>
<td>10.0</td>
<td>LIDAR rotation frequency.</td>
<tr>
<td><code>upper_fov</code></td>
<td>float</td>
<td>10.0</td>
<td>Angle in degrees of the highest laser.</td>
<tr>
<td><code>lower_fov</code></td>
<td>float</td>
<td>-30.0</td>
<td>Angle in degrees of the lowest laser.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>horizontal_angle</code></td>
<td>float</td>
<td>Angle (radians) in the XY plane of the LIDAR in the current frame.</td>
<tr>
<td><code>channels</code></td>
<td>int</td>
<td>Number of channels (lasers) of the LIDAR.</td>
<tr>
<td><code>get_point_count(channel)</code></td>
<td>int</td>
<td>Number of points per channel captured in the current frame.</td>
<tr>
<td><code>raw_data</code></td>
<td>bytes</td>
<td>Array containing the point cloud with instance and semantic information. For each point, four 32-bits floats are stored. <br>
- XYZ coordinates. <br>
- cosine of the incident angle. <br>
- Unsigned int containing the index of the object hit.  <br>
- Unsigned int containing the semantic tag of the object it.
</tbody>
</table>

---
## Semantic segmentation camera

*   __Blueprint:__ sensor.camera.semantic_segmentation
*   __Output:__ [carla.Image](python_api.md#carla.Image) per step (unless `sensor_tick` says otherwise).

This camera classifies every object in sight by displaying it in a different color according to its tags (e.g., pedestrians in a different color than vehicles).
When the simulation starts, every element in scene is created with a tag. So it happens when an actor is spawned. The objects are classified by their relative file path in the project. For example, meshes stored in `Unreal/CarlaUE4/Content/Static/Pedestrians` are tagged as `Pedestrian`.

![ImageSemanticSegmentation](img/ref_sensors_semantic.jpg)

The server provides an image with the tag information __encoded in the red channel__: A pixel with a red value of `x` belongs to an object with tag `x`.
This raw [carla.Image](python_api.md#carla.Image) can be stored and converted it with the help of __CityScapesPalette__  in [carla.ColorConverter](python_api.md#carla.ColorConverter) to apply the tags information and show picture with the semantic segmentation.

```py
...
raw_image.save_to_disk("path/to/save/converted/image",carla.cityScapesPalette)
```

The following tags are currently available:

<table class ="defTable">
<thead>
<th>Value</th>
<th>Tag</th>
<th>Converted color</th>
<th>Description</th>
</thead>
<tbody>
<td><code>0</code> </td>
<td>Unlabeled</td>
<td><code>(0, 0, 0)</code></td>
<td>Elements that have not been categorized are considered <code>Unlabeled</code>. This category is meant to be empty or at least contain elements with no collisions.</td>
<tr>
<td><code>1</code> </td>
<td>Building</td>
<td><code>(70, 70, 70)</code></td>
<td>Buildings like houses, skyscrapers,... and the elements attached to them. <br> E.g. air conditioners, scaffolding, awning or ladders and much more.</td>
<tr>
<td><code>2</code> </td>
<td>Fence</td>
<td><code>(100, 40, 40)</code></td>
<td>Barriers, railing, or other upright structures. Basically wood or wire assemblies that enclose an area of ground.</td>
<tr>
<td><code>3</code> </td>
<td>Other</td>
<td><code>(55, 90, 80)</code></td>
<td> Everything that does not belong to any other category.</td>
<tr>
<td><code>4</code> </td>
<td>Pedestrian</td>
<td><code>(220,  20,  60)</code></td>
<td>Humans that walk or ride/drive any kind of vehicle or mobility system. <br> E.g. bicycles or scooters, skateboards, horses, roller-blades, wheel-chairs, etc.</td>
<tr>
<td><code>5</code> </td>
<td>Pole</td>
<td><code>(153, 153, 153)</code></td>
<td>Small mainly vertically oriented pole. If the pole has a horizontal part (often for traffic light poles) this is also considered pole. <br> E.g. sign pole, traffic light poles.</td>
<tr>
<td><code>6</code> </td>
<td>RoadLine</td>
<td><code>(157, 234, 50)</code></td>
<td>The markings on the road.</td>
<tr>
<td><code>7</code> </td>
<td>Road</td>
<td><code>(128, 64, 128)</code></td>
<td>Part of ground on which cars usually drive. <br> E.g. lanes in any directions, and streets.</td>
<tr>
<td><code>8</code> </td>
<td>SideWalk</td>
<td><code>(244, 35, 232)</code></td>
<td>Part of ground designated for pedestrians or cyclists. Delimited from the road by some obstacle (such as curbs or poles), not only by markings. This label includes a possibly delimiting curb, traffic islands (the walkable part), and pedestrian zones.</td>
<tr>
<td><code>9</code> </td>
<td>Vegetation</td>
<td><code>(107, 142, 35)</code></td>
<td> Trees, hedges, all kinds of vertical vegetation. Ground-level vegetation is considered <code>Terrain</code>.</td>
<tr>
<td><code>10</code> </td>
<td>Vehicles</td>
<td><code>(0, 0, 142)</code></td>
<td>Cars, vans, trucks, motorcycles, bikes, buses, trains.</td>
<tr>
<td><code>11</code> </td>
<td>Wall</td>
<td><code>(102, 102, 156)</code></td>
<td>Individual standing walls. Not part of a building.</td>
<tr>
<td><code>12</code> </td>
<td>TrafficSign</td>
<td><code>(220, 220, 0)</code></td>
<td>Signs installed by the state/city authority, usually for traffic regulation. This category does not include the poles where signs are attached to. <br> E.g. traffic- signs, parking signs, direction signs...</td>
<tr>
<td><code>13</code> </td>
<td>Sky</td>
<td><code>(70, 130, 180)</code></td>
<td>Open sky. Includes clouds and the sun.</td>
<tr>
<td><code>14</code> </td>
<td>Ground</td>
<td><code>(81, 0, 81)</code></td>
<td>Any horizontal ground-level structures that does not match any other category. For example areas shared by vehicles and pedestrians, or flat roundabouts delimited from the road by a curb.</td>
<tr>
<td><code>15</code> </td>
<td>Bridge</td>
<td><code>(150, 100, 100)</code></td>
<td>Only the structure of the bridge. Fences, people, vehicles, an other elements on top of it are labeled separately.</td>
<tr>
<td><code>16</code> </td>
<td>RailTrack</td>
<td><code>(230, 150, 140)</code></td>
<td>All kind of rail tracks that are non-drivable by cars. <br> E.g. subway and train rail tracks.</td>
<tr>
<td><code>17</code> </td>
<td>GuardRail</td>
<td><code>(180, 165, 180)</code></td>
<td>All types of guard rails/crash barriers.</td>
<tr>
<td><code>18</code> </td>
<td>TrafficLight</td>
<td><code>(250, 170, 30)</code></td>
<td>Traffic light boxes without their poles.</td>
<tr>
<td><code>19</code> </td>
<td>Static</td>
<td><code>(110, 190, 160)</code></td>
<td>Elements in the scene and props that are immovable. <br> E.g. fire hydrants, fixed benches, fountains, bus stops, etc.</td>
<tr>
<td><code>20</code> </td>
<td>Dynamic</td>
<td><code>(170, 120, 50)</code></td>
<td>Elements whose position is susceptible to change over time. <br> E.g. Movable trash bins, buggies, bags, wheelchairs, animals, etc.</td>
<tr>
<td><code>21</code> </td>
<td>Water</td>
<td><code>(45, 60, 150)</code></td>
<td>Horizontal water surfaces. <br> E.g. Lakes, sea, rivers.</td>
<tr>
<td><code>22</code> </td>
<td>Terrain</td>
<td><code>(145, 170, 100)</code></td>
<td>Grass, ground-level vegetation, soil or sand. These areas are not meant to be driven on. This label includes a possibly delimiting curb.</td>
</tbody>
</table>
<br>

!!! Note
    Read [this](tuto_D_create_semantic_tags.md) tutorial to create new semantic tags. 

#### Basic camera attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>fov</code> </td>
<td>float</td>
<td>90.0</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>image_size_x</code></td>
<td>int</td>
<td>800</td>
<td>Image width in pixels.</td>
<tr>
<td><code>image_size_y</code></td>
<td>int</td>
<td>600</td>
<td>Image height in pixels.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
</tbody>
</table>
<br>

#### Camera lens distortion attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>lens_circle_falloff</code> </td>
<td>float</td>
<td>5.0</td>
<td>Range: [0.0, 10.0]</td>
<tr>
<td><code>lens_circle_multiplier</code></td>
<td>float</td>
<td>0.0</td>
<td>Range: [0.0, 10.0]</td>
<tr>
<td><code>lens_k</code></td>
<td>float</td>
<td>-1.0</td>
<td>Range: [-inf, inf]</td>
<tr>
<td><code>lens_kcube</code></td>
<td>float</td>
<td>0.0</td>
<td>Range: [-inf, inf]</td>
<tr>
<td><code>lens_x_size</code></td>
<td>float</td>
<td>0.08</td>
<td>Range: [0.0, 1.0]</td>
<tr>
<td><code>lens_y_size</code></td>
<td>float</td>
<td>0.08</td>
<td>Range: [0.0, 1.0]</td>
</tbody>
</table>
<br>

#### Output attributes

<table class ="defTable">
<thead>
<th>Sensor data attribute</th>
<th>Type</th>
<th>Description</th>
</thead>
<tbody>
<td><code>fov</code></td>
<td>float</td>
<td>Horizontal field of view in degrees.</td>
<tr>
<td><code>frame</code> </td>
<td>int</td>
<td>Frame number when the measurement took place.</td>
<tr>
<td><code>height</code></td>
<td>int</td>
<td>Image height in pixels.</td>
<tr>
<td><code>raw_data</code></td>
<td>bytes</td>
<td>Array of BGRA 32-bit pixels.</td>
<tr>
<td><code>timestamp</code></td>
<td>double</td>
<td>Simulation time of the measurement in seconds since the beginning of the episode.</td>
<tr>
<td><code>transform</code></td>
<td><a href="../python_api#carlatransform">carla.Transform</a></td>
<td>Location and rotation in world coordinates of the sensor at the time of the measurement.</td>
<tr>
<td><code>width</code></td>
<td>int</td>
<td>Image width in pixels.</td>
</tbody>
</table>

<br>

---
## DVS camera

*   __Blueprint:__ sensor.camera.dvs
*   __Output:__ [carla.DVSEventArray](python_api.md#carla.DVSEventArray) per step (unless `sensor_tick` says otherwise).


A Dynamic Vision Sensor (DVS) or Event camera is a sensor that works radically differently from a conventional camera. Instead of capturing
intensity images at a fixed rate, event cameras measure changes of intensity asynchronously, in the form of a stream of events, which encode per-pixel
brightness changes. Event cameras possess outstanding properties when compared to standard cameras. They have a very high dynamic range (140 dB
versus 60 dB), no motion blur, and high temporal resolution (in the order of microseconds). Event cameras are thus sensors that can provide high-quality
visual information even in challenging high-speed scenarios and high dynamic range environments, enabling new application domains for vision-based
algorithms.

The DVS camera outputs a stream of events. An event `e=(x,y,t,pol)` is triggered at a pixel `x`, `y` at a timestamp `t` when the change in
logarithmic intensity `L` reaches a predefined constant threshold `C` (typically between 15% and 30%).

``
L(x,y,t) - L(x,y,t-\delta t) = pol C
``

`t-\delta t` is the time when the last event at that pixel was triggered and `pol` is the polarity of the event according to the sign of the
brightness change. The polarity is positive `+1` when there is increment in brightness and negative `-1` when a decrement in brightness occurs. The
working principles depicted in the following figure. The standard camera outputs frames at a fixed rate, thus sending redundant information
when no motion is present in the scene. In contrast, event cameras are data-driven sensors that respond to brightness changes with microsecond
latency. At the plot, a positive (resp. negative) event (blue dot, resp. red dot) is generated whenever the (signed) brightness change exceeds the
contrast threshold `C` for one dimension `x` over time `t`. Observe how the event rate grows when the signal changes rapidly.

![DVSCameraWorkingPrinciple](img/sensor_dvs_scheme.jpg)

The current implementation of the DVS camera works in a uniform sampling manner between two consecutive synchronous frames. Therefore, in order to
emulate the high temporal resolution (order of microseconds) of a real event camera, the sensor requires to execute at a high frequency (much higher
frequency than a conventional camera). Effectively, the number of events increases as faster a CARLA car drives. Therefore, the sensor frequency
should increase accordingly with the dynamic of the scene. The user should find their balance between time accuracy and computational cost.

The provided script `manual_control.py` uses the DVS camera in order to show how to configure the sensor, how to get the stream of events and how to depict such events in an image format, usually called event frame.

![DVSCameraWorkingPrinciple](img/sensor_dvs.gif)

DVS is a camera and therefore has all the attributes available in the RGB camera. Nevertheless, there are few attributes exclusive to the working principle of an Event camera.

#### DVS camera attributes

<table class ="defTable">
<thead>
<th>Blueprint attribute</th>
<th>Type</th>
<th>Default</th>
<th>Description</th>
</thead>
<tbody>
<td>
<code>positive_threshold</code> </td>
<td>float</td>
<td>0.3</td>
<td>Positive threshold C associated to a increment in brightness change (0-1).</td>
<tr>
<td><code>negative_threshold</code></td>
<td>float</td>
<td>0.3</td>
<td>Negative threshold C associated to a decrement in brightness change (0-1).</td>
<tr>
<td><code>sigma_positive_threshold</code></td>
<td>float</td>
<td>0</td>
<td>White noise standard deviation for positive events (0-1).</td>
<tr>
<td><code>sigma_negative_threshold</code></td>
<td>float</td>
<td>0</td>
<td>White noise standard deviation for negative events (0-1).</td>
<tr>
<td><code>refractory_period_ns</code></td>
<td>int</td>
<td>0.0</td>
<td> Refractory period (time during which a pixel cannot fire events just after it fired one), in nanoseconds. It limits the highest frequency of triggering events.</td>
<tr>
<td><code>use_log</code></td>
<td>bool</td>
<td>true</td>
<td>Whether to work in the logarithmic intensity scale.</td>
<tr>
<td><code>log_eps</code></td>
<td>float</td>
<td>0.001</td>
<td>Epsilon value used to convert images to log: <code>L = log(eps + I / 255.0)</code>.<br> Where <code>I</code> is the grayscale value of the RGB image: <br><code>I = 0.2989*R +  0.5870*G + 0.1140*B</code>.</td>
</tbody>
</table>
<br>
