CARLA Simulator
===============

Thanks for downloading CARLA!

<https://carla.org/>

How to run CARLA
----------------

Launch a terminal in this folder and start the simulator:

```sh
./CarlaUnreal.sh
```

This opens a window with a view over the city. That is the "spectator" view: you can fly around with the mouse and the WASD keys, but you cannot interact with the world from it. The simulator is now running as a server, waiting for a client to connect.

Install the Python client that matches this package:

```sh
pip3 install PythonAPI/carla/dist/carla-*.whl
```

Let's add some life to the city. Open a new terminal in this folder and run:

```sh
python3 PythonAPI/examples/generate_traffic.py -n 80
```

That spawns 80 vehicles driving in autopilot mode. They keep driving until the script is stopped, so leave it running.

It is nice and sunny in CARLA, which is not a very interesting driving condition. The weather and the time of day can be changed from a client:

```sh
python3 PythonAPI/examples/dynamic_day_night_weather_cycle.py
```

The city is now ready to drive in:

```sh
python3 PythonAPI/examples/manual_control.py
```

This opens a window with a third-person view of a car that you can drive with the WASD or arrow keys. Press `h` to see every option available.

More examples are in `PythonAPI/examples/`, and `PythonAPI/python_api.md` is the API reference for this release.

Running options
---------------

```sh
./CarlaUnreal.sh -RenderOffScreen          # no display window
./CarlaUnreal.sh -carla-rpc-port=3000      # listen on another port
./CarlaUnreal.sh -quality-level=Low        # cheaper rendering
./CarlaUnreal.sh -ros2                     # native ROS 2 interface
```

For more details and running options, refer to the online documentation:

<https://carla-ue5.readthedocs.io>

JPEG-enabled builds use libjpeg-turbo. This software is based in part on the
work of the Independent JPEG Group.
