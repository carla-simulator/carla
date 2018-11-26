import random

# General, Carla-related
IMAGE_SIZE = (150, 200)
STEER_BOUND = 6
THROTTLE_BOUND = 1

# "Controller regularization"
STEER_NOISE = lambda : random.uniform(-0.1, 0.1)
THROTTLE_NOISE = lambda: random.uniform(-0.05, 0.05)

# NN training
BATCH_SIZE = 32
NUM_EPOCHS = 50
NUM_EPISODES = 10

# Modelling
PREDICT_THROTTLE = False
NUM_X_DIFF_CHANNELS = 0
NUM_X_CHANNELS = 1
