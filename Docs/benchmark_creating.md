Benchmarking your Agent
---------------------------

![Benchmark_structure](img/benchmark.pn)

The agent benchmark is associated with other two modules.
The *agent* module, that is a controller which performs in a
second module, the *experiment suite*.
Both modules are abstract classes that must be redefined by
the user.

In the following code excerpt is
a example on how to apply a agent benchmark

    agent = Forward() 
    experiment_suite = Basic()
    benchmark = AgentBenchmark() 
    benchmark_summary = benchmark.benchmark_agent(experiment_suite, agent, client)



In this tutorial we are going to show how to create 
a basic experiment suite and a trivial forward going agent.



####Defining the Agent

The tested agent must  inherit the base *Agent* class.
Lets start by deriving a simple Forward agent. 

    from carla.agent.agent import Agent
    from carla.client import VehicleControl
    
    class Forward(Agent):


To have its performance evaluated, the Forward derived class _must_ redefine the *run_step* 
function as it is done in the following excerpt:

    def run_step(self, measurements, sensor_data, directions, target):
        """
        Function to run a control step in the CARLA vehicle.
	    """
        control = VehicleControl()
        control.throttle = 0.9
        return control


This function receives the following parameters:
 
 * [Measurements](measurements.md): the entire state of the world received
 by the client from the CARLA Simulator. These measurements contains agent position, orientation,
 dynamic objects information, etc.
 * Sensor Data: The measured data from defined sensors, such as Lidars or RGB cameras.
 * Directions: Information from the high level planner. Currently the planner sends
 a high level command from the set: STRAIGHT, RIGHT, LEFT, NOTHING.
 * Target Position: The position and orientation of the target.
 
 With all this information, the *run_step* function is expected 
 to return a control to the car containing, 
 steering value, throttle value, brake value, etc.



####Defining the Experiment Suite

The defined set of experiments must derive the *ExperimentSuite* class
as in the following code. 

    from carla.agent_benchmark.experiment import Experiment
    from carla.sensor import Camera
    from carla.settings import CarlaSettings
    
    from .experiment_suite import ExperimentSuite
    
    
    class Basic(ExperimentSuite):
    
Then, it must select the weathers to be used. It should select the set
of test weathers and the set of train weathers. This is defined as a
class property.

    @property
    def train_weathers(self):
        return [1]
    @property
    def test_weathers(self):
        return [1]
        

The second and final step is to build the set of experiments. For selecting
the positions one could use the following procedure. Lets run the
view_posible position script and select for instance, position 
34,35. Figure shows where these positions would be located.



Now lets select more positions to fill the benchmark, both for city one 
and for city two. 
Lets put each of this positions as a task. (Structure should be before, definetly)


        if self._city_name == 'Town01':
            poses_tasks = [[[36, 40]], [[138, 17]], [[105, 29]], [[105, 29]]]
            vehicles_tasks = [0, 0, 0, 20]
            pedestrians_tasks = [0, 0, 0, 50]
        else:
            poses_tasks = [[[38, 34]], [[37, 76]], [[19, 66]], [[19, 66]]]
            vehicles_tasks = [0, 0, 0, 15]
            pedestrians_tasks = [0, 0, 0, 50]
            
            
The full code could be found at basic.py



#### Executing and expected results

