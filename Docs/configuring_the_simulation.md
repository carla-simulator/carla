<h1>Configuring the simulation</h1>

Before you start running your own experiments there are few details to take into
account at the time of configuring your simulation. In this document we cover
the most important ones.

For the full list of settings please see [CARLA Settings](carla_settings.md).

Fixed time-step
---------------

The time-step is the _simulation-time_ elapsed between two steps of the
simulation. In video-games, this _simulation-time_ is almost always adjusted to
real time for better realism. This is achieved by having a **variable
time-step** that adjusts the simulation to keep up with real-time. In
simulations however, it is better to detach the _simulation-time_ from real-
time, and let the simulation run as fast as possible using a **fixed time-
step**. Doing so, we are not only able to simulate longer periods in less time,
but also gain repeatability by reducing the float-point arithmetic errors that a
variable time-step introduces.

CARLA can be run in both modes.

<h4>Variable time-step</h4>

The simulation tries to keep up with real-time. To do so, the time-step is
slightly adjusted each update. Simulations are not repeatable. By default, the
simulator starts in this mode

<h4>Fixed time-step</h4>

The simulation runs as fast as possible, simulating the same time increment on
each step. To run the simulator this way you need to pass two parameters in the
command-line, one to enable the fixed time-step mode, and the second to specify
the FPS of the simulation (i.e. the inverse of the time step). For instance, to
run the simulation at a fixed time-step of 0.2 seconds we execute

    $ ./CarlaUE4.sh -benchmark -fps=5

It is important to note that this mode can only be enabled when launching the
simulator since this is actually a feature of Unreal Engine.

<!-- Disabled for now...

Synchronous vs Asynchronous mode
--------------------------------

The client-simulator communication can be synchronized by using the _synchronous
mode_. The synchronous mode enables two things

  * The simulator waits for the sensor data to be ready before sending the
    measurements.
  * The simulator halts each frame until a control message is received.

This is very useful when dealing with slow client applications, as the
simulation is halted until the client is ready to continue. This also ensures
that the generated data of every sensor is received every frame by the client.
As opposed to _asynchronous mode_, in which the sensor data may arrive a couple
of frames later or even be lost if the client is not fast enough.

However, there are a couple of caveats to bear in mind when using the
synchronous mode. First of all, **it is very important to run the simulator at
fixed time-step when using the synchronous mode**. Otherwise the physics engine
will try to recompute at once all the time spent waiting for the client, this
usually results in inconsistent or not very realistic physics.

Secondly, the synchronous mode imposes a significant performance penalty. There
is a price in waiting for the render thread to have the images ready and halting
the simulation when the client is slow. There is a trade-off in using the
synchronous mode.

The synchronous mode can be enabled at the beginning of each episode both in the
INI file or the Python API

**Python**

```py
settings = CarlaSettings()
settings.set(SynchronousMode=True)
```

**CarlaSettings.ini**

```ini
[CARLA/Server]
SynchronousMode=true
```
 -->
