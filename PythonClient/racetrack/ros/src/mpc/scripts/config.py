def deg2rad(deg):
    return deg * 3.1415 / 180


CTE_COEFF = 1000
EPSI_COEF = 1000
SPEED_COEFF = 2
ACC_COEFF = 2
STEER_COEFF = 2

CONSEC_ACC_COEFF = 500
CONSEC_STEER_COEFF = 500

Lf = 0.325  # TODO(MD): consult with Lukasz
THROTTLE_LOWER_BOUND = 5  # TODO(MD): consult with Lukasz
THROTTLE_UPPER_BOUND = 15  # TODO(MD): consult with Lukasz
STEER_BOUND = deg2rad(25)  # TODO(MD): consult with Lukasz

TARGET_SPEED = 1
STEPS_AHEAD = 10
TIME_STEP = 0.1

# For the optimizer
TOLERANCE = 1.0  # Higher tolerance means that the minimization ends after
                 #  fewer iterations (and it has a higher chance of converging)
                 # BUT the of the cost function can be sub-optimal, and thus
                 # the actuators that correspond to that cost function
