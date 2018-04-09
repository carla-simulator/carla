
Driving Benchmark Performance Metrics
------------------------------



The benchmark module provides the following performance metrics, which 
are related to infraction:


* Off Road Intersection: The number of times the agent goes out of the road. 
 The intersection is only counted if the area of the vehicle outside
  of the road is bigger than a *threshold*.
  
* Other Lane Intersection: The number of times the agent goes to the other
 lane. The intersection is only counted if the area of the vehicle on the
 other lane is bigger than a *threshold*.
   
* Vehicle Collisions: The number of collisions with vehicles that have
  an impact bigger than a *threshold*.

* Pedestrian Collisions: The number of collisions with pedestrians
 that have an impact bigger than a *threshold*.

* General Collisions: The number of collisions with all other
objects with an impact bigger than a *threshold*.


These results can be computed with the metrics module, by using the following
code excerpt:

    metrics_object = Metrics(metrics_parameters)
    summary_dictionary = metrics_object.compute(path_to_execution_log)

The class is instanced with a dictionary of parameters. 
These parameters could be changed by changing 
The function receives the full path to the execution log and a dictionary with
parameters. It returns a dictionary with the metrics.

The parameters are:
* Threshold: The threshold used by the metrics.

* Frames  Recount: After making the infraction, set the number
of frames that the agent needs to keep doing the infraction for
it to be counted as another infraction. 

*Frames Skip: It is related to the number of frames that are
skipped after a collision or a intersection starts.



On your experiment suite class definition you could also
redefine the metrics experiment.


####Benchmark Execution


During the execution the benchmark module stores
the [measurements](measurements.md)  and
 [controls](measurements.md) for every single step.
 These results are stored on the *_benchmarks_results*
 folder.
