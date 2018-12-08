import random
from collections import OrderedDict

# General, Carla-related
IMAGE_SIZE = (150, 200)
STEER_BOUND = 6
THROTTLE_BOUND = 1
# If you're running low on memory, you may consider switching to `np.float32`
DTYPE = 'float32'

# "Controller regularization"
STEER_NOISE = lambda : random.uniform(-0.1, 0.1)
THROTTLE_NOISE = lambda: random.uniform(-0.05, 0.05)

# NN training
BATCH_SIZE = 32
NUM_EPOCHS = 30
NUM_TRAIN_EPISODES = 10
WEIGHT_EXPONENT = 0

# Modelling
NUM_X_DIFF_CHANNELS = 0
NUM_X_CHANNELS = 2
IMAGE_CLIP_UPPER = 60
IMAGE_CLIP_LOWER = 100

OUTPUTS_SPEC = OrderedDict([
    ('steer', ['mse', 1.0]),
    ('steer__5', ['mse', 1.0]),
    ('steer__10', ['mse', 1.0]),
    ('steer__20', ['mse', 1.0]),
    ('steer__50', ['mse', 1.0]),

    ('throttle', ['mse', 1.0]),
    ('throttle__5', ['mse', 1.0]),
    ('throttle__10', ['mse', 1.0]),
    ('throttle__20', ['mse', 1.0]),
    ('throttle__50', ['mse', 1.0]),

    ('speed', ['mse', 1e-2]),
])

# For plotting
ERROR_PLOT_UPPER_BOUNDS = {
    'steer': 0.05,
    'steer__5': 0.05,
    'steer__10': 0.05,
    'steer__20': 0.05,
    'steer__50': 0.05,

    'throttle': 0.5,
    'throttle__5': 0.5,
    'throttle__10': 0.5,
    'throttle__20': 0.5,
    'throttle__50': 0.5,

    'speed': 40,
}
SCATTER_PLOT_BOUNDS = {
    'steer': [-STEER_BOUND, STEER_BOUND],
    'steer__5': [-STEER_BOUND, STEER_BOUND],
    'steer__10': [-STEER_BOUND, STEER_BOUND],
    'steer__20': [-STEER_BOUND, STEER_BOUND],
    'steer__50': [-STEER_BOUND, STEER_BOUND],

    'throttle': [-THROTTLE_BOUND, THROTTLE_BOUND],
    'throttle__5': [-THROTTLE_BOUND, THROTTLE_BOUND],
    'throttle__10': [-THROTTLE_BOUND, THROTTLE_BOUND],
    'throttle__20': [-THROTTLE_BOUND, THROTTLE_BOUND],
    'throttle__50': [-THROTTLE_BOUND, THROTTLE_BOUND],

    'speed': [0, 80],
}
