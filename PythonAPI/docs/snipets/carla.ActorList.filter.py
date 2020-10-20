# ===================================================
# Filter actors in ActorList or Blueprints in the BlueprintLibrary
# ===================================================

# This snipet shows how to use the filter() method to retrieve specific sets of actors or blueprints
#   If you are looking for a specific instance of an actor that has been spawned, it is likely you 
#   are looking for the .find() method in carla.World and carla.ActorList, which requires the actor ID. 

# Do not mix .find() and .filter(), as the later will return any actor/blueprint that matches the wildcard pattern used. 


# The filter uses four main special characters to create the wildcard pattern.
#   * = anything goes until something else is specified.
#   ? = a single character.
#   [seq] = a specific set of characters.
#   [!seq] = a specific set of characters does NOT appear.

# First let's return all the actors in the scene.
all_actors = my_world.get_actors()

# Now, let's filter these: 

#   Retrieve all the vehicles of any type.
all_vehicles = all_actors.filter('vehicle.*')
all_tesla = all_actors.filter('vehicle.tesla.*')

#   Retrieve all vehicles but the CarlaCola van.
all_not_carlacacola = all_actors.filter('vehicle.[!carlamotors.carlacola]*')


# Similarly, let's use the blueprint library to filter some blueprints.
blueprint_library = my_world.get_blueprint_library()

# Let's suppose we want to retrieve all the walkers blueprints numbered from 0 to 99.
specific_blueprints = blueprint_library.filter('walker.pedestrian.00??')




# The filter is based on the fnmatch pattern matching standard. Find out more about it to learn about its possibilities: 
#   https://docs.python.org/2/library/fnmatch.html
