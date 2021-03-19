class Constants:

    CPM_DELTA_T = 1
    CPM_DELTA_S = 0.5
    CPM_DELTA_P = 4
    """
    CPM_DELTA_T: 1 (sec)
    CPM_DELTA_S: 0.5 (m/sec)
    CPM_DELTA_P: 4 (m)
    cite from: Thandavarayan, G., Sepulcre, M., & Gozalvez, J. (2020). Generation of Cooperative Perception Messages for Connected and Automated Vehicles. IEEE Transactions on Vehicular Technology, 9545(c). https://doi.org/10.1109/TVT.2020.3036165
    """

    TARGET_ROAD_WIDTH = 3.2 * 3
    TARGET_ROAD_LENGTH = 150
    VEHICLE_WIDTH = 1.8
    VEHICLE_LENGTH = 5.0
    """
    TARGET_ROAD_WIDTH: 3.2 (m) * 3 (lanes)
    cite from:  https://sumo.dlr.de/docs/Simulation/SublaneModel.html#:~:text=The%20default%20lane%2Dwidth%20of,of%20that%20width%20per%20lane.

    TARGET_ROAD_LENGTH: 150 (m)
    cite from: ETSI. (2019). Intelligent Transport Systems (ITS); Vehicular Communications; Basic Set of Applications; Analysis of the Collective Perception Service (CPS). Draft TR 103 562 V0.0.16, 1, 1–119.

    VEHICLE_WIDTH: 1.8 (m)
    VEHICLE_LENGTH: 5.0 (m)
    cite from: https://sumo.dlr.de/docs/Definition_of_Vehicles%2C_Vehicle_Types%2C_and_Routes.html
    """

    VALID_TIME_DELTA = 8
    LOCATION_THRESHOLD = max([VEHICLE_WIDTH, VEHICLE_LENGTH])
    SENSOR_TICK = 1.0
