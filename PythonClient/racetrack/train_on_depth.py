import pandas as pd
np = pd.np

import keras.backend as K
from keras.models import Model
from keras.layers import Dropout, Flatten, Dense, Input, Conv2D
from keras.layers.pooling import MaxPooling2D
from keras.layers.advanced_activations import ELU
from keras.layers import concatenate
from keras.optimizers import Adam
from keras.regularizers import l2

import pickle
import matplotlib.pyplot as plt


IMAGE_SIZE = (150, 200)
BATCH_SIZE = 32
NUM_EPOCHS = 60
NUM_EPISODES = 10
EPSILON = 1
STEER_BOUND = 6
THROTTLE_BOUND = 1
STEER_DIFF = False

PREFIX = 'mpc'


def get_lenet_like_model(
        input_shape, output_shape,
        l2_reg=1e-3, filter_sz=5, num_filters=16,
):
    inp = Input(input_shape)
    x = Conv2D(num_filters, (filter_sz, filter_sz),
               padding='same', kernel_regularizer=l2(l2_reg))(inp)
    x = MaxPooling2D(2, 2)(x)
    x = ELU()(x)

    x = Conv2D(2*num_filters, (filter_sz, filter_sz),
               padding='same', kernel_regularizer=l2(l2_reg))(x)
    x = MaxPooling2D(2, 2)(x)
    x = ELU()(x)

    x = Conv2D(4*num_filters, (filter_sz, filter_sz),
               padding='same', kernel_regularizer=l2(l2_reg))(x)
    x = MaxPooling2D(2, 2)(x)
    x = ELU()(x)

    print('Shape before Flatten: {}'.format(x))
    x = Dropout(.5)(x)
    x = Flatten()(x)

    x = Dense(256, kernel_regularizer=l2(l2_reg))(x)
    x = Dropout(.5)(x)
    x = ELU()(x)
    x = Dense(128, kernel_regularizer=l2(l2_reg))(x)
    x = Dropout(.5)(x)
    x = ELU()(x)
    x = Dense(64, kernel_regularizer=l2(l2_reg))(x)
    x = ELU()(x)
    x = Dense(output_shape, kernel_regularizer=l2(l2_reg))(x)

    return Model(inp, x)


def extract_y(prefix, episode):
    DF_log = pd.read_csv('{}_log{}.txt'.format(prefix, episode))
    which_OK = (DF_log['speed'] > 0.01)
    steer = DF_log[which_OK]['steer']
    throttle = DF_log[which_OK]['throttle']
    return which_OK, steer, throttle


def get_data(prefix, episode):
    which_OK, steer, throttle = extract_y(prefix, episode)
    X = (
        pd.np.load('{}_depth_data{}.npy'.format(prefix, episode))[..., which_OK]
        .transpose([2, 0, 1])
    )
    # Need to expand dimensions to be able to use convolutions
    X = np.expand_dims(X, 3)

    # TODO: move the casting to float16 to the client script
    return X, steer.astype('float16'), throttle.astype('float16')


def get_preds_and_plot(X_train, y_train, X_test, y_test, model, epoch):
    preds_train = model.predict(X_train)
    preds_test = model.predict(X_test)

    plot_scatter(
        'steer',
        y_train[:, 0], y_test[:, 0],
        preds_train[:, 0], preds_test[:, 0],
        STEER_BOUND
    )

    plot_scatter(
        'throttle',
        y_train[:, 1], y_test[:, 1],
        preds_train[:, 1], preds_test[:, 1],
        THROTTLE_BOUND
    )


def plot_scatter(filename, y_train, y_test, preds_train, preds_test, bound):
    plt.scatter(y_train, preds_train)
    plt.scatter(y_test, preds_test)
    plt.xlim(-bound, bound)
    plt.ylim(-bound, bound)
    # plt.plot([-bound, -bound], [bound, bound], 'k-') TODO: fix this
    plt.savefig('images/{}_scatter{}.png'.format(filename, epoch))
    plt.clf()


def concat(array, coeff=-1):
    return np.concatenate([array, coeff*array], axis=0)


def prepare_data(X, steer, throttle):
    X = np.concatenate([
        X[1:], np.diff(X, axis=0)
    ], axis=3)

    y = steer[:-1]

    # Return mirror-reflected `X`es and corresponding `y`s
    y = np.c_[
        concat(y),
        concat(throttle[:-1], coeff=1)
    ]
    X = np.concatenate([X, X[:, :, ::-1, :]], axis=0)

    return X, y


if __name__ == "__main__":
    model = get_lenet_like_model(
        input_shape=(*IMAGE_SIZE, 2),  # two channels: depth map + diff of depth map
        output_shape=2,  # steer and throttle
    )

    model.compile(
        loss='mse',
        optimizer=Adam(lr=1e-5),
        metrics=['mse']
    )

    # Prepare the test set
    X_test, steer_test, throttle_test = get_data(PREFIX, NUM_EPISODES-1)
    X_test, y_test = prepare_data(X_test, num_diff_channels, steer_test, throttle_test)

    for epoch in range(1, NUM_EPOCHS+1):
        for episode in range(NUM_EPISODES-1):
            print('EPOCH: {}, EPISODE: {}'.format(epoch, episode))

            X, steer, throttle = get_data(PREFIX, episode)

            X, y = prepare_data(X, num_diff_channels, steer, throttle)

            model.fit(
                X,
                y,
                batch_size=BATCH_SIZE,
                epochs=1,
                verbose=1,
                validation_split=0.05,
            )

        if epoch % 5 == 0:
            model.save('{}_model{}.h5'.format(PREFIX, epoch))

        get_preds_and_plot(
            X, y,
            X_test, y_test,
            model, epoch
        )
