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


INPUT_SHAPE = (150, 200, 1)
BATCH_SIZE = 4
NUM_EPOCHS = 150
NUM_EPISODES = 10
EPSILON = 1
STEER_BOUND = 4


def get_lenet_like_model(
        l2_reg=1e-3, filter_sz=5, num_filters=16,
        aux_inputs=None,
):
    inp = Input(INPUT_SHAPE)
    if aux_inputs:
        add_inp = [Input(shape) for shape in aux_inputs]
    else:
        add_inp = []
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
    x = concatenate([x] + add_inp)

    x = Dense(256, kernel_regularizer=l2(l2_reg))(x)
    x = Dropout(.5)(x)
    x = ELU()(x)
    x = Dense(128, kernel_regularizer=l2(l2_reg))(x)
    x = Dropout(.5)(x)
    x = ELU()(x)
    x = Dense(64, kernel_regularizer=l2(l2_reg))(x)
    x = ELU()(x)
    x = Dense(1, kernel_regularizer=l2(l2_reg))(x)

    return Model([inp] + add_inp, x)


def extract_y(episode):
    DF_log = pd.read_csv('log{}.txt'.format(episode))
    which_OK = (DF_log['speed'] > 0.01)
    steer = DF_log[which_OK]['steer']
    speed = DF_log[which_OK]['speed']
    return which_OK, steer, speed


def get_data(episode):
    which_OK, steer, speed = extract_y(episode)
    X = (
        pd.np.load('depth_data{}.npy'.format(episode))[..., which_OK]
        .transpose([2, 0, 1])
    )
    return X, steer, speed


def get_preds_and_plot(
    X_train, X_aux_train, y_train,
    X_test, X_aux_test, y_test,
    model
):
    preds_train = model.predict([X_train, X_aux_train])
    preds_test = model.predict([X_test, X_aux_test])

    plt.scatter(y_train, preds_train)
    plt.scatter(y_test, preds_test)
    plt.xlim(-STEER_BOUND, STEER_BOUND)
    plt.ylim(-STEER_BOUND, STEER_BOUND)
    plt.savefig('images/scatter{}.png'.format(epoch))
    plt.clf()


def weighted_mse(y_true, y_pred):
    w = K.abs(y_true + EPSILON)
    return K.mean(w * K.square(y_true-y_pred))


def concat_negative(array):
    return np.concatenate([array, -array], axis=0)


def prepare_data(X, steer, speed):
    X = np.diff(X, axis=0)
    y = steer[:-1]
    # Pad with a zero in front
    curr_steer = np.pad(steer[1:-1], (1, 0), 'constant')
    curr_speed = np.pad(speed[1:-1], (1, 0), 'constant')

    # A bit of augmentation
    X = np.concatenate([X, X[:, :, ::-1]], axis=0)
    y = concat_negative(y)
    curr_steer = concat_negative(curr_steer)
    curr_speed = concat_negative(curr_speed)

    # Need to expand dimensions to be able to use convolutions
    X = np.expand_dims(X, 3)

    X_aux = np.c_[curr_steer, curr_speed]

    return X, X_aux, y


if __name__ == "__main__":
    model = get_lenet_like_model(
        aux_inputs=[(2, )]
    )

    model.compile(
        loss='mse',  # 'mse'
        optimizer=Adam(lr=1e-5),
        metrics=['mse']
    )

    for epoch in range(NUM_EPOCHS):
        for episode in range(NUM_EPISODES-1):
            print('EPOCH: {}, EPISODE: {}'.format(epoch, episode))

            X, steer, speed = get_data(episode)

            X, X_aux, y = prepare_data(X, steer, speed)

            model.fit(
                [X, X_aux],
                y,
                batch_size=BATCH_SIZE,
                epochs=1,
                verbose=1,
                validation_split=0.05,
            )

        # Prepare the test set
        X_test, steer_test, speed_test = get_data(NUM_EPISODES-1)
        X_test, X_aux_test, y_test = prepare_data(X_test, steer_test, speed_test)
        get_preds_and_plot(
            X, X_aux, y,
            X_test, X_aux_test y_test,
            model
        )

    model.save('model.h5')
