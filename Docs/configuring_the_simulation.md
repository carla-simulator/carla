<h1>Configuring the simulation</h1>

Before you start running your own experiments there are few details to take into
account at the time of configuring your simulation. In this document we cover
the most important ones.

For the full list of settings please see [CARLA Settings](carla_settings.md).

Fixed time-step
---------------

The time-step is the _simulation-time_ elapsed between two steps of the
simulation. In video-games, this _simulation-time_ is almost always adjusted to
real time for better realism. This is achieved having a **variable time-step**
that adjusts the simulation to keep up with real-time. In simulations however,
it is better to detach the _simulation-time_ from real-time, and let the
simulation run as fast as possible using a **fixed time-step**. Doing so, we are
not only able to simulate longer periods in less time, but also gain
repeatability by reducing the float-point arithmetic errors that a variable
time-step introduces.

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

Synchronous vs Asynchronous mode
--------------------------------

> TODO
