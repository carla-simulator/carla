# Sensors reference

  * [__Collision detector__](#collision-detector)
  * [__Depth camera__](#depth-camera)
  * [__GNSS sensor__](#gnss-sensor)
  * [__IMU sensor__](#imu-sensor)
  * [__Lane invasion detector__](#lane-invasion-detector)
  * [__Lidar raycast sensor__](#lidar-raycast-sensor)
  * [__Obstacle detector__](#obstacle-detector)
  * [__Radar sensor__](#radar-sensor)
  * [__RGB camera__](#rgb-camera)
  * [__RSS sensor__](#rss-sensor)
  * [__Semantic segmentation camera__](#semantic-segmentation-camera)


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

![ImageDepth](img/capture_depth.png)


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
## Lidar raycast sensor

* __Blueprint:__ sensor.lidar.ray_cast
* __Output:__ [carla.LidarMeasurement](python_api.md#carla.LidarMeasurement) per step (unless `sensor_tick` says otherwise).

This sensor simulates a rotating Lidar implemented using ray-casting.
The points are computed by adding a laser for each channel distributed in the vertical FOV. The rotation is simulated computing the horizontal angle that the Lidar rotated in a frame. The point cloud is calculated by doing a ray-cast for each laser in every step:
`points_per_channel_each_step = points_per_second / (FPS * channels)`

A Lidar measurement contains a packet with all the points generated during a `1/FPS` interval. During this interval the physics are not updated so all the points in a measurement reflect the same "static picture" of the scene.

This output contains a cloud of simulation points and thus, can be iterated to retrieve a list of their [`carla.Location`](python_api.md#carla.Location):

```py
for location in lidar_measurement:
    print(location)
```

!!! Tip
    Running the simulator at [fixed time-step](adv_synchrony_timestep.md) it is possible to tune the rotation for each measurement. Adjust the step and the rotation frequency to get, for instance, a 360 view each measurement.

![LidarPointCloud](img/lidar_point_cloud.gif)

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
<td>Lidar rotation frequency.</td>
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
<td>Angle (radians) in the XY plane of the lidar this frame.</td>
<tr>
<td><code>channels</code></td>
<td>int</td>
<td>Number of channels (lasers) of the lidar.</td>
<tr>
<td><code>get_point_count(channel)</code></td>
<td>int</td>
<td>Number of points per channel captured this frame.</td>
<tr>
<td><code>raw_data</code></td>
<td>bytes</td>
<td>Array of 32-bits floats (XYZ of each point).</td>
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

![ImageRadar](img/sensor_radar.png)

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

![ImageRGB](img/capture_scenefinal.png)

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
<td>1200.0</td>
<td>The camera sensor sensitivity.</td>
<tr>
<td><code>gamma</code></td>
<td>float</td>
<td>2.2</td>
<td>Target gamma value of the camera.</td>
<tr>
<td><code>sensor_tick</code></td>
<td>float</td>
<td>0.0</td>
<td>Simulation seconds between sensor captures (ticks).</td>
<tr>
<td><code>shutter_speed</code></td>
<td>float</td>
<td>60.0</td>
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
<td><code>manual</code></td>
<td>Can be <code>manual</code> or <code>histogram</code>. More in <a href="https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/AutomaticExposure/index.html">UE4 docs</a>.</td>
<tr>
<td><code>exposure_compensation</code> </td>
<td>float</td>
<td>3.0</td>
<td>Logarithmic adjustment for the exposure. 0: no adjustment, -1:2x darker, -2:4 darker, 1:2x brighter, 2:4x brighter.</td>
<tr>
<td><code>exposure_min_bright</code> </td>
<td>float</td>
<td>0.1</td>
<td>In <code>exposure_mode: "histogram"</code>. Minimum brightness for auto exposure. The lowest the eye can adapt within. Must be greater than 0 and less than or equal to <code>exposure_max_bright</code>.</td>
<tr>
<td><code>exposure_max_bright</code> </td>
<td>float</td>
<td>2.0</td>
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

These controllers can be generated by an *Automated Driving* stack or user input. For instance, hereunder there is a fragment of code from `PythonAPI/examples/manual_control_rss.py`, where the user input is modified using RSS when necessary.  

__1.__ Checks if the __RssSensor__ generates a valid response containing restrictions.  
__2.__ Gathers the current dynamics of the vehicle and the vehicle physics.  
__3.__ Applies restrictions to the vehicle control using the response from the RssSensor, and the current dynamics and physicis of the vehicle.  

```py
rss_restriction = self._world.rss_sensor.acceleration_restriction if self._world.rss_sensor and self._world.rss_sensor.response_valid else None
if rss_restriction:
    rss_ego_dynamics_on_route = self._world.rss_sensor.ego_dynamics_on_route
    vehicle_physics = world.player.get_physics_control()
...
        vehicle_control = self._restrictor.restrict_vehicle_control(
            vehicle_control, rss_restriction, rss_ego_dynamics_on_route, vehicle_physics)
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
<td>
<code>ego_vehicle_dynamics</code> </td>
<td><a href="https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/">libad_rss_python.RssDynamics</a></td>
<td>RSS parameters to be applied for the ego vehicle </td>
<tr>
<td>
<code>other_vehicle_dynamics</code> </td>
<td><a href="https://intel.github.io/ad-rss-lib/ad_rss/Appendix-ParameterDiscussion/">libad_rss_python.RssDynamics</a></td>
<td>RSS parameters to be applied for the other vehicles</td>
<tr>
<td><code>road_boundaries_mode</code></td>
<td><a href="../python_api#carlarssroadboundariesmode">carla.RssRoadBoundariesMode</a></td>
<td>Enables/Disables the <a href="https://intel.github.io/ad-rss-lib/ad_rss_map_integration/HandleRoadBoundaries">stay on road</a> feature. Default is <b>Off</b>.</td>
<tr>
<td><code>visualization_mode</code></td>
<td><a href="../python_api#carlarssvisualizationmode">carla.RssVisualizationMode</a></td>
<td>States the visualization of the RSS calculations. Default is <b>All</b>.</td>
</table>
<br>

```py
# Fragment of manual_control_rss.py
# The carla.RssSensor is updated when listening for a new carla.RssResponse
def _on_rss_response(weak_self, response):
...
        self.timestamp = response.timestamp
        self.response_valid = response.response_valid
        self.proper_response = response.proper_response
        self.acceleration_restriction = response.acceleration_restriction
        self.ego_dynamics_on_route = response.ego_dynamics_on_route
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
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1ProperResponse.html">libad_rss_python.ProperResponse</a></td>
<td>Proper response that the RSS calculated for the vehicle.</td>
<tr>
<td><code>acceleration_restriction</code></td>
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1world_1_1AccelerationRestriction.html">libad_rss_python.AccelerationRestriction</a></td>
<td>Acceleration restrictions of the RSS calculation.</td>
<tr>
<td><code>rss_state_snapshot</code></td>
<td><a href="https://intel.github.io/ad-rss-lib/doxygen/ad_rss/structad_1_1rss_1_1state_1_1RssStateSnapshot.html">libad_rss_python.RssStateSnapshot</a></td>
<td>RSS states at the current point in time.</td>
<tr>
<td><code>ego_dynamics_on_route</code></td>
<td><a href="../python_api#carlarssegodynamicsonroute">carla.RssEgoDynamicsOnRoute</a></td>
<td>Current ego vehicle dynamics regarding the route.</td>
</tbody>
</table>

---
## Semantic segmentation camera

*   __Blueprint:__ sensor.camera.semantic_segmentation  
*   __Output:__ [carla.Image](python_api.md#carla.Image) per step (unless `sensor_tick` says otherwise).  

This camera classifies every object in sight by displaying it in a different color according to its tags (e.g., pedestrians in a different color than vehicles).
When the simulation starts, every element in scene is created with a tag. So it happens when an actor is spawned. The objects are classified by their relative file path in the project. For example, meshes stored in `Unreal/CarlaUE4/Content/Static/Pedestrians` are tagged as `Pedestrian`.  

The server provides an image with the tag information __encoded in the red channel__: A pixel with a red value of `x` belongs to an object with tag `x`.
This raw [carla.Image](python_api.md#carla.Image) can be stored and converted it with the help of __CityScapesPalette__  in [carla.ColorConverter](python_api.md#carla.ColorConverter) to apply the tags information and show picture with the semantic segmentation.
The following tags are currently available:

<table class ="defTable">
<thead>
<th>Value</th>
<th>Tag</th>
<th>Converted color</th>
</thead>
<tbody>
<td><code>0</code> </td>
<td>Unlabeled</td>
<td>(  0,   0,   0)</td>
<tr>
<td><code>1</code> </td>
<td>Building</td>
<td>( 70,  70,  70)</td>
<tr>
<td><code>2</code> </td>
<td>Fence</td>
<td>(190, 153, 153)</td>
<tr>
<td><code>3</code> </td>
<td>Other</td>
<td>(250, 170, 160)</td>
<tr>
<td><code>4</code> </td>
<td>Pedestrian</td>
<td>(220,  20,  60)</td>
<tr>
<td><code>5</code> </td>
<td>Pole</td>
<td>(153, 153, 153)</td>
<tr>
<td><code>6</code> </td>
<td>Road line</td>
<td>(157, 234,  50)</td>
<tr>
<td><code>7</code> </td>
<td>Road</td>
<td>(128,  64, 128)</td>
<tr>
<td><code>8</code> </td>
<td>Sidewalk</td>
<td>(244,  35, 232)</td>
<tr>
<td><code>9</code> </td>
<td>Vegetation</td>
<td>(107, 142,  35)</td>
<tr>
<td><code>10</code> </td>
<td>Car</td>
<td>(  0,   0, 142)</td>
<tr>
<td><code>11</code> </td>
<td>Wall</td>
<td>(102, 102, 156)</td>
<tr>
<td><code>12</code> </td>
<td>Traffic sign</td>
<td>(220, 220,   0)</td>
</tbody>
</table>
<br>

!!! Note
    **Adding new tags**:
    It requires some C++ coding. Add a new label to the `ECityObjectLabel` enum in "Tagger.h", and its corresponding filepath check inside `GetLabelByFolderName()` function in "Tagger.cpp".

![ImageSemanticSegmentation](img/capture_semseg.png)

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
