# Network ports

Every TCP and UDP port CARLA and its tooling open, what listens on it, and how to move it.

Nothing here shares a default with anything else, which took moving two of them off `8000`: the Unreal MCP server to `6256` and the Cosmos example server to `8010`, since the Traffic Manager was there first. The documentation preview still defaults to `8000`, so do not run `mkdocs serve` and a Traffic Manager on one machine without moving one of them.

!!! agent "Agentic CARLA"
    *"Which CARLA ports are in use on this machine, and what is holding them? I want to start a second simulator."*

    Skills: `check-carla-ports`. [Set up Agentic CARLA](agent_skills.md#set-up-agentic-carla).

## Simulator

| Port | Service | Bound by | Change it with |
| --- | --- | --- | --- |
| `2000` | RPC server — everything [carla.Client](python_api.md#carla.Client) calls | the simulator | `-carla-rpc-port=N`, `-world-port=N` or `-carla-world-port=N`; `[CARLA/Server] RPCPort` or `WorldPort` in the settings file |
| `2001` | Sensor data streaming | the simulator | `-carla-streaming-port=N`; `[CARLA/Server] StreamingPort`. Defaults to RPC + 1, and `0` makes the server pick any free port |
| `2002` | Multi-GPU secondary router — where secondary servers connect in | a primary simulator | `-carla-secondary-port=N`; `[CARLA/Server] SecondaryPort`. Defaults to RPC + 2 |
| `2002` | The primary's router, as dialled by a secondary | a secondary simulator | `-carla-primary-port=N` with `-carla-primary-host=IP`; `[CARLA/Server] PrimaryPort` |

The three simulator ports move together: `-carla-rpc-port=3654` also shifts streaming to `3655` and the secondary router to `3656`, unless a more specific flag overrides one. Three apart is therefore the minimum spacing between two simulators on one machine.

Every simulator opens all three, secondaries included, even though no client talks to a secondary's RPC port. [Multi-GPU](core_multigpu.md) gives each secondary a round RPC port a thousand apart — `3000` and `4000`, claiming `3000`-`3002` and `4000`-`4002` — which leaves room to read a port number and know which server it belongs to.

A port already in use shuts the simulator down at startup rather than aborting the editor, with:

```
ERROR: CARLA server failed to start on RPC port 2000: <reason>. Another CARLA server is probably running on this port -- close it or launch with a different -carla-rpc-port.
```

See [Multi-GPU](core_multigpu.md) for the primary/secondary topology.

## Traffic Manager

| Port | Service | Bound by | Change it with |
| --- | --- | --- | --- |
| `8000` | Traffic Manager RPC | the first client to call [carla.Client.get_trafficmanager](python_api.md#carla.Client.get_trafficmanager) | the `port` argument; `--tm-port` or `--traffic-manager-port` in the scripts that take one |

The Traffic Manager runs inside a client process, not inside the simulator, and the first client to ask for one on a given port becomes its server. Later clients naming the same port attach to that instance instead of starting their own, which is how several scripts share one Traffic Manager.

If the port is taken by anything else, construction retries 20 times at 500 ms apart on the same port and then raises:

```
RuntimeError: trying to create rpc server for traffic manager; but the system failed to create because of bind error.
```

It never falls back to another port, so the fix is to free `8000` or to name a different one. Give each simulator its own Traffic Manager port when running more than one.

See [Traffic Manager](core_traffic_manager.md).

## ROS 2

CARLA's native ROS 2 interface speaks DDS, which derives its UDP ports from the domain id rather than taking a port directly. Set the domain with `--ros-domain-id=N` or the `ROS_DOMAIN_ID` environment variable; unset keeps the middleware's own default of `0`.

For domain `d` and participant `p`, RTPS uses:

| Purpose | Port |
| --- | --- |
| Discovery, multicast | `7400 + 250d` |
| Discovery, unicast | `7400 + 250d + 10 + 2p` |
| User traffic, multicast | `7400 + 250d + 1` |
| User traffic, unicast | `7400 + 250d + 11 + 2p` |

So domain `0` occupies `7400`-`7500` and domain `1` starts at `7650`. Two stacks that must not see each other need different domain ids, not different ports.

See [Native ROS 2 interface](ros2_native.md).

## Tooling

| Port | Service | Notes |
| --- | --- | --- |
| `6256` | Unreal MCP server, inside the editor | Editor only, bound to `127.0.0.1`. Set by `ServerPortNumber` in `Unreal/CarlaUnreal/Config/DefaultEditorPerProjectUserSettings.ini`, overridden with `-ModelContextProtocolPort=N`. Change it and update `.mcp.json` to match. See [Agentic CARLA skills](agent_skills.md) |
| `8767` | Alpamayo 2 Super model service | `127.0.0.1` by default; `--model-host` and `--model-port`. See [Alpamayo 2 Super](nvidia_alpamayo.md) |
| auto | NuRec neural rendering, gRPC to the NRE container | A free port is picked unless `--nurec-port`/`-np` is given. See [NVIDIA NuRec](nvidia_nurec.md) |
| `8010` | NVIDIA Cosmos example server | `--port` or `COSMOS_PORT`; the client reads `COSMOS_URL`. Always `8000` *inside* the container, published to `8010` on the host. See [NVIDIA Cosmos](nvidia_cosmos.md) |
| `8000` | `mkdocs serve` documentation preview | `-a 127.0.0.1:N`. See [Documentation standard](cont_doc_standard.md) |

Unreal Engine itself opens more while the editor runs, none of them CARLA's: `1981` and `1989` for UnrealTraceServer, `1985` and `8558` for Zen Server.

## Checking what is in use

```sh
ss -ltnp | grep -E ':(2000|2001|2002|6256|8000|8010|8767)\b'
```

The `check-carla-ports` skill does this and names the role of each listener it finds.
