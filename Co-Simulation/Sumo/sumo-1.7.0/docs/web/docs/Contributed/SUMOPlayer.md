---
title: Contributed/SUMOPlayer
permalink: /Contributed/SUMOPlayer/
---

!!! note
    SUMOPlayer was removed in release 0.24.0. You should be able to use [traceExporter.py](../Tools/TraceExporter.md) for the same task.

## Meta-Information

- **Location**: The build-file and the sources can be found in <SUMO_DIST\>/tools/contributed/sumoplayer
- **Language**: Java \>=1.5
- **Author**: Michael Willigens

## From README

    # This is file gives a brief overview on SUMPlayer usage.
    # SUMOPlayer can be used to play SUMO network-dump files
    # in realtime (if possible) to latitude/longitude coordinates.
    # It can be customized in any way by implemting
    # de.psi.telco.sumoplayer.SUMOLocationListener
    # SEE:
    # public interface SUMOLocationListener {
    #   public void LocationUpdated(String vehicleId, int timestep, double lon, double lat, double speed);
    # }
    # You can call SUMOPlayer with your own implementation by using the
    # -l "<classname>" parameter.
    # If not used, SUMOPlayer uses STDOUT CVS style like:
    # <vehicle_id>|<timestep>|<lon>|<lat>|<speed>

    # Eclipse integration is easily possible by:
    # new-project -> from existing ant buildfile (build.xml)

    # see possible ant tasks:
    $> ant -projecthelp

    # compile
    $> ant sumoplayer.jar

    # run sumoPlayer with:
    $> ant sumplayer.run
    # ... or even better integrated in your project by jarfile:
    $> java -jar sumoplayer.jar
    # possible parameters:
    # ANT param                         # description
    # JAR param

     -Dnetfile=<sumo_network_xml_file>  # the used sumo network file
     -n                                 # needed for geo coordinate lookups

     -Dndump=<sumo_ndump_xml_file>      # sumo network-dump simulation file
     -d

     -Dequipped=<propability>           # a floating value between 0 and 1
     -e                                 # defining the number of tracked
                                        # vehicles. 0.01 is 1%

     -Djitter=<value_in_meters>         # a random GPS jitter in meters
     -j

     -Dlistener=<full_java_classname>   # the used java implementation of
     -l                                 # SUMOLocationListener
                                        # default:
                                        # de.psi.telco.sumoplayer.SUMOPipedLocationListener
                                        #    (stdout CSV style data)



    # JAVA DOC
    $> ant javadoc                      # creates JAVA DOCS in /build/javadoc

## Additional Information

You can run the player as following:

```
sumo ... --no-step-log --netstate-dump - | java -jar SUMOPlayer.jar -d - -c <propability> -n <used_network_file> ...
```

("-" redirects the dump to stdout)

The basic functionality is as following

- the player reads the network file for projection information
- the player reads projected lanes
- the player tries to put each output into one second
- the player chooses vehicles randomly
- the player writes the longitude and latitude of every vehicle into a
  file or prints them