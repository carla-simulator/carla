import numpy as np
import cv2
import scipy.ndimage as ndi
from gym import spaces

from stable_baselines3.common.preprocessing import is_image_space
from stable_baselines3.common.vec_env.base_vec_env import VecEnv, VecEnvStepReturn, VecEnvWrapper


class VecResizeImage(VecEnvWrapper):
    """
    Resize HxW

    :param venv: (VecEnv)
    """

    def __init__(self, venv: VecEnv):
        assert is_image_space(venv.observation_space), "The observation space must be an image"
        self.height = 90
        self.width = 160

        observation_space = self.resize_space(venv.observation_space)
        super(VecResizeImage, self).__init__(venv, observation_space=observation_space)

    def resize_space(self, observation_space: spaces.Box) -> spaces.Box:
        """
        Resize an observation space (resize image).

        :param observation_space: (spaces.Box)
        :return: (spaces.Box)
        """
        assert is_image_space(observation_space), "The observation space must be an image"
        self.ori_height, self.ori_width, channels = observation_space.shape
        new_shape = (self.height, self.width, channels)
        return spaces.Box(low=0, high=255, shape=new_shape, dtype=observation_space.dtype)

    def resize_image(self, image: np.ndarray) -> np.ndarray:
        """
        Resize an image or batch of images.

        :param image: (np.ndarray)
        :return: (np.ndarray)
        """
        if len(image.shape) == 3:
            return cv2.resize(image, (self.width, self.height), interpolation=INTER_LINEAR)
        return ndi.zoom(image, (1, self.height/self.ori_height, self.width/self.ori_width, 1), order=2)

    def step_wait(self) -> VecEnvStepReturn:
        observations, rewards, dones, infos = self.venv.step_wait()
        return self.resize_image(observations), rewards, dones, infos

    def reset(self) -> np.ndarray:
        """
        Reset all environments
        """
        return self.resize_image(self.venv.reset())

    def close(self) -> None:
        self.venv.close()


