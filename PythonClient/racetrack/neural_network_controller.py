import numpy as np
import keras

from utils import clip_throttle
from abstract_controller import Controller


class NNController(Controller):
    def __init__(self, target_speed, model_dir_name, which_model, throttle_coeff):
        self.target_speed = target_speed
        assert '/' not in model_dir_name, (
            'Just to make sure, the `model_dir_name` needs to be a local'
            ' directory, no "/" allowed'
        )
        controller_name, is_throttle, X_channels, dX_channels = model_dir_name.split('_')
        self.predict_throttle = True if is_throttle == 'throttle' else False
        self.num_X_channels = int(X_channels.replace('Xchan', ''))
        self.num_Xdiff_channels = int(dX_channels.replace('dXchan', ''))
        self.num_channels_max = max(self.num_X_channels, self.num_Xdiff_channels+1)

        if which_model == 'best':
            which_model = '_best'
        model_file_name = '{model_dir_name}/{controller_name}_model{which_model}.h5'.format(
            model_dir_name=model_dir_name,
            controller_name=controller_name,
            which_model=which_model
        )
        self.model = keras.models.load_model(model_file_name)

        self.prev_depth_arrays = []
        self.steer = 0
        self.throttle = 1
        self.throttle_coeff = throttle_coeff

    def control(self, pts_2D, measurements, depth_array):
        location = self._extract_location(measurements)
        which_closest, _ = self._find_closest(pts_2D, location)

        depth_array = np.expand_dims(np.expand_dims(depth_array, 0), 3)

        self.prev_depth_arrays.append(depth_array)

        if len(self.prev_depth_arrays) < self.num_channels_max:
            pred = None
        else:
            self.prev_depth_arrays = self.prev_depth_arrays[-self.num_channels_max:]
            X_full = np.concatenate(self.prev_depth_arrays[-self.num_channels_max:])
            X = X_full[-self.num_X_channels:]
            if self.num_Xdiff_channels > 0:
                X_diff = np.diff(
                    X_full[-(self.num_Xdiff_channels+1):],
                    axis=0
                )
                X = np.concatenate([X, X_diff])

            # Move channels to the front
            X = np.transpose(X, (3, 1, 2, 0))

            pred = self.model.predict(X)

        if pred is not None:
            if self.predict_throttle:
                self.steer = pred[0][0]
                self.throttle = pred[0][1]
                self.throttle *= self.throttle_coeff
            else:
                self.steer = pred
                curr_speed = measurements.player_measurements.forward_speed * 3.6
                self.throttle = clip_throttle(
                    self.throttle,
                    curr_speed,
                    self.target_speed
                )

        one_log_dict = {
            'steer': self.steer,
            'throttle': self.throttle,
        }

        return one_log_dict, which_closest
