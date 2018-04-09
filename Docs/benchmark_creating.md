Benchmarking your Agent
---------------------------

![Benchmark_structure](img/benchmark_diagram_small.png )

The agent benchmark is associated with other two modules.
The *agent* module, a controller which performs in a
another module, the *experiment suite*.
Both modules are abstract classes that must be redefined by
the user.

The following code excerpt is
an example on how to apply a agent benchmark

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


To create a Experiment Suite class you need to perform
the following steps:

* Create your custom class by inheriting the ExperimentSuite base class.
* Define the test and train weather conditions to be used.
* Build the *Experiment* objects 



#####Definition


The defined set of experiments must derive the *ExperimentSuite* class
as in the following code. 

    from carla.agent_benchmark.experiment import Experiment
    from carla.sensor import Camera
    from carla.settings import CarlaSettings
    
    from .experiment_suite import ExperimentSuite
    
    
    class Basic(ExperimentSuite):
    
#####Define the used weathers

The user must select the weathers to be used. One should select the set
of test weathers and the set of train weathers. This is defined as a
class property as in the following example.

    @property
    def train_weathers(self):
        return [1]
    @property
    def test_weathers(self):
        return [1]
        

##### Building Experiments

The [experiments are composed by a *task* that is defined by a set of *poses*](benchmark_structure.md).
Lets start by selecting poses for one of the cities, Town01.
 First of all, we need to see all the possible positions, for that, with
  a CARLA simulator running in a terminal, run:
    
    python view_start_positions.py
 
 ![town01_positions](img/welcome.png)
 
  
  
Now lets choose, for instance, 105 as start position and 29
as end. This two positions can be visualized by running. 
    
    python view_start_positions.py --pos 105,29 --no-labels



 Lets define
two more poses, one for going straight, other one for one simple turn.
Also, lets also choose three poses for Town02.
Figure 3, shows these defined poses for both carla towns.


 ![town01_positions](img/initial_positions.png)
 >Figure 3: The poses used on this basic *Experimental Suite* example. Poses are
 a tuple of start and end position of a goal-directed episode. Start positions are
 shown in Blue, end positions in Red. Left: Straight poses,
 where the goal is just straight away from the start position. Middle: One turn
 episode, where the goal is one turn away from the start point. Arbitrary position,
 the goal is far away from the start position, usually more than one turn.


We define each of this defined poses as tasks. Plus, we also set
the number of dynamic objects for each of these tasks and repeat
the arbitrary position task to have it also defined with dynamic
objects. This is defined
in the following code excerpt:
    
    poses_tasks = [[[36, 40]], [[138, 17]], [[105, 29]], [[105, 29]]]
    vehicles_tasks = [0, 0, 0, 20]
    pedestrians_tasks = [0, 0, 0, 50]

Keep in mind that a task is a set of episodes with start and end points.

Finally by using the defined tasks we can build the experiments
vector as we show in the following code excerpt:

     experiments_vector = []
        for weather in self.weathers:

            for iteration in range(len(poses_tasks)):
                poses = poses_tasks[iteration]
                vehicles = vehicles_tasks[iteration]
                pedestrians = pedestrians_tasks[iteration]

                conditions = CarlaSettings()
                conditions.set(
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=vehicles,
                    NumberOfPedestrians=pedestrians,
                    WeatherId=weather

                )
                # Add all the cameras that were set for this experiments
                conditions.add_sensor(camera)
                experiment = Experiment()
                experiment.set(
                    Conditions=conditions,
                    Poses=poses,
                    Task=iteration,
                    Repetitions=1
                )
                experiments_vector.append(experiment)




            
The full code could be found at basic.py



#### Executing and expected results

