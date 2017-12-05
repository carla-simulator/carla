
from .metrics import plot_summary
try:
    from carla import carla_server_pb2 as carla_protocol
except ImportError:
    raise RuntimeError('cannot import "carla_server_pb2.py", run the protobuf compiler to generate this file')


import json, csv, time


sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)

class Benchmark(object,):

	# Param @name to be used for saving purposes
	def __init__(self,city_name,name):
		self._city_name = city_name # The name of the city that is going to be used.
		self._base_name = name # Sends a base name, the rest will be saved with respect to what the episode was about
		self._dict_stats = {'exp_id':-1,
		'rep':-1,
		'weather':-1,
		'start_point':-1,
		'end_point':-1,
		'result':-1,
		'initial_distance':-1,
		'final_distance':-1,
		'final_time':-1,
		'time_out':-1


		}


		self._dict_rewards = {
		'exp_id':-1,
		'rep':-1,
		'weather':-1,
		'collision_gen':-1,
		'collision_ped':-1,
		'collision_car':-1,
		'lane_intersect':-1,
		'sidewalk_intersect':-1,
		'pos_x':-1,
		'pos_y':-1
		}


	def run_navigation_episode(self, agent,carla, time_out, target):


		curr_x = -1
		curr_y = -1
		prev_x = -1
		prev_y = -1
		measurements,sensor_data= carla.read_data()
		carla.send_control(carla_protocol.Control())
		t0 = measurements.game_timestamp
		t1=t0
		success = False
		step = 0
		accum_lane_intersect = 0.0
		accum_sidewalk_intersect = 0.0
		distance = 100000
		measurement_vec=[]
		while((t1-t0) < (time_out*1000) and not success):
			capture_time = time.time()
			measurements,sensor_data = carla.read_data()

			control = agent.run_step(measurements,sensor_data,target)
			print ('STEER ',control.steer,'GAS ',control.throttle,'Brake ',control.brake)
			carla.send_control(control)


			# meassure distance to target

			prev_x = curr_x
			prev_y = curr_y
			curr_x = measurements.player_measurements.transform.location.x
			curr_y = measurements.player_measurements.transform.location.y


			measurement_vec.append(measurements.player_measurements)

			t1 = measurements.game_timestamp


			step += 1
			# The distance is based on graph but quite not exact.
			distance = sldist([curr_x,curr_y],[target.location.x, target.location.y])


			
			print('[d=%f] c_x = %f, c_y = %f ---> t_x = %f, t_y = %f' % (float(distance), curr_x, curr_y, target.location.x, target.location.y))

			if(distance < 200.0):
				success = True



		if(success):
			return (1, measurement_vec, float(t1-t0)/1000.0,distance)
		else:
			return (0, measurement_vec, time_out,distance)


	def benchmark_agent(self,agent,carla,starting_position=0):

		

		experiments = self._build_experiments() # Returns a experiment class that is build from a benchmark inherited class
		self._suffix_name  = self._get_experiments_names(experiments[starting_position:]) # The fixed name considering all the experiments being run
		with open(self._base_name + self._suffix_name , 'wb') as ofd:

			w = csv.DictWriter(ofd, self._dict_stats.keys())
			w.writeheader()


		with open('rewards_' + self._base_name + self._suffix_name , 'wb') as rfd:

			rw = csv.DictWriter(rfd, self._dict_rewards.keys())
			rw.writeheader()



		self.write_experiment() # write the experiment being run


		for experiment in experiments[starting_position:]:



			positions = carla.load_settings(experiment.conditions).player_start_spots

			for pose in experiment.poses:
				for rep in range(experiment.repetitions):



					start_point = pose[0]
					end_point = pose[1]

					carla.start_episode(start_point)

					print('======== !!!! ==========')
					print(' Start Position ',start_point,' End Position ',end_point)

					path_distance = agent.get_distance(positions[start_point],positions[end_point])

					time_out = self._calculate_time_out(path_distance)
					# running the agent
					(result, reward_vec, final_time, remaining_distance) = self.run_navigation_episode(agent,carla,time_out,positions[end_point])

					

					# compute stats for the experiment

					self.write_summary_results(experiment,pose,rep,path_distance,remaining_distance,final_time,time_out,result)



					self.write_reward_results(experiment,rep,reward_vec)



					if(result > 0):
						print('+++++ Target achieved in %f seconds! +++++' % final_time)
					else:
						print('----- Tmeout! -----')

		return list_stats




	def write_experiment(self):
		
		with open(self._get_details() , 'wb') as ofd:
			pass

		
	def write_summary_results(self,experiment,pose,rep,path_distance,remaining_distance,final_time,time_out,result):

		self._dict_stats['exp_id'] = experiment.id
		self._dict_stats['rep'] = rep
		self._dict_stats['weather'] = experiment.Conditions.WeatherId
		self._dict_stats['start_point'] = pose[0]
		self._dict_stats['end_point'] = pose[1]
		self._dict_stats['result'] = result
		self._dict_stats['initial_distance'] = path_distance
		self._dict_stats['final_distance'] = remaining_distance
		self._dict_stats['final_time'] = final_time
		self._dict_stats['time_out'] = time_out

		 		

		with open(self._base_name + self._suffix_name , 'a+') as ofd:

			w = csv.DictWriter(ofd, self._dict_stats.keys())


			w.writerow(self._dict_stats)



	def write_reward_results(self,experiment,rep,reward_vec):

		with open('rewards_' + self._base_name + self._suffix_name , 'a+') as rfd:

			rw = csv.DictWriter(rfd, self._dict_rewards.keys())
			rw.writeheader()


			for i in range(len(reward_vec)):
				self._dict_rewards['exp_id'] = experiment.id
				self._dict_rewards['rep'] = rep
				self._dict_rewards['weather'] = experiment.Conditions.WeatherId
				self._dict_rewards['collision_gen'] = reward_vec[i].collision_other
				self._dict_rewards['collision_ped'] = reward_vec[i].collision_pedestrians
				self._dict_rewards['collision_car'] = reward_vec[i].collision_vehicles
				self._dict_rewards['lane_intersect'] = reward_vec[i].intersection_otherlane
				self._dict_rewards['sidewalk_intersect'] = reward_vec[i].intersection_offroad
				self._dict_rewards['pos_x'] = reward_vec[i].transform.location.x
				self._dict_rewards['pos_y'] = reward_vec[i].transform.location.y

				rw.writerow(self._dict_rewards)





	def plot_summary_test(self):


		summary_weathers = {'train_weather': [1,3,6,8]}

		summary = plot_summary(self._base_name + self._suffix_name,summary_weathers)


	def plot_summary_train(self):


		summary_weathers = {'test_weather': [4,14]}

		summary = plot_summary(self._base_name + self._suffix_name,summary_weathers)



	# To be redefined on subclasses on how to calculate timeout for an episode
	def _calculate_time_out(self,distance):
		return 0

	# To be redefined on subclasses
	def _build_experiments(self):
		pass


	def _get_experiments_names(self,experiments):

		name_cat ='_t'
		for  experiment in experiments:

			name_cat += str(experiment.id) + '.'

		name_cat ='_w'

		for  experiment in experiments:

			name_cat += str(experiment.Conditions.WeatherId) +'.'


		return name_cat




"""
		w = csv.DictWriter(ofd, dict_stats.keys())
		w.writeheader()
		rw = csv.DictWriter(rfd, dict_rewards.keys())
		rw.writeheader()



		carla = opt_dict['CARLA']
		width  = opt_dict['WIDTH']
		height = opt_dict['HEIGHT']
		host = opt_dict['HOST']
		port = opt_dict['PORT']

		output_summary = opt_dict['OUTPUT_SUMMARY']
		runnable = opt_dict['RUNNABLE']
		experiments_to_run = opt_dict['EXPERIMENTS_TO_RUN']
		pedestrians = opt_dict['PEDESTRIANS']
		vehicles = opt_dict['VEHICLES']
		repetitions_per_experiment = opt_dict['REPETITIONS']
		weathers = opt_dict['WEATHERS']
		start_goal_poses = opt_dict['START_GOAL_POSES']
		cameras = opt_dict['CAMERAS']

		list_stats = []

"""
