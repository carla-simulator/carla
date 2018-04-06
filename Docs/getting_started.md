<h1>Getting started with CARLA</h1>

![Welcome to CARLA](img/welcome.png)

Welcome to CARLA! This tutorial provides the basic steps for getting started
using CARLA.

<!-- Latest release button -->
<p align="middle"><a href="https://github.com/carla-simulator/carla/releases/latest" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release"><span class="icon icon-github"></span> Get the latest release</a></p>

Download the latest release from our GitHub page and extract all the contents of
the package in a folder of your choice.

The release package contains the following

  * The CARLA simulator.
  * The "carla" Python module.
  * Some Python client examples.

For now we will focus on the simulator only. The simulator can be run in two
different modes

  * **Server mode:** The simulator is controlled by a client application that
    collects data and sends driving instructions. In this mode the simulator
    hangs until a client starts a connection.
  * **Standalone mode:** The simulator starts in sort of _video-game mode_ in
    which you can control the vehicle with the keyboard.

In the next item in this tutorial we will run the **standalone mode** to take a
first look into CARLA.
