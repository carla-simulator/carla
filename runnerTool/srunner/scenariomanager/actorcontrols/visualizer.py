#!/usr/bin/python

# Copyright (c) 2021 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides an OpenCV based visualizer for camera sensors
attached to an actor.

The modul can for example be used inside an OSC Actor controller,
such as simple_vehicle_control.py

It can also be used as blueprint to implement custom visualizers.
"""

import cv2
import numpy as np

import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider


class Visualizer(object):

    """
    Visualizer class for actor controllers.

    The class provides a birdeye camera and a camera mounted in the front
    bumper of the actor. The resolution is 1000x400 for both RGB cameras.

    To use this class, it is only required to:
    1. Add an instance inside the controller constructor
        visualizer = Visualizer(actor)
    2. Call the render method on a regular basis
        visualizer.render()
    3. Call the reset method during cleanup
        visualizer.reset()

    Args:
        actor (carla.Actor): Vehicle actor the cameras should be attached to.

    Attributes:
        _actor (carla.Actor): The reference actor
        _cv_image_bird (numpy array): OpenCV image for the birdeye camera
        _cv_image_actor (numpy array): OpenCV image for the bumper camera
        _camera_bird (carla.Camera): Birdeye camera
        _camera_actor (carla.Camera): Bumper camera
        _video_writer (boolean): Flag to disable/enable writing the image stream into a video
    """

    _video_writer = False

    def __init__(self, actor):
        self._actor = actor
        self._cv_image_bird = None
        self._cv_image_actor = None
        self._camera_bird = None
        self._camera_actor = None

        bp = CarlaDataProvider.get_world().get_blueprint_library().find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', '1000')
        bp.set_attribute('image_size_y', '400')
        self._camera_bird = CarlaDataProvider.get_world().spawn_actor(bp, carla.Transform(
            carla.Location(x=20.0, z=50.0), carla.Rotation(pitch=-90, yaw=-90)), attach_to=self._actor)
        self._camera_bird.listen(lambda image: self._on_camera_update(
            image, birdseye=True))  # pylint: disable=unnecessary-lambda

        bp = CarlaDataProvider.get_world().get_blueprint_library().find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', '1000')
        bp.set_attribute('image_size_y', '400')
        self._camera_actor = CarlaDataProvider.get_world().spawn_actor(bp, carla.Transform(
            carla.Location(x=2.3, z=1.0)), attach_to=self._actor)
        self._camera_actor.listen(lambda image: self._on_camera_update(
            image, birdseye=False))  # pylint: disable=unnecessary-lambda

        if self._video_writer:
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            self._video = cv2.VideoWriter('recorded_video.avi', fourcc, 13, (1000, 800))

    def reset(self):
        """
        Reset cameras
        """
        if self._camera_bird:
            self._camera_bird.destroy()
            self._camera_bird = None
        if self._camera_actor:
            self._camera_actor.destroy()
            self._camera_actor = None

    def _on_camera_update(self, image, birdseye):
        """
        Callback for the camera sensor

        Sets the OpenCV image (_cv_image). Requires conversion from BGRA to RGB.
        """
        if not image:
            return

        image_data = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        np_image = np.reshape(image_data, (image.height, image.width, 4))
        np_image = np_image[:, :, :3]
        np_image = np_image[:, :, ::-1]
        if not birdseye:
            self._cv_image_actor = cv2.cvtColor(np_image, cv2.COLOR_BGR2RGB)
        else:
            self._cv_image_bird = cv2.cvtColor(np_image, cv2.COLOR_BGR2RGB)

    def render(self):
        """
        Render images in an OpenCV window (has to be called on a regular basis)
        """
        if self._cv_image_actor is not None and self._cv_image_bird is not None:
            im_v = cv2.vconcat([self._cv_image_actor, self._cv_image_bird])
            cv2.circle(im_v, (900, 300), 80, (170, 170, 170), -1)
            text = str(int(round((self._actor.get_velocity().x * 3.6))))+" kph"

            speed = np.sqrt(self._actor.get_velocity().x**2 + self._actor.get_velocity().y**2)

            text = str(int(round((speed * 3.6))))+" kph"
            text = ' '*(7-len(text)) + text
            im_v = cv2.putText(im_v, text, (830, 310), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 0), 2, cv2.LINE_AA)
            cv2.imshow("", im_v)
            cv2.waitKey(1)

            if self._video_writer:
                self._video.write(im_v)
