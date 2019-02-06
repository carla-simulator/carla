import copy
import logging
import numpy as np

import carla

class CallBack(object):
    def __init__(self, tag, sensor, data_provider):
        self._tag = tag
        self._data_provider = data_provider

        self._data_provider.register_sensor(tag, sensor)

    def __call__(self, data):
        if isinstance(data, carla.Image):
            self._parse_image_cb(data, self._tag)
        elif isinstance(data, carla.LidarMeasurement):
            self._parse_lidar_cb(data, self._tag)
        elif isinstance(data, carla.GnssEvent):
            self._parse_gnss_cb(data, self._tag)
        else:
            logging.error('No callback method for this sensor.')

    def _parse_image_cb(self, image, tag):
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = copy.deepcopy(array)
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        self._data_provider.update_sensor(tag, array, image.frame_number)

    def _parse_lidar_cb(self, lidar_data, tag):
        points = np.frombuffer(lidar_data.raw_data, dtype=np.dtype('f4'))
        points = copy.deepcopy(points)
        points = np.reshape(points, (int(points.shape[0] / 3), 3))
        self._data_provider.update_sensor(tag, points, lidar_data.frame_number)


    def _parse_gnss_cb(self, gnss_data, tag):
        array = np.array([gnss_data.latitude,
                          gnss_data.longitude,
                          gnss_data.altitude], dtype=np.float32)
        self._data_provider.update_sensor(tag, array, gnss_data.frame_number)


class DataProvider(object):
    def __init__(self):
        self._sensors_objects = {}
        self._data_buffers = {}
        self._timestamps = {}

    def register_sensor(self, tag, sensor):
        if tag  in self._sensors_objects:
            raise ValueError("Duplicated sensor tag [{}]".format(tag))

        self._sensors_objects[tag] = sensor
        self._data_buffers[tag] = None
        self._timestamps[tag] = -1

    def update_sensor(self, tag, data, timestamp):
        if tag  not in self._sensors_objects:
            raise ValueError("The sensor with tag [{}] has not been "
                             "created!".format(tag))
        self._data_buffers[tag] = data
        self._timestamps[tag] = timestamp

    def all_sensors_ready(self):
        for key in self._sensors_objects.keys():
            if self._data_buffers[key] is None:
                return False
        return True

    def get_data(self):
        data_dict = {}

        for key in self._sensors_objects.keys():
            data_dict[key] = (self._timestamps[key],
                              copy.deepcopy(self._data_buffers[key]))
        return data_dict