# Collision
class CarlaEnv: #World
    def __init__(self, params):        
        self.collision_hist = []
        self.collision_bp = self.blueprint_library.find('sensor.other.collision')

    transform = self.player.get_transform()
    bounding_box = self.player.bounding_box
    acceleration = self.player.get_acceleration()
    forward_speed = self.player.get_velocity()

    def reset(self):
        self.rgb_camera = self.blueprint_library.find('sensor.camera.rgb')
		self.rgb_camera.set_attribute("image_size_x", f"{self.im_width}")
		self.rgb_camera.set_attribute("image_size_y", f"{self.im_height}")
		self.rgb_camera.set_attribute("fov", f"110")
		self.collision_sensor = self.world.spawn_actor(self.collision_bp, carla.Transform(), attach_to=self.vehicle)
        # self.collision_sensor = self.world.spawn_actor(self.collision_bp, transform(), attach_to=self.ego)
		self.actor_list.append(self.collision_sensor)
		self.collision_sensor.listen(lambda event: self.get_collision_data(event))
        self.collision_hist = []

        while self.front_camera is None:
			time.sleep(0.01)

        if self.render_display:
            self.camera_display = self.world.spawn_actor(
                blueprint_library.find('sensor.camera.rgb'),
                carla.Transform(carla.Location(x=-5.5, z=2.8), carla.Rotation(pitch=-15)),
                attach_to=self.vehicle)
            self.actor_list.append(self.camera_display)
        bp = blueprint_library.find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', str(self.vision_size))
        bp.set_attribute('image_size_y', str(self.vision_size))
        bp.set_attribute('fov', str(self.vision_fov))
        location = carla.Location(x=1.6, z=1.7)
        self.camera_vision = self.world.spawn_actor(bp, carla.Transform(location, carla.Rotation(yaw=0.0)), attach_to=self.vehicle)
        self.actor_list.append(self.camera_vision)


    def get_collision_data(event):
        # What we collided with and what was the impulse
        collision_actor_id = event.other_actor.type_id
        collision_impulse = event.normal_impulse
        collision_intensity = np.sqrt(impulse.x**2 + impulse.y**2 + impulse.z**2)

        # Filter collisions
        for actor_id, impulse in settings.COLLISION_FILTER:
            if actor_id in collision_actor_id and (intensity == -1 or collision_intensity <= intensity):
                return

        # Add collision
        self.collision_hist.append(collision_intensity)
        # self.collision_hist.append(event)
        # if len(self.collision_hist)>self.collision_hist_l:
        #     self.collision_hist.pop(0)
    
    def step(self):
        return self.get_reward()

    def get_reward(self):
        if len(self.collision_hist) > 0:
            done = True
            reward = -1


    def process_img(self, image):
		i = np.array(image.raw_data)
		print(i.shape)
		i2 = i.reshape((self.im_height, self.im_width, 4))

		# The entire height, the entire width, thr first three of r, g, b, a
		# Kinda nifty way to grab r,g,b there is an open cv method to do the exact same thing
		# Basically converts rgb alpha to just straight up  rgb, interestingly enough it is slower than the numpy method
		# You would have expected it to do the same thing, but simply does not
		i3 = i2[:, :, :3]
		if self.SHOW_CAM:
			cv2.imshow("", i3)
			cv2.waitKey(1)
		self.front_camera = i3
		# return i3/255.0

     def on_invasion(self, event):
        '''Whenever the car crosses the lane, the flag is set to True'''
        lane_types = set(x.type for x in event.crossed_lane_markings)
        text = ['%r' % str(x).split()[-1] for x in lane_types]
        self.lane_crossed_type = text[0]
        self.lane_crossed = True

    def get_reward(self):
        transform = self.player.get_transform()
        d_x = transform.location.x
        d_y = transform.location.y
        d_z = transform.location.z
        player_location = np.array([d_x, d_y, d_z])
        goal_location = np.array([self.target.x,
                                  self.target.y,
                                  self.target.z])
        d = np.linalg.norm(player_location - goal_location) / 1000

        # Speed
        velocity = self.player.get_velocity()
        # v = np.linalg.norm([velocity.x, velocity.y, velocity.z])
        v = math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2) * 3.6

        # Collision damage      
        colhist = self.collision_sensor.get_collision_history()
        # collision = [colhist[x + self.frame - 200] for x in range(0, 200)]
        max_col = max(1.0, max(colhist))
        # c_v = measurements.collision_vehicles
        # c_p = measurements.collision_pedestrians
        # c_o = measurements.collision_other
        # c = c_v + c_p + c_o

        # # Intersection with sidewalk
        # s = measurements.intersection_offroad

        # # Intersection with opposite lane
        # o = measurements.intersection_otherlane

        # Compute reward
        r = 0
        if self.state is not None:
            r += 1000 * (self.state['d'] - d)
            r += 0.05 * (v - self.state['v'])
            r -= 0.00002 * (c - self.state['c'])
            # r -= -0.1 * float(s > 0.001)
            # r -= 2 * (o - self.state['o'])

        return r   

# class CarlaEnvSettings:
#     def __init__():
#         # We want to track NPC collisions so we can remove and spawn new ones
#         # Collisions are really not rare when using built-in autopilot
#         self.collisions = Queue()

#     def _collision_data(self, collision):
#         self.collisions.put(collision)

#     ## in Loop
#     def update_settings_in_loop(self):
#         # Handle all registered collisions
#         while not self.collisions.empty():

#             # Gets first collision from the queue
#             collision = self.collisions.get()

#             # Gets car NPC's id and destroys it
#             car_npc = collision.actor.id
#             self._destroy_car_npc(car_npc)