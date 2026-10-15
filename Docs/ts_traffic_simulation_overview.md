# Traffic Simulation in CARLA

Traffic simulation is integral to the accurate and efficient training and testing of autonomous driving stacks. CARLA provides a number of different options to simulate traffic and specific traffic scenarios. This section is an overview of the options available to help decide which is the best fit for your use case.

| Option | Needs installing | How a scenario is written | Best for |
| --- | --- | --- | --- |
| [Traffic Manager](#traffic-manager) | no, it is part of CARLA | Python API calls on individual vehicles | Background traffic, and ego testing against it |
| [ScenarioRunner](#scenariorunner-and-openscenario) | yes, separately | Python classes or OpenSCENARIO `.xosc` | Specific, repeatable maneuvers with several vehicles |
| [Scenic](#scenic) | yes, separately | Scenic, a probabilistic language | Many variations of one situation, sampled |
| [Inverted AI](#inverted-ai) | yes, with an API key | Python API, through their service | Learned, human-like traffic |

- [__Traffic Manager__](#traffic-manager)
- [__ScenarioRunner and OpenSCENARIO__](#scenariorunner-and-openscenario)
- [__Scenic__](#scenic)
- [__Inverted AI__](#inverted-ai)

---

## Traffic Manager

[__Traffic Manager__](adv_traffic_manager.md) is a module within CARLA that controls certain vehicles in a simulation from the client side. Vehicles are registered to Traffic Manager via the [`carla.Vehicle.set_autopilot`](python_api.md#carla.Vehicle.set_autopilot) method or [`command.SetAutopilot`](python_api.md#command.SetAutopilot) class. Control of each vehicle is managed through a cycle of [distinct stages](adv_traffic_manager.md#stages-of-the-control-loop), each running on its own thread.

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

ScenarioRunner has to be installed [separately](https://github.com/carla-simulator/scenario_runner) from the main CARLA package.

!!! Note
    Match the branch to the CARLA line: the UE 5 branch is `ue5-master`, and it is a partial port — it runs on `Town10HD_Opt`, and only a subset of the scenario configurations has been brought across. `master` targets the 0.9.x (UE 4.26) API and will not work here.

__Useful for:__

- Creating complex traffic scenarios and routes to prepare AD agents for evaluation in the [CARLA leaderboard](https://leaderboard.carla.org/).
- Defining bespoke [metrics](https://scenario-runner.readthedocs.io/en/latest/metrics_module/) that can be run against recordings of the scenario simulation, foregoing the need to run simulations repeatedly.

<div class="build-buttons">
<p>
<a href="https://scenario-runner.readthedocs.io" target="_blank" class="btn btn-neutral" title="Go to ScenarioRunner">
Go to ScenarioRunner</a>
</p>
</div>

---

## Scenic

[Scenic](https://scenic-lang.readthedocs.io) is a domain-specific probabilistic programming language for modeling the environments of cyber-physical systems like robots and autonomous cars. Scenic provides a [specialized domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.html) to facilitate execution of Scenic scripts on the CARLA simulator.

Scenic scenario definitions are easy to read and construct. A tutorial to create a simple scenario is provided [here](tuto_G_scenic.md).

__Useful for:__

- Generating multiple, diverse scenarios with a single scenario definition.
- Defining probabilistic policies for dynamic agents to take actions over time in response to the state of the world.

<div class="build-buttons">
<p>
<a href="../tuto_G_scenic" class="btn btn-neutral" title="Go to Scenic Tutorial">
Go to Scenic Tutorial</a>
</p>
</div>

---

## Inverted AI

[Inverted AI](inverted_ai.md) drives the traffic with models learned from real driving data, through a hosted service, so the surrounding vehicles behave the way human drivers do rather than following hand-written rules. It needs an API key and the `invertedai` package; `PythonAPI/examples/invertedai_traffic.py` is a working example.

__Useful for:__

- Realistic, human-like background traffic, especially in dense junctions.
- Testing an ego against behavior that was not written by the person running the test.

---

## Co-simulation

CARLA's SUMO and PTV-Vissim co-simulation are part of the UE 4.26 line and have not been ported to this one.

---

If you have any doubts about the different options available to simulate traffic in CARLA, feel free to post in the forum or in [Discord](https://discord.gg/8kqACuC).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

