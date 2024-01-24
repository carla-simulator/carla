import random 
import carla
from datetime import datetime
from utils import *

# TODO check upper and lower bounds

def changeAbsoluteTargetSpeed(root, config):
    threshold = config["AbsoluteTargetSpeed"]["threshold"]
    sigma = config["AbsoluteTargetSpeed"]["sigma"]
    absoluteTargetSpeed_element = root.findall(".//AbsoluteTargetSpeed")
    for absoluteTargetSpeed_element in absoluteTargetSpeed_element:
        if random.random() > threshold:
            absoluteTargetSpeed_element.attrib["value"] = str(clip_value(random.gauss(float(absoluteTargetSpeed_element.attrib["value"]), sigma), 0, 100))
     
        
def changePerformances(root, config):
    threshold = config["Performances"]["threshold"]
    sigma = config["Performances"]["sigma"]
    performances = root.findall('.//Performance')
    for performance in performances:
        if random.random() > threshold:
            performance.attrib['maxSpeed'] = str(clip_value(random.gauss(float(performance.attrib["maxSpeed"]), sigma), 0, 10000)) 
        if random.random() > threshold:
            performance.attrib['maxAcceleration'] += str(clip_value(random.gauss(float(performance.attrib["maxAcceleration"]), sigma), 0, 10000))
        if random.random() > threshold:
            performance.attrib['maxDeceleration'] += str(clip_value(random.gauss(float(performance.attrib["maxAcceleration"]), sigma), 0, 10000))
        
        
def changeSpawnpoints(root, world_map, config):
    threshold = config["Spawnpoints"]["threshold"]
    mu = config["Spawnpoints"]["mu"]
    sigma = config["Spawnpoints"]["sigma"]
    worldpositions = root.findall(".//TeleportAction/Position/WorldPosition")
    for worldposition in worldpositions:
        if random.random() > threshold:
            location = carla.Location(x = float(worldposition.attrib["x"]), y = float(worldposition.attrib["y"]), z = float(worldposition.attrib["z"]))
            try:
                if random.random() < 0.5:
                    newLocation = world_map.get_waypoint(location).next(random.gauss(mu, sigma))[0].transform.location
                else:
                    newLocation = world_map.get_waypoint(location).previous(random.gauss(mu, sigma))[0].transform.location
            except: # No waypoint found in that distance
                newLocation = location
                
            worldposition.attrib["x"] = str(newLocation.x)
            worldposition.attrib["y"] = str(newLocation.y)
            worldposition.attrib["z"] = str(newLocation.z)


def changeDatetime(root, config):
    threshold = config["Datetime"]["threshold"]
    sigma = config["Datetime"]["sigma"]
    dateTime_element = root.find(".//Init/Actions/GlobalAction/EnvironmentAction/Environment/TimeOfDay")
    if random.random() > threshold:
        parsed_datetime = datetime.strptime(dateTime_element.attrib["dateTime"], '%Y-%m-%dT%H:%M:%S')
        parsed_datetime = parsed_datetime.replace(hour = clip_value(parsed_datetime.hour + int(random.gauss(parsed_datetime.hour , sigma)), 0, 23))
        dateTime_element.attrib["dateTime"] = parsed_datetime.strftime('%Y-%m-%dT%H:%M:%S')
        

def changeRoutes(root, world_map, config):
    threshold = config["Routes"]["threshold"]
    mu = config["Routes"]["mu"]
    sigma = config["Routes"]["sigma"]
    routes = root.findall(".//Route")
    for route in routes:
        if random.random() > threshold:
            # TODO: Now always all waypoints or also based on threshold?
            for wp in route.findall(".//WorldPosition"):#
                location = carla.Location(x = float(wp.attrib["x"]), y = float(wp.attrib["y"]), z = float(wp.attrib["z"]))
                try:
                    if random.random() < 0.5:
                        newLocation = world_map.get_waypoint(location).next(random.gauss(mu, sigma))[0].transform.location
                    else:
                        newLocation = world_map.get_waypoint(location).previous(random.gauss(mu, sigma))[0].transform.location
                except:
                    newLocation = location
                
                wp.attrib["x"] = str(newLocation.x)
                wp.attrib["y"] = str(newLocation.y)
                wp.attrib["z"] = str(newLocation.z)


def changeWeather(root, config):
    threshold = config["Weather"]["threshold"]
    sigma = config["Weather"]["sigma"]
    weather_element = root.find(".//Init/Actions/GlobalAction/EnvironmentAction/Environment/Weather")
    if weather_element is not None:
        # Extract and update numeric values
        for element_name in ['Sun', 'Fog', 'Precipitation']:
            current_element = weather_element.find(f'./{element_name}')
            for attribute in current_element.attrib.keys():
                if threshold > random.random():
                    if attribute in ["precipitationType", "fogType", "sunState"]:
                        pass # TODO: Find out other possible values
                    else:
                        current_element.attrib[attribute] = str(clip_value(random.gauss(float(current_element.attrib[attribute]), sigma), 0, 100))
                

        #weather_element.attrib["cloudstate"] = # TODO: Find out other possible values
        
        
def change(root, world_map, config):
    changeAbsoluteTargetSpeed(root, config)
    changePerformances(root, config)
    changeSpawnpoints(root, world_map, config)
    changeDatetime(root, config)
    changeRoutes(root, world_map, config)
    changeWeather(root, config)
