config = {
    "scenarioFolder": "test",
    "xosc_file_name": "original.xosc",
    "num_randomized_scenarios": 4, # per intenstiy
    "intensity_levels": [1],
    "AbsoluteTargetSpeed": {
        "sigma": 3  # standard deviation of absolute target speed in m/s
        }, 
    "Performances": {
        "maxSpeed": {
            "sigma": 3 # standard deviation of max speed in m/s
        },
        "maxAcceleration": {
            "sigma": 10 # standard deviation of max acceleration in m/s^2
        },
        "maxDeceleration": {
            "sigma": 10 # standard deviation of max deceleration in m/s^2
        }
    },
    "Spawnpoints": {
        "mu": 0,   # distance from original spawnpoint
        "sigma": 1 # standard deviation of distance from original spawnpoint
    },
    "Datetime": {
        "sigma": 12 # standard deviation of datetime in hours
    },
    "Routes": {
        "mu": 0, # distance from original route waypoints
        "sigma": 1 # standard deviation of distance from original spawnpoint
    },
    "Weather": {
        "sigma": 8 # standard deviation of weather parameters (all values between 0 and 100)
    }
}

def get_config():
    global config
    return config
