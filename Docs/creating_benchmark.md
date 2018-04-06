



Benchmarking your Agent
---------------------
The benchmark works by calling three lines of code

    corl = CoRL2017(city_name=args.city_name, name_to_save=args.log_name)
    agent = Manual(args.city_name)
    results = corl.benchmark_agent(agent, client)

This excerpt is executed at the [run_benchmark.py](https://github.com/carla-simulator/carla/blob/master/PythonClient/run_benchmark.py) example.

First a *benchmark* object is defined, for this case, a CoRL2017 benchmark. This is object is used to benchmark a certain Agent. <br>
On the second line of our sample code, there is an object of a Manual class instanced. This class inherited an Agent base class
that is used by the *benchmark* object.
To be benchmarked, an Agent subclass must redefine the *run_step* function as it is done in the following excerpt:

    def run_step(self, measurements, sensor_data, directions, target):
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
The function receives measurements from the world, sensor data and a target position. With this,
 the function must return a control to the car, *i.e.* steering value, throttle value, brake value, etc.

The [measurements](measurements.md), [target](measurements.md), [sensor_data](cameras_and_sensors.md) 
and [control](measurements.md) types are described on the documentation.
