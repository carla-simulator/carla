Weather
-------
The new weather system is simplifyed to work using less variables. Any combination of the 6 current variables should give you a valid climatic configuration.


This is the actor in charge of modifying all the lighting, environmental actors and anything that affects the impression of the climate. It runs automatically with the client and is then modified as needed in game. It has also Its own actor to launch in editor mode. 


!!! Important
    To fully work It will need One of each of the following actors:
     SkySphere, Skylight, Light Source and atmosferic fog to exist in the world.



  
  - Cloudyness: Level of clouds in the sky. 0 is clear and 100 is full overcast.
  - Precipitation: Level of precipitation 0 is none and 100 is the maximum set in the perticle emitter (The maximum is set practically by performance but can be tweaked up).
  - PrecipitationDeposits: The deposits of the current precipitation in the floor.
  - WindIntensity: The intensity of the Wind effect on the trees and water from 0 (Not windy at all) to 100 Very windy.
  - Sun Azimuth: Sets the sun position over the horizon.
  - SunAltitude: Sets the position of the sun in the sky using angles. 
  - Hour: Only used when "HourDependant" is checked(Not currently supported by the API. Sets the sun position in the sky based on the time of day (Works as if the player was positioned in the ecuator).

Most of this variables are entangled between them and have an effect between each other. Both sun altitude and cloudyness determine color and direction of the level lights, Cloudyness and precipitation affect the atmosferic fog effect and such... 

You can modify this parameters from the client Using the following Function:

```python   
weather = carla.WeatherParameters()

weather.cloudyness = 25.0
weather.precipitation = 00.0
weather.precipitation_deposits = 50.0
weather.wind_intensity = 0.0
weather.sun_azimuth_angle = 50.0
weather.sun_altitude_angle = 35.0

world.set_weather(weather)
```
