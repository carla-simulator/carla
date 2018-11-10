import pandas as pd
np = pd.np

import keras.backend as K
from keras.models import Model
from keras.layers import Dropout, Flatten, Dense, Input, Conv2D
from keras.layers.pooling import MaxPooling2D
from keras.layers.advanced_activations import ELU
from keras.optimizers import Adam
from keras.regularizers import l2

import pickle
import matplotlib.pyplot as plt


INPUT_SHAPE = (150, 200, 1)
BATCH_SIZE = 64
NUM_EPOCHS = 150
NUM_EPISODES = 10
EPSILON = 1


def get_lenet_like_model(use_conv_1x1=True, l2_reg=1e-3, filter_sz=5, num_filters=16):
    inp = Input(INPUT_SHAPE)
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
    x = Dense(1, kernel_regularizer=l2(l2_reg))(x)

    return Model(inp, x)


def get_simplistic_model(l2_reg=1e-3):
    inp = Input(INPUT_SHAPE)
    reg_kwargs = {
        'activation': 'elu',
        'kernel_regularizer': l2(l2_reg),
        # 'activity_regularizer': l2(l2_reg)
    }
    x = Flatten()(inp)
    x = Dropout(rate=0.5)(x)
    x = Dense(1000, **reg_kwargs)(x)
    x = Dropout(rate=0.5)(x)
    x = Dense(1000, **reg_kwargs)(x)
    x = Dropout(rate=0.5)(x)
    x = Dense(1000, **reg_kwargs)(x)
    # x = Dropout(rate=0.5)(x)
    x = Dense(1)(x)
    return Model(inp, x)


def get_sophisticated_model(l2_reg=1e-3):
    inp = Input(INPUT_SHAPE)
    reg_kwargs = {
        'activation': 'elu',
        'kernel_regularizer': l2(l2_reg),
        # 'activity_regularizer': l2(l2_reg)
    }
    x = Flatten()(inp)
    x = Dropout(rate=0.5)(x)
    x = Dense(1000, **reg_kwargs)(x)
    x = Dropout(rate=0.5)(x)
    x = Dense(1000, **reg_kwargs)(x)
    x = Dropout(rate=0.5)(x)
    x = Dense(1000, **reg_kwargs)(x)
    # x = Dropout(rate=0.5)(x)
    x = Dense(1)(x)
    return Model(inp, x)


def extract_y(episode):
    DF_log = pd.read_csv('log{}.txt'.format(episode))
    which_OK = (DF_log['speed'] > 0.01)
    y = DF_log[which_OK]['steer']
    return which_OK, y


def get_data(episode):
    which_OK, y = extract_y(episode)
    X = pd.np.load('depth_data{}.npy'.format(episode))[:, :, which_OK].transpose([2, 0, 1])
    return X, y


def get_preds_and_plot(X_train, y_train, X_test, y_test, model):
    preds_train, preds_test = model.predict(X_train), model.predict(X_test)
    plt.scatter(y_train, preds_train)
    plt.scatter(y_test, preds_test)
    min_, max_ = y_test.min(), y_test.max()
    plt.xlim(-4, 4)
    plt.ylim(-4, 4)
    plt.savefig('images/scatter{}.png'.format(epoch))
    plt.clf()


def weighted_mse(y_true, y_pred):
    w = K.abs(y_true + EPSILON)
    return K.mean(w * K.square(y_true-y_pred))


def prepare_data(X, y):
    X = np.diff(X, axis=0)
    y = y[:-1]

    # A bit of augmentation
    X = np.concatenate([X, X[:, :, ::-1]], axis=0)
    y = np.concatenate([y, -y], axis=0)

    # Need to expand dimensions to be able to use convolutions
    X = np.expand_dims(X, 3)

    return X, y


if __name__ == "__main__":
    model = get_lenet_like_model()

    model.compile(
        loss='mse', # 'mse'
        optimizer=Adam(lr=1e-5),
        metrics=['mse']
    )

    X_test, y_test = get_data(NUM_EPISODES-1)

    for epoch in range(NUM_EPOCHS):
        for episode in range(NUM_EPISODES-1):
            print('EPOCH: {}, EPISODE: {}'.format(epoch, episode))

            X, y = get_data(episode)

            X, y = prepare_data(X, y)

            model.fit(
                X,
                y,
                batch_size=BATCH_SIZE,
                epochs=1,
                verbose=1,
                validation_split=0.05,
            )

        # Prepare the test set
        X_test, y_test = prepare_data(X_test, y_test)
        get_preds_and_plot(
            X, y,
            X_test, y_test,
            model
        )


    model.save('model.h5')
