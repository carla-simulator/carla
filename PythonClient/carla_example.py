#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Basic CARLA client example."""



from __future__ import print_function
# General Imports
import numpy as np
from PIL import Image
import random
import time
import sys
import argparse
import logging
import os

# Carla imports, Everything can be imported directly from "carla" package

from carla import CARLA
from carla import Control
from carla import Measurements

# Constant that set how offten the episodes are reseted

RESET_FREQUENCY = 100

"""
Print function, prints all the measurements saving
the images into a folder. WARNING just prints the first BGRA image
Args:
    param1: The measurements dictionary to be printed
    param2: The iterations

Returns:
    None
Raises:
    None
"""

def print_pack(measurements,i,write_images):

	if write_images:
		image_result = Image.fromarray( measurements['BGRA'][0])

		b, g, r,a = image_result.split()
		image_result = Image.merge("RGBA", (r, g, b,a))

		if not os.path.exists('images'):
			os.mkdir('images')
		image_result.save('images/image' + str(i) + '.png')


	print ('Pack ',i)
	print ('	Wall Time: ',measurements['WallTime'])	
	print ('	Game Time: ',measurements['GameTime'])
	print ('	Player Measurements ')
	
	print ('		Position: (%f,%f,%f)' % (measurements['PlayerMeasurements'].\
		transform.location.x,measurements['PlayerMeasurements'].transform.location.y,\
		measurements['PlayerMeasurements'].transform.location.z ))
	print ('		Orientation: (%f,%f,%f)' % (measurements['PlayerMeasurements'].\
		transform.orientation.x,measurements['PlayerMeasurements'].transform.orientation.y,\
		measurements['PlayerMeasurements'].transform.orientation.z ))

	print ('		Acceleration: (%f,%f,%f)' % (measurements['PlayerMeasurements'].\
		acceleration.x,measurements['PlayerMeasurements'].acceleration.y,measurements['PlayerMeasurements'].acceleration.z ))
	print ('		Speed: ',measurements['PlayerMeasurements'].forward_speed)
	print ('		Collision Vehicles (Acum. Impact): ',measurements['PlayerMeasurements'].collision_vehicles)
	print ('		Collision Pedestrians (Acum. Impact): ',measurements['PlayerMeasurements'].collision_pedestrians)	
	print ('		Collision Other (Acum. Impact): ',measurements['PlayerMeasurements'].collision_other)
	print ('		Intersection Opposite Lane (% Volume): ',measurements['PlayerMeasurements'].intersection_otherlane)
	print ('		Intersection Opposite Sidewalk (% Volume): ',measurements['PlayerMeasurements'].intersection_offroad)


	print ('	',len(measurements['Agents']),' Agents (Positions not printed)')
	print ('		',end='')
	for agent in measurements['Agents']:
		
		if agent.HasField('vehicle'):
			print ('Car',end='')
			
		elif agent.HasField('pedestrian'):
			print ('Pedestrian',end='')

		elif agent.HasField('traffic_light'):
			print ('Traffic Light',end='')


		elif agent.HasField('speed_limit_sign'):
			print ('Speed Limit Sign',end='')
		print(',',end='')
	print('')



def use_example(ini_file,port = 2000, host ='127.0.0.1',print_measurements =False,images_to_disk=False):

    # We assume the CARLA server is already waiting for a client to connect at
    # host:port. To create a connection we can use the CARLA
    # constructor, it creates a CARLA client object and starts the
    # connection. It will throw an exception if something goes wrong. 

	carla =CARLA(host,port)
	
	""" As a first step, Carla must have a configuration file loaded. This will load a map in the server
		with the properties specified by the ini file. It returns all the posible starting positions on the map
		in a vector.
	"""
	positions = carla.loadConfigurationFile(ini_file)

	""" 
		Ask Server for a new episode starting on position of index zero in the positions vector
	"""
	carla.newEpisode(0)
	
	capture = time.time()
	# General iteratior
	i = 1
	# Iterator that will go over the positions on the map after each episode
	iterator_start_positions = 1

	while True:
		try:
			"""
				User get the measurements dictionary from the server. 
				Measurements contains:
				* WallTime: Current time on Wall from server machine.
				* GameTime: Current time on Game. Restarts at every episode
				* PlayerMeasurements : All information and events that happens to player
				* Agents : All non-player agents present on the map information such as cars positions, traffic light states
				* BRGA : BGRA optical images
				* Depth : Depth Images
				* Labels : Images containing the semantic segmentation. NOTE: the semantic segmentation must be
					previously activated on the server. See documentation for that. 

			"""


			measurements = carla.getMeasurements()

			# Print all the measurements... Will write images to disk
			if print_measurements:
				print_pack(measurements,i,images_to_disk)

			"""
				Sends a control command to the server
				This control structue contains the following fields:
				* throttle : goes from 0 to -1
				* steer : goes from -1 to 1
				* brake : goes from 0 to 1
				* hand_brake : Activate or desactivate the Hand Brake.
				* reverse: Activate or desactive the reverse gear.

			"""

			control = Control()
			control.throttle = 0.9
			control.steer = (random.random() * 2) - 1

			carla.sendCommand(control)
	
					
			
			i+=1


			if i % RESET_FREQUENCY ==0:
					
				print ('Fps for this episode : ',(1.0/((time.time() -capture)/100.0)))

				
				""" 
					Starts another new episode, the episode will have the same configuration as the previous
					one. In order to change configuration, the loadConfigurationFile could be called at any
					time.
				"""
				if iterator_start_positions < len(positions):
					carla.newEpisode(iterator_start_positions)
					iterator_start_positions+=1
				else :
					carla.newEpisode(0)
					iterator_start_positions = 1

				print("Now Starting on Position: ",iterator_start_positions-1)
				capture = time.time()


		except Exception as e:

			logging.exception('Exception raised to the top')
			time.sleep(1)


	
	
if __name__ == "__main__" :
	parser = argparse.ArgumentParser(description='Run the carla client example that connects to a server')
	parser.add_argument('host', metavar='HOST', type=str, help='host to connect to')
	parser.add_argument('port', metavar='PORT', type=int, help='port to connect to')

	parser.add_argument("-c", "--config", help="the path for the server ini file that the client sends",type=str,default="CarlaSettings.ini") 


	parser.add_argument("-l", "--log", help="activate the log file",action="store_true") 
	parser.add_argument("-lv", "--log_verbose", help="activate log and put the log file to screen",action="store_true")

	parser.add_argument("-pm", "--print", help=" prints the game measurements",action="store_true")
	parser.add_argument(
		'-i', '--images-to-disk',
		action='store_true',
		help='save images to disk')


	args = parser.parse_args()
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
	else:
		sys.tracebacklimit=0 # Avoid huge exception messages out of debug mode



	use_example(args.config,port=args.port, host=args.host,print_measurements=args.print,images_to_disk= args.images_to_disk)
