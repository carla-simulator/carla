---
name: check-carla-ports
description: Reports which network ports CARLA and its tooling use on this machine and which are already taken — the simulator's RPC/streaming/secondary trio, the Traffic Manager, the Unreal MCP server, Alpamayo, NuRec and Cosmos — and names the process holding each one. Use before starting a second simulator or Traffic Manager, when a bind error appears ("failed to create because of bind error", "CARLA server failed to start on RPC port"), when a client connects to the wrong instance, or to pick a free port for a parallel run.
license: MIT
compatibility: Any Linux with bash and `ss` (iproute2); `lsof` works for everything but the full listing. NO CARLA checkout and NO running server required. Port defaults read from ue58-dev HEAD — RPC 2000, streaming RPC+1, secondary RPC+2, Traffic Manager 8000, Unreal MCP 6256, Cosmos 8010.
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: ../../../Docs/ref_ports.md
---

# Which ports does CARLA use, and what is holding them?

> **Paths.** `scripts/…` below is relative to the directory holding this SKILL.md.
> Your working directory is the user's project, not that directory, so prefix
> them with its absolute path or the command is not found.

The failure this prevents: **a bind error is read as a broken install.** CARLA's components do not search for a free port when theirs is taken. The simulator exits, and the Traffic Manager retries the same port twenty times and then raises. Both messages describe the symptom, neither names the process at fault — and on a developer machine the culprit is usually another CARLA, an editor, or a tool that defaults to the same number.

[`Docs/ref_ports.md`](../../../Docs/ref_ports.md) is the full reference. This skill answers the live question: what is in use right now.

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh     (only fails with no ss and no lsof)
- [ ] Step 2: check — the CARLA ports and who holds them
- [ ] Step 3: act — free the port, or move the service that wants it
```

### Step 1 — prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2 — check

```bash
bash scripts/ports.sh check
```

Prints one row per CARLA port with its expected role and the process actually listening:

```
PORT    STATE      ROLE                                          LISTENER
2000    IN USE     CARLA RPC server (-carla-rpc-port)            UnrealEditor 48250
2001    IN USE     CARLA sensor streaming (RPC+1)                UnrealEditor 48250
2002    IN USE     CARLA multi-GPU secondary router (RPC+2)      UnrealEditor 48250
8000    free       Traffic Manager RPC                           -
6256    IN USE     Unreal MCP server (editor only)               UnrealEditor 48250
8010    free       NVIDIA Cosmos example server                  -
8767    free       Alpamayo 2 Super model service                -
```

__Read the LISTENER column, not the ROLE column.__ ROLE is what CARLA *expects* on that port; LISTENER is what is really there. A mismatch between them is the bug, and `check` calls out the common one — an editor serving MCP on the Traffic Manager's `8000`.

Other subcommands:

| Command | Does |
| --- | --- |
| `ports.sh check` | the CARLA ports only (default) |
| `ports.sh list` | every listening TCP port on the machine, CARLA roles named |
| `ports.sh free PORT` | exits `0` if free, `1` if taken; for scripting |
| `ports.sh role PORT` | what CARLA uses that port for |

Non-default bases come from the environment, so a server on another port still reports correctly:

```bash
PORT=3654 TM_PORT=8050 bash scripts/ports.sh check
```

### Step 3 — act

Pick by which side is easier to move.

| Symptom | Fix |
| --- | --- |
| `CARLA server failed to start on RPC port N` | Close the other simulator, or launch with `-carla-rpc-port=M`. Streaming and secondary follow at `M+1` and `M+2`, so space parallel servers at least 3 apart |
| `trying to create rpc server for traffic manager; but the system failed to create because of bind error.` | Free the port, or pass `--traffic-manager-port M` / `client.get_trafficmanager(M)`. The Traffic Manager never falls back by itself |
| A client attaches to a simulator it should not | Two servers share an RPC port, or the client's port is wrong. `ports.sh list` shows every one |
| `unreal-mcp` fails with `ECONNREFUSED` | The editor is not running, or it predates the move to `6256`. Restart it and reconnect with `/mcp` |

__Never free a port by killing a process you did not start.__ An editor on these ports usually holds someone's unsaved level. Report what holds it and let the user decide.

## Gotchas

*   __The Traffic Manager lives in a client, not in the simulator.__ The first client to call `get_trafficmanager(port)` becomes its server; later clients on the same port attach to it. A free `8000` with vehicles already on autopilot means they are driven from another process.
*   __`ports.sh` only sees TCP.__ ROS 2 is DDS over UDP, and its ports come from the domain id (`7400 + 250 × domain`), not from a port argument. Separate two ROS 2 stacks with `ROS_DOMAIN_ID`, not with ports.
*   __A `0.0.0.0` bind is reachable from the network; `127.0.0.1` is not.__ The simulator binds all interfaces, the Unreal MCP server only localhost.
*   __Unreal's own ports are not CARLA's.__ `1981`/`1989` are UnrealTraceServer and `1985`/`8558` are Zen Server; they appear whenever the editor runs and are nothing to do with a CARLA bind error.
