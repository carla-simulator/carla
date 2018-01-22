CARLA Benchmark
===============

!!! important
    Benchmark code is currently in beta and can be found only in the
    `benchmark_branch`.

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

The [measurements](measurements.md), [target](measurements#player-measurements.md), [sensor_data](cameras_and_sensors.md) and [control](measurements.md) types are described at the documentation.



Creating your Benchmark
---------------------
Tutorial to be added
