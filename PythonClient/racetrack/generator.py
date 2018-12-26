import numpy as np
from collections import OrderedDict
from copy import deepcopy

from config import DTYPE
from utils import compose_input_for_nn, get_ordered_dict_for_labels

from joblib import Parallel, delayed


FUNCS = {
    'mean': np.mean,
    'last': lambda x: x[-1],
}


def get_data_gen(
    X, labels, weights, num_X_channels, num_Xdiff_channels, outputs_spec,
    flip_prob=.5, val_part=100, validation=False
):
    assert len(weights) == X.shape[0]

    future_labels_spec = {
        label_name: (label_name.split('__')[0], int(label_name.split('__')[1]), FUNCS[label_name.split('__')[2]])
        # E.g. "steer__50__mean": ("steer", 50)
        for label_name in outputs_spec.keys() if '__' in label_name
    }
    furthest_into_the_future = max(
        [0] + [steps_into_future for _, steps_into_future, _ in future_labels_spec.values()]
    )
    steer_labels = [
        label_name
        for label_name in outputs_spec.keys() if 'steer' in label_name
    ]

    num_channels_max = max(num_X_channels, num_Xdiff_channels+1)

    margin = max(num_X_channels, num_Xdiff_channels+1)
    range_ = np.arange(margin, len(weights)-furthest_into_the_future)
    which_for_val = (range_ % val_part == 0)

    range_ = range_[which_for_val] if validation else range_[~which_for_val]
    weights = weights[range_]

    # Make sure `weights` are normalized
    weights /= weights.sum()

    while True:
        index = np.random.choice(range_, p=weights)
        slc = slice(index-num_channels_max, index+1)
        X_out = compose_input_for_nn(X[slc], num_X_channels, num_Xdiff_channels)
        labels_out = {
            label_name: labels[label_name][index]
            for label_name in outputs_spec.keys() if '__' not in label_name
        }

        future_labels_out = {
            label_name: func(labels[source_label_name][index:index+steps_into_future])
            for label_name, (source_label_name, steps_into_future, func) in future_labels_spec.items()
        }
        labels_out = {**labels_out, **future_labels_out}

        if np.random.rand() < flip_prob:
            X_out = X_out[:, :, ::-1, :]
            for steer_label in steer_labels:
                labels_out[steer_label] = -labels_out[steer_label]

        yield X_out, labels_out


def batcher(gen, batch_size, outputs_spec, speed_as_input):
    if speed_as_input:
        assert 'speed' in outputs_spec

    # Just to initialize
    x, stats = next(gen)
    x_shape = (batch_size, *x.shape[1:])
    if speed_as_input:
        outputs_spec = remove_speed_labels_from_outputs_spec(deepcopy(outputs_spec))

    while True:
        X = np.zeros(x_shape, dtype=DTYPE)
        if speed_as_input:
            speed = np.zeros((batch_size, 1), dtype=DTYPE)
        Y = get_ordered_dict_for_labels(outputs_spec, batch_size)
        for i in range(batch_size):
            x, stats = next(gen)
            X[i] = x
            if speed_as_input:
                speed[i][0] = stats['speed']
            for label_name in outputs_spec.keys():
                Y[label_name][i] = stats[label_name]

        if speed_as_input:
            X = [X, speed]
            
        yield X, list(Y.values())


def remove_speed_labels_from_outputs_spec(outputs_spec):
    for label_name in outputs_spec.keys():
        if 'speed' in label_name:
            del outputs_spec[label_name]
    return outputs_spec


# def batcher2(gen, batch_size, outputs_spec, speed_as_input):
#     if speed_as_input:
#         assert 'speed' in outputs_spec
#     # Just to initialize
#     x, stats = next(gen)
#     x_shape = (batch_size, *x.shape[1:])
#     if speed_as_input:
#         outputs_spec = remove_speed_labels_from_outputs_spec(outputs_spec)
#
#     while True:
#         X = np.zeros(x_shape, dtype=DTYPE)
#         Y = get_ordered_dict_for_labels(outputs_spec, batch_size)
#         res = Parallel(n_jobs=2, prefer='threads')(delayed(lambda: next(gen))() for i in range(batch_size))
#         for i in range(batch_size):
#             x, stats = res[i]
#             X[i] = x
#             for label_name in outputs_spec.keys():
#                 Y[label_name][i] = stats[label_name]
#
#         yield X, list(Y.values())
