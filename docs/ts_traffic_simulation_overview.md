# Traffic Simulation in CARLA

Traffic simulation is integral to the accurate and efficient training and testing of autonomous driving stacks. CARLA provides a number of different options to simulate traffic and specific traffic scenarios. This section is an overview of the options available to help decide which is the best fit for your use case.

- [__Scenario Runner and OpenScenario__](#scenario-runner-and-openscenario)
- [__Traffic Manager__](#traffic-manager)
- [__Scenic__](#scenic)
- [__SUMO__](#sumo)

---

## Scenario Runner and OpenScenario

Scenario Runner provides [predefined traffic scenarios](https://carla-scenariorunner.readthedocs.io/en/latest/list_of_scenarios/) out of the box and also allows users to [define their own](https://carla-scenariorunner.readthedocs.io/en/latest/creating_new_scenario/) scenarios using either Python or the [OpenSCENARIO 1.0 standard](https://releases.asam.net/OpenSCENARIO/1.0.0/ASAM_OpenSCENARIO_BS-1-2_User-Guide_V1-0-0.html#_foreword).

The primary use of OpenSCENARIO is the description of complex maneuvers that involve multiple vehicles. Users can see which features of OpenSCENARIO are supported by Scenario Runner [here](https://carla-scenariorunner.readthedocs.io/en/latest/openscenario_support/). These features include Maneuvers, Actions, Conditions, Stories and the Storyboard. 

Scenario Runner has to be installed [separately](https://github.com/carla-simulator/scenario_runner) from the main CARLA package.

__Useful for:__

- Creating complex traffic scenarios and routes to prepare AD agents for evaluation in the [CARLA leaderboard](https://leaderboard.carla.org/).
- Defining bespoke [metrics](https://carla-scenariorunner.readthedocs.io/en/latest/metrics_module/) that can be run against recordings of the scenario simulation, foregoing the need to run simulations repeatedly.

<div class="build-buttons">
<p>
<a href="https://carla-scenariorunner.readthedocs.io" target="_blank" class="btn btn-neutral" title="Go to Scenario Runner">
Go to Scenario Runner</a>
</p>
</div>

---

## Traffic Manager

Traffic Manager is a module within CARLA that controls certain vehicles in a simulation from the client side. Vehicles are registered to Traffic Manager via the [`carla.Vehicle.set_autopilot`](https://carla.readthedocs.io/en/latest/python_api/#carla.Vehicle.set_autopilot) method or [`command.SetAutopilot`](https://carla.readthedocs.io/en/latest/python_api/#commandsetautopilot) class. Control of each vehicle is managed through a cycle of [distinct stages](adv_traffic_manager.md#stages) which each run on a different thread.

__Useful for:__

- Populating a simulation with realistic urban traffic conditions. 
- [Customizing traffic behaviours](adv_traffic_manager.md#general-considerations) to set specific learning circumstances.
- Developing phase-related functionalities and data structures while improving computational efficiency.

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/latest/adv_traffic_manager/" target="_blank" class="btn btn-neutral" title="Go to Traffic Manager">
Go to Traffic Manager</a>
</p>
</div>

---

## Scenic

[Scenic](https://scenic-lang.readthedocs.io) is a domain-specific probabilistic programming language for modeling the environments of cyber-physical systems like robots and autonomous cars. Scenic provides an [specialized domain](https://scenic-lang.readthedocs.io/en/latest/modules/scenic.simulators.carla.html) to facilitate execution of Scenic scripts on the CARLA simulator. 

Scenic scenario definitions are easy to read and construct. A tutorial to create a simple scenario is provided [here](tuto_G_scenic.md).

__Useful for:__

- Generating multiple, diverse scenarios with a single scenario definition.
- Defining probabilistic policies for dynamic agents to take actions over time in response to the state of the world.

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/latest/tuto_G_scenic/" target="_blank" class="btn btn-neutral" title="Go to Scenic Tutorial">
Go to Scenic Tutorial</a>
</p>
</div>

---

## SUMO

[SUMO](https://sumo.dlr.de/docs/SUMO_at_a_Glance.html) is an open source, microscopic, multi-modal traffic simulation. In SUMO, each vehicle is modelled explicitly, has its own route, and moves individually through the network. Simulations are deterministic by default but there are various options for introducing randomness. 

CARLA provides a co-simulation feature with SUMO that allows distribution of tasks between the two simulators. Vehicles can be spawned in CARLA through SUMO, and managed by SUMO much as the Traffic Manager would do. 

__Useful for:__

- Exploitation of the capabilities of both CARLA and SUMO in one package.

<div class="build-buttons">
<p>
<a href="https://carla.readthedocs.io/en/latest/adv_sumo/" target="_blank" class="btn btn-neutral" title="Go to SUMO Co-Simulation">
Go to SUMO Co-Simulation</a>
</p>
</div>

---

If you have any doubts about the different options available to simulate traffic in CARLA, feel free to post in the forum or in [Discord](https://discord.gg/8kqACuC).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

