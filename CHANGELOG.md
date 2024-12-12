## CARLA 0.10.0

* Unreal Engine migration from version 4.26 to version 5.5
    - Nanite enabled for entire Town 10
    - Lumen powers new realistic illumination
    - Physics migrated from PhysX to Chaos
* Remodeled Town 10:
    - New buildings
    - Road topology includes uneven surface, potholes, speed bumps
* Remodeled 13 vehicles with more detail:
    - Nissan Patrol
    - Dodge Charger
    - Dodge Charger Police
    - Mercedes Coupe
    - Ford Crown Taxi
    - Lincoln MKZ
    - Mini Cooper
    - Audi TT
    - Firetruck
    - CarlaCola
    - Ambulance
    - Mercedes Sprinter
    - Mitsubishi Fusorosa bus
* Rework of build system to use cmake
* Refactor of semantic segmentation and instance segmentation sensors
    - Semantic segmentation can now be visualized in UE editor for debugging
    - Only supports meshes with Nanite enabled
* Updated Python API scripts with extra examples:
    - 
* Python API now supports Python 3.9, 3.10 and 3.11
* Added Mine01 off-road mining map from Synkrotron

