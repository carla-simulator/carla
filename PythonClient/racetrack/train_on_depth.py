# python3 train_on_depth.py -tr 01,02 -te 01,02 -c mpc

import argparse
import os
from copy import deepcopy
from pathlib import Path
import shutil
import time
from datetime import datetime as dt
import pandas as pd
import numpy as np

from sklearn.metrics import mean_squared_error
from scipy.stats import pearsonr

import keras.backend as K
from keras.models import Model
from keras.layers import BatchNormalization, Dropout, Flatten, Dense, Input, Conv2D, concatenate
from keras.layers.pooling import MaxPooling2D
from keras.layers import concatenate
from keras.optimizers import Adam
from keras.regularizers import l2

import matplotlib.pyplot as plt
import matplotlib as mpl
mpl.style.use('seaborn-dark-palette')
mpl.style.use('seaborn-whitegrid')

from utils import compose_input_for_nn, get_ordered_dict_for_labels

from generator import get_data_gen, batcher, remove_speed_labels_from_outputs_spec
from config import (
    IMAGE_DECIMATION,
    IMAGE_SIZE, THROTTLE_BOUND, STEER_BOUND,
    BATCH_SIZE, NUM_EPOCHS,
    NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS,
    TRAIN_SET, TEST_SET,
    WEIGHT_EXPONENT, MIN_SPEED,
    IMAGE_CLIP_UPPER, IMAGE_CLIP_LOWER,
    SPEED_AS_INPUT, OUTPUTS_SPEC,
    ERROR_PLOT_UPPER_BOUNDS, SCATTER_PLOT_BOUNDS,
    BASE_FONTSIZE
)


def get_lenet_like_embedder(
    input_shape,
    act='elu', l2_reg=1e-3, filter_sz=5, num_filters=24, num_dense_neurons=512,
):
    """
    Returns a 2-tuple of (input, embedding_layer) that can later be used
    to create a model that builds on top of the embedding_layer.
    """
    x = inp = Input(input_shape)
    # x = BatchNormalization()(x)
    x = Conv2D(num_filters, (filter_sz, filter_sz),
               padding='same', kernel_regularizer=l2(l2_reg),
               activation=act)(x)
    x = MaxPooling2D(2, 2)(x)

    x = Conv2D(2*num_filters, (filter_sz, filter_sz),
               padding='same', kernel_regularizer=l2(l2_reg),
               activation=act)(x)
    x = MaxPooling2D(2, 2)(x)

    x = Conv2D(4*num_filters, (filter_sz, filter_sz),
               padding='same', kernel_regularizer=l2(l2_reg),
               activation=act)(x)
    x = MaxPooling2D(2, 2)(x)

    x = Dropout(.5)(x)
    x = Flatten()(x)

    x = Dense(num_dense_neurons, kernel_regularizer=l2(l2_reg), activation=act)(x)
    x = Dropout(.5)(x)
    x = Dense(num_dense_neurons//2, kernel_regularizer=l2(l2_reg), activation=act)(x)
    enc = Dropout(.5)(x)

    return inp, enc

def add_throttle_upon_steer_wo_odometry(
    outputs_spec, embed_getter,
    act='elu', l2_reg=1e-3, num_dense_neurons=512,
):
    inp, enc = embed_getter()

    steer_outputs = []
    for layer_name in outputs_spec.keys():
        if 'throttle' not in layer_name:
            x = Dense(
                num_dense_neurons//4,
                kernel_regularizer=l2(l2_reg),
                activation=act,
            )(enc)
            steer_outputs.append(
                Dense(1, kernel_regularizer=l2(l2_reg), name=layer_name)(x)
            )

    throttle_outputs = []
    for layer_name in outputs_spec.keys():
        if 'throttle' in layer_name:
            x = concatenate([enc] + steer_outputs)
            x = Dense(
                num_dense_neurons//4,
                kernel_regularizer=l2(l2_reg),
                activation=act,
            )(x)
            throttle_outputs.append(
                Dense(1, kernel_regularizer=l2(l2_reg), name=layer_name)(x)
            )

    output_layers = steer_outputs + throttle_outputs

    return Model(inp, output_layers)


def add_throttle_upon_steer_w_odometry(
    outputs_spec, embed_getter,
    act='elu', l2_reg=1e-3, num_dense_neurons=512,
):
    """Build output layers on top of the embedding layer, and return a model.

    An example of the `outputs_spec` argument is an OrderedDict specifying
    the names of the output layers, their respective: activation function,
    loss, and weight (multiplicative constant modifying the loss), e.g.:

        outputs_spec = OrderedDict(
            [('steer', {'act': 'linear', 'loss': 'mse', 'weight': 1.0})]
            + [('steer__{}__last'.format(i), {'act': 'linear', 'loss': 'mse', 'weight': 1.0}) for i in STEPS_INTO_NEAR_FUTURE]

            + [('throttle', {'act': 'sigmoid', 'loss': 'mse', 'weight': 1.0})]
            + [('throttle__{}__last'.format(i), {'act': 'sigmoid', 'loss': 'mse', 'weight': 1.0}) for i in STEPS_INTO_NEAR_FUTURE]
        )

    where `STEPS_INTO_NEAR_FUTURE` is for example `range(1, 11)` if we'd like to
    additionally predict 10 steps into the future.
    """
    inp, emb = embed_getter()

    inp_speed = Input((1, ), name='speed')

    # First, each steering angle gets its own hidden layer + a prediction neuron
    steer_outputs = []
    for layer_name in outputs_spec.keys():
        if 'steer' in layer_name:
            x = Dense(
                num_dense_neurons//4,
                kernel_regularizer=l2(l2_reg),
                activation=act,
            )(emb)
            steer_outputs.append(
                Dense(
                    1,
                    kernel_regularizer=l2(l2_reg),
                    activation=outputs_spec[layer_name]['act'],
                    name=layer_name,
                )(x)
            )

    # Now, we concatenate the embedding layer with the speed (provided as input)
    # and the outputs for the steering angles
    emb = concatenate([emb, inp_speed] + steer_outputs)
    throttle_outputs = []
    for layer_name in outputs_spec.keys():
        if 'throttle' in layer_name:
            x = Dense(
                num_dense_neurons//4,
                kernel_regularizer=l2(l2_reg),
                activation=act,
            )(emb)
            throttle_outputs.append(
                Dense(
                    1,
                    kernel_regularizer=l2(l2_reg),
                    activation=outputs_spec[layer_name]['act'],
                    name=layer_name,
                )(x)
            )

    return Model([inp, inp_speed], steer_outputs+throttle_outputs)


def extract_y(filename):
    relevant_component = filename.split('/')[1].split('_depth_data')[0]
    episode = filename.split('_depth_data')[1].split('.npy')[0]
    DF_log = pd.read_csv('logs/{}_log{}.txt'.format(relevant_component, episode))
    if 'speed' in DF_log:
        which_OK = (DF_log['speed'] > MIN_SPEED)
        speed = DF_log[which_OK]['speed']
    else:
        which_OK = DF_log.shape[0] * [True]
        speed = pd.Series(DF_log.shape[0] * [-1])
    steer = DF_log[which_OK]['steer']
    throttle = DF_log[which_OK]['throttle']
    return which_OK, steer, throttle, speed


def _get_data_from_one_racetrack(filename):
    which_OK, steer, throttle, speed = extract_y(filename)
    try:
        X = pd.np.load(filename)[..., which_OK].transpose([2, 0, 1])
    except Exception as e:
        import ipdb; ipdb.set_trace()

    if X.shape[1] != (IMAGE_CLIP_LOWER-IMAGE_CLIP_UPPER) // IMAGE_DECIMATION:
        X = X[:, IMAGE_CLIP_UPPER:IMAGE_CLIP_LOWER, :][:, ::IMAGE_DECIMATION, ::IMAGE_DECIMATION]

    # Need to expand dimensions to be able to use convolutions
    X = np.expand_dims(X, 3)

    return X, {
        'steer': steer.values,
        'throttle': throttle.values,
        'speed': speed.values
    }


def get_data(filenames):
    X_all = []
    labels_all = []
    racetrack_labels = []
    for filename in filenames:
        X, labels = _get_data_from_one_racetrack(filename)
        X_all.append(X)
        labels_all.append(labels)
        racetrack_index = int(filename.split('racetrack')[1].split('_')[0])
        racetrack_labels += len(labels)*[racetrack_index]

    label_names = labels.keys()
    X_out = np.concatenate(X_all)
    labels_out = {
        label_name: np.concatenate([labels[label_name] for labels in labels_all])
        for label_name in label_names
    }
    labels_out['racetrack'] = pd.get_dummies(racetrack_labels).values
    return X_out, labels_out


def get_preds_and_plot(
    model,
    X_train, labels_train,
    X_test, labels_test,
    num_X_channels, num_Xdiff_channels, outputs_spec, speed_as_input,
    epoch, results_dir,
    plot_bounds
):
    local_batch_size = 128
    num_batches = 200
    y_shape = local_batch_size * num_batches
    y_train = get_ordered_dict_for_labels(outputs_spec, y_shape)
    y_test = get_ordered_dict_for_labels(outputs_spec, y_shape)
    preds_train = get_ordered_dict_for_labels(outputs_spec, y_shape)
    preds_test = get_ordered_dict_for_labels(outputs_spec, y_shape)

    train_gen = get_data_gen(
        X_train, labels_train, np.ones(X_train.shape[0]),
        num_X_channels, num_Xdiff_channels, outputs_spec,
        validation=False,
        flip_prob=0.5
    )
    train_batch_gen = batcher(train_gen, local_batch_size, outputs_spec, speed_as_input)

    test_gen = get_data_gen(
        X_test, labels_test, np.ones(X_test.shape[0]),
        num_X_channels, num_Xdiff_channels, outputs_spec,
        validation=False,
        flip_prob=0.5
    )
    test_batch_gen = batcher(test_gen, local_batch_size, outputs_spec, speed_as_input)

    labels = outputs_spec.keys()
    if speed_as_input:
        labels = [label_name for label_name in labels if 'speed' not in label_name]
    for i in range(num_batches):
        X_train, y_train_tmp = next(train_batch_gen)
        X_test, y_test_tmp = next(test_batch_gen)
        for label_index, label_name in enumerate(labels):
            y_train[label_name][local_batch_size*i:local_batch_size*(i+1)] = y_train_tmp[label_index]
            y_test[label_name][local_batch_size*i:local_batch_size*(i+1)] = y_test_tmp[label_index]

        preds_train_tmp = model.predict(X_train)
        preds_test_tmp = model.predict(X_test)
        for label_index, label_name in enumerate(labels):
            # The `.reshape(-1)` is here because if there is one label the shape
            # of `preds_*` is (128), and if there are more it's (128,1)
            preds_train[label_name][local_batch_size*i:local_batch_size*(i+1)] = preds_train_tmp[label_index].reshape(-1)
            preds_test[label_name][local_batch_size*i:local_batch_size*(i+1)] = preds_test_tmp[label_index].reshape(-1)

    mse_plural = {}
    for label_name in labels:
        plot_scatter(
            label_name,
            y_train[label_name], y_test[label_name],
            preds_train[label_name], preds_test[label_name],
            plot_bounds[label_name],
            epoch,
            results_dir
        )
        mse_plural[label_name] = mean_squared_error(y_test[label_name], preds_test[label_name])

    return mse_plural


def plot_scatter(actuator_name, y_train, y_test, preds_train, preds_test, bounds, epoch, results_dir):
    plt.plot(bounds, bounds, 'k:', alpha=0.5)
    scatter_train = plt.scatter(y_train, preds_train, alpha=0.5)
    scatter_test = plt.scatter(y_test, preds_test, alpha=0.5)

    plt.xlim(*bounds)
    plt.ylim(*bounds)
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


def generator_fit(
    model, X, labels, weights, num_X_channels, num_Xdiff_channels, batch_size,
    outputs_spec, speed_as_input, verbosity=2,
):
    train_gen = get_data_gen(
        X, labels, weights,
        num_X_channels, num_Xdiff_channels, outputs_spec,
        validation=False,
        flip_prob=0.5
    )
    train_batch_gen = batcher(train_gen, batch_size, outputs_spec, speed_as_input)

    valid_gen = get_data_gen(
        X, labels, np.ones(X.shape[0]),
        num_X_channels, num_Xdiff_channels, outputs_spec,
        validation=True,
        flip_prob=0.0
    )
    valid_batch_gen = batcher(valid_gen, batch_size, outputs_spec, speed_as_input)

    epoch_size = 4*X.shape[0]  # To account for mirror reflections

    model.fit_generator(
        train_batch_gen,
        steps_per_epoch=epoch_size//batch_size,
        epochs=1,
        validation_data=valid_batch_gen,
        validation_steps=0.01*epoch_size,
        verbose=verbosity,
        # workers=8,
        # use_multiprocessing=True
    )

    return model


def plot_and_store_errors(errors, upper_plot_limit, title, results_dir):
    plt_title = (
        'MSE({}) on the test set\nnum_X_channels={}, num_Xdiff_channels={}'
        .format(title, NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS)
    )
    plt.title(plt_title, fontsize=BASE_FONTSIZE)
    plt.xlabel('epoch', fontsize=BASE_FONTSIZE-4)
    plt.ylabel('MSE', fontsize=BASE_FONTSIZE-4)
    ticks = [1] + list(range(5, len(errors)+1, 5))
    plt.xticks(ticks)
    plt.ylim(0, upper_plot_limit)
    plt.xlim(1, len(errors)+1)
    plt.plot(range(1, len(errors)+1), errors, alpha=0.5, marker='o', linestyle='--')
    plt.savefig('{}/{}_mse.png'.format(results_dir, title))
    plt.clf()
    pd.to_pickle(errors, '{}/{}_errors.pkl'.format(results_dir, title))


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-p', '--patience',
        default=100,
        type=int,
        dest='patience',
        help='Early stopping patience')
    argparser.add_argument(
        '-r', '--results_dir',
        default=dt.today().strftime('%Y-%m-%d-%H-%M'),
        dest='results_dir',
        help='Directory for storing the results')

    args = argparser.parse_args()

    results_dir = args.results_dir
    if not os.path.exists(results_dir):
        os.makedirs(results_dir)

    shutil.copy('config.py', results_dir)
    (Path(results_dir) / '__init__.py').touch()

    input_shape = (
        (IMAGE_CLIP_LOWER-IMAGE_CLIP_UPPER) // IMAGE_DECIMATION,
        IMAGE_SIZE[1] // IMAGE_DECIMATION,
        NUM_X_CHANNELS+NUM_X_DIFF_CHANNELS
    )
    embed_getter = lambda: get_lenet_like_embedder(input_shape)

    if SPEED_AS_INPUT:
        top_layer_adder = add_throttle_upon_steer_w_odometry
    else:
        top_layer_adder = add_throttle_upon_steer_wo_odometry

    model = top_layer_adder(OUTPUTS_SPEC, embed_getter)

    loss = {layer_name: spec['loss'] for layer_name, spec in OUTPUTS_SPEC.items()}
    loss_weights = {layer_name: spec['weight'] for layer_name, spec in OUTPUTS_SPEC.items()}
    if SPEED_AS_INPUT:
        for layer_name in OUTPUTS_SPEC.keys():
            if 'speed' in layer_name:
                del loss[layer_name]
                del loss_weights[layer_name]
    model.compile(
        loss=loss,
        loss_weights=loss_weights,
        optimizer=Adam(1e-5),
    )

    # Get the test set
    X_test, labels_test = get_data(TEST_SET)

    errors_storage = {key: [] for key in OUTPUTS_SPEC}
    min_error = 10000
    for epoch in range(1, NUM_EPOCHS+1):
        weights = None
        for episode, filename in enumerate(TRAIN_SET):
            print('EPOCH: {}, EPISODE: {}'.format(epoch, episode))

            print('Getting data...')
            start = time.time()
            X, labels = get_data([filename])
            print('Took {:.2f}s'.format(time.time() - start))

            if weights is None:
                weights = np.ones(X.shape[0])
            else:
                print('Calculating weights...')
                start = time.time()
                if WEIGHT_EXPONENT == 0:
                    weights = np.ones(X.shape[0])
                else:
                    # TODO: can be done more efficiently
                    preds = np.zeros(X.shape[0])
                    num_channels_max = max(NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS+1)
                    # I'm using only the errors in the steering angles as weights
                    for index in range(num_channels_max, X.shape[0]):
                        slc = slice(index-num_channels_max, index+1)
                        preds[slc] = model.predict(
                            compose_input_for_nn(X[slc], NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS)
                        )[0]  # TODO: don't hardcode 'steer' as position 0
                    weights = np.abs(preds - labels['steer'])**WEIGHT_EXPONENT
                print('Took {:.2f}s'.format(time.time() - start))

            model = generator_fit(
                model, X, labels, weights,
                NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS, BATCH_SIZE,
                OUTPUTS_SPEC, SPEED_AS_INPUT
            )

        if epoch % 5 == 0:
            model.save('{}/model{}.h5'.format(results_dir, epoch))

        errors = get_preds_and_plot(
            model,
            X, labels,
            X_test, labels_test,
            NUM_X_CHANNELS, NUM_X_DIFF_CHANNELS, OUTPUTS_SPEC, SPEED_AS_INPUT,
            epoch, results_dir,
            SCATTER_PLOT_BOUNDS,
        )

        for label_name in errors:
            errors_storage[label_name].append(errors[label_name])

        if errors['steer'] < min_error:
            print('Newest lowest steer MSE: {:.4f}'.format(errors['steer']))
            model.save('{}/model_best.h5'.format(results_dir))
        min_error = min(errors_storage['steer'])
        position_best = errors_storage['steer'].index(min_error)
        if (len(errors_storage['steer'])-position_best) == args.patience:
            break

        for label_name in errors_storage:
            upper = ERROR_PLOT_UPPER_BOUNDS[label_name]
            plot_and_store_errors(errors_storage[label_name], upper, label_name, results_dir)



if __name__ == '__main__':
    main()
