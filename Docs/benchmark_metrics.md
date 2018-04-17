
Driving Benchmark Performance Metrics
------------------------------

This page explains the performance metrics module.
This module is used to compute a summary of results based on the actions
 performed by the agent during the benchmark.


### Provided performance metrics

The driving benchmark performance metrics module provides the following performance metrics:

* **Percentage of Success**: The percentage of episodes (poses from tasks),
that the agent successfully completed.

* **Average Completion**: The average distance towards the goal that the
agent was able to travel.

* **Off Road Intersection**: The number of times the agent goes out of the road.
 The intersection is only counted if the area of the vehicle outside
  of the road is bigger than a *threshold*.
  
* **Other Lane Intersection**: The number of times the agent goes to the other
 lane. The intersection is only counted if the area of the vehicle on the
 other lane is bigger than a *threshold*.
   
* **Vehicle Collisions**: The number of collisions with vehicles that had
  an impact bigger than a *threshold*.

* **Pedestrian Collisions**: The number of collisions with pedestrians
 that had an impact bigger than a *threshold*.

* **General Collisions**: The number of collisions with all other
objects with an impact bigger than a *threshold*.


### Executing and Setting Parameters

The metrics are computed as the final step of the benchmark
and stores a summary of the results a json file.
Internally it is executed as follows:

```python
metrics_object = Metrics(metrics_parameters)
summary_dictionary = metrics_object.compute(path_to_execution_log)
```

The Metric's compute function
receives the full path to the execution log.
The Metric class  should be instanced with some parameters.
The parameters are:

* **Threshold**: The threshold used by the metrics.
* **Frames  Recount**: After making the infraction, set the number
of frames that the agent needs to keep doing the infraction for
it to be counted as another infraction. 
* **Frames Skip**: It is related to the number of frames that are
skipped after a collision or a intersection starts.

These parameters are defined as property of the *Experiment Suite*
base class and can be redefined at your 
[custom *Experiment Suite*](benchmark_creating/#defining-the-experiment-suite).

The default parameters are:


    @property
        def metrics_parameters(self):
        """
        Property to return the parameters for the metrics module
        Could be redefined depending on the needs of the user.
        """
        return {

            'intersection_offroad': {'frames_skip': 10,
                                     'frames_recount': 20,
                                     'threshold': 0.3
                                     },
            'intersection_otherlane': {'frames_skip': 10,
                                       'frames_recount': 20,
                                       'threshold': 0.4
                                       },
            'collision_other': {'frames_skip': 10,
                                'frames_recount': 20,
                                'threshold': 400
                                },
            'collision_vehicles': {'frames_skip': 10,
                                   'frames_recount': 30,
                                   'threshold': 400
                                   },
            'collision_pedestrians': {'frames_skip': 5,
                                      'frames_recount': 100,
                                      'threshold': 300
                                      },

              }
