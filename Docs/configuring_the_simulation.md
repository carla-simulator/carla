<h1>Configuring the simulation</h1>

Before you start running your own experiments there are few details to take into
account at the time of configuring your simulation. In this document we cover
the most important ones.

Fixed time-step
---------------

The time-step is the _simulation-time_ elapsed between two steps of the
simulation. In video-games, this _simulation-time_ is almost always adjusted to
real time for better realism. This is achieved by having a **variable
time-step** that adjusts the simulation to keep up with real-time. In
simulations however, it is better to detach the _simulation-time_ from
real-time, and let the simulation run as fast as possible using a **fixed
time-step**. Doing so, we are not only able to simulate longer periods in less
time, but also gain repeatability by reducing the float-point arithmetic errors
that a variable time-step introduces.

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
run the simulation at a fixed time-step of 0.1 seconds we execute

    $ ./CarlaUE4.sh -benchmark -fps=10

It is important to note that this mode can only be enabled when launching the
simulator since this is actually a feature of Unreal Engine.

!!! important
    **Do not decrease the frame-rate below 10 FPS.**<br>
    Our settings are adjusted to clamp the physics engine to a minimum of 10
    FPS. If the game tick falls below this, the physics engine will still
    simulate 10 FPS. In that case, things dependent on the game's delta time are
    no longer in sync with the physics engine.
    Ref. [#695](https://github.com/carla-simulator/carla/issues/695)


Changing the map
----------------

The map can be selected by passing the path to the map as first argument when
launching the simulator

```sh
# Linux
./CarlaUE4.sh /Game/Carla/Maps/Town01
```

```cmd
rem Windows
CarlaUE4.exe /Game/Carla/Maps/Town01
```

The path "/Game/" maps to the Content folder of our repository in
"Unreal/CarlaUE4/Content/".

Running off-screen
------------------

In Linux, you can force the simulator to run off-screen by setting the
environment variable `DISPLAY` to empty

!!! important
    **DISPLAY= only works with OpenGL**<br>
    Vulkan is now the default graphics API used by Unreal Engine and CARLA on
    Linux. Unreal Engine currently crashes when Vulkan is used when running off-screen.
    Therefore the -opengl flag must be added to force the engine to use OpenGL instead.
    We hope that this issue is addressed by Epic in the near future.

```sh
# Linux
DISPLAY= ./CarlaUE4.sh -opengl
```

This launches the simulator without simulator window, of course you can still
connect to it normally and run the example scripts. Note that with this method,
in multi-GPU environments, it's not possible to select the GPU that the
simulator will use for rendering. To do so, follow the instruction in
[Running without display and selecting GPUs](carla_headless.md).

No-rendering mode
-----------------

It is possible to completely disable rendering in the simulator by enabling
_no-rendering mode_ in the world settings. This way is possible to simulate
traffic and road behaviours at very high frequencies without the rendering
overhead. Note that in this mode, cameras and other GPU-based sensors return
empty data.

```py
settings = world.get_settings()
settings.no_rendering_mode = True
world.apply_settings(settings)
```

Synchronous mode
----------------

!!! important
    **Always run the simulator at fixed time-step when using the synchronous
    mode**. Otherwise the physics engine will try to recompute at once all the
    time spent waiting for the client, this usually results in inconsistent or
    not very realistic physics.

The client-simulator communication can be synchronized by using the _synchronous
mode_. When the synchronous mode is enabled, the simulation is halted each
update until a _tick_ message is received.

This is very useful when dealing with slow client applications, as the simulator
waits until the client is ready to continue. This mode can also be used to
synchronize data among sensors by waiting until all the data is received. Note
that data coming from GPU-based sensors (cameras) is usually generated with a
delay of a couple of frames respect to data coming from CPU-based sensors.

The synchronous mode can be enabled at any time in the world settings.

```py
# Example: Synchronizing a camera with synchronous mode.

settings = world.get_settings()
settings.synchronous_mode = True
world.apply_settings(settings)

camera = world.spawn_actor(blueprint, transform)
image_queue = queue.Queue()
camera.listen(image_queue.put)

while True:
    world.tick()
    timestamp = world.wait_for_tick()

    image = image_queue.get()
```

Other command-line options
--------------------------

  * `-carla-port=N` Listen for client connections at port N, streaming port is set to N+1.
  * `-quality-level={Low,Epic}` Change graphics quality level, "Low" mode runs significantly faster.
  * `-carla-server-timeout=10000ms` Set server timeout.
  * `-no-rendering` Disable rendering.
  * [Full list of UE4 command-line arguments][ue4clilink].

[ue4clilink]: https://docs.unrealengine.com/en-US/Programming/Basics/CommandLineArguments
