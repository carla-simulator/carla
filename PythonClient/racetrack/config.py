import random
from collections import OrderedDict

# General, Carla-related
IMAGE_SIZE = (150, 200)
STEER_BOUND = 6
THROTTLE_BOUND = 1
MIN_SPEED = 10
# If you're running low on memory, you may consider switching to `np.float32`
DTYPE = 'float32'

# "Controller regularization"
STEER_NOISE = lambda : random.uniform(-0.1, 0.1)
THROTTLE_NOISE = lambda: random.uniform(-0.05, 0.05)

# NN training
BATCH_SIZE = 32
NUM_EPOCHS = 2
NUM_TRAIN_EPISODES = 10
WEIGHT_EXPONENT = 0

# Modelling
NUM_X_DIFF_CHANNELS = 1
NUM_X_CHANNELS = 2
IMAGE_CLIP_UPPER = 60
IMAGE_CLIP_LOWER = 100
SPEED_AS_INPUT = False

OUTPUTS_SPEC = OrderedDict([
    ('steer', ['mse', 1.0]),
    ('steer__1__last', ['mse', 1.0]),
    ('steer__2__last', ['mse', 1.0]),
    ('steer__3__last', ['mse', 1.0]),
    ('steer__4__last', ['mse', 1.0]),
    ('steer__5__last', ['mse', 1.0]),
    ('steer__6__last', ['mse', 1.0]),
    ('steer__7__last', ['mse', 1.0]),
    ('steer__8__last', ['mse', 1.0]),
    ('steer__9__last', ['mse', 1.0]),
    ('steer__10__last', ['mse', 1.0]),

    ('throttle', ['mse', 1.0]),
    ('throttle__1__last', ['mse', 1.0]),
    ('throttle__2__last', ['mse', 1.0]),
    ('throttle__3__last', ['mse', 1.0]),
    ('throttle__4__last', ['mse', 1.0]),
    ('throttle__5__last', ['mse', 1.0]),
    ('throttle__6__last', ['mse', 1.0]),
    ('throttle__7__last', ['mse', 1.0]),
    ('throttle__8__last', ['mse', 1.0]),
    ('throttle__9__last', ['mse', 1.0]),
    ('throttle__10__last', ['mse', 1.0]),

    ('racetrack', ['categorical_crossentropy', 1.0])

    # ('speed', ['mse', 1e-2]),
])

# For plotting
ERROR_PLOT_UPPER_BOUNDS = {
    key: {
        'steer': 0.05,
        'throttle': 0.5,
        'speed': 40,
        'racetrack': 1.0,
    }[key.split('__')[0]]
    for key in OUTPUTS_SPEC
}
SCATTER_PLOT_BOUNDS = {
    key: {
        'steer': [-STEER_BOUND, STEER_BOUND],
        'throttle': [-THROTTLE_BOUND, THROTTLE_BOUND],
        'speed': [0, 80],
        'racetrack': [0, 2],
    }[key.split('__')[0]]
    for key in OUTPUTS_SPEC
}
