#!/usr/bin/env python
# Scenario simulation
#by Joe

import glob
import os
import sys
import time

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import random

def main():

    client = carla.Client("127.0.0.1", 2000)
    client.set_timeout(10.0)


    try:
        world = client.get_world()


        blueprints = world.get_blueprint_library().filter('vehicle.mercedes*')

        spawn_points = world.get_map().get_spawn_points()

        # --------------
        # Spawn vehicles
        # --------------
        
        transform=spawn_points[:]

        blueprint = random.choice(blueprints)
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)
        if blueprint.has_attribute('driver_id'):
            driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
            blueprint.set_attribute('driver_id', driver_id)
        blueprint.set_attribute('role_name', 'autopilot')
        # for x in range(len(transform)):
        # 	print(transform[x])
        print(blueprint)
        
        #Specify location
        transform.location.x=41.2
        transform.location.y=19.9
        transform.rotation.yaw=193
        print(transform.location.x)
        actor = world.spawn_actor(blueprint, transform)



        # print("tom is here!!!")
        actors_list=world.get_actors()
        actors_veh=actors_list.filter('vehicle.*')
        print(actors_veh)
        vehicles_list=actors_list

       
        print('spawned %d vehicles, press Ctrl+C to exit.' % len(vehicles_list))
        print("Scenario: forbidden maneuver")
    
        counter=0
        while True:

            world.wait_for_tick()
            counter+=1	

            #Scenario---------------------------
            control = carla.VehicleControl()
            if counter <1600:
	            control.throttle = 1
	            #actor.set_autopilot(enabled=True)
	            actor.apply_control(control)
            else:
           		control.throttle = 0
           		control.hand_brake=True
           		actor.apply_control(control)
           	#----------------------------------------
    finally:

        actor.destroy()
        time.sleep(0.5)

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
