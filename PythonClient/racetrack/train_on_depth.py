import argparse
import os
import shutil
import pandas as pd
import numpy as np

from sklearn.metrics import mean_squared_error
from scipy.stats import pearsonr

import keras.backend as K
from keras.models import Model
from keras.layers import Dropout, Flatten, Dense, Input, Conv2D
from keras.layers.pooling import MaxPooling2D
from keras.layers.advanced_activations import ELU
from keras.layers import concatenate
from keras.optimizers import Adam
from keras.regularizers import l2

import matplotlib.pyplot as plt
import matplotlib as mpl
mpl.style.use('seaborn-dark-palette')
mpl.style.use('seaborn-whitegrid')

from generator import get_data_gen, batcher
from config import (
    IMAGE_SIZE, THROTTLE_BOUND, STEER_BOUND,
    BATCH_SIZE, NUM_EPOCHS, NUM_EPISODES,
    PREDICT_THROTTLE,
    NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS
)

BASE_FONTSIZE = 14


def get_lenet_like_model(
        input_shape, num_outputs,
        l2_reg=1e-3, filter_sz=5, num_filters=24, num_dense_neurons=512,
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

    x = Dense(num_dense_neurons, kernel_regularizer=l2(l2_reg))(x)
    x = Dropout(.5)(x)
    x = ELU()(x)
    x = Dense(num_dense_neurons//2, kernel_regularizer=l2(l2_reg))(x)
    x = Dropout(.5)(x)
    x = ELU()(x)
    x = Dense(num_dense_neurons//4, kernel_regularizer=l2(l2_reg))(x)
    x = ELU()(x)
    x = Dense(num_outputs, kernel_regularizer=l2(l2_reg))(x)

    return Model(inp, x)


def extract_y(prefix, episode):
    DF_log = pd.read_csv('logs/{}_log{}.txt'.format(prefix, episode))
    which_OK = (DF_log['speed'] > 0.01)
    steer = DF_log[which_OK]['steer']
    throttle = DF_log[which_OK]['throttle']
    return which_OK, steer, throttle


def get_data(prefix, episode):
    which_OK, steer, throttle = extract_y(prefix, episode)
    X = (
        pd.np.load('depth_data/{}_depth_data{}.npy'.format(prefix, episode))[..., which_OK]
        .transpose([2, 0, 1])
    )
    # Need to expand dimensions to be able to use convolutions
    X = np.expand_dims(X, 3)

    return X, steer.values, throttle.values


def get_preds_and_plot(
    model,
    X_train, steer_train, throttle_train,
    X_test, steer_test, throttle_test,
    num_X_channels, num_Xdiff_channels, use_throttle,
    epoch, controller_name, results_dir
):
    local_batch_size = 128
    num_batches = 200
    y_shape = (local_batch_size * num_batches, 2) if use_throttle else (local_batch_size * num_batches, 1)
    y_train = np.zeros(y_shape)
    y_test = np.zeros(y_shape)
    preds_train = np.zeros(y_shape)
    preds_test = np.zeros(y_shape)

    train_gen = get_data_gen(
        X_train, steer_train, throttle_train,
        num_X_channels, num_Xdiff_channels,
        validation=False,
        flip_prob=0.5
    )
    train_batch_gen = batcher(train_gen, local_batch_size, use_throttle)

    test_gen = get_data_gen(
        X_test, steer_test, throttle_test,
        num_X_channels, num_Xdiff_channels,
        validation=False,
        flip_prob=0.5
    )
    test_batch_gen = batcher(test_gen, local_batch_size, use_throttle)

    for i in range(num_batches):
        X_train, y_train_tmp = next(train_batch_gen)
        X_test, y_test_tmp = next(test_batch_gen)
        y_train[local_batch_size*i:local_batch_size*(i+1)] = y_train_tmp
        y_test[local_batch_size*i:local_batch_size*(i+1)] = y_test_tmp

        preds_train_tmp = model.predict(X_train)
        preds_test_tmp = model.predict(X_test)
        preds_train[local_batch_size*i:local_batch_size*(i+1)] = preds_train_tmp
        preds_test[local_batch_size*i:local_batch_size*(i+1)] = preds_test_tmp

    steer_train = y_train[:, 0]
    steer_test = y_test[:, 0]
    plot_scatter(
        controller_name,
        'steer',
        steer_train, steer_test,
        preds_train[:, 0], preds_test[:, 0],
        STEER_BOUND,
        epoch,
        results_dir
    )

    if use_throttle:
        throttle_train = y_train[:, 1]
        throttle_test = y_test[:, 1]
        plot_scatter(
            controller_name,
            'throttle',
            throttle_train, throttle_test,
            preds_train[:, 1], preds_test[:, 1],
            THROTTLE_BOUND,
            epoch,
            results_dir
        )
        mse = mean_squared_error(np.c_[steer_test, throttle_test], preds_test)
    else:
        mse =  mean_squared_error(steer_test, preds_test)

    return mse


def plot_scatter(controller_name, actuator_name, y_train, y_test, preds_train, preds_test, bound, epoch, results_dir):
    plt.plot([-bound, bound], [-bound, bound], 'k:', alpha=0.5)
    scatter_train = plt.scatter(y_train, preds_train, alpha=0.5)
    scatter_test = plt.scatter(y_test, preds_test, alpha=0.5)

    plt.xlim(-bound, bound)
    plt.ylim(-bound, bound)
    plt.title('Epoch {: >5}'.format(epoch), fontsize=BASE_FONTSIZE)
    plt.xlabel('{}_true'.format(actuator_name), fontsize=BASE_FONTSIZE-4)
    plt.ylabel('{}_pred'.format(actuator_name), fontsize=BASE_FONTSIZE-4)

    corr_train = pearsonr(y_train, preds_train)[0]
    corr_test = pearsonr(y_test, preds_test)[0]
    plt.legend(
        handles=[scatter_train, scatter_test],
        labels=['Train (corr: {:.2f})'.format(corr_train), 'Test (corr: {:.2f})'.format(corr_test)],
        loc='lower right',
        fontsize=BASE_FONTSIZE-6
    )
    plt.savefig('{}/{}_scatter{}.png'.format(results_dir, actuator_name, epoch))
    plt.clf()


def concat(array, coeff=-1):
    return np.concatenate([array, coeff*array], axis=0)


def generator_fit(model, X, steer, throttle, num_X_channels, num_Xdiff_channels, batch_size, predict_throttle):
    train_gen = get_data_gen(
        X, steer, throttle,
        num_X_channels, num_Xdiff_channels,
        validation=False,
        flip_prob=0.5
    )
    train_batch_gen = batcher(train_gen, batch_size, predict_throttle)

    valid_gen = get_data_gen(
        X, steer, throttle,
        num_X_channels, num_Xdiff_channels,
        validation=True,
        flip_prob=0.0
    )
    valid_batch_gen = batcher(valid_gen, batch_size, predict_throttle)

    epoch_size = 2*X.shape[0]  # To account for mirror reflections

    model.fit_generator(
        train_batch_gen,
        steps_per_epoch=epoch_size//batch_size,
        epochs=1,
        validation_data=valid_batch_gen,
        validation_steps=0.01*epoch_size,
    )

    return model


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-c', '--controller',
        default='mpc',
        dest='controller_name',
        help='Controller name')
    argparser.add_argument(
        '-lr', '--learning_rate',
        default=1e-5,
        type=float,
        dest='learning_rate',
        help='Learning rate')
    argparser.add_argument(
        '-p', '--patience',
        default=100,
        type=int,
        dest='patience',
        help='Early stopping patience')
    argparser.add_argument(
        '-r', '--results_dir',
        default=None,
        dest='results_dir',
        help='Directory for storing the results')

    args = argparser.parse_args()

    if args.results_dir:
        results_dir = args.results_dir
    else:
        results_dir = '{controller}_{throttle}_{num_X_channels}_{num_Xdiff_channels}'.format(
            controller=args.controller_name,
            throttle=('throttle' if PREDICT_THROTTLE else 'noThrottle'),
            num_X_channels='{}Xchan'.format(NUM_X_CHANNELS),
            num_Xdiff_channels='{}dXchan'.format(NUM_X_DIFF_CHANNELS),
        )
    if not os.path.exists(results_dir):
        os.makedirs(results_dir)

    shutil.copy('config.py', results_dir)

    num_outputs = 2 if PREDICT_THROTTLE else 1

    model = get_lenet_like_model(
        input_shape=(*IMAGE_SIZE, NUM_X_CHANNELS+NUM_X_DIFF_CHANNELS),
        num_outputs=num_outputs,
    )

    model.compile(
        loss='mse',
        optimizer=Adam(lr=args.learning_rate),
        metrics=['mse']
    )

    # Get the test set
    X_test, steer_test, throttle_test = get_data(args.controller_name, NUM_EPISODES-1)

    errors = []
    min_mse = 10000
    for epoch in range(1, NUM_EPOCHS+1):
        for episode in range(NUM_EPISODES-1):
            print('EPOCH: {}, EPISODE: {}'.format(epoch, episode))

            X, steer, throttle = get_data(args.controller_name, episode)

            model = generator_fit(model, X, steer, throttle, NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS, BATCH_SIZE, PREDICT_THROTTLE)

        if epoch % 5 == 0:
            model.save('{}/{}_model{}.h5'.format(results_dir, args.controller_name, epoch))

        mse = get_preds_and_plot(
            model,
            X, steer, throttle,
            X_test, steer_test, throttle_test,
            NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS, PREDICT_THROTTLE,
            epoch, args.controller_name, results_dir
        )
        print('Test MSE: {:.4f}'.format(mse))

        if mse < min_mse:
            model.save('{}/{}_model_best.h5'.format(results_dir, args.controller_name))

        errors.append(mse)
        min_mse = min(errors)

        position_best = errors.index(min_mse)
        if (len(errors)-position_best) == args.patience:
            break

        plt_title = (
            'MSE on the test set\nthrottle={}, num_X_channels={}, num_Xdiff_channels={}'
            .format(PREDICT_THROTTLE, NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS)
        )
        plt.title(plt_title, fontsize=BASE_FONTSIZE)
        plt.xlabel('epoch', fontsize=BASE_FONTSIZE-4)
        plt.ylabel('MSE', fontsize=BASE_FONTSIZE-4)
        ticks = [1] + list(range(5, len(errors)+1, 5))
        plt.xticks(ticks)
        plt.ylim(0, 0.07)
        plt.xlim(1, len(errors)+1)
        plt.plot(range(1, len(errors)+1), errors, alpha=0.5, marker='o', linestyle='--')
        plt.savefig('{}/mse.png'.format(results_dir))
        plt.clf()

    pd.to_pickle(errors, '{}/errors.pkl'.format(results_dir))


if __name__ == '__main__':
    main()
