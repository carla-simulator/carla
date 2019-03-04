import random
from collections import OrderedDict


# GENERAL
IMAGE_DECIMATION = 4
IMAGE_SIZE = (240, 320)
# If you're running low on memory, you may consider switching to `np.float32`
DTYPE = 'float32'

# MPC-RELATED
# Constraints for MPC
STEER_BOUND = 1.0
STEER_BOUNDS = (-STEER_BOUND, STEER_BOUND)
THROTTLE_BOUND = 1.0
THROTTLE_BOUNDS = (0, THROTTLE_BOUND)



# "Controller regularization"
STEER_NOISE = lambda: random.uniform(-0.1, 0.1)
THROTTLE_NOISE = lambda: random.uniform(-0.05, 0.05)

# If you'd like to test out a (synthetic) stochastic policy
STEER_NOISE_NN = lambda: 0 #random.uniform(-0.05, 0.05)
THROTTLE_NOISE_NN = lambda: 0 #random.uniform(-0.05, 0.05)



# Dataset
CONTROLLER = 'pad'

TRAIN_SET = [
    'depth_data/{}_racetrack{}_depth_data{}.npy'.format(CONTROLLER, racetrack, episode)
    for episode in range(9)
    for racetrack in ['01', '02']
    #for racetrack in ['02']
    # for racetrack in ['11']
]
TEST_SET = [
    'depth_data/{}_racetrack{}_depth_data{}.npy'.format(CONTROLLER, racetrack, episode)
    for episode in [9]
    for racetrack in ['01', '02']
    # for racetrack in ['02']
    # for racetrack in ['11']
]

# TRAIN_SET = [
#     # 'depth_data/mpc_racetrack01_depth_data0.npy',
#     'depth_data/normedfilled_racetrack11_depth_data0.npy',
#     # 'depth_data/mpc_racetrack01_depth_data1.npy',
#     'depth_data/normedfilled_racetrack11_depth_data1.npy',
#     # 'depth_data/mpc_racetrack01_depth_data2.npy',
#     'depth_data/normedfilled_racetrack11_depth_data2.npy',
# ]
# TEST_SET = [
#     'depth_data/normedfilled_racetrack10_depth_data0.npy'
# ]


if (set(TRAIN_SET) & set(TEST_SET)) != set():
    print('TRAIN_SET and TEST_SET are not disjoint!!!!!')

# We're throwing away data that occured at low speed
MIN_SPEED = 10

# NN training
BATCH_SIZE = 32
NUM_EPOCHS = 30
WEIGHT_EXPONENT = 0

# INPUT
NUM_X_DIFF_CHANNELS = 0
NUM_X_CHANNELS = 1
IMAGE_CLIP_UPPER = 0
IMAGE_CLIP_LOWER = IMAGE_SIZE[0]

SPEED_AS_INPUT = True

STEPS_INTO_NEAR_FUTURE = range(1, 11)
OUTPUTS_SPEC = OrderedDict(
    [('steer', {'act': 'linear', 'loss': 'mse', 'weight': 1.0})]
    + [('steer__{}__last'.format(i), {'act': 'linear', 'loss': 'mse', 'weight': 1.0}) for i in STEPS_INTO_NEAR_FUTURE]

    + [('throttle', {'act': 'sigmoid', 'loss': 'mse', 'weight': 1.0})]
    + [('throttle__{}__last'.format(i), {'act': 'sigmoid', 'loss': 'mse', 'weight': 1.0}) for i in STEPS_INTO_NEAR_FUTURE]

    # TODO ('racetrack', ['categorical_crossentropy', 1.0])
)
if SPEED_AS_INPUT:
    OUTPUTS_SPEC['speed'] = {'act': 'linear', 'loss': 'mse', 'weight': 1e-2}

# PLOTTING
BASE_FONTSIZE = 14

ERROR_PLOT_UPPER_BOUNDS = {
    key: {
        'steer': 0.15,
        'throttle': 0.5,
        'speed': 40,
        'racetrack': 1.0,
    }[key.split('__')[0]]
    for key in OUTPUTS_SPEC
}
SCATTER_PLOT_BOUNDS = {
    key: {
        'steer': STEER_BOUNDS,
        'throttle': THROTTLE_BOUNDS,
        'speed': [0, 100],
        'racetrack': [0, 2],
    }[key.split('__')[0]]
    for key in OUTPUTS_SPEC
}
