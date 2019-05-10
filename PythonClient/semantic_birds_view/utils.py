import os
import numpy as np
from numpy.linalg import norm
import matplotlib.pyplot as plt
from math import ceil, floor
from pathlib import Path
from operator import itemgetter

from threading import Lock
import concurrent.futures
import itertools

from moviepy.editor import VideoClip
from moviepy.video.io.bindings import mplfig_to_npimage

from keras.models import load_model


LABELS = {
    0: 'None',
    1: 'Buildings',
    2: 'Fences',
    3: 'Other',
    4: 'Pedestrians',
    5: 'Poles',
    6: 'RoadLines',
    7: 'Roads',
    8: 'Sidewalks',
    9: 'Vegetation',
    10: 'Vehicles',
    11: 'Walls',
    12: 'TrafficSigns',
}

REVERSE_LABELS = dict(zip(LABELS.values(), LABELS.keys()))

IMAGE_SHAPE = (200, 300)


def plot_semantic(x, cmap='gist_stern'):
    num_classes = x.shape[-1]
    plt.clf()
    plt.figure(figsize=(40, 10))
    plt.imshow(np.argmax(x[0, ...], axis=2), vmin=0, vmax=num_classes, cmap=cmap)
    plt.show()


def class_names_to_class_numbers(class_names):
    return [
        [REVERSE_LABELS[c] for c in classes]
        for classes in class_names
    ]


def trim(x, trim_to_be_divisible_by):
    height, width = x.shape[0:2]

    divisor = trim_to_be_divisible_by  # Easier to read
    remainder = height % divisor
    top_trim, bottom_trim = floor(remainder / 2), ceil(remainder / 2)

    remainder = width % divisor
    left_trim, right_trim = floor(remainder / 2), ceil(remainder / 2)

    return x[bottom_trim:(height-top_trim), left_trim:(width-right_trim)].astype('uint8')


def one_storage(
    camera_id, decimation, trim_to_be_divisible_by,
    episode_len, num_racetracks, num_episodes
):
    divisor = trim_to_be_divisible_by  # Easier to read
    height = IMAGE_SHAPE[0] // decimation - IMAGE_SHAPE[0] // decimation % divisor
    width = IMAGE_SHAPE[1] // decimation - IMAGE_SHAPE[1] // decimation % divisor
    if 'Top' in camera_id:
        height, width = width, height

    # I've also experimented with RGB
    num_channels = 1 if 'SS' in camera_id else 3
    return np.zeros(
        (height, width, num_channels, episode_len * num_racetracks * num_episodes),
        'uint8'
    )


def get_X_and_Y_old(
    racetracks, episodes, decimation, camera_ids,
    storage=None, trim_to_be_divisible_by=8, episode_len=1000
):
    num_racetracks = len(racetracks)
    num_episodes = len(episodes)
    expected_shape = num_racetracks*num_episodes*episode_len

    if storage is None or storage[camera_ids[0]].shape[-1] != expected_shape:
        storage = {
            id_: one_storage(id_, decimation, trim_to_be_divisible_by, episode_len, num_racetracks, num_episodes)
            for id_ in camera_ids
        }

    for racetrack_idx, racetrack in enumerate(racetracks):
        for episode_idx, episode in enumerate(episodes):
            for id_ in camera_ids:
                start = (racetrack_idx*num_episodes + episode_idx) * episode_len
                end = start + episode_len

                storage[id_][:, :, :, start:end] = trim(
                    np.load(
                        'camera_storage/{}_{}_{}.npy'
                        .format(id_, racetrack, episode)
                    )[::decimation, ::decimation],
                    trim_to_be_divisible_by
                )

    return storage


def get_X_and_Y(
    racetracks, episodes, decimation, camera_ids,
    storage=None, trim_to_be_divisible_by=8, episode_len=1000
):
    num_racetracks = len(racetracks)
    num_episodes = len(episodes)
    expected_shape = num_racetracks*num_episodes*episode_len

    if storage is None or storage[camera_ids[0]].shape[-1] != expected_shape:
        storage = {
            id_: one_storage(id_, decimation, trim_to_be_divisible_by, episode_len, num_racetracks, num_episodes)
            for id_ in camera_ids
        }

    lock = Lock()
    def update_storage(camera_id, racetrack_idx, episode_idx):
        start = (racetrack_idx*num_episodes + episode_idx) * episode_len
        end = start + episode_len

        racetrack = racetracks[racetrack_idx]
        episode = episodes[episode_idx]

        to_store = trim(
            np.load(
                'camera_storage/{}_{}_{}.npy'
                .format(camera_id, racetrack, episode)
            )[::decimation, ::decimation],
            trim_to_be_divisible_by
        )

        # To avoid a race condition (I'm not 100% sure if one would occur,
        # but it's better to be safe than sorry)
        with lock:
            storage[camera_id][:, :, :, start:end] = to_store

    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
        tripples = itertools.product(camera_ids, range(num_racetracks), range(num_episodes))
        for tripple in tripples:
            executor.submit(update_storage, *tripple)

    return storage


def unwrap_to_ohe(x, classes_numbers):
    x = np.stack([
        np.where(np.isin(x, classes_numbers[i]), 1, 0).astype(np.uint8)
        for i in range(len(classes_numbers))
    ])
    return np.transpose(x[..., 0], [1, 2, 0])


def extract_observation_for_batch(X, y, index, flip, classes_numbers):
    X_out = [x[..., index] for x in X]
    y_out = y[..., index]

    if flip:
        X_out = [np.fliplr(x) for x in X_out]
        X_out[1], X_out[2] = X_out[2], X_out[1]
        y_out = np.fliplr(y_out)

    # After mirror flipping we can transpose `y`
    y_out = np.fliplr(np.transpose(y_out, [1, 0, 2]))

    X_out = [unwrap_to_ohe(x, classes_numbers) for x in X_out]
    y_out = unwrap_to_ohe(y_out, classes_numbers)

    return X_out, y_out


def get_data_gen(
    X, y, classes_names,
    flip_prob=.5, val_part=25, validation=False,
):
    range_ = np.arange(X[0].shape[-1])
    which_for_val = (range_ % val_part == 0)
    range_ = range_[which_for_val] if validation else range_[~which_for_val]

    classes_numbers = class_names_to_class_numbers(classes_names)

    while True:
        index = np.random.choice(range_)

        flip = (np.random.rand() < flip_prob)
        X_out, y_out = extract_observation_for_batch(X, y, index, flip, classes_numbers)

        yield X_out, y_out


def batcher(gen, batch_size, zero_array=None, cast_to=None):
    # Just to initialize
    x, y = next(gen)
    x_shape = (batch_size, *x[0].shape)
    y_shape = (batch_size, *y.shape)

    while True:
        X = [np.zeros(x_shape, dtype=np.uint8) for _ in range(len(x))]
        Y = np.zeros(y_shape, dtype=np.uint8)
        for i in range(batch_size):
            x, y = next(gen)
            for j in range(len(x)):
                X[j][i] = x[j]
                if cast_to is not None:
                    X[j][i] = X[j][i].dtype(cast_to)
            Y[i] = y
            if cast_to is not None:
                Y[i] = Y[i].dtype(cast_to)

        out_X = X + [Y]
        out_Y = X + [Y, Y] if zero_array is None else X + [Y, Y, zero_array]

        yield out_X, out_Y


def sequential_batcher(
    X, Y, classes_names, return_sequences=True,
    batch_size=8, sequence_len=16, episode_len=1000,
    val_part=0.05, validation=False, flip_prob=0.5,
):
    classes_numbers = class_names_to_class_numbers(classes_names)

    num_episodes = X[0].shape[-1] // episode_len

    x, y = extract_observation_for_batch(X, Y, 0, False, classes_numbers)
    x_shape = (batch_size, sequence_len, *x[0].shape)
    y_shape = (batch_size, sequence_len, *y.shape)

    if validation:
        range_ = range(0, int(val_part*(episode_len - sequence_len)))
    else:
        range_ = range(int(val_part*(episode_len - sequence_len)), episode_len - sequence_len)

    while True:
        X_out = [np.zeros(x_shape) for x in X]
        y_out = np.zeros(y_shape)
        flips = np.random.choice([True, False], batch_size, p=[flip_prob, 1-flip_prob])
        starting_points = np.random.choice(range_, batch_size)
        episodes = np.random.choice(range(num_episodes), batch_size)
        starting_points += episode_len * episodes

        for batch_idx in range(batch_size):
            for sequence_idx in range(sequence_len):
                flip = flips[batch_idx]
                x, y = extract_observation_for_batch(
                    X, Y,
                    starting_points[batch_idx] + sequence_idx,
                    flips[batch_idx],
                    classes_numbers,
                )
                for i in range(len(x)):
                    X_out[i][batch_idx, sequence_idx] = x[i]
                y_out[batch_idx, sequence_idx] = y

        y_out = y_out if return_sequences else y_out[:, -1]

        yield X_out, y_out


def make_movie(
    multi_model_path, racetrack, episode, decimation, classes_names, camera_ids,
    multi_model=None, episode_len=1000, fps=20, which_preds=5, batch_size=32,
    cmap='gist_stern'
):
    if multi_model is None:
        multi_model = load_model(multi_model_path)

    storage = get_X_and_Y([racetrack], [episode], decimation, camera_ids)
    X = [storage[id_] for id_ in camera_ids if 'Top' not in id_]
    Y = [storage[id_] for id_ in camera_ids if 'Top' in id_][0]

    classes_numbers = class_names_to_class_numbers(classes_names)

    X_final, y_final = [[] for _ in range(len(X))], []
    for index in range(episode_len):
        X_out, y_out = extract_observation_for_batch(X, Y, index, False, classes_numbers)
        for j in range(len(X_final)):
            X_final[j].append(X_out[j])
        y_final.append(y_out)

    X_final = [np.stack(x) for x in X_final]
    y_final = np.stack(y_final)

    preds = multi_model.predict(X_final + [y_final], batch_size=batch_size)

    data = preds[which_preds]  # Easier to read

    duration = data.shape[0] // fps

    fig, ax = plt.subplots()

    ax.grid(False)

    fig.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=0, hspace=0)
    ax.margins(0, 0)

    def make_frame(t, data):
        frame_idx = int(t*fps)

        height, width, num_classes = y_final.shape[1:]

        unit_length = int(height / 2)

        frame_height = int(6.8 * unit_length)
        frame_width = int(15.5 * unit_length)

        color_shift = 0
        frame = np.zeros((frame_height, frame_width)) + num_classes + color_shift

        one_fifth = int(unit_length / 5)

        # Front
        gap_1 = int(1.8 * unit_length)
        frame[one_fifth:(one_fifth+height), gap_1:(gap_1+width)] = np.argmax(X_final[0][frame_idx], axis=2)

        # Left
        gap_2 = int(one_fifth + height + one_fifth)
        frame[gap_2:(gap_2+height), one_fifth:(one_fifth+width)] = np.argmax(X_final[1][frame_idx], axis=2)

        # Right
        gap_3 = one_fifth + width + one_fifth
        frame[gap_2:(gap_2+height), gap_3:(gap_3+width)] = np.argmax(X_final[2][frame_idx], axis=2)

        # Rear
        gap_4 = one_fifth + height + one_fifth + height + one_fifth
        frame[gap_4:(gap_4+height), gap_1:(gap_1+width)] = np.argmax(X_final[3][frame_idx], axis=2)

        # Top (true)
        gap_5 = gap_1 + width + gap_1 + one_fifth
        frame[2*one_fifth:(2*one_fifth+2*width), gap_5:(gap_5+2*height)] = np.flipud(np.kron(
            np.argmax(y_final[frame_idx], axis=2).T,
            np.ones((2, 2))
        ))

        # Top (pred)
        gap_6 = gap_5 + 2*height + 2*one_fifth
        frame[2*one_fifth:(2*one_fifth+2*width), gap_6:(gap_6+2*height)] = np.flipud(np.kron(
            np.argmax(data[frame_idx], axis=2).T,
            np.ones((2, 2))
        ))

        ax.clear()
        ax.imshow(frame, cmap=cmap, aspect='auto', vmin=0, vmax=data.shape[-1])
        ax.axis('off')
        return mplfig_to_npimage(fig)


    animation = VideoClip(
        lambda t: make_frame(t, data),
        duration=duration
    )
    file_name = multi_model_path.replace('models/', '').replace('.h5', '')
    file_name = os.path.join('movies', file_name)
    file_name += '_racetrack=' + racetrack
    file_name += '_episode=' + str(episode)
    file_name += '.mp4'
    animation.write_videofile(file_name, fps=fps)

    return multi_model


def find_waypoints(
    frame,
    road_class=0, max_angle=40*np.pi/180, num_angles=12, steps=5, step_len=10,
    sphere_radius=7, draw_waypoints=True,
    grass_class=1, grass_class_weight=-0.5,
    car_class=2, car_class_weight=-1,
):
    # This is the center of the frame and where the car is
    starting_point = np.array([frame.shape[0] // 2, frame.shape[1] // 2])

    waypoints = []
    angle = 1.5 * np.pi  # "Go straight" angle
    point = starting_point
    for step in range(steps):
        scores = []
        for angle_idx in range(-num_angles, num_angles+1):
            # We're iterating over candidate angles that for a cone of [-40, 40] degrees
            angle_i = angle + angle_idx * max_angle / num_angles
            vector = step_len * np.array([np.sin(angle_i), np.cos(angle_i)])
            new_position = point + vector

            lower_bound, upper_bound = (
                (new_position - sphere_radius).astype(int),
                (new_position + sphere_radius).astype(int),
            )
            x, y = np.meshgrid(
                np.arange(max(0, lower_bound[0]), min(frame.shape[0], upper_bound[0])),
                np.arange(max(0, lower_bound[1]), min(frame.shape[1], upper_bound[1])),
            )
            xy = np.array([x.flatten(), y.flatten()]).T
            which_in_sphere = (norm(xy - new_position, axis=1) < sphere_radius)

            x_idx, y_idx = np.round(new_position).astype(int)

            means = frame[xy[:, 0], xy[:, 1]].mean(axis=0)
            score = (
                means[road_class]
                + grass_class_weight * means[grass_class]
                + car_class_weight * means[car_class]
            )

            scores.append([score, -abs(angle_idx), angle_i, new_position])

        # We choose the waypoint that has the best score, but if two waypoints
        #  have the same score, we choose the one that's closest to going the
        #  same direction as previously
        best_score, best_angle_idx, best_angle, best_new_position = max(
            scores, key=itemgetter(0, 1)
        )
        point = best_new_position
        angle = best_angle
        waypoints.append(point)

        if draw_waypoints:
            x_idx, y_idx = np.round(best_new_position).astype(int)
            # FIXME: sometimes a waypoint goes around the frame
            frame[x_idx-1:x_idx+1, y_idx-1:y_idx+1] = [1, 1, 1]

    return waypoints, frame


def rgb_frame(preds, y_final, idx, draw_waypoints):
    predicted = np.flipud(np.transpose(preds[idx], axes=[1, 0, 2]))
    ground_truth = np.flipud(np.transpose(y_final[idx], axes=[1, 0, 2]))
    if draw_waypoints:
        _, predicted = find_waypoints(predicted)
        _, ground_truth = find_waypoints(ground_truth)
    gap = np.zeros_like(predicted)[:, ::5]
    inside = np.concatenate([predicted, gap, ground_truth], axis=1)

    width, height, num_channels = inside.shape
    _, margin, _ = gap.shape
    final_frame = np.zeros((
        margin + width + margin,
        margin + height + margin,
        num_channels
    ))
    final_frame[margin:-margin, margin:-margin] = inside

    return final_frame


def make_rgb_movie(
    birds_view_model, racetrack, episode, decimation, classes_names, camera_ids,
    episode_len=1000, fps=20, batch_size=32, draw_waypoints=True,
):

    storage = get_X_and_Y([racetrack], [episode], decimation, camera_ids)
    X = [storage[id_] for id_ in camera_ids if 'Top' not in id_]
    Y = [storage[id_] for id_ in camera_ids if 'Top' in id_][0]

    classes_numbers = class_names_to_class_numbers(classes_names)

    X_final, y_final = [[] for _ in range(len(camera_ids[:-1]))], []
    for index in range(episode_len):
        X_out, y_out = extract_observation_for_batch(X, Y, index, False, classes_numbers)
        for j in range(len(camera_ids[:-1])):
            X_final[j].append(X_out[j])
        y_final.append(y_out)

    X_final = [np.stack(x) for x in X_final]
    y_final = np.stack(y_final)

    preds = birds_view_model.predict(X_final, batch_size=batch_size)

    fig, ax = plt.subplots()

    ax.grid(False)

    fig.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=0, hspace=0)
    ax.margins(0, 0)


    def make_frame(t, preds, y_final):
        frame_idx = int(t*fps)
        frame = rgb_frame(preds, y_final, frame_idx, draw_waypoints)

        ax.clear()
        ax.imshow(frame, aspect='auto')
        ax.text(30, 177, 'predicted', color='white', fontsize=24, fontweight='bold')
        ax.text(160, 177, 'actual', color='white', fontsize=24, fontweight='bold')
        ax.axis('off')
        return mplfig_to_npimage(fig)


    duration = preds.shape[0] // fps

    animation = VideoClip(
        lambda t: make_frame(t, preds, y_final),
        duration=duration
    )
    file_name = 'birds_view_model__racetrack={}_episode={}_RGB.mp4'.format(racetrack, episode)
    file_name = Path('movies') / file_name
    file_name = str(file_name)
    animation.write_videofile(file_name, fps=fps)
