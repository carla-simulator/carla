# Traffic Simulation in CARLA

Traffic simulation is integral to the accurate and efficient training and testing of autonomous driving stacks. CARLA provides a number of different options to simulate traffic and specific traffic scenarios. This section is an overview of the options available to help decide which is the best fit for your use case.

| Option | Needs installing | How a scenario is written | Best for |
| --- | --- | --- | --- |
| [Traffic Manager](#traffic-manager) | no, it is part of CARLA | Python API calls on individual vehicles | Background traffic, and ego testing against it |
| [ScenarioRunner](#scenariorunner-and-openscenario) | yes, separately | Python classes or OpenSCENARIO `.xosc` | Specific, repeatable maneuvers with several vehicles |
| [Scenic](#scenic) | yes, separately | Scenic, a probabilistic language | Many variations of one situation, sampled |
| [Inverted AI](#inverted-ai) | an API key and the `invertedai` package | a shipped example script | Learned, human-like traffic |

!!! Note
    SUMO and PTV-Vissim co-simulation were part of the Unreal Engine 4.26 line. They are not in the Unreal Engine 5 builds.

- [__Traffic Manager__](#traffic-manager)
- [__ScenarioRunner and OpenSCENARIO__](#scenariorunner-and-openscenario)
- [__Scenic__](#scenic)
- [__Inverted AI__](#inverted-ai)

---

## Traffic Manager

[__Traffic Manager__](adv_traffic_manager.md) is a module within CARLA that controls certain vehicles in a simulation from the client side. Vehicles are registered to Traffic Manager via the [`carla.Vehicle.set_autopilot`](python_api.md#carla.Vehicle.set_autopilot) method or [`command.SetAutopilot`](python_api.md#command.SetAutopilot) class. Control of each vehicle is managed through a cycle of [distinct stages](adv_traffic_manager.md#stages-of-the-control-loop), each running on its own thread.

To drive a vehicle along a route with your own logic instead, use the [CARLA agents](adv_agents.md).

__Useful for:__

- Populating a simulation with realistic urban traffic conditions. 
- [Customizing traffic behaviours](tuto_G_traffic_manager.md) to set specific learning circumstances.
- Reproducible runs: a seed (`set_random_device_seed`) plus synchronous mode makes the traffic deterministic.

<div class="build-buttons">
<p>
<a href="../adv_traffic_manager" class="btn btn-neutral" title="Go to Traffic Manager">
Go to Traffic Manager</a>
</p>
</div>


---

## ScenarioRunner and OpenSCENARIO

ScenarioRunner provides [predefined traffic scenarios](https://scenario-runner.readthedocs.io/en/latest/list_of_scenarios/) out of the box and also allows users to [define their own](https://scenario-runner.readthedocs.io/en/latest/creating_new_scenario/) scenarios using either Python or the [OpenSCENARIO 1.0 standard](https://releases.asam.net/OpenSCENARIO/1.0.0/ASAM_OpenSCENARIO_BS-1-2_User-Guide_V1-0-0.html#_foreword).

The primary use of OpenSCENARIO is the description of complex maneuvers that involve multiple vehicles. Users can see which features of OpenSCENARIO are supported by ScenarioRunner [here](https://scenario-runner.readthedocs.io/en/latest/openscenario_support/). These features include Maneuvers, Actions, Conditions, Stories and the Storyboard.

ScenarioRunner is [its own repository](https://github.com/carla-simulator/scenario_runner) and has to be installed separately. Use the `ue5-master` branch, which is the one for Unreal Engine 5; `master` targets the 0.9.x API of the Unreal Engine 4.26 line and does not work here.

__Useful for:__

- Scripting a specific situation — a cut-in, a pedestrian stepping out, a vehicle running a light — and replaying it against an agent as many times as needed.
- Writing those situations in OpenSCENARIO, so they can be exchanged with other simulators.
- Defining bespoke [metrics](https://scenario-runner.readthedocs.io/en/latest/metrics_module/) that run against a recording of the scenario, instead of simulating it again.
- Running routes: the [CARLA Leaderboard](https://leaderboard.carla.org/) is built on ScenarioRunner, so an agent evaluated there is being driven through it.

<div class="build-buttons">
<p>
<a href="https://scenario-runner.readthedocs.io" target="_blank" class="btn btn-neutral" title="Go to ScenarioRunner">
Go to ScenarioRunner</a>
</p>
</div>


---

## Scenic

[Scenic](https://scenic-lang.readthedocs.io) is a domain-specific probabilistic programming language for modeling the environments of cyber-physical systems like robots and autonomous cars. Scenic provides a [specialized domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.html) to facilitate execution of Scenic scripts on the CARLA simulator.

Scenic scenario definitions are easy to read and construct. A tutorial to create a simple scenario is provided [here](scenic.md).

__Useful for:__

- Generating multiple, diverse scenarios with a single scenario definition.
- Defining probabilistic policies for dynamic agents to take actions over time in response to the state of the world.

<div class="build-buttons">
<p>
<a href="../scenic" class="btn btn-neutral" title="Go to Scenic Tutorial">
Go to Scenic Tutorial</a>
</p>
</div>


---

## Inverted AI

[Inverted AI](inverted_ai.md) drives the traffic with models learned from real driving data, through a hosted service, so the surrounding vehicles behave the way human drivers do rather than following hand-written rules. It needs an API key and the `invertedai` package; `PythonAPI/examples/invertedai_traffic.py` is a working example.

__Useful for:__

- Realistic, human-like background traffic, especially in dense junctions.
- Testing an ego against behavior that was not written by the person running the test.
