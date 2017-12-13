      

    # Projecting the nodes
        node_source = self.make_node(source)
        node_target = self.make_node(target)

        source_ori = np.array([source_ori[0], source_ori[1], source_ori[2]])
        source_ori = source_ori.dot(self.worldrotation)

        # Trunkate !
        node_source = tuple([int(x) for x in node_source])
        node_target = tuple([int(x) for x in node_target])
        #target_ori = self.get_target_ori(target)
        # Set to zero if it is less than zero.

        target_ori = np.array([target_ori[0], target_ori[1], 0])
        target_ori = target_ori.dot(self.worldrotation)

        node_source = (max(0, node_source[0]), max(0, node_source[1]))
        node_source = (min(self.resolution[
                       0]-1, node_source[0]), min(self.resolution[1]-1, node_source[1]))


        # is it x or y ? Check to avoid  special corner cases

        if math.fabs(source_ori[0]) > math.fabs(source_ori[1]):
            source_ori = (source_ori[0], 0.0, 0.0)
        else:
            source_ori = (0.0, source_ori[1], 0.0)

        node_source = self.search(node_source[0], node_source[1])
        node_target = self.search(node_target[0], node_target[1])

   def _route_compute(self,node_source,source_ori,node_target,target_ori):


        # GOes to Grid
        #added_walls = self.set_grid_direction(node_source,source_ori,node_target)
        #print added_walls
        # Goes to Walls
        #added_walls=added_walls.union(self.set_grid_direction_target(node_target,target_ori,node_source))
        #print added_walls


        self.previous_source = node_source

        #print self.grid

        #self.a_star =AStar()
        self.init(node_source, node_target)
        self.route = self.solve()
        #print route # JuSt a Corner Case 
        if self.route == None:
            for i in added_walls:
                self.walls.remove(i)

                self.grid[i[0],i[1]] = 0.0
            added_walls = self.set_grid_direction(node_source,source_ori,node_target)
            self.a_star =AStar()
            self.init(node_source, node_target)
            self.route = self.solve()

            
        for i in added_walls:
            self.walls.remove(i)

            self.grid[i[0],i[1]] = 0.0



        return self.route

        # This is to avoid computing a new route when inside the route
        # distance_node = self.get_distance_closest_node(node_source)
        # Planner shouldnt have knowledge about node

        #if (distance_node > 1 \
        # and self.previous_source != node_source) or self.complete_distance == 0:
