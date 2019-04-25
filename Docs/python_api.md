### Overview
- [**carla**](#carla) <sub>_Module_</sub>  
    - [**Map**](#carla.Map) <sub>_Class_</sub>  
        - [**name**](#carla.Map.name) <sub>_Instance variable_</sub>
        - [**get_spawn_points**(**self**)](#carla.Map.get_spawn_points) <sub>_Method_</sub>
        - [**get_waypoint**(**self**)](#carla.Map.get_waypoint) <sub>_Method_</sub>
        - [**get_topology**(**self**)](#carla.Map.get_topology) <sub>_Method_</sub>
        - [**generate_waypoints**(**self**)](#carla.Map.generate_waypoints) <sub>_Method_</sub>
        - [**transform_to_geolocation**(**self**)](#carla.Map.transform_to_geolocation) <sub>_Method_</sub>
        - [**to_opendrive**(**self**)](#carla.Map.to_opendrive) <sub>_Method_</sub>
        - [**save_to_disk**(**self**, **path**)](#carla.Map.save_to_disk) <sub>_Method_</sub>
    - [**Client**](#carla.Client) <sub>_Class_</sub>  
        - [**\__init__**(**self**, **host**, **port**, **worker_threads**=0)](#carla.Client.__init__) <sub>_Method_</sub>
        - [**set_timeout**(**self**, **seconds**)](#carla.Client.set_timeout) <sub>_Method_</sub>
        - [**get_client_version**(**self**)](#carla.Client.get_client_version) <sub>_Method_</sub>
        - [**get_server_version**(**self**)](#carla.Client.get_server_version) <sub>_Method_</sub>
        - [**get_world**(**self**)](#carla.Client.get_world) <sub>_Method_</sub>
        - [**get_available_maps**(**self**)](#carla.Client.get_available_maps) <sub>_Method_</sub>
        - [**reload_world**(**self**)](#carla.Client.reload_world) <sub>_Method_</sub>
        - [**load_world**(**self**, **map_name**)](#carla.Client.load_world) <sub>_Method_</sub>
        - [**start_recorder**(**self**, **filename**)](#carla.Client.start_recorder) <sub>_Method_</sub>
        - [**stop_recorder**(**self**)](#carla.Client.stop_recorder) <sub>_Method_</sub>
        - [**show_recorder_file_info**(**self**)](#carla.Client.show_recorder_file_info) <sub>_Method_</sub>
        - [**show_recorder_collisions**(**self**)](#carla.Client.show_recorder_collisions) <sub>_Method_</sub>
        - [**show_recorder_actors_blocked**(**self**)](#carla.Client.show_recorder_actors_blocked) <sub>_Method_</sub>
        - [**replay_file**(**self**)](#carla.Client.replay_file) <sub>_Method_</sub>
        - [**set_replayer_time_factor**(**self**)](#carla.Client.set_replayer_time_factor) <sub>_Method_</sub>
        - [**apply_batch**(**self**)](#carla.Client.apply_batch) <sub>_Method_</sub>
        - [**apply_batch_sync**(**self**)](#carla.Client.apply_batch_sync) <sub>_Method_</sub>

## <a name="carla.Map"></a>Map <sub><sup>_Class_</sup></sub>

### Instance Variables
- <a name="carla.Map.name"></a>**name**  
    Map name.  

### Methods
- <a name="carla.Map.get_spawn_points"></a>**<font color="#64BA2E">get_spawn_points</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.get_waypoint"></a>**<font color="#64BA2E">get_waypoint</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.get_topology"></a>**<font color="#64BA2E">get_topology</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.generate_waypoints"></a>**<font color="#64BA2E">generate_waypoints</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.transform_to_geolocation"></a>**<font color="#64BA2E">transform_to_geolocation</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.to_opendrive"></a>**<font color="#64BA2E">to_opendrive</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Map.save_to_disk"></a>**<font color="#64BA2E">save_to_disk</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**path**</font>)  
    Save the OpenDrive of the current map to disk.  
    - **Parameters:**
        - `path`  –             Path where will be saved.  

---

## <a name="carla.Client"></a>Client <sub><sup>_Class_</sup></sub>

### Methods
- <a name="carla.Client.__init__"></a>**<font color="#64BA2E">\__init__</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**host**</font>, <font color="#2980B9">**port**</font>, <font color="#2980B9">**worker_threads**=0</font>)  
    Client constructor.  
    - **Parameters:**
        - `host` (_str_) –             IP where Carla is running.  
        - `port` (_int_) –             Port where Carla is running.  
        - `worker_threads` (_int_) –             Number of working threads.  
- <a name="carla.Client.set_timeout"></a>**<font color="#64BA2E">set_timeout</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**seconds**</font>)  
    Sets the server timeout in seconds.  
    - **Parameters:**
        - `seconds` (_float_) –             New timeout value in seconds.  
- <a name="carla.Client.get_client_version"></a>**<font color="#64BA2E">get_client_version</font>**(<font color="#2980B9">**self**</font>)  
    Get the client version as a string.  
- <a name="carla.Client.get_server_version"></a>**<font color="#64BA2E">get_server_version</font>**(<font color="#2980B9">**self**</font>)  
    Get the server version as a string.  
- <a name="carla.Client.get_world"></a>**<font color="#64BA2E">get_world</font>**(<font color="#2980B9">**self**</font>)  
    Get the server version as a string.  
- <a name="carla.Client.get_available_maps"></a>**<font color="#64BA2E">get_available_maps</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.reload_world"></a>**<font color="#64BA2E">reload_world</font>**(<font color="#2980B9">**self**</font>)  
    - **Raises:** RuntimeError  
- <a name="carla.Client.load_world"></a>**<font color="#64BA2E">load_world</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**map_name**</font>)  
    - **Parameters:**
        - `map_name` (_str_) –             Name of the map to load.  
- <a name="carla.Client.start_recorder"></a>**<font color="#64BA2E">start_recorder</font>**(<font color="#2980B9">**self**</font>, <font color="#2980B9">**filename**</font>)  
    - **Parameters:**
        - `filename` (_str_) –             Name of the recorder file to load.  
- <a name="carla.Client.stop_recorder"></a>**<font color="#64BA2E">stop_recorder</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.show_recorder_file_info"></a>**<font color="#64BA2E">show_recorder_file_info</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.show_recorder_collisions"></a>**<font color="#64BA2E">show_recorder_collisions</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.show_recorder_actors_blocked"></a>**<font color="#64BA2E">show_recorder_actors_blocked</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.replay_file"></a>**<font color="#64BA2E">replay_file</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.set_replayer_time_factor"></a>**<font color="#64BA2E">set_replayer_time_factor</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.apply_batch"></a>**<font color="#64BA2E">apply_batch</font>**(<font color="#2980B9">**self**</font>)  
- <a name="carla.Client.apply_batch_sync"></a>**<font color="#64BA2E">apply_batch_sync</font>**(<font color="#2980B9">**self**</font>)  

---
