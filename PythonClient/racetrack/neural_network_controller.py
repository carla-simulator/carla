import numpy as np
import importlib

import keras

from utils import clip_throttle, compose_input_for_nn
from abstract_controller import Controller


class NNController(Controller):
    def __init__(self, target_speed, model_dir_name, which_model,
                 throttle_coeff_A, throttle_coeff_B,
                 ensemble_prediction,
        ):
        self.target_speed = target_speed
        assert '/' not in model_dir_name, (
            'Just to make sure, the `model_dir_name` needs to be a local'
            ' directory, no "/" allowed'
        )

        self.predict_throttle = True # TODO: move to client_controller args

        config = importlib.import_module(model_dir_name + '.config')
        if self.predict_throttle:
            self.throttle_index = [
                i for i, key in enumerate(config.OUTPUTS_SPEC) if key == 'throttle'
            ][0]
        self.speed_as_input = config.SPEED_AS_INPUT
        self.predict_throttle = ('throttle' in config.OUTPUTS_SPEC)
        self.num_X_channels = config.NUM_X_CHANNELS
        self.num_Xdiff_channels = config.NUM_X_DIFF_CHANNELS
        self.num_channels_max = max(self.num_X_channels, self.num_Xdiff_channels+1)

        if which_model == 'best':
            which_model = '_best'

        model_file_name = '{model_dir_name}/model{which_model}.h5'.format(
            model_dir_name=model_dir_name,
            which_model=which_model
        )
        self.model = keras.models.load_model(model_file_name)

        self.prev_depth_arrays = []
        self.steer = 0
        self.throttle = 1
        self.throttle_coeff_A = throttle_coeff_A
        self.throttle_coeff_B = throttle_coeff_B

        self.ensemble_prediction = ensemble_prediction
        if self.ensemble_prediction:
            steps_into_future = config.STEPS_INTO_NEAR_FUTURE
            self.actuator_indexes = {
                'steer': [0] + list(steps_into_future),
                'throttle': [self.throttle_index] + [self.throttle_index + i for i in steps_into_future],
            }
            num_preds = len(steps_into_future) + 1
            self.past_preds = {
                'steer': np.zeros((num_preds, num_preds)),
                'throttle': np.zeros((num_preds, num_preds)),
            }

        if self.ensemble_prediction and not self.predict_throttle:
            raise NotImplementedError(
                "I've only implemented aggregated steering for the case in"
                " which throttle is also being predicted (but it's easy to extend)"
            )

    def control(self, pts_2D, measurements, depth_array):
        which_closest, _, _ = self._calc_closest_dists_and_location(
            measurements,
            pts_2D
        )
        curr_speed = measurements.player_measurements.forward_speed * 3.6

        depth_array = np.expand_dims(np.expand_dims(depth_array, 0), 3)

        self.prev_depth_arrays.append(depth_array)

        if len(self.prev_depth_arrays) < self.num_channels_max:
            pred = None
        else:
            self.prev_depth_arrays = self.prev_depth_arrays[-self.num_channels_max:]
            X_full = np.concatenate(self.prev_depth_arrays[-self.num_channels_max:])
            X = compose_input_for_nn(X_full, self.num_X_channels, self.num_Xdiff_channels)


            if self.speed_as_input:
                pred = self.model.predict([X, np.array([curr_speed])])
            else:
                pred = self.model.predict(X)

        if pred is not None:
            if self.predict_throttle:
                curr_steer_pred = pred[0][0, 0]
                curr_throttle_pred = pred[self.throttle_index][0, 0]

                if self.ensemble_prediction:
                    curr_steer_pred = self._ensemble_prediction(pred, 'steer')
                    curr_throttle_pred = self._ensemble_prediction(pred, 'throttle')

                self.steer = curr_steer_pred
                self.throttle = curr_throttle_pred
                self.throttle = self.throttle_coeff_A * self.throttle + self.throttle_coeff_B
            else:
                self.steer = pred
                self.throttle = clip_throttle(
                    self.throttle,
                    curr_speed,
                    self.target_speed
                )

        one_log_dict = {
            'steer': self.steer,
            'throttle': self.throttle,
            'which_closest': which_closest,
        }

        return one_log_dict

    def _ensemble_prediction(self, pred, which_actuator):
        self.past_preds[which_actuator] = np.roll(self.past_preds[which_actuator], shift=1, axis=0)
        self.past_preds[which_actuator][0] = [
            pred[i][0, 0] for i in self.actuator_indexes[which_actuator]
        ]
        return np.mean(self.past_preds[which_actuator].diagonal())
