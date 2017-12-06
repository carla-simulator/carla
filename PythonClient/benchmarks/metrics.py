
import numpy as np
import math
import matplotlib.pyplot as plt


import argparse
sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)
flatten = lambda l: [item for sublist in l for item in sublist]



def task_complete_percentages(data_matrix):

	complete_per = []
	pos_ant =0
	for pos in reset_positions:
		complete_per.append(sum(data_matrix[pos_ant:pos,1])/len(reset_positions))
		pos_ant =pos

	return complete_per

def task_average_time_percentages(data_matrix,reset_positions):

	complete_per = []
	pos_ant =0
	for pos in reset_positions:
		complete_per.append(sum(data_matrix[pos_ant:pos,0])/25.0)
		pos_ant =pos

	return complete_per


def get_colisions(selected_matrix,header):

	count_gen =0
	count_ped =0
	count_car = 0
	i=1

	while i < selected_matrix.shape[0]:
		if (selected_matrix[i,header.index('collision_gen')] - selected_matrix[(i-10),header.index('collision_gen')]) > 40000:
			count_gen+=1
			i+=20
		i+=1


	i=1
	while i < selected_matrix.shape[0]:
		if (selected_matrix[i,header.index('collision_car')] - selected_matrix[(i-10),header.index('collision_car')]) > 40000:
			count_car+=1
			i+=30
		i+=1


	i=1
	while i < selected_matrix.shape[0]:
		if (selected_matrix[i,header.index('collision_ped')] - selected_matrix[i-5,header.index('collision_ped')]) > 30000:
			count_ped+=1
			i+=100
		i+=1


	return count_gen,count_car,count_ped

def get_distance_traveled(selected_matrix,header):

	prev_x = selected_matrix[0,header.index('pos_x')]
	prev_y = selected_matrix[0,header.index('pos_y')]

	i =1
	acummulated_distance =0
	while i < selected_matrix.shape[0]:

		x = selected_matrix[i,header.index('pos_x')]
		y = selected_matrix[i,header.index('pos_y')]


		acummulated_distance += sldist((x,y),(prev_x,prev_y))
		#print sldist((x,y),(prev_x,prev_y))

		prev_x =x
		prev_y =y

		i+=1
	return (float(acummulated_distance)/float(100*1000))

def get_end_positions(data_matrix):


	i=0
	end_positions_vec = []
	accumulated_time = 0
	while i < data_matrix.shape[0]:

		end_positions_vec.append(accumulated_time)
		accumulated_time += data_matrix[i,2]*10
		i+=1

	return end_positions_vec


def is_car_static(pos,reward_matrix):


	x = reward_matrix[pos,0]
	y = reward_matrix[pos,1]

	prev_x = reward_matrix[pos,0]
	prev_y = reward_matrix[pos,1]

	if sldist((x,y),(prev_x,prev_y)) > 100:
		return False
	else:
		return True






def get_end_positions_state(end_positions,data_matrix, reward_matrix):


	vector_of_infractions = [0,0,0,0] # Inf1+inf3 , inf2+inf3 or inf3,  , inf1+inf4, timeout


	for i in range(len(end_positions)):
		pos = int(end_positions[i] -20)

		if data_matrix[i,1] == 0: # if it failed, lets find the reason

			if reward_matrix[pos,4] > 30000 and is_car_static(pos,reward_matrix): # If it crashed_general

				if reward_matrix[pos,5] > 0.4: # Check if it is out of road
					# Case 0
					vector_of_infractions[0] +=1
				else: # Check it is out of lane or whaever
					vector_of_infractions[1] +=1






			elif reward_matrix[pos,2] > 30000 and is_car_static(pos,reward_matrix):


				if reward_matrix[pos,6] > 0.1: # Check if it is out of lane
					vector_of_infractions[2]+=1

				else:  # Likely that the it bumped the car but it didn't bother
					vector_of_infractions[3]+=1

			else:  # TimeOUt
				vector_of_infractions[3]+=1


	return vector_of_infractions





def get_out_of_road_lane(selected_matrix,header):

	count_road =0
	count_lane =0


	i=0

	while i < selected_matrix.shape[0]:
		#print selected_matrix[i,6]
		if (selected_matrix[i,header.index('sidewalk_intersect')] - selected_matrix[(i-10),header.index('sidewalk_intersect')]) > 0.3:
			count_road+=1
			i+=20
		if i >= selected_matrix.shape[0]:
			break

		if (selected_matrix[i,header.index('lane_intersect')] - selected_matrix[(i-10),header.index('lane_intersect')]) > 0.4:
			count_lane+=1
			i+=20

		i+=1




	return count_lane,count_road

def print_infractions(infractions):
	print '		Killometers Without Pedestrians Colision - > ',1.0/(infractions[4]+0.0001)
	print '		Average Colision',3.0/(infractions[4]+infractions[3] + infractions[2]+0.0001)
	print '		Killometers Without Car Colision - > ',1.0/(infractions[3]+0.0001)
	print '		Killometers Without Other Colision - > ',1.0/(infractions[2]+0.0001)
	print '		Killometers Without Crossing Lane - > ',1.0/(infractions[0]+0.0001)
	print '		Killometers Without Going to Sidewalk - > ',1.0/(infractions[1]+0.0001)
	print '		Average Infraction ',2.0/(infractions[0]+infractions[1]+0.0001)


def plot_summary(file,summary_weathers):

	intervention_acc =[0,0,0,0,0]
	completions_acc = [0,0,0,0]
	infractions_vec = [0,0,0,0,0]

	compute_infractions = True
	# Separate the PATH and the basename
	path = os.path.dirname(file)
	base_name = os.path.basename(file)


	f = open(file, "rb")
	header = f.readline()
	header= header.split(',')
	header[-1] = header[-1][:-2]
	f.close()
	print header

	f = open(os.path.join(path,'rewards_' , base_name), "rb")
	header_rewards = f.readline()
	header_rewards= header_rewards.split(',')
	header_rewards[-1] = header_rewards[-1][:-2]
	f.close()
	print header_rewards
	data_matrix = np.loadtxt(open(file, "rb"), delimiter=",", skiprows=1)

	tasks = np.unique(data_matrix[:,header.index('exp_id')])

	reward_matrix = np.loadtxt(open(os.path.join(path,'rewards_' , base_name), "rb"), delimiter=",", skiprows=1)

	for t in tasks:
		task_data_matrix = data_matrix[data_matrix[:,header.index('exp_id')]== t]
		weathers = np.unique(task_data_matrix[:,header.index('weather')])
		summaries = {}
		for sw in summary_weathers:
			summaries[sw] = {'completion': 0., 'infractions': np.zeros(5, dtype=np.float), 'num_weathers': 0}
		for w in weathers:

			task_data_matrix  =data_matrix[np.logical_and(data_matrix[:,header.index('exp_id')]== t, data_matrix[:,header.index('weather')]== w)]
			if compute_infractions:
				task_reward_matrix =reward_matrix[np.logical_and(reward_matrix[:,header_rewards.index('exp_id')]== float(t), reward_matrix[:,header_rewards.index('weather')]== float(w))]

			completed_episodes = sum(task_data_matrix[:,header.index('result')])/task_data_matrix.shape[0]
			print 'Task ',t , 'Weather', w

			print '		Entire Episodes Completed (%) - > ', completed_episodes
			print ''

			#completions_acc = [sum(x) for x in zip(completions, completions_acc)]

			for sw in summary_weathers:
				if w in summary_weathers[sw]:
					summaries[sw]['completion'] += completed_episodes
					summaries[sw]['num_weathers'] += 1

			if compute_infractions:
				print '		==== Infraction Related ====='
				km_run = get_distance_traveled(task_reward_matrix,header_rewards)
				print '		Drove (KM) - > ', km_run
				lane_road = get_out_of_road_lane(task_reward_matrix,header_rewards)
				colisions = get_colisions(task_reward_matrix,header_rewards)
				infractions = [lane_road[0]/km_run,lane_road[1]/km_run,colisions[0]/km_run,colisions[1]/km_run,colisions[2]/km_run]
				print_infractions(infractions)

				for sw in summary_weathers:
					if w in summary_weathers[sw]:
						# print summaries[sw]
						# print infractions
						summaries[sw]['infractions'] += np.array(infractions)

		print '\n\n >>> Task', t, 'summary <<<\n\n'
		for sw in summary_weathers:
			print sw, summary_weathers[sw]
			print 'Num weathers', summaries[sw]['num_weathers']
			print 'Avg completion', summaries[sw]['completion']/summaries[sw]['num_weathers']
			print 'Avg infractions'
			print_infractions(summaries[sw]['infractions']/summaries[sw]['num_weathers'])


	return summaries




