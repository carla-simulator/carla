#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
    Keyboard controlling for carla. Please refer to carla_use_example for a simpler and more
    documented example.

"""
from __future__ import print_function
from carla import CARLA


#from scene_parameters import SceneParams

from PIL import Image
import numpy as np

import random
import time
import sys
import argparse
import logging
from socket import error as socket_error

import matplotlib.pyplot as plt

from carla import  Control,Measurements

import pygame
from pygame.locals import *



def join_classes(labels_image):
  
    classes_join = {0:[0,0,0],1:[64,64,64],2:[96,96,96],3:[255,255,255],5:[128,128,128],12:2,9:[0,255,0],\
    11:[32,32,32],4:[255,0,0],10:[0,0,255],8:[255,0,255],6:[196,196,196],7:[128,0,128]}


    compressed_labels_image = np.zeros((labels_image.shape[0],labels_image.shape[1],3)) 
    for key,value in classes_join.iteritems():
        compressed_labels_image[np.where(labels_image==key)] = value


    return compressed_labels_image


# Function for making colormaps

def grayscale_colormap(img,colormap):

  cmap = plt.get_cmap(colormap)

  rgba_img = cmap(img)

  rgb_img = np.delete(rgba_img, 3, 2)
  return rgb_img


# Function to convert depth to human readable format 
def convert_depth(depth):

    depth = depth.astype(np.float32)
    gray_depth = ((depth[:,:,0] +  depth[:,:,1]*256.0 +  depth[:,:,2]*256.0*256.0)/(256.0*256.0*256.0 ))
    color_depth =grayscale_colormap(gray_depth,'jet')*255

    return color_depth


class App:
    def __init__(self, port=2000, host='127.0.0.1', config='./CarlaSettings.ini',\
     resolution=(2400,600),verbose=True):
        self._running = True
        self._display_surf = None
        self.port = port
        self.host = host
        self.config = config
        self.verbose = verbose
        self.resolution = resolution
        self.size = self.weight, self.height = resolution


 
    def on_init(self):
        pygame.init()
        print (" \n \n \n Welcome to CARLA manual control \n USE ARROWS for control \n Press R for reset \n"\
        +"STARTING in a few seconds...")
        time.sleep(3)
        self._display_surf = pygame.display.set_mode(self.size, pygame.HWSURFACE | pygame.DOUBLEBUF)
        logging.debug('Started the PyGame Library')
        self._running = True
        self.step = 0
        self.prev_step = 0
        self.prev_time = time.time()

        self.carla =CARLA(self.host, self.port)

        positions = self.carla.loadConfigurationFile(self.config)
        self.num_pos = len(positions)
        print ("Staring Episode on Position ",self.num_pos)
        self.carla.newEpisode(np.random.randint(self.num_pos))
        self.prev_restart_time = time.time()
 
    def on_event(self, event):
        if event.type == pygame.QUIT:
            self._running = False


    def on_loop(self):
        self.step += 1
        keys=pygame.key.get_pressed()
        gas = 0
        steer = 0
        restart = False
        pressed_keys = []
        if keys[K_LEFT]:
            steer = -1.
            pressed_keys.append('left')
        if keys[K_RIGHT]:
            pressed_keys.append('right')
            steer = 1.
        if keys[K_UP]:
            pressed_keys.append('up')
            gas = 1.
        if keys[K_DOWN]:
            pressed_keys.append('down')
            gas = -1.
        if keys[K_r]:
            pressed_keys.append('r')
            if time.time() - self.prev_restart_time > 2.:
                self.prev_restart_time = time.time()
                restart = True
        if time.time() - self.prev_restart_time < 2.:
            gas = 0.
            steer = 0.

        control = Control()
        control.throttle = gas
        control.steer = steer
        self.carla.sendCommand(control)
        measurements = self.carla.getMeasurements()
        pack = measurements['PlayerMeasurements']
        self.img_vec = measurements['BGRA']
        self.depth_vec = measurements['Depth']
        self.labels_vec = measurements['Labels']


        if time.time() - self.prev_time > 1.:
            print('Step', self.step, 'FPS', float(self.step - self.prev_step) / (time.time() - self.prev_time))

            
            print('speed', pack.forward_speed, 'collision', pack.collision_other, \
            'collision_car', pack.collision_vehicles, 'colision_ped', pack.collision_pedestrians, 'pressed:', pressed_keys)            
            self.prev_step = self.step
            self.prev_time = time.time()
            
        if restart:
            print('\n *** RESTART *** \n')

            player_pos = np.random.randint(self.num_pos)

            print('  Player pos %d \n' % (player_pos))
            self.carla.newEpisode(player_pos)
        
        
    """
        The render method plots the First RGB, the First Depth and First Semantic Segmentation Camera
    """
    def on_render(self):

        pos_x =0


        
        if len(self.depth_vec) > 0:
            self.depth_vec[0] = self.depth_vec[0][:,:,:3]
            self.depth_vec[0] = self.depth_vec[0][:,:,::-1]
            self.depth_vec[0] = convert_depth(self.depth_vec[0])
            surface = pygame.surfarray.make_surface(np.transpose(self.depth_vec[0], (1,0,2)))
            self._display_surf.blit(surface,(pos_x,0))
            pos_x += self.depth_vec[0].shape[1]

        if len(self.img_vec) > 0:
            self.img_vec[0] = self.img_vec[0][:,:,:3]
            self.img_vec[0] = self.img_vec[0][:,:,::-1]
            surface = pygame.surfarray.make_surface(np.transpose(self.img_vec[0], (1,0,2)))
            self._display_surf.blit(surface,(pos_x,0))
            pos_x += self.img_vec[0].shape[1]

        if len(self.labels_vec) > 0:
            self.labels_vec[0] = join_classes(self.labels_vec[0][:,:,2])
            surface = pygame.surfarray.make_surface(np.transpose(self.labels_vec[0], (1,0,2)))
            self._display_surf.blit(surface,(pos_x,0))
            pos_x += self.labels_vec[0].shape[1]


            
        pygame.display.flip()

        
    def on_cleanup(self):
        self.carla.closeConections()
        pygame.quit()
 
    def on_execute(self):
        if self.on_init() == False:
            self._running = False
 
        while( self._running ):
            try:

                for event in pygame.event.get():
                    self.on_event(event)
                self.on_loop()
                self.on_render()

            except Exception as e:
                logging.exception(e)
                self._running = False
                break


        self.on_cleanup()
            
                

        
 
if __name__ == "__main__" :
    parser = argparse.ArgumentParser(description='Run the carla client manual that connects to CARLA server')
    parser.add_argument('host', metavar='HOST', type=str, help='host to connect to')
    parser.add_argument('port', metavar='PORT', type=int, help='port to connect to')

    parser.add_argument("-c", "--config", help="the path for the server .ini config file that the client sends",type=str,default="./CarlaSettings.ini") 

    parser.add_argument("-l", "--log", help="activate the log file",action="store_true") 
    parser.add_argument("-lv", "--log_verbose", help="put the log file to screen",action="store_true") 
    args = parser.parse_args()

    print(args)

    if args.log or args.log_verbose:
        LOG_FILENAME = 'log_manual_control.log'
        logging.basicConfig(filename=LOG_FILENAME,level=logging.DEBUG)
        if args.log_verbose:  # set of functions to put the logging to screen


            root = logging.getLogger()
            root.setLevel(logging.DEBUG)
            ch = logging.StreamHandler(sys.stdout)
            ch.setLevel(logging.DEBUG)
            formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
            ch.setFormatter(formatter)
            root.addHandler(ch)
        


    theApp = App(port=args.port, host=args.host, config=args.config)
    theApp.on_execute()
    
    
    
    
