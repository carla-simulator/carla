# Aimsun Next Co-Simulation

CARLA also supports co-simulation with **Aimsun Next**, enabling advanced traffic modeling capabilities combined with CARLA’s realistic vehicle dynamics. This integration allows simulation of vehicle behavior in both microscopic and detailed physics environments.

*   [__Requisites__](#requisites)  
*   [__Run a co-simulation__](#run-the-co-simulation)  
	*   [Create a new network](#create-a-new-network)  

---

## Requisites

To run the co-simulation, make sure the following are in place:

*   A licensed version of [__Aimsun Next__](https://www.aimsun.com/aimsun-next/).  
*   A properly configured CARLA setup.  
*   The Python co-simulation controller (`aimsun_eai_controller.py`) must be located inside CARLA’s PythonAPI folder (e.g., `examples/`).  
*   Modify line 117 of the script to match the location of the CARLA map being used (`.xodr` file).  
*   Set the correct vehicle types in Aimsun Next External Agent API Interface under the Scenario -> API menu (see the powerpoint file in the co-simulation folder)
---

## Run a co-simulation

Everything related to this feature can be found in the provided folder, including a sample Aimsun network.

To run a co-simulation:

1. Open Aimsun and load the network file:  
   ```bash
   example_network.ang
   ```
2. Launch the CARLA simulator.  
3. In Aimsun, start the simulation by running **Replication 442**.  
4. Immediately after, run the controller script from a terminal or your IDE:  
   ```bash
   python3 aimsun_eai_controller.py
   ```
5. The co-simulation will begin, exchanging data between Aimsun and CARLA in real time.

!!! Note
    All Python scripts must reside within CARLA’s directory (e.g., under `CARLA/PythonAPI/examples/`). The Aimsun network can be located elsewhere.

### Map Synchronization

In the script `aimsun_eai_controller.py`, on line 117, make sure to set the correct `.xodr` file path that matches your CARLA environment:

```python
map_path = "your_path_to_map/new19.xodr"
```

### Create a new network

If you want to use your own network:

*   Build or import your road network in Aimsun.  
*   Export or set it up similarly to `example_network.ang`.  
*   Ensure the External Agent Interface (EAI) is enabled.  
*   Use the existing script or adapt it for new replication IDs or vehicle configurations.

---

That’s all you need to get started with Aimsun Next co-simulation in CARLA.

If anything is unclear, feel free to reach out or check CARLA’s community discussions.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
