#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from __future__ import print_function

from .datastream import DataStream
import socket
import time
import os, signal
import sys
import logging
import struct

import re
from .protoc import *

from . import socket_util



def get_image_resolution(data):
	return int(re.search('[\n\r].*ImageSizeX\s*=([^\n\r]*)', data).group(1)),int(re.search('[\n\r].*ImageSizeY\s*=([^\n\r]*)', data).group(1))


class CARLA(object):

	"""
	Normal instanciation of the class, creating also the thread class responsible for receiving data
	"""
	def __init__(self,host,port):
		self._host = host
		self._port = port
		logging.debug('selected host %s' % host)
		logging.debug('selected port %s' % port)


		self._port_control = self._port +2
		self._port_stream = self._port +1

		# Default start. Keep it as class param for eventual restart
		self._image_x =0
		self._image_y = 0


		self._socket_world = socket_util.pers_connect(self._host ,self._port)
		print ('Successfully Connected to Carla Server')
		logging.debug("Connected to Unreal Server World Socket")
		self._socket_stream = 0
		self._socket_control = 0
		self._latest_start = 0
		self._agent_is_running = False

		self._episode_requested = False

		self._data_stream = None
		logging.debug("Started Unreal Client")



	"""
	Starting the Player Agent. The image stream port
	and the control port 

	Args:
		None
	Returns:
	    None

	"""


	def startAgent(self):

		self._data_stream = DataStream(self._image_x,self._image_y)

		logging.debug("Going to Connect Stream and start thread")
		# Perform persistent connections, try up to 10 times
		try:
			self._socket_stream = socket_util.pers_connect(self._host ,self._port_stream)
		except Exception:
			logging.exception("Attempts to connect Stream all failed, restart...")	
			self.restart()

		self._data_stream.start(self._socket_stream)
		logging.debug("Streaming Thread  Started")

		try:
			self._socket_control = socket_util.pers_connect(self._host ,self._port_control)
		except Exception:
			logging.exception("Attempts to connect Agent all failed, restart ...")
			self.restart()

		logging.debug("Control Socket Connected")
		self._agent_is_running = True

	def stopAgent(self):


		logging.debug("Going to  Stop thread and Disconect Stream")
		self._data_stream.stop()
		
		logging.debug("Streaming Thread  Stoped")

		socket_util.disconnect(self._socket_control)
		logging.debug("Control Socket DisConnected")
		self._agent_is_running = False




	# This function requests a new episode and send the string containing this episode configuration file


	def receiveSceneConfiguration(self):

		try:
			logging.debug("Reading for the scene configuration")
			data = socket_util.get_message(self._socket_world)
			
			scene = SceneDescription()
			scene.ParseFromString(data)
			logging.debug("Received Scene Configuration")


			return scene.player_start_spots


		except Exception  as e:
			logging.exception("Server not responing when receiving configuration")
			return self.restart()

			
	def requestNewEpisode(self,ini_path=None):



		if ini_path == None:
			ini_file = self._config_path
		else:
			ini_file = ini_path
			self._config_path = ini_path # We just save the last config file in case the client dies

		requestEpisode = RequestNewEpisode()
		with open (ini_file, "r") as myfile:
			data=myfile.read()
		try:
			self._image_x,self._image_y =  get_image_resolution(data)
		except Exception as e:
			logging.exception("No image resolution found on config file")


		logging.debug("Resolution %d , %d",self._image_x,self._image_y)

		logging.debug("Set the Init File")
		logging.debug("sent %s" % (data))
		requestEpisode.ini_file = data.encode('utf-8')
		try:
			socket_util.send_message(self._socket_world,requestEpisode)
		except Exception as e:
			logging.exception("Server not responding when requesting new episode")
			self.restart()
		else:
			logging.debug("Successfully sent the new episode Request")



		if self._agent_is_running:	
			self.stopAgent()
		self._episode_requested = True

		return self.receiveSceneConfiguration()

	def loadConfigurationFile(self,ini_path):

		return self.requestNewEpisode(ini_path)




	def newEpisode(self,start_index):

		# Save the latest new episode positon, just in case of crashes
		self._latest_start = start_index

		if not self._episode_requested:
			positions = self.requestNewEpisode(self._config_path)

		scene_init = EpisodeStart()
		scene_init.player_start_spot_index = start_index
		try:
			socket_util.send_message(self._socket_world,scene_init)
		except Exception:
			logging.exception("Server not responding when trying to send episode start confirmation")
			self.restart()
		else:
			logging.debug("Successfully sent the new episode Message")

		episode_ready = EpisodeReady()
		episode_ready.ready = False

		try:
			
			data = socket_util.get_message(self._socket_world)
			logging.debug("Got the episode ready message")
			episode_ready.ParseFromString(data)
		except Exception:
			logging.exception("Server not responding when trying to receive episode reading")
			self.restart()
		else:
			logging.debug("Episode is Ready")

		self.startAgent()
		self._episode_requested = False



	"""  Measurements 
		 returns
		 @game time
		 @wall time
		 @player measurements
		 @non_player_agents : vector with all agents present on the game
		 @image_data

	"""


	def getMeasurements(self):
		
		while True:
			try:
				meas_dict = self._data_stream.get_the_latest_data()


				logging.debug("Got A new Measurement")
				return meas_dict
			except AttributeError:
				logging.exception("Unitialized DataStream. Tip: Connect and start an episode before requesting measurements") 
				return None
			except Exception:
				logging.exception("Got an empty Measurement") 
				self.restart()



		

	""" Command contains:
		Steering: -1 to 1
		Acc : -1 to 1
	"""

	def sendCommand(self,control):

		logging.debug("Send Control Comand : throttle -> %f , steer %f, brake %f, hand_brake %d, gear %d" % (control.throttle,control.steer,control.brake,control.hand_brake,control.reverse))
		try:
			socket_util.send_message(self._socket_control,control)
		except Exception:

			logging.exception("Problems on sending the commands... restarting")
			self.restart() # the mensage is not resend because it likely lost its relevance.
			


	def restart(self):
		logging.debug("Trying to close clients") 
		self.closeConections()
		connected = False 
		if self._data_stream != None:
			self._data_stream._running = False
		self._agent_is_running = False
		while not connected:
			try:
				logging.debug("Trying to connect to the world thread")
				self._socket_world = socket_util.connect(self._host ,self._port)		
				connected = True
			except Exception:
				logging.exception("Couldn't connected ... retry in 10 seconds...")
				time.sleep(10)

		self._data_stream = DataStream(self._image_x,self._image_y)

		
		positions = self.requestNewEpisode()
		self.newEpisode(self._latest_start)
		logging.debug("restarted the world connection") 
		return positions

	def stop(self):
		self.closeConections()
		connected = False 

		self._data_stream._running = False
		self._data_stream = DataStream(self._image_x,self._image_y)

	def closeConections(self):

		try:
			self._socket_world.shutdown(socket.SHUT_RDWR)
			self._socket_world.close()

			logging.debug("Close world")
		except Exception as ex:
			logging.exception("Exception on closing Connections")

		try:
			self._socket_stream.shutdown(socket.SHUT_RDWR)
			self._socket_stream.close()
			logging.debug("Close Stream")
		except Exception as ex:
			logging.exception("Exception on closing Connections")

		try:
			self._socket_control.shutdown(socket.SHUT_RDWR)
			self._socket_control.close()
			logging.debug("Close Control")
		except Exception as ex:
			logging.exception("Exception on closing Connections")