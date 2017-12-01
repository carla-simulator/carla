import math
import numpy as np
from matplotlib import collections  as mc
import matplotlib.pyplot as plt

def string_to_node(string):
	vec = string.split(',')

	return (int(vec[0]),int(vec[1]))

def string_to_floats(string):
	vec = string.split(',')

	return (float(vec[0]),float(vec[1]),float(vec[2]))

def angle_between(v1,v2):
    return np.arccos(np.dot(v1,v2) / np.linalg.norm(v1) / np.linalg.norm(v2))
def signal(v1,v2):
    return np.cross(v1,v2) / np.linalg.norm(v1) / np.linalg.norm(v2)

sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)

sldist3 = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2 + (c2[2] - c1[2])**2)

class Graph(object):
	"""
	A simple directed, weighted graph
	"""




	def __init__(self,graph_file=None):


		self.nodes = set()
		self.angles ={}
		self.edges = {}
		self.distances = {}
		if graph_file != None:
			with  open(graph_file, 'r') as file:
				for i in range(5):
					next(file)
				for line in file:

					from_node, to_node, d = line.split()
					from_node = string_to_node(from_node)
					to_node = string_to_node(to_node)

					if from_node not in self.nodes:
						self.add_node(from_node)
					if to_node not in self.nodes:
						self.add_node(to_node)


					
					self.edges.setdefault(from_node,[])
					self.edges[from_node].append(to_node)
					self.distances[(from_node, to_node)] = float(d)



	def add_node(self, value):
		self.nodes.add(value)



	def make_orientations(self,node,heading):

		import collections
		distance_dic = {}
		for node_iter in self.nodes:
			if node_iter != node:
				distance_dic[sldist(node,node_iter)] = node_iter




		distance_dic = collections.OrderedDict(sorted(distance_dic.items()))

		self.angles[node ] = heading
		for k, v in distance_dic.iteritems():

			#print k
			#print v

			start_to_goal = np.array([node[0] - v[0], node[1] - v[1]])

			print start_to_goal

			self.angles[v]  = start_to_goal /  np.linalg.norm(start_to_goal)


	def add_edge(self, from_node, to_node, distance):
		self._add_edge(from_node, to_node, distance)


	def _add_edge(self, from_node, to_node, distance):
		self.edges.setdefault(from_node, [])
		self.edges[from_node].append(to_node)
		self.distances[(from_node, to_node)] = distance

	def intersection_nodes(self):

		intersect_nodes = []
		for node in self.nodes:
			if len(self.edges[node]) > 2:
				intersect_nodes.append(node)

		return intersect_nodes

	# This contains also the non-intersection turns...

	def turn_nodes(self):


		return self.nodes

	def plot_ori(self,c):
		line_len = 1
		print self.angles
		lines = [[(p[0], p[1]), (p[0] + line_len*self.angles[p][0], p[1] + \
												line_len*self.angles[p][1])] for p in self.nodes]
		lc = mc.LineCollection(lines, linewidth=2,color='green')
		fig, ax = plt.subplots()
		ax.add_collection(lc)


		ax.autoscale()
		ax.margins(0.1)

		xs = [p[0] for p in self.nodes]
		ys = [p[1] for p in self.nodes]


		plt.scatter(xs, ys,color=c)    



	def plot(self,c):

		xs = [p[0] for p in self.nodes]
		ys = [p[1] for p in self.nodes]


		plt.scatter(xs, ys,color=c)    

		




