import carla
import random
import weakref

def get_actor_blueprints(world, filter, generation):
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed
    # and therefore, we ignore the generation
    if len(bps) == 1:
        return bps

    try:
        int_generation = int(generation)
        # Check if generation is in available generations
        if int_generation in [1, 2]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []

class V2XSensor(object):
    def __init__(self, parent_actor):
        self.sensor = None
        self._parent = parent_actor
        world = self._parent.get_world()
        #bp = world.get_blueprint_library().find('sensor.other.v2x_custom')
        bp = world.get_blueprint_library().find('sensor.other.v2x')
        self.sensor = world.spawn_actor(
            bp, carla.Transform(), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(
            lambda sensor_data: V2XSensor._V2X_callback(weak_self, sensor_data))
    
    def destroy(self):
        self.sensor.stop()
        self.sensor.destroy()

    @staticmethod
    def _V2X_callback(weak_self, sensor_data):
        self = weak_self()
        if not self:
            return
        for data in sensor_data:
            msg = data.get()
            # stationId = msg["Header"]["Station ID"]
            power = data.power 
            print(msg)
            # print('Cam message received from %s ' % stationId)
            print('Cam message received with power %f ' % power)
        
client = carla.Client("localhost",2000)
client.set_timeout(2000.0)

world = client.get_world()
smap = world.get_map()
# acl = world.get_actor(28)
# acl.send("test")


spawn_points = smap.get_spawn_points()
spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
blueprint = random.choice(get_actor_blueprints(world, "vehicle.*", "2"))
blueprint.set_attribute('role_name', "test")
player = world.try_spawn_actor(blueprint, spawn_point)
v2x_sensor = V2XSensor(player)

world.wait_for_tick()
try:
    while True:
        world.wait_for_tick()
finally:
    v2x_sensor.destroy()
    player.destroy()