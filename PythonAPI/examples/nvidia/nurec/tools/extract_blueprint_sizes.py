#!/usr/bin/env python
# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Blueprint Size Extraction Tool

This tool connects to a CARLA server, spawns each available vehicle and walker blueprint,
measures their bounding box dimensions, and saves the results to JSON files.

The extracted dimensions are used by the NUREC integration to correctly size objects
when replaying scenarios in CARLA.
"""
import argparse
import carla
import json
import time
    
def main():
    """
    Main function that connects to CARLA, spawns actors, measures their dimensions,
    and saves the results to JSON files.
    """
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    args = argparser.parse_args()

    try:
        # Connect to the CARLA server
        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)
        world = client.get_world()
        blueprint_library = world.get_blueprint_library()
        spawn_points = world.get_map().get_spawn_points()
        print(f"Found {len(spawn_points)} spawn points")
        
        # Lists to store the blueprint names and their dimensions
        blueprint_sizes_vehicle = []
        blueprint_sizes_walker = []
        
        # Lists to keep track of spawned actors
        walkers = []
        vehicles = []
        
        # Spawn actors and measure their dimensions
        i = 0
        for blueprint in blueprint_library:
            name = blueprint.id
            if "vehicle" in name:
                # Spawn vehicle
                spawn_point = spawn_points[i]
                vehicle = world.spawn_actor(blueprint, spawn_point)
                vehicles.append((name, vehicle))
            elif "walker" in name:
                # Spawn walker
                spawn_point = spawn_points[i]
                walker = world.spawn_actor(blueprint, spawn_point)
                walkers.append((name, walker))
            i += 1
            
            # If we've used all spawn points, measure and destroy the current batch
            # before starting a new one
            if i > len(spawn_points):
                print("Too many vehicles and walkers")
                time.sleep(1)  # Wait for physics to settle
                
                # Process vehicles
                for name, vehicle in vehicles:
                    # Get dimensions from the bounding box
                    dimensions = vehicle.bounding_box.extent
                    dimensions = [dimensions.x, dimensions.y, dimensions.z]
                    box_location = vehicle.bounding_box.location
                    offset = [box_location.x, box_location.y, box_location.z]
                    blueprint_sizes_vehicle.append((name, dimensions, offset))
                    vehicle.destroy()
                
                # Process walkers
                for name, walker in walkers:
                    dimensions = walker.bounding_box.extent
                    dimensions = [dimensions.x, dimensions.y, dimensions.z]
                    box_location = walker.bounding_box.location
                    offset = [box_location.x, box_location.y, box_location.z]
                    blueprint_sizes_walker.append((name, dimensions, offset))
                    walker.destroy()
                
                # Reset for next batch
                i = 0
                vehicles = []
                walkers = []
                
        
        # Process any remaining vehicles and walkers
        time.sleep(1)  # Wait for physics to settle
        for name, vehicle in vehicles:
            # Get dimensions
            dimensions = vehicle.bounding_box.extent
            dimensions = [dimensions.x, dimensions.y, dimensions.z]
            box_location = vehicle.bounding_box.location
            offset = [box_location.x, box_location.y, box_location.z]
            blueprint_sizes_vehicle.append((name, dimensions, offset))
            vehicle.destroy()
        
        for name, walker in walkers:
            dimensions = walker.bounding_box.extent
            dimensions = [dimensions.x, dimensions.y, dimensions.z]
            box_location = walker.bounding_box.location
            offset = [box_location.x, box_location.y, box_location.z]
            blueprint_sizes_walker.append((name, dimensions, offset))
            walker.destroy()

        # Filter out invalid dimensions (infinity, NaN)
        blueprint_sizes_vehicle = [x for x in blueprint_sizes_vehicle 
                                  if x[1] != [float("inf"), float("inf"), float("inf")] 
                                  and x[1] != [float("nan"), float("nan"), float("nan")] 
                                  and x[1] != [float("-inf"), float("-inf"), float("-inf")]]
        
        blueprint_sizes_walker = [x for x in blueprint_sizes_walker 
                                 if x[1] != [float("inf"), float("inf"), float("inf")] 
                                 and x[1] != [float("nan"), float("nan"), float("nan")] 
                                 and x[1] != [float("-inf"), float("-inf"), float("-inf")]]
        
        print(f"Found {len(blueprint_sizes_vehicle)} vehicles and {len(blueprint_sizes_walker)} walkers")
        # Save the results to JSON files
        with open("blueprint_sizes_vehicle.json", "w") as f:
            json.dump(blueprint_sizes_vehicle, f, indent=4)
        
        with open("blueprint_sizes_walker.json", "w") as f:
            json.dump(blueprint_sizes_walker, f, indent=4)

    except Exception as e:
        print(e)
    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')