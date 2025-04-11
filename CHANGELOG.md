## LATEST Changes

* Added actor description as Actor TAGs


## CARLA 0.10.0

* Unreal Engine migration from version 4.26 to version 5.5
    - Nanite enabled for entire Town 10
    - Lumen powers new realistic illumination
    - Improved lighting with new post-process configuration
    - Physics migrated from PhysX to Chaos
* Remodeled Town 10:
    - New buildings
    - Road topology includes uneven surface, potholes, speed bumps
* Remodeled 13 vehicles with more detail:
    - Nissan Patrol
    - Dodge Charger
    - Dodge Charger Police
    - Ford Crown Taxi
    - Lincoln MKZ
    - Mini Cooper
    - Firetruck
    - CarlaCola
    - Ambulance
    - Mercedes Sprinter
    - Mitsubishi Fusorosa bus
    - Mining earth mover (provided by Synkrotron)
* Rework of build system to use cmake
* Native server support for ROS2 (without CARLA-ROS bridge)
* Refactor of semantic segmentation and instance segmentation sensors
    - Semantic segmentation can now be visualized in UE editor for debugging
    - Only supports meshes with Nanite enabled
* Included support to data-driven realistic traffic generation powered by InvertedAI
* Updated Python API scripts with extra examples in the PythonAPI folder:
    - examples/invertedai_traffic.py
    - examples/interpolate_camera.py
    - examples/vehicle_doors_demo.py
    - examples/vehicle_lights_demo.py
    - examples/visualize_radar.py
    - examples/ros2_native.py
    - util/tools/generate_video_from_frames.py
    - util/apply_texture.py
    - util/change_map_layer.py
    - util/manage_environment_objects.py
    - util/manage_traffic_light.py
    - util/recorder_comparer.py
    - util/show_crosswalks.py
    - util/show_junctions.py
    - util/show_topology.py
    - util/spectator_transform.py
* Motion blur disabled below 1080p resolution to avoid artefacts
* Python API now supports Python 3.8, 3.9, 3.10, 3.11 and 3.12
* Python API support dropped for Python versions 3.7 and lower
* Added support for Scenic 3.0
* RSS functionality removed from docs
* Removed Light Manager from API and docs
* Added Mine01 off-road mining map from Synkrotron



