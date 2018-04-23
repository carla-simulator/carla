
Driving Benchmark Structure
-------------------

The figure below  shows the general structure of the driving
benchmark module. 



![Benchmark_structure](img/benchmark_diagram.png)
>Figure: The general structure of the agent benchmark module.


The *driving benchmark* is the module responsible for evaluating a certain
*agent* in an *experiment suite*.
  
The *experiment suite* is an abstract module.
Thus, the user must define its own derivation
of *experiment suite*. We already provide the CoRL2017 suite and a simple
*experiment suite*  for testing.

The *experiment suite* is composed by set of *experiments*.
Each *experiment* contains a *task* that consists of a set of navigation
episodes, represented by a set of *poses*.
These *poses* are  tuples containing the start and end points of an
episode.

The *experiments* are also associated with a *condition*. A
 condition is represented by a [carla settings](carla_settings.md) object.
 The conditions specify simulation parameters such as: weather, sensor suite, number of
vehicles and pedestrians, etc.


The user also should derivate an *agent* class. The *agent* is the active
part which will be evaluated on the driving benchmark.

The driving benchmark also contains two auxiliary modules.
The *recording module* is used to keep track of all measurements and
can be used to pause and continue a driving benchmark.
The  [*metrics module*](benchmark_metrics.md) is used to compute the performance metrics
by using the recorded measurements.




Example: CORL 2017 
----------------------

We already provide the CoRL 2017 experiment suite used to benchmark the
agents for the [CoRL 2017 paper](http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf).

The CoRL 2017 experiment suite has the following composition:

* A total of 24 experiments for each CARLA town containing:
    * A task for going straight.
    * A task for making a single turn.
    * A task for going to an arbitrary position.
    * A task for going to an arbitrary position with dynamic objects.
* Each task is composed of 25 poses that are repeated in 6 different weathers (Clear Noon, Heavy Rain Noon, Clear Sunset, After Rain Noon, Cloudy After Rain and Soft Rain Sunset).
* The entire experiment set has 600 episodes.
* The CoRL 2017 can take up to 24 hours to execute for Town01 and up to 15
hours for Town02 depending on the agent performance.
  
 


