from graph import *
from PIL import Image
import math
from astar import *
import time
import collections

compare = lambda x, y: collections.Counter(x) == collections.Counter(y)

def angle_between(v1,v2):
    return np.arccos(np.dot(v1,v2) / np.linalg.norm(v1) / np.linalg.norm(v2))

sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)

def color_to_angle(color):
	return ((float(color)/255.0) ) *2*math.pi 



class Planner(object):

	# The built graph. This is the exact same graph that unreal builds. This is a generic structure used for many cases
	def __init__(self,city_file,map_file):
		# read the conversion parameters from the city file
		with  open(city_file, 'r') as file:

			linewordloffset = file.readline()
			# The offset of the world from the zero coordinates ( The coordinate we consider zero)
			self.worldoffset = string_to_floats(linewordloffset)

			#WARNING: for now just considering the y angle
			lineworldangles = file.readline()
			self.angles =  string_to_floats(lineworldangles)
			#self.worldrotation = np.array([[math.cos(math.radians(self.angles[0])),0,math.sin(math.radians(self.angles[0])) ],[0,1,0],[-math.sin(math.radians(self.angles[0])),0,math.cos(math.radians(self.angles[0]))]])

			self.worldrotation = np.array([[math.cos(math.radians(self.angles[2])),-math.sin(math.radians(self.angles[2])) ,0.0],[math.sin(math.radians(self.angles[2])),math.cos(math.radians(self.angles[2])),0.0],[0.0,0.0,1.0]])

			# Ignore for now
			lineworscale = file.readline()

			linemapoffset = file.readline()

			# The offset of the map zero coordinate
			self.mapoffset =  string_to_floats(linemapoffset)		

			# the graph resolution.
			linegraphres = file.readline()
			self.resolution =  string_to_node(linegraphres)	
				
		
		#This is the bmp map that will be associated with the graph. 
		#This map contains the navigable paths drawed
		self.graph = Graph(city_file)
		
		self.map_image = Image.open(map_file)
		self.map_image.load()
		self.map_image = np.asarray(self.map_image, dtype="int32" )


		import os
		dir_path = os.path.dirname(__file__)
		self.central_path_map_image = Image.open(map_file[:-4] +'Central.png')
		self.central_path_map_image.load()
		self.central_path_map_image = np.asarray(self.central_path_map_image, dtype="int32" )
		self.central_path_map_image =self.central_path_map_image[:,:,0] # Just take the red dimension

		self.grid = self.make_grid()
		self.walls = self.make_walls()

		
		self.previous_source = (0,0)
		self.distance = 0
		self.complete_distance = 0

		#print self.map_image
		self.commands = []

		self.route =[]


		# The number of game units per pixel 
		self.pixel_density = 16.43
		#A pixel positions with respect to graph node position is:  Pixel = Node*50 +2
		self.node_density = 50.0
		# This function converts the 2d map into a 3D one in a vector.





	def _draw_line(self,grid,xi,yi,xf,yf):

		if xf< xi:
			aux = xi
			xi = xf
			xf = aux

		if yf< yi:
			aux = yi
			yi = yf
			yf = aux


		for i in range(xi,xf+1):

			for j in range(yi,yf+1):

				grid[i,j] = 0.0

		return grid

	def make_grid(self): # The resolution could be easily increased


		grid = np.ones((self.resolution[0],self.resolution[1]))

		for key,connections in self.graph.edges.iteritems():

			# draw a line
			for con in connections:

				#print key[0],key[1],con[0],con[1]
				grid = self._draw_line(grid,key[0],key[1],con[0],con[1])
				#print grid


		np.set_printoptions( linewidth =206,threshold=np.nan)


		return grid


	def make_walls(self):
		walls = set()

		for i in range(self.grid.shape[0]):
			
			for j in range(self.grid.shape[1]):
				if self.grid[i,j] == 1.0:
					walls.add((i,j))

		return walls

	def init(self,source,target):

		self.a_star.init_grid(self.resolution[0],self.resolution[1], self.walls,source,target)


	def solve(self):
		return self.a_star.solve()




	# Convert world position into "Graph World" node positions
	def make_node(self,worldvertex):

		pixel = self.make_map_world(worldvertex)


		node = []




		node.append((pixel[0])/self.node_density - 2)
		node.append((pixel[1])/self.node_density - 2 )

		return tuple(node)

	def make_map_world(self,world):

		relative_location = []
		pixel=[]

		rotation = np.array([world[0],world[1],world[2]])
		rotation = rotation.dot(self.worldrotation)



		relative_location.append(rotation[0] + self.worldoffset[0] - self.mapoffset[0])
		relative_location.append(rotation[1] + self.worldoffset[1] - self.mapoffset[1])
		relative_location.append(rotation[2] + self.worldoffset[2] - self.mapoffset[2])

	
		pixel.append(math.floor(relative_location[0]/float(self.pixel_density)))
		pixel.append(math.floor(relative_location[1]/float(self.pixel_density)))

		return pixel
	def make_map_node(self,node):
		pixel = []
		pixel.append((node[0] +2) *self.node_density)
		pixel.append((node[1] +2) *self.node_density)

		return pixel

	def make_world_map(self,pixel):

		relative_location =[]
		world_vertex = []	
		relative_location.append(pixel[0]*self.pixel_density)
		relative_location.append(pixel[1]*self.pixel_density)

		world_vertex.append(relative_location[0]+self.mapoffset[0] -self.worldoffset[0])
		world_vertex.append(relative_location[1]+self.mapoffset[1] -self.worldoffset[1])
		world_vertex.append(22)
		return world_vertex


	def make_world_node(self,node):
		
		return self.make_world_map(self.make_map_node(node))


	def get_distance_closest_node(self,pos):
		import collections
		distance  = []
		for node_iter in self.graph.intersection_nodes():

			distance.append( sldist(node_iter,pos))

		return sorted(distance)[0]

	def get_distance_closest_node(self,pos):
		import collections
		distance  = []
		for node_iter in self.graph.intersection_nodes():

			distance.append( sldist(node_iter,pos))

		return sorted(distance)[0]

	def get_distance_closest_node_route(self,pos,route):
		import collections
		distance  = []
		#if self.graph.intersection_nodes() == set():
			
		for node_iter in route:

			if node_iter in self.graph.intersection_nodes():

				distance.append( sldist(node_iter,pos))

		if not distance:

			return sldist(route[-1],pos)
		return sorted(distance)[0]


	def get_target_ori(self,target_pos):
		
		relative_location = []
		pixel=[]
		rotation = np.array([target_pos[0],target_pos[1],target_pos[2]])
		rotation = rotation.dot(self.worldrotation)

		#print 'rot ', rotation
		
		relative_location.append(rotation[0] + self.worldoffset[0] - self.mapoffset[0])
		relative_location.append(rotation[1] + self.worldoffset[1] - self.mapoffset[1])
		relative_location.append(rotation[2] + self.worldoffset[2] - self.mapoffset[2])
		#print 'trans ', relative_location

		pixel.append(math.floor(relative_location[0]/float(self.pixel_density)))
		pixel.append(math.floor(relative_location[1]/float(self.pixel_density)))
		#print self.map_image.shape
		ori = self.map_image[int(pixel[1]),int(pixel[0]),2]
		ori = ((float(ori)/255.0) ) *2*math.pi 

		#print self.map_image[int(pixel[1]),int(pixel[0]),:]
		#print ori
		#print (math.cos(ori),math.sin(ori))
		#print exit()

		return (-math.cos(ori),-math.sin(ori))







	def search(self,x,y):
		visit = [[0,1],[0,-1],[1,0],[1,1],[1,-1],[-1,0],[-1,1],[-1,-1]]
	 	c_x,c_y = x,y
		scale=1
		while(self.grid[c_x,c_y] !=0 ):
			for offset in visit:
				c_x,c_y = x + offset[0]*scale,y + offset[1]*scale

				if c_x>= 0 and c_x <self.resolution[0] and c_y>= 0 and c_y <self.resolution[1]:
					if self.grid[c_x,c_y] ==0:
						break
				else:
					c_x,c_y = x,y
			scale +=1

		return (c_x,c_y)

	def get_adjacent_free_nodes(self,pos):

		""" Acht nodes in total """
		visit = [[0,1],[0,-1],[1,0],[1,1],[1,-1],[-1,0],[-1,1],[-1,-1]]

		adjacent = set()
		for offset in visit:
			node = (pos[0] +offset[0],pos[1]+offset[1])

			if node[0]>= 0 and node[0] <self.resolution[0] and node[1]>= 0 and node[1] <self.resolution[1]:

				if self.grid[node[0],node[1]]  == 0.0:
					adjacent.add(node)

		return adjacent


	def  set_grid_direction(self,pos,pos_ori,target):

		free_nodes = self.get_adjacent_free_nodes(pos)


		added_walls =set()
		heading_start = np.array([pos_ori[0], pos_ori[1]])
		for adj in free_nodes:

			start_to_goal = np.array([adj[0]  - pos[0],  adj[1] - pos[1] ])
			angle = angle_between(heading_start,start_to_goal)
			if (angle > 1.6 and adj !=target) :
				self.grid[adj[0],adj[1]] =1.0

				added_walls.add((adj[0],adj[1]))
				self.walls.add((adj[0],adj[1]))

		return added_walls

	def  set_grid_direction_target(self,pos,pos_ori,source):

		free_nodes = self.get_adjacent_free_nodes(pos)


		added_walls =set()
		heading_start = np.array([pos_ori[0], pos_ori[1]])
		for adj in free_nodes:

			start_to_goal = np.array([adj[0]  - pos[0],  adj[1] - pos[1] ])
			angle = angle_between(heading_start,start_to_goal)

			if (angle < 1.0 and adj !=source) :
				self.grid[adj[0],adj[1]] =1.0

				added_walls.add((adj[0],adj[1]))
				self.walls.add((adj[0],adj[1]))

		return added_walls


	#from the shortest path graph, transform it into a list of commands
	# @param the sub graph containing the shortest path
	# @param Orientation of the car
	# returns list of commands ( 3 is left, 4 is right, 5 is straight)

	def graph_to_commands(self, route ):

		commands_list = []
		
		for i in range(0,len(route)):
			if route[i] not in self.graph.intersection_nodes():
				continue

			current = route[i]
			past =route[i-1]
			future = route[i+1]

			past_to_current = np.array([current[0]-past[0], current[1]-past[1]])
			current_to_future = np.array([future[0]-current[0], future[1]-current[1]])
			angle =signal(current_to_future,past_to_current)

			command = 0.0
			if angle < -0.1:
				command = 4.0
			elif angle > 0.1:
				command = 3.0
			else:
				command =5.0

			commands_list.append(command)


		return commands_list

	def check_command_completed(self,commands,previous_commands):
		if compare(commands,previous_commands):
			return False,False
		elif (len(commands) + 1) < len(previous_commands):
			return True,False

		elif len(commands) < len(previous_commands):



			return True,compare(commands,previous_commands[1:])
		else:
			return True,False



	def get_full_distance_route(self,pos,route):
		import collections
		distance  = 0
		#if self.graph.intersection_nodes() == set():
		current_pos = pos
		for node_iter in route:



			distance +=  sldist(node_iter,current_pos)
			current_pos = node_iter


		return distance





	def get_next_command(self,source,source_ori,  target,target_ori):


		node_source = self.make_node(source)
		node_target =	self.make_node(target)

		source_ori = np.array([source_ori[0],source_ori[1],source_ori[2]])
		source_ori = source_ori.dot(self.worldrotation)

		
		# Trunkate !
		node_source  =   tuple([ int(x) for x in node_source ])
		node_target  =   tuple([ int(x) for x in node_target ])
		target_ori   =    self.get_target_ori(target)
		# Set to zero if it is less than zero.



		target_ori = np.array([target_ori[0],target_ori[1],0])
		target_ori = target_ori.dot(self.worldrotation)


		node_source =(max(0,node_source[0]),max(0,node_source[1]))
		node_source =(min(self.resolution[0]-1,node_source[0]),min(self.resolution[1]-1,node_source[1]))
		# is it x or y ? Check to avoid  special corner cases


		if math.fabs(source_ori[0]) > math.fabs(source_ori[1]):
			source_ori = (source_ori[0],0.0,0.0)
		else:
			source_ori = (0.0,source_ori[1],0.0)



		node_source = self.search(node_source[0],node_source[1])
		node_target	= self.search(node_target[0],node_target[1])
		#print ''
		#print node_source
		#print node_target
		#print self.grid

		# reach the goal
		if node_source == node_target:
			return 0,0


		# This is to avoid computing a new route when inside the route
		distance_node = self.get_distance_closest_node(node_source)

		if (distance_node >1 and self.previous_source != node_source) or self.complete_distance ==0:

			#print node_source
			#print node_target
			added_walls = self.set_grid_direction(node_source,source_ori,node_target)
			#print added_walls
			added_walls=added_walls.union(self.set_grid_direction_target(node_target,target_ori,node_source))
			#print added_walls
			self.previous_source = node_source

			#print self.grid

			self.a_star =AStar()
			self.init(node_source, node_target)
			route = self.solve()
			#print route # JuSt a Corner Case 
			if route == None:
				for i in added_walls:
					self.walls.remove(i)

					self.grid[i[0],i[1]] = 0.0
				added_walls = self.set_grid_direction(node_source,source_ori,node_target)
				self.a_star =AStar()
				self.init(node_source, node_target)
				route = self.solve()

			#print route

			# We recompute the distance based on route
			self.distance= self.get_distance_closest_node_route(node_source,route)
			self.complete_distance = self.get_full_distance_route(node_source,route)*50.0*16.42

			for i in added_walls:
				self.walls.remove(i)

				self.grid[i[0],i[1]] = 0.0

			commands = self.graph_to_commands(route)
			made_turn,completed_command = self.check_command_completed(commands,self.commands)
			

			self.commands = commands
			next_node=route[0]
			for i in route:
				if i in self.graph.nodes:
					next_node = i
					break

			 

			if self.distance > 4: 
				return 2.0,self.complete_distance
			else:
				if self.commands:
					return self.commands[0],self.complete_distance
				else:
					return 2.0,self.complete_distance
		else:

			if self.distance >4:
				return 2.0,self.complete_distance

			if self.commands:
				return self.commands[0],self.complete_distance
			else:
				return 2.0,self.complete_distance

