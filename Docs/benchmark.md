CARLA Benchmark
===============

Running the Benchmark
---------------------

The "carla" api provides a basic benchmarking system, that allows making several
tests on a certain agent. We already provide the same benchmark used in the CoRL
2017 paper. By running this benchmark you can compare the results of your agent
to the results obtained by the agents show in the paper.


Besides the requirements of the CARLA client, the benchmark package also needs
the future package

    $ sudo pip install future

By running the benchmark a default agent that just go straight will be tested.
To run the benchmark you need a server running. For a default localhost server
on port 2000, to run the benchmark you just need to run

    $ ./run_benchmark.py

or

    $ python run_benchmark.py

Run the help command to see options available

    $ ./run_benchmark.py --help

Benchmarking your Agent
---------------------
The benchmark works by calling three lines of code

    corl = CoRL2017(city_name=args.city_name, name_to_save=args.log_name)
    agent = Manual(args.city_name)
    results = corl.benchmark_agent(agent, client)

This is excerpt is executed in the [run_benchmark.py](https://github.com/carla-simulator/carla/blob/master/PythonClient/run_benchmark.py) example.

First a *benchmark* object is defined, for this case, a CoRL2017 benchmark. This is object is used to benchmark a certain Agent. <br>
On the second line of our sample code, there is an object of a Manual class instanced. This class inherited an Agent base class
that is used by the *benchmark* object.
To be benchmarked, an Agent subclass must redefine the *run_step* function as it is done in the following excerpt:

    def run_step(self, measurements, sensor_data, target):
        """
        Function to run a control step in the CARLA vehicle.
		:param measurements: object of the Measurements type
		:param sensor_data: images list object
		:param target: target position of Transform type
	    :return: an object of the control type.
	    """
        control = VehicleControl()
        control.throttle = 0.9
        return control
The function receives measurements from the world, sensor data and a target position. With this, the function must return a control to the car, *i.e.* steering value, throttle value, brake value, etc.

The [measurements](measurements.md), [target](measurements.md), [sensor_data](cameras_and_sensors.md) and [control](measurements.md) types are described on the documentation.

Benchmark Structure
-------------------

The benchmark  structure in set of *Experiments*.
Each *experiment* contains a set of poses, that are tuples containing
a start and an end point.
The *experiments* also are associated with a condition which is
a [carla settings](carla_settings.md) object.




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

*Frames Skip: It is related to the number of frames you have
to skip after a collision or a intersection starts.
