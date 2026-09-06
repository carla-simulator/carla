CARLA Simulator
===============

Thanks for downloading CARLA!

<http://carla.org/>

How to run CARLA
----------------

Launch a terminal in this folder and execute the simulator by running

```sh
 ./CarlaUnreal.sh
```

this will launch a window with a view over the city. This is the "spectator"
view, you can fly around the city using the mouse and WASD keys, but you cannot
interact with the world in this view. The simulator is now running as a server,
waiting for a client app to connect and interact with the world.

Let's start by adding some live to the city, open a new terminal window and
execute

```sh
./generate_traffic.py -n 80
```

This adds 80 vehicles to the world driving in "autopilot" mode. Back to the
simulator window we should see these vehicles driving around the city. They will
keep driving randomly until we stop the script. Let's leave them there for now.

Now, it's nice and sunny in CARLA, but that's not a very interesting driving
condition. One of the cool features of CARLA is that you can control the weather
and lighting conditions of the world. We'll launch now a script that dynamically
controls the weather and time of the day, open yet another terminal window and
execute

```sh
./dynamic_weather.py
```

The city is now ready for us to drive, we can finally run

```sh
./manual_control.py
```

This should open a new window with a 3rd person view of a car, you can drive
this car with the WASD/arrow keys. Press 'h' to see all the options available.

For more details and running options please refer to our online documentation

<http://carla.readthedocs.io>
