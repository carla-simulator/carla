# Synchrony and time-step 

This section deals with two fundamental concepts in CARLA. Their configuration defines how does time go by in the simulation, and how does the server make the simulation move forward.  

*   [__Simulation time-step__](#simulation-time-step)  
	*   [Variable time-step](#variable-time-step)  
	*   [Fixed time-step](#fixed-time-step)  
	*   [Tips when recording the simulation](#tips-when-recording-the-simulation)  
	*   [Time-step limitations](#time-step-limitations)  
*   [__Client-server synchrony__](#client-server-synchrony)  
	*   [Setting synchronous mode](#setting-synchronous-mode)  
	*   [Using synchronous mode](#using-synchronous-mode)  
*   [__Possible configurations__](#possible-configurations)  

---
## Simulation time-step

There is a difference between real time, and simulation time. The simulated world has its own clock and time, conducted by the server. Computing two simulation steps takes some real time. However, there is also the time span that went by between those two simulation moments, the time-step.  

To clarify, the server can take a few milliseconds to compute two steps of a simulation. However, the time-step between those two simulation moments can be configured to be, for instance, always a second.  

Time-step can be fixed or variable depending on user preferences.  

!!! Note
    Time-step and synchrony are intertwined concepts. Make sure to read both sections to get a full understanding of how does CARLA work.  

### Variable time-step

The default mode in CARLA. The simulation time that goes by between steps will be the time that the server takes to compute these.  

```py
settings = world.get_settings()
settings.fixed_delta_seconds = None # Set a variable time-step
world.apply_settings(settings)
```
`PythonAPI/util/config.py` sets the time-step using an argument. Zero equals variable time-step. 
```sh
cd PythonAPI/util && python3 config.py --delta-seconds 0
``` 

### Fixed time-step

The elapsed time remains constant between steps. If it is set to 0.5 seconds, there will be two frames per simulated second. Using the same time increment on each step is the best way to gather data from the simulation. Physics and sensor data will correspond to an easy to comprehend moment of the simulation. Also, if the server is fast enough, it makes possible to simulate longer time periods in less real time.  

Fixed delta seconds can be set in the world settings. To run the simulation at a fixed time-step of 0.05 seconds apply the following settings. In this case, the simulator will take twenty steps (1/0.05) to recreate one second of the simulated world.

```py
settings = world.get_settings()
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)
```
This can also be set using the provided script `PythonAPI/util/config.py`.

```sh
cd PythonAPI/util && python3 config.py --delta-seconds 0.05
``` 

### Tips when recording the simulation

CARLA has a [recorder feature](adv_recorder.md) that allows a simulation to be recorded and then reenacted. However, when looking for precision, some things need to be taken into account.  

* With a __fixed time-step__, reenacting it will be easy. The server can be set to the same time-step used in the original simulation.  

* With a __variable time-step__, things are a bit more complicated.  

	* If the __server runs with a variable time-step__, the time-steps will be different from the original one, as logic cycles differ from time to time. The information will then be interpolated using the recorded data.  

	* If the __server is forced to reproduce the exact same time-steps__, the steps simulated will be the same, but the real time between them changes. Time-steps should be passed one by one. Those original time-steps were the result of the original simulation running as fast as possible. As the time taken to represent these will mostly be different, the simulation is bound to be reproduced with weird time fluctuations.  

	* There is also a __float-point arithmetic error__ that variable time-step introduces. The simulation is running with a time-step equal to the real one. Real time is a continuous variable, represented in the simulation with a `float` value, which has decimal limitations. The time that is cropped for each step accumulates, and prevents the simulation from a precise repetition of what has happened.  

### Time-step limitations

Physics must be computed within very low time steps to be precise. The more time goes by, the more variables and chaos come to place, and the more defective the simulation will be. 
CARLA uses up to 6 substeps to compute physics in every step, each with a maximum delta time of 0.016667s.  

To know how many of these are needed, the time-step used gets divided by the maximum delta time a substep can use `number_of_substeps = time_step/0.016667`. Being these a maximum of 6, `6*0.016667 = 0.1`. If the time-step is greater than `0.1`, there will not be enough physical substeps. Physics will not be in synchrony with the delta time. 

!!! Warning
    __Do not use a time-step greater than 0.1s.__<br>
    As explained above, the physics will not be representative for the simulation. The original issue can be found in ref. [#695](https://github.com/carla-simulator/carla/issues/695)

---
## Client-server synchrony 

CARLA is built over a client-server architecture. The server runs the simulation. The client retrieves information, and demands for changes in the world. This section deals with communication between client and server.  

By default, CARLA runs in __asynchronous mode__. The server runs the simulation as fast as possible, without waiting for the client. On __synchronous mode__, the server waits for a client tick, a "ready to go" message, before updating to the following simulation step.  

!!! Note
    In a multiclient architecture, only one client should tick. The server reacts to every tick received as if it came from the same client. Many client ticks will make the create inconsistencies between server and clients. 

### Setting synchronous mode

Changing between synchronous and asynchronous mode is just a matter of a boolean state.  
```py
settings = world.get_settings()
settings.synchronous_mode = True # Enables synchronous mode
world.apply_settings(settings)
```
!!! Warning
    If synchronous mode is enabled, and there is a Traffic Manager running, this must be set to sync mode too. Read [this](adv_traffic_manager.md#synchronous-mode) to learn how to do it. 

To disable synchronous mode just set the variable to false or use the script `PythonAPI/util/config.py`. 
```sh
cd PythonAPI/util && python3 config.py --no-sync # Disables synchronous mode
``` 
Synchronous mode cannot be enabled using the script, only disabled. Enabling the synchronous mode makes the server wait for a client tick. Using this script, the user cannot send ticks when desired. 

### Using synchronous mode

The synchronous mode becomes specially relevant with slow client applications, and when synchrony between different elements, such as sensors, is needed. If the client is too slow and the server does not wait, there will be an overflow of information. The client will not be able to manage everything, and it will be lost or mixed. On a similar tune, with many sensors and asynchrony, it would be impossible to know if all the sensors are using data from the same moment in the simulation.  

The following fragment of code extends the previous one. The client creates a camera sensor, stores the image data of the current step in a queue, and ticks the server after retrieving it from the queue. A more complex example regarding several sensors can be found [here][syncmodelink].

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
    Data coming from GPU-based sensors, mostly cameras, is usually generated with a delay of a couple of frames. Synchrony is essential here. 


The world has asynchrony methods to make the client wait for a server tick, or do something when it is received. 

```py
# Wait for the next tick and retrieve the snapshot of the tick.
world_snapshot = world.wait_for_tick()

# Register a callback to get called every time we receive a new snapshot.
world.on_tick(lambda world_snapshot: do_something(world_snapshot))
```

---
## Possible configurations 

The configuration of time-step and synchrony, leads for different settings. Here is a brief summary on the possibilities.

<table class ="defTable">
<thead>
<th></th>
<th><b>Fixed time-step</b></th>
<th><b>Variable time-step</b></th>
</thead>
<tbody>
<td><b>Synchronous mode</b></td>
<td>Client is in total control over the simulation and its information.</td>
<td>Risk of non reliable simulations.</td>
<tr>
<td><b>Asynchronous mode</b></td>
<td>Good time references for information. Server runs as fast as possible.</td>
<td>Non easily repeatable simulations.</td>
</tbody>
</table>
<br>

* __Synchronous mode + variable time-step.__ This is almost for sure a non-desirable state. Physics cannot run properly when the time-step is bigger than 0.1s and. If the server has to wait for the client to compute the steps, this is likely to happen. Simulation time and physics will not be in synchrony. The simulation will not be reliable.  

* __Asynchronous mode + variable time-step.__ This is the default CARLA state. Client and server are asynchronous. The simulation time flows according to the real time. Reenacting the simulation needs to take into account float-arithmetic error, and possible differences in time steps between servers.  

* __Asynchronous mode + fixed time-step.__ The server will run as fast as possible. The information retrieved will be easily related with an exact moment in the simulation. This configuration makes possible to simulate long periods of time in much less real time, if the server is fast enough. 

* __Synchronous mode + fixed time-step.__ The client will rule the simulation. The time step will be fixed. The server will not compute the following step until the client sends a tick. This is the best mode when synchrony and precision is relevant. Especially when dealing with slow clients or different elements retrieving information. 

!!! Warning 
    __In synchronous mode, always use a fixed time-step__. If the server has to wait for the user, and it is using a variable time-step, time-steps will be too big. Physics will not be reliable. This issue is better explained in the __time-step limitations__ section. 


---

That is all there is to know about the roles of simulation time and client-server synchrony in CARLA.  

Open CARLA and mess around for a while. Any suggestions or doubts are welcome in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
