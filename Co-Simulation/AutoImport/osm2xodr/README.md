# osm2xodr
converter for OpenStreetMaps (.osm) to OpenDrive (.xodr) format (just the drivable roads right now - you can edit the code for other osm-ways - its in the function "parseAll()").

just adjust the filenames/paths in the main.py and run it.

The requirements are osmread, numpy, PIL and pyproj

If a topographymap is used it should be in 16bit integer format and the max/min latitudes/longitudes of the map should match the points in the osm-file.





This code was created as a part of
<p align="center"><img src="https://github.com/JHMeusener/osm2xodr/blob/master/Projekt%20und%20F%C3%B6rderlogos%20EN_28.11.2019.jpg" /></p>
