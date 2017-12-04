#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Basic CARLA client example."""


import sys
is_py2 = sys.version[0] == '2'
if is_py2:
    import Queue as Queue
else:
    import queue as Queue

from threading import Thread
import time
import random
from PIL import Image
from .socket_util import *
import io
import sys
import numpy as np
import logging

from .protoc import *





def threaded(fn):
    def wrapper(*args, **kwargs):
        thread = Thread(target=fn, args=args, kwargs=kwargs)
        thread.setDaemon(True)
        thread.start()

        return thread
    return wrapper



class DataStream(object):

    def __init__(self,image_x=640,image_y=480):
        self._data_buffer = Queue.Queue(1)
        self._image_x = image_x
        self._image_y = image_y

        self._socket = 0
        self._running = True



    def _read_data_type(self, imagedata,pointer):
        pointer += 4*2
        data_type = struct.unpack('<L',imagedata[pointer:(pointer + 4)])[0]
        return data_type

    def _read_image(self,imagedata,pointer):


        width = struct.unpack('<L',imagedata[pointer:(pointer+4)])[0]

        pointer +=4
        height = struct.unpack('<L',imagedata[pointer:(pointer+4)])[0]
        pointer +=4
        im_type = struct.unpack('<L',imagedata[pointer:(pointer+4)])[0]
        pointer +=4


        image_size = width*height*4

        image_bytes = imagedata[pointer:(pointer+image_size)]


        dt = np.dtype("uint8")

        new_image =np.frombuffer(image_bytes,dtype=dt)

        new_image = np.reshape(new_image,(self._image_y,self._image_x,4)) # TODO: make this generic

        pointer += image_size



        return new_image,im_type,pointer


    def _read_lidar_measurement(self, imagedata, pointer):

        horizontal_angle = struct.unpack('<d',imagedata[pointer:(pointer+8)])[0]
        pointer +=8
        im_type = struct.unpack('<L',imagedata[pointer:(pointer+4)])[0]
        pointer +=4
        channels_count = struct.unpack('<L',imagedata[pointer:(pointer+4)])[0]
        pointer +=4

        points_count_by_channel_size = channels_count * 4
        points_count_by_channel_bytes = imagedata[pointer:(pointer + points_count_by_channel_size)]
        points_count_by_channel = np.frombuffer(points_count_by_channel_bytes, dtype=np.dtype('uint32'))
        pointer += points_count_by_channel_size

        points_in_one_channel = points_count_by_channel[0]
        points_size = channels_count * points_in_one_channel * 3 * 8
        points = np.frombuffer(imagedata[pointer:(pointer + points_size)], dtype='float')
        points = np.reshape(points, (channels_count, points_in_one_channel, 3))
        pointer += points_size

        lidar_measurement = {
            'horizontal_angle' : horizontal_angle,
            'channels_count' : channels_count,
            'points_count_by_channel' : points_count_by_channel,
            'points' : points
        }

        return lidar_measurement, im_type, pointer


    def receive_data(self):

        depths = []

        #  First we get the message of the google protocol
        capture_time = time.time()
        try:
            data  = get_message(self._socket)
        except Exception:
            if self._running:
                logging.exception("Error on Datastream, Raise Again")
                raise Exception
            return [] # return something empty, since it is not running anymore


        measurements = Measurements()
        measurements.ParseFromString(data)

        player_measures = measurements.player_measurements
        non_player_agents = measurements.non_player_agents

        try:
            logging.debug(" Trying to get the image")
            imagedata  = get_message(self._socket)
        except Exception:
            if self._running:
                logging.exception("Error on Datastream, Raise Again")
                raise Exception
            return [] # return something empty, since it is not running anymore


        meas_dict ={}
        meas_dict.update({'RAW_BGRA':[]})

        meas_dict.update({'BGRA':[]})

        meas_dict.update({'Depth':[]})

        meas_dict.update({'Labels':[]})

        meas_dict.update({'Lidars':[]})


        pointer = 0
        while pointer < len(imagedata):
            im_type = self._read_data_type(imagedata, pointer)
            if im_type == 0:

                image,im_type,pointer = self._read_image(imagedata,pointer)
                meas_dict['RAW_BGRA'].append(image)
                logging.debug("RECEIVED rgb_raw")
            if im_type == 1:

                image,im_type,pointer = self._read_image(imagedata,pointer)
                meas_dict['BGRA'].append(image)
                logging.debug("RECEIVED rgb")
            if im_type == 2:

                image,im_type,pointer = self._read_image(imagedata,pointer)
                meas_dict['Depth'].append(image)
                logging.debug("RECEIVED depht")
            if im_type == 3:

                image,im_type,pointer = self._read_image(imagedata,pointer)
                meas_dict['Labels'].append(image)
                logging.debug("RECEIVED scene_seg")
            if im_type == 10:

                lidar_measurement,im_type,pointer = self._read_lidar_measurement(imagedata,pointer)
                meas_dict['Lidars'].append(lidar_measurement)
                logging.debug("RECEIVED lidar_measurement")

        meas_dict.update({'WallTime':measurements.platform_timestamp})

        meas_dict.update({'GameTime':measurements.game_timestamp})

        meas_dict.update({'PlayerMeasurements':player_measures})

        meas_dict.update({'Agents':non_player_agents})

        return meas_dict


    def get_the_latest_data(self):

        try:
            data = self._data_buffer.get(timeout=20)

        except Queue.Empty:
            logging.exception("ERROR: No Data in 20 seconds, disconecting and reconecting from server ")
            self._running = False
            raise Queue.Empty

        except Exception:
            logging.exception("Other error on getting queue")
            raise Exception

        else:
            self._data_buffer.task_done()

            return data



    def start(self,socket):

        self._socket = socket

        self.run()


    def stop(self):

        self._running = False

        disconnect(self._socket)
        self.clean()

    # We clean the buffer so that no old data is going to be used
    def clean(self):
        while True:
            try:
                aux=self._data_buffer.get(False)
            except Queue.Empty:
                return


    @threaded
    def run(self):
        try:
            while self._running:
                try:
                    self._data_buffer.put(self.receive_data(),timeout=20)
                except Queue.Full:
                    logging.exception("ERROR: Queue Full for more than 20 seconds...")
                except Exception as e:
                    logging.exception("Some internal Socket error ")
                    self._running = False

        except RuntimeError:
            logging.exception("Unexpected RuntimeError")
            self._running = False
        finally:

            logging.debug("We Are finishing the datastream thread ")
