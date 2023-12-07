import random 
import carla
from datetime import datetime
from utils import *

def changeAbsoluteTargetSpeed(root, config, intensity):
    sigma = config["AbsoluteTargetSpeed"]["sigma"] # m/s
    absoluteTargetSpeed_element = root.findall(".//AbsoluteTargetSpeed")
    for absoluteTargetSpeed_element in absoluteTargetSpeed_element:
        absoluteTargetSpeed_element.attrib["value"] = str(clip_value(random.gauss(float(absoluteTargetSpeed_element.attrib["value"]), sigma * intensity), 0, 100))
     
        
def changePerformances(root, config, intensity):
    sigma_max_speed = config["Performances"]["maxSpeed"]["sigma"]
    sigma_max_acceleration = config["Performances"]["maxAcceleration"]["sigma"]
    sigma_max_deceleration = config["Performances"]["maxDeceleration"]["sigma"]
    performances = root.findall('.//Performance')
    for performance in performances:
        performance.attrib['maxSpeed'] = str(clip_value(random.gauss(float(performance.attrib["maxSpeed"]), sigma_max_speed * intensity), 0, 100)) # m/s
        performance.attrib['maxAcceleration'] += str(clip_value(random.gauss(float(performance.attrib["maxAcceleration"]), sigma_max_acceleration * intensity), 0, 1000)) # m/s^2
        performance.attrib['maxDeceleration'] += str(clip_value(random.gauss(float(performance.attrib["maxAcceleration"]), sigma_max_deceleration * intensity), 0, 1000)) # m/s^2
        
        
def changeSpawnpoints(root, world_map, config, intensity):
    mu = config["Spawnpoints"]["mu"]
    sigma = config["Spawnpoints"]["sigma"]
    worldpositions = root.findall(".//TeleportAction/Position/WorldPosition")
    for worldposition in worldpositions:
        location = carla.Location(x = float(worldposition.attrib["x"]), y = float(worldposition.attrib["y"]), z = float(worldposition.attrib["z"]))
        try:
            distance = random.gauss(mu, sigma * intensity)
            if distance > 0:
                newLocation = world_map.get_waypoint(location).next(distance)[0].transform.location
            else:
                newLocation = world_map.get_waypoint(location).previous(abs(distance))[0].transform.locationp.get_waypoint(location).previous(random.gauss(mu, sigma))[0].transform.location
        except: # No waypoint found in that distance
            newLocation = location
            
        worldposition.attrib["x"] = str(newLocation.x)
        worldposition.attrib["y"] = str(newLocation.y)
        worldposition.attrib["z"] = str(newLocation.z)


def changeDatetime(root, config, intensity):
    sigma = config["Datetime"]["sigma"]
    dateTime_element = root.find(".//Init/Actions/GlobalAction/EnvironmentAction/Environment/TimeOfDay")
    parsed_datetime = datetime.strptime(dateTime_element.attrib["dateTime"], '%Y-%m-%dT%H:%M:%S')
    parsed_datetime = parsed_datetime.replace(hour = clip_value(parsed_datetime.hour + int(random.gauss(parsed_datetime.hour , sigma * intensity)), 0, 23))
    dateTime_element.attrib["dateTime"] = parsed_datetime.strftime('%Y-%m-%dT%H:%M:%S')
        

def changeRoutes(root, world_map, config, intensity):
    mu = config["Routes"]["mu"]
    sigma = config["Routes"]["sigma"]
    routes = root.findall(".//Route")
    for route in routes:
        # TODO: Now always all waypoints or also based on threshold?
        for wp in route.findall(".//WorldPosition"):#
            location = carla.Location(x = float(wp.attrib["x"]), y = float(wp.attrib["y"]), z = float(wp.attrib["z"]))
            try:
                distance = random.gauss(mu, sigma * intensity)
                if distance > 0:
                    newLocation = world_map.get_waypoint(location).next(distance)[0].transform.location
                else:
                    newLocation = world_map.get_waypoint(location).previous(abs(distance))[0].transform.location
            except:
                newLocation = location
            
            wp.attrib["x"] = str(newLocation.x)
            wp.attrib["y"] = str(newLocation.y)
            wp.attrib["z"] = str(newLocation.z)


def changeWeather(root, config, intensity):
    sigma = config["Weather"]["sigma"]
    weather_element = root.find(".//Init/Actions/GlobalAction/EnvironmentAction/Environment/Weather")
    if weather_element is not None:
        # Extract and update numeric values
        for element_name in ['Sun', 'Fog', 'Precipitation']:
            current_element = weather_element.find(f'./{element_name}')
            for attribute in current_element.attrib.keys():
                if attribute in ["precipitationType", "fogType", "sunState"]:
                    pass # TODO: Find out other possible values
                else:
                    current_element.attrib[attribute] = str(clip_value(random.gauss(float(current_element.attrib[attribute]), sigma * intensity), 0, 100))
                

        #weather_element.attrib["cloudstate"] = # TODO: Find out other possible values
        
        
def change(root, world_map, config, intensity):
    changeAbsoluteTargetSpeed(root, config, intensity)
    changePerformances(root, config, intensity)
    changeSpawnpoints(root, world_map, config, intensity)
    changeDatetime(root, config, intensity)
    changeRoutes(root, world_map, config, intensity)
    changeWeather(root, config, intensity)
