# Scenarios

Scenarios are how an autonomous driving stack is trained and tested: background traffic that behaves like real drivers, and specific, repeatable situations (a cut-in, a pedestrian stepping out, a vehicle running a red light) to check how the stack reacts. CARLA provides several options to build them, from the built-in Traffic Manager to scenario languages and benchmarks. This page is an overview of the options available, to help decide which is the best fit for your use case.

| Option | Needs installing | How a scenario is written | Best for |
| --- | --- | --- | --- |
| [Traffic Manager](#traffic-manager) | no, it is part of CARLA | Python API calls on individual vehicles | Background traffic, and ego testing against it |
| [Traffic Agents](#traffic-agents) | no, it is part of the Python API | a Python class that returns a control each step | Driving one vehicle along a route with your own logic |
| [ScenarioRunner](#scenariorunner-and-openscenario) | yes, separately | Python classes or OpenSCENARIO `.xosc` | Specific, repeatable maneuvers with several vehicles |
| [Scenic](#scenic) | yes, separately | Scenic, a probabilistic language | Many variations of one situation, sampled |
| [CARLA Leaderboard](#carla-leaderboard) | yes, separately | routes with scenario trigger points | Scoring a complete driving agent |
| [Inverted AI](#inverted-ai) | an API key and the `invertedai` package | a shipped example script | Learned, human-like traffic |

!!! Note
    SUMO and PTV-Vissim co-simulation were part of the Unreal Engine 4.26 line. They are not in the Unreal Engine 5 builds.

- [__Traffic Manager__](#traffic-manager)
- [__Traffic Agents__](#traffic-agents)
- [__ScenarioRunner and OpenSCENARIO__](#scenariorunner-and-openscenario)
- [__Scenic__](#scenic)
- [__CARLA Leaderboard__](#carla-leaderboard)
- [__Inverted AI__](#inverted-ai)

---

## Traffic Manager

[__Traffic Manager__](core_traffic_manager.md) is a module within CARLA that controls certain vehicles in a simulation from the client side. Vehicles are registered to Traffic Manager via the [`carla.Vehicle.set_autopilot`](python_api.md#carla.Vehicle.set_autopilot) method or [`command.SetAutopilot`](python_api.md#command.SetAutopilot) class. Control of each vehicle is managed through a cycle of [distinct stages](core_traffic_manager.md#stages-of-the-control-loop), each running on its own thread.

__Useful for:__

- Populating a simulation with realistic urban traffic conditions. 
- [Customizing traffic behaviours](tuto_traffic_manager.md) to set specific learning circumstances.
- Reproducible runs: a seed (`set_random_device_seed`) plus synchronous mode makes the traffic deterministic.

!!! agent "Agentic CARLA"
    *"Spawn 50 vehicles on autopilot, make them drive 20% under the speed limit and keep 3 m to the car ahead."*

    Skills: `spawn-vehicles`, `control-traffic`. [Set up Agentic CARLA](agent_skills.md#set-up-agentic-carla).

<div class="build-buttons">
<p>
<a href="../core_traffic_manager" class="btn btn-neutral" title="Go to Traffic Manager">
Go to Traffic Manager</a>
</p>
</div>


---

## Traffic Agents

[__Traffic Agents__](core_agents.md), known until now simply as *agents*, are Python classes that drive one vehicle each: built around a vehicle you have spawned, they plan a route to a destination, follow it, stop for traffic lights and react to what is in the way, returning a [`carla.VehicleControl`](python_api.md#carla.VehicleControl) every step. Where the Traffic Manager drives many vehicles with one shared logic, an agent drives one vehicle with logic you can read and change. Three ship with CARLA (`BasicAgent`, `BehaviorAgent` and `ConstantVelocityAgent`), and any of them can be subclassed.

__Useful for:__

- Driving a vehicle to a chosen destination, for example a lead vehicle or the ego in a data-collection run.
- Prototyping your own planning and control logic on top of a working route follower.
- Repeatable runs: `ConstantVelocityAgent` holds a fixed speed whatever the traffic does.

!!! agent "Agentic CARLA"
    *"Spawn a vehicle and drive it to the other side of the map with a cautious BehaviorAgent."*

    Skills: `navigate-to`, `spawn-vehicles`. [Set up Agentic CARLA](agent_skills.md#set-up-agentic-carla).

<div class="build-buttons">
<p>
<a href="../core_agents" class="btn btn-neutral" title="Go to Traffic Agents">
Go to Traffic Agents</a>
</p>
</div>


---

## ScenarioRunner and OpenSCENARIO

ScenarioRunner ships 96 ready-to-run scenario configurations and also lets you [write your own](scenario_runner.md#write-your-own-scenario), in Python, in the [ASAM OpenSCENARIO 1.x standard](https://releases.asam.net/OpenSCENARIO/1.0.0/ASAM_OpenSCENARIO_BS-1-2_User-Guide_V1-0-0.html#_foreword) or in OpenSCENARIO 2.0.

The primary use of OpenSCENARIO is the description of complex maneuvers that involve multiple vehicles. Users can see which features of OpenSCENARIO are supported by ScenarioRunner [here](https://scenario-runner.readthedocs.io/en/latest/openscenario_support/). These features include Maneuvers, Actions, Conditions, Stories and the Storyboard.

ScenarioRunner is [its own repository](https://github.com/carla-simulator/scenario_runner) and has to be installed separately. Use the `ue58-dev` branch, which is the one for this CARLA; `ue5-master` was for 0.10.0 and `master` targets the 0.9.x API of the Unreal Engine 4.26 line and does not work here.

__Useful for:__

- Scripting a specific situation (a cut-in, a pedestrian stepping out, a vehicle running a light) and replaying it against an agent as many times as needed.
- Writing those situations in OpenSCENARIO, so they can be exchanged with other simulators.
- Defining bespoke [metrics](https://scenario-runner.readthedocs.io/en/latest/metrics_module/) that run against a recording of the scenario, instead of simulating it again.
- Running routes: driving an ego along a route and triggering the scenarios placed along it, the same mechanism the [CARLA Leaderboard](#carla-leaderboard) uses.

!!! agent "Agentic CARLA"
    *"Install the ScenarioRunner that matches my CARLA and run the FollowLeadingVehicle scenario, then summarise the results."*

    Skills: `install-scenario-runner`, `run-scenario`, `run-openscenario`, `create-scenario`, `analyze-scenario-results`. [Set up Agentic CARLA](agent_skills.md#set-up-agentic-carla).

<div class="build-buttons">
<p>
<a href="../scenario_runner" class="btn btn-neutral" title="Go to ScenarioRunner">
Go to ScenarioRunner</a>
</p>
</div>


---

## Scenic

[Scenic](scenic.md) is a domain-specific probabilistic programming language for modeling the environments of cyber-physical systems like robots and autonomous cars. Scenic provides a [specialized domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.html) to facilitate execution of Scenic scripts on the CARLA simulator.

Scenic scenario definitions are easy to read and construct. A tutorial to create a simple scenario is provided [here](scenic.md).

__Useful for:__

- Generating multiple, diverse scenarios with a single scenario definition.
- Defining probabilistic policies for dynamic agents to take actions over time in response to the state of the world.

!!! agent "Agentic CARLA"
    *"Install Scenic and write a scenario where a car cuts in from the right lane in front of the ego, then run it."*

    Skills: `install-scenic`, `create-scenic-scenario`, `run-scenic-scenario`. [Set up Agentic CARLA](agent_skills.md#set-up-agentic-carla).

<div class="build-buttons">
<p>
<a href="../scenic" class="btn btn-neutral" title="Go to Scenic">
Go to Scenic</a>
</p>
</div>


---

## CARLA Leaderboard

The [CARLA Leaderboard](leaderboard.md) evaluates a complete driving agent: the agent drives a set of routes, each with scenarios triggered along the way, and is scored on route completion and infractions (collisions, red lights, stop signs, leaving the route). It is installed separately, and on `ue58-dev` it carries its own copy of the scenario engine, so it needs no ScenarioRunner checkout.

__Useful for:__

- Measuring an agent end to end with the same metrics as the public benchmark.
- Building your own route sets, with scenario trigger points placed where you need them.

<div class="build-buttons">
<p>
<a href="../leaderboard" class="btn btn-neutral" title="Go to the CARLA Leaderboard">
Go to the CARLA Leaderboard</a>
</p>
</div>

---

## Inverted AI

[Inverted AI](inverted_ai.md) drives the traffic with models learned from real driving data, through a hosted service, so the surrounding vehicles behave the way human drivers do rather than following hand-written rules. It needs an API key and the `invertedai` package; `PythonAPI/examples/invertedai_traffic.py` is a working example.

__Useful for:__

- Realistic, human-like background traffic, especially in dense junctions.
- Testing an ego against behavior that was not written by the person running the test.

<div class="build-buttons">
<p>
<a href="../inverted_ai" class="btn btn-neutral" title="Go to Inverted AI">
Go to Inverted AI</a>
</p>
</div>
