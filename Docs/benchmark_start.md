Agent Benchmark
===============

One of the main objectives of CARLA is to serve as a tool
to compare different driving controllers in controlled conditions.
CARLA is whiling to evolve into an standard benchmark for autonomous.
This module serves exactly in this direction, a way to use CARLA
to evaluate certain driving controller (agent) and obtain a some
metrics about its performance. 

Getting Started
---------------------

The "carla" api provides a basic benchmarking system, that allows making several
tests on a certain agent. We already provide the same benchmark used in the CoRL
2017 paper. By running this benchmark you can compare the results of your agent
to the results obtained by the agents shown in the paper.


By running the benchmark a default agent that just go straight will be tested.
To run the benchmark you need a server running. For a default localhost server
on port 2000, to run the benchmark you just need to run

    $ ./run_benchmark.py

or

    $ python run_benchmark.py

Run the help command to see options available.

    $ ./run_benchmark.py --help


Now we will show 




####Benchmark Execution


During the execution the benchmark module stores
the [measurements](measurements.md)  and
 [controls](measurements.md) for every single step.
 These results are stored on the *_benchmarks_results*
 folder.



####Benchmark Metrics

The benchmark module provides the following metrics, which 
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
 that have an impact bigger than a threshold.

* General Collisions: The number of collisions with all other
objects.


These results can be computed with the metrics module, by using the following
function

`summary_dictionary = metrics.compute_summary(path_to_execution_log,parameters)`

The function receives the full path to the execution log and a dictionary with
parameters. It returns a dictionary with the metrics.

The parameters are:
* Threshold: The threshold used by the metrics.

* Frames  Recount: After making the infraction, set the number
of frames that the agent needs to keep doing the infraction for
it to be counted as another infraction. 

*Frames Skip: It is related to the number of frames that are
skipped after a collision or a intersection starts.
