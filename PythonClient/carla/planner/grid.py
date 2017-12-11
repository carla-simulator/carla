

class Grid(object):

    def __init__(self,graph):

        self.graph = graph
        self.structure = self._make_structure()
        self.walls = self._make_walls()

        #np.set_printoptions(linewidth=206, threshold=np.nan)






    def _draw_line(self, grid, xi, yi, xf, yf):

        if xf < xi:
            aux = xi
            xi = xf
            xf = aux

        if yf < yi:
            aux = yi
            yi = yf
            yf = aux

        for i in range(xi, xf+1):

            for j in range(yi, yf+1):

                grid[i, j] = 0.0

        return grid

    def _make_structure(self):
        structure =np.ones((self.graph.resolution[0], self.graph.resolution[1]))

        for key, connections in self.graph.edges.iteritems():

            # draw a line
            for con in connections:

                # print key[0],key[1],con[0],con[1]
                structure = self._draw_line(structure, key[0], key[1], con[0], con[1])
                # print grid
        return structure

    def _make_walls(self):
        walls = set()

        for i in range(self.grid.shape[0]):

            for j in range(self.grid.shape[1]):
                if self.grid[i, j] == 1.0:
                    walls.add((i, j))

        return walls

    def search_on_grid(self, x, y):
        visit = [[0, 1], [0, -1], [1, 0], [1, 1],
                 [1, -1], [-1, 0], [-1, 1], [-1, -1]]
        c_x, c_y = x, y
        scale = 1
        while(self.grid[c_x, c_y] != 0):
            for offset in visit:
                c_x, c_y = x + offset[0]*scale, y + offset[1]*scale

                if c_x >= 0 and c_x < self.resolution[0] and c_y >= 0 and c_y < self.resolution[1]:
                    if self.grid[c_x, c_y] == 0:
                        break
                else:
                    c_x, c_y = x, y
            scale += 1

        return (c_x, c_y)



    def get_adjacent_free_nodes(self, pos):
        """ Acht nodes in total """
        visit = [[0, 1], [0, -1], [1, 0], [1, 1],
                 [1, -1], [-1, 0], [-1, 1], [-1, -1]]

        adjacent = set()
        for offset in visit:
            node = (pos[0] + offset[0], pos[1]+offset[1])

            if node[0] >= 0 and node[0] < self.resolution[0] and node[1] >= 0 and node[1] < self.resolution[1]:

                if self.grid[node[0], node[1]] == 0.0:
                    adjacent.add(node)

        return adjacent

    def set_grid_direction(self, pos, pos_ori, target):

        free_nodes = self.get_adjacent_free_nodes(pos)

        added_walls = set()
        heading_start = np.array([pos_ori[0], pos_ori[1]])
        for adj in free_nodes:

            start_to_goal = np.array([adj[0] - pos[0],  adj[1] - pos[1]])
            angle = angle_between(heading_start, start_to_goal)
            if (angle > 1.6 and adj != target):
                self.grid[adj[0], adj[1]] = 1.0

                added_walls.add((adj[0], adj[1]))
                self.walls.add((adj[0], adj[1]))

        return added_walls

    def set_grid_direction_target(self, pos, pos_ori, source):

        free_nodes = self.get_adjacent_free_nodes(pos)

        added_walls = set()
        heading_start = np.array([pos_ori[0], pos_ori[1]])
        for adj in free_nodes:

            start_to_goal = np.array([adj[0] - pos[0],  adj[1] - pos[1]])
            angle = angle_between(heading_start, start_to_goal)

            if (angle < 1.0 and adj != source):
                self.grid[adj[0], adj[1]] = 1.0

                added_walls.add((adj[0], adj[1]))
                self.walls.add((adj[0], adj[1]))

        return added_walls