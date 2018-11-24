import numpy as np


def get_data_gen(X, steer, throttle, num_X_channels, num_X_diff_channels, flip_prob=.5, val_part=100, validation=False):
    while True:
        margin = max(num_X_channels, num_X_diff_channels+1)
        index = np.random.randint(margin, X.shape[0])
        is_for_val = (index % val_part == 0)
        if is_for_val and not validation:
            continue
        if not is_for_val and validation:
            continue

        slc = slice(index-num_X_channels+1, index+1)
        image = X[slc]
        angle = steer[index]
        accel = throttle[index]

        if num_X_diff_channels > 0:
            slc = slice(index-num_X_diff_channels, index+1)
            diff = np.diff(X[slc], axis=0)
            image = np.concatenate([image, diff])

        # Move channels to the front
        image = np.transpose(image, (3, 1, 2, 0))

        if np.random.rand() < flip_prob:
            image = image[:, :, ::-1, :]
            angle = -angle

        yield image, angle, accel


def batcher(gen, batch_size, predict_throttle):
    # Just to get shape of X to initialize an array of zeros (performance)
    x, _, _ = next(gen)
    x_shape = (batch_size, *x.shape[1:])
    y_shape = (batch_size, 2) if predict_throttle else (batch_size, 1)
    while True:
        X = np.zeros(x_shape, dtype='float16')
        Y = np.zeros(y_shape, dtype='float16')
        for i in range(batch_size):
            x, angle, accel = next(gen)
            y = np.c_[angle, accel] if predict_throttle else np.c_[angle]
            X[i] = x
            Y[i] = y
        yield X, Y
