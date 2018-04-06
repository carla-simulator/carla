
Agent Benchmark Structure
-------------------



![Benchmark_structure](img/benchmark_diagram.png)
>Figure: The general structure of the benchmark. 



The *benchmark* is the module responsible for evaluating a certain
*agent* in a *experiment suite*. 
  
The *experiment suite* is composed by set of *experiments*.
Each *experiment* is related to *task* that contains a set of poses.
These *poses* are  are tuples containing a start and end point of a navigation
task.
The *experiments* also are associated with a *condition* which is
a [carla settings](carla_settings.md) object. The conditions specify
simulation parameters such as: weather, sensor suite, number of
vehicles and pedestrians, etc.


The user must define its own experiment suite to be tested
on an agent. The user also should define the agent class. The agent is the active
part which will be evaluated by the agent benchmark module.


To know how see: creating your set of experiments tutorial.


CORL 2017 
----------------------

We already provide the CORL 2017 experiment suite used to benchmark the
agents for the CORL 2017 paper.


