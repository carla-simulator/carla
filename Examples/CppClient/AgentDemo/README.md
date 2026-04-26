# AgentDemo — driveability test for the C++ navigation agents

End-to-end demo of the `carla::agents::navigation` module ported from
`PythonAPI/carla/agents/navigation/` (issue
[carla-simulator/carla#9554](https://github.com/carla-simulator/carla/issues/9554)).

Spawns a random vehicle, picks a faraway spawn point as the destination,
wraps the vehicle with `BehaviorAgent` (Normal profile by default) and
ticks the agent until it reports `Done()` — applying the resulting
`VehicleControl` to the vehicle each step. The spectator camera follows
in chase mode so the run is visible in the simulator window.

## Build (in-tree)

The demo is registered as the `carla-agent-demo` CMake target alongside
the other CppClient examples.

```bash
cd /path/to/carla
cmake -B Build -DBUILD_CARLA_CLIENT=ON -DBUILD_PYTHON_API=OFF
cmake --build Build --target carla-agent-demo -j
```

## Run

A CARLA server must be reachable on the chosen host:port.

```bash
# defaults — localhost:2000, current map, Normal behavior
./Build/Examples/carla-agent-demo

# custom endpoint
./Build/Examples/carla-agent-demo 192.168.1.10 2000

# choose map + behavior
./Build/Examples/carla-agent-demo localhost 2000 Town01 cautious
./Build/Examples/carla-agent-demo localhost 2000 Town03 aggressive
```

Behavior values: `cautious` / `normal` (default) / `aggressive` — they
map to `BehaviorParameters::Cautious()`/`::Normal()`/`::Aggressive()`.

Press `Ctrl-C` to stop early; the demo destroys the vehicle and exits
cleanly.

## Expected output

```
Client 0.10.0  ↔  Server 0.10.0
Map: /Game/Carla/Maps/Town01
Spawned vehicle.tesla.model3 at (88.5, -45.2, 0.30)
Destination: (245.6, 12.4, 0.30)
Behavior:    normal
Agent route set; entering control loop. Ctrl-C to stop.
  step 50   throttle=0.62 brake=0 steer=-0.04   speed=18.2 km/h
  step 100  throttle=0.55 brake=0 steer=0.12    speed=22.7 km/h
  ...
Agent reached destination after 1240 steps.
Vehicle destroyed.
```

## Standalone build

Same flow as `Examples/CppClient/Standalone/` — install CARLA to a
prefix, then build outside the source tree:

```bash
# install CARLA once
cmake --install Build --prefix /opt/carla

# build the demo standalone
cd Examples/CppClient/AgentDemo
cmake -B build -DCMAKE_PREFIX_PATH=/opt/carla
cmake --build build -j
./build/carla-agent-demo
```
