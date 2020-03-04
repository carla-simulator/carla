# Synchrony and time-step 

This section deals with two concepts that are fundamental to fully comprehend CARLA and gain control over it to achieve the desired results. There are different configurations that define how does time go by in the simulation and how does the server running said simulation work. The following sections will dive deep into these concepts:

  * [__Simulation time-step__](#simulation-time-step)  
	* Variable time-step
	* Fixed time-step  
	* Tips when recording the simulation  
	* Time-step limitations  
  * [__Client-server synchrony__](#client-server-synchrony)
	* Setting synchronous mode  
	* Using synchronous mode 
  * [__Possible configurations__](#possible-configurations)

---
## Simulation time-step

The very first and essential concept to understand in this section is the difference between real time and simulation time. The simulated world has its own clock and time, conducted by the server. Between two steps of the simulation, there is the time spent to compute said steps (real time) and a time span that went by in those two moments of the simulation (simulated time). This latest is the time-step.  
Just an example for the sake of comprehension: When simulating, the server can take a few miliseconds to compute two steps of a simulation, but the time-step, the time that went by in the simulated world, can be configured to be, for instance, always a second.  
The time-step can be fixed or variable depending on user preferences, and CARLA can run in both modes. 

!!! Note
    After reading this section it would be a great idea to go for the following one, __Client-server synchrony__, especially the part about synchrony and time-step. Both are related concepts and affect each other when using CARLA. 

#### Variable time-step

This is the default mode in CARLA. When the time-step is variable, the simulation time that goes by between steps will be the time that the server takes to compute these. 
In order to set the simulation to a variable time-step the code could look like this: 
```py
settings = world.get_settings()
settings.fixed_delta_seconds = None
world.apply_settings(settings)
```
The provided script  `PythonAPI/util/config.py` automatically sets time-step with a flag where variable time-step is zero. 
```sh
cd PythonAPI/util && ./config.py --delta-seconds 0
``` 

#### Fixed time-step

Going for a fixed time-step makes the server run a simulation where the elapsed time remains constant between steps. If it is set to 0.5 seconds, there will be two frames per simulated second. 
Using the same time increment on each step is the best way to gather data from the simulation, as physics and sensor data will correspond to an easy to comprehend moment of the simulation. Also, if the server is fast enough, it makes possible to simulate longer time periods in less real time.  
To enable this mode set a fixed delta seconds in the world settings. For instance, to run the simulation at a fixed time-step of 0.05 seconds apply the following settings:

```py
settings = world.get_settings()
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)
```
Thus, the simulator will take twenty steps (1/0.05) to recreate one second of the simulated world. This can also be set using the provided script `PythonAPI/util/config.py` that automatically does this with a flag. 
```sh
cd PythonAPI/util && ./config.py --delta-seconds 0.05
``` 

#### Tips when recording the simulation

CARLA has a [recorder feature](adv_recorder.md) that allows a simulation to be recorded and then reenacted. However, when looking for precision, some things need to be taken into account.  
If the simulation ran with a fixed time-step, reenacting it will be easy, as the server can be set to the same time-step used in the original simulation. However, if the simulation used a variable time-step, things are a bit more complicated.  
Firstly, if the server reenacting the simulation also runs with a variable time-step, the time-steps will be different from the original one, as logic cycles differ from time to time. The information will then be interpolated using the recorded data.  
Secondly, the server can be forced to reproduce the exact same time-steps passing them one by one. Must be mentioned though that as those time steps were the result of the original simulation running as fast as possible, as the time taken to represent this time-steps now will mostly be different, the simulation is bound to be reproduced with weird time fluctuations. The steps simulated are the same, but the real-time between them changes.  
Finally there is also the float-point arithmetic error that working with a variable time-step introduces. As the simulation is running with a time-step equal to the real one, being real time a continuous and simulation one a float variable, the time-steps show decimal limitations. The time that is cropped for each step is an error that accumulates and prevents the simulation from a precise repetition of what has happened. 


#### Time-step limitations

Physics must be computed within very low time steps to be precise. The more time goes by, the more variables and chaos come to place and so, the more defective the simulation will be. 
CARLA uses up to 6 substeps to compute physics in every step, each with a maximum delta time of 0.016667s.  
To know how many of these are needed, the time-step used gets divided by the maximum delta time a substep can use `number_of_substeps = time_step/0.016667`.  
Being these a maximum of 6, `6*0.016667 = 0.1`. If the time-step is greater there will not be enough physical substeps and thus, the physics will not be in synchrony with the delta time. 

!!! Warning
    __Do not use a time-step greater than 0.1s.__<br>
    As explained above, the physics will not be representative for the simulation. The original issue can be found here: Ref. [#695](https://github.com/carla-simulator/carla/issues/695)

---
## Client-server synchrony 

CARLA is built over a client-server architecture. This has been previously stated: the server runs the simulation and the client retrieves information and demands for changes in the world. But how do these two elements communicate?  
By default, CARLA runs in __asynchronous mode__, meaning that the server runs the simulation as fast as possible, without waiting for the client. On the contrary, running on __synchronous mode__ will make the server wait for a client tick, a "ready to go" message, before updating to the following simulation step.  

!!! Note
    In a multiclient architecture, only one client should make the tick. The server would react to receiving many as if these were all coming from one client and thus, take one step per tick.

#### Setting synchronous mode

Changing between synchronous and asynchronous mode is just a matter of a boolean state. In the following example, there is the code to make the simulation run on synchronous mode: 
```py
settings = world.get_settings()
settings.synchronous_mode = True
world.apply_settings(settings)
```

To disable synchronous mode just set the variable to false or use the provided script in `PythonAPI/util/config.py`. 
```sh
cd PythonAPI/util && ./config.py --no-sync
``` 
Must be mentioned that synchronous mode cannot be enabled using the script, only disabled. Enabling the synchronous mode makes the server wait for a client tick, and using this script the user cannot send ticks when desired. 

#### Using synchronous mode

The synchronous mode becomes specially relevant when running with slow clients applications and when synchrony between different elements, such as sensors, is needed.  If the client is too slow and the server does not wait for it, the amount of information received will be impossible to manage and it can easily be mixed. On a similar tune, if there are ten sensors waiting to retrieve data and the server is sending all these information without waiting for all of them to have the previous one, it would be impossible to know if all the sensors are using data from the same moment in the simulation.  
As a little extension to the previous code, in the following fragment, the client creates a camera sensor that puts the image data received in the current step in a queue and sends ticks to the server only after retrieving it from the queue. A more complex example regarding several sensors can be found [here][syncmodelink].

```py
settings = world.get_settings()
settings.synchronous_mode = True
world.apply_settings(settings)

camera = world.spawn_actor(blueprint, transform)
image_queue = queue.Queue()
camera.listen(image_queue.put)

while True:
    world.tick()
    image = image_queue.get()
```
[syncmodelink]: https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/synchronous_mode.py


!!! Important
    Data coming from GPU-based sensors (cameras) is usually generated with a delay of a couple of frames when compared with CPU based sensors, so synchrony is essential here. 


The world also has asynchrony methods to make the client wait for a server tick or do something when it is received: 

```py
# Wait for the next tick and retrieve the snapshot of the tick.
world_snapshot = world.wait_for_tick()

# Register a callback to get called every time we receive a new snapshot.
world.on_tick(lambda world_snapshot: do_something(world_snapshot))
```

---
## Possible configurations 

The configuration of both concepts explained in this page, simulation time-step and client-server synchrony, leads for different types of simulation and results. Here is a brief summary on the possibilities and a better explanation of the reasoning behind it: 

|  | __Fixed time-step__ | __Variable time-step__ |
| --- | --- | --- |
| __Synchronous mode__ | Client is in total control over the simulation and its information. | Risk of non reliable simulations. |
| __Asynchronous mode__ | Good time references for information. Server runs as fast as possible. | Non easily repeatable simulations. |  

<br>

* __Synchronous mode + variable time-step:__ This is almost for sure a non-desirable state. Physics cannot run properly when the time-step is bigger than 0.1s and, if the server needs to wait for the client to compute the steps, this is likely to happen. Simulation time and physics then will not be in synchrony and thus, the simulation is not reliable.  

* __Aynchronous mode + variable time-step:__ This is the default CARLA state. Client and server are asynchronous but the simulation time flows according to the real time. Reenacting the simulation needs to take into account float-arithmetic error and possible differences in time steps between servers.  

* __Asynchronous mode + fixed time-step:__ The server will run as fast as possible, and yet, the information retrieved will be easily related with an exact moment in the simulation. This configuration makes possible to simulate long periods of time in much less real time if the server is fast enough. 

* __Synchronous mode + fixed time-step:__ In this mode, the client will have complete ruling over the simulation. The time step will be fixed and the server will not compute the following step until the client sends a tick saying so. This is the best mode when synchrony and precision is relevant, especially when dealing with slow clients or different elements retrieving information. 

!!! Warning 
    __In synchronous mode, always use a fixed time-step__. If the server has to wait for the user to compute the following step, and it is using a variable time-step, the simulation world will use time-steps too big for the physics to be reliable. This issue is better explained in the __time-step limitations__ section. 


---

That is all there is to know about the roles of simulation time and client-server synchrony in CARLA.  
Open CARLA and mess around for a while to make sure that everything is clear and yet, if there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>