# Agent skills

CARLA ships a library of 62 vetted procedures for coding agents, under [`Skills/`](https://github.com/carla-simulator/carla/tree/ue58-dev/Skills) in the repository. An agent working inside a CARLA checkout finds them automatically and follows them instead of improvising a procedure of its own.

Each skill is a directory holding a `SKILL.md` — YAML frontmatter (`name`, `description`, `compatibility` and `metadata`) followed by the step-by-step procedure — plus the `scripts/` and `references/` it needs. The procedures encode the failure modes that make the obvious approach fail, which is the point of them: they are the source of truth for these tasks, not a summary of the docs.

In practice this means an agent asked to "package CARLA with only Town10" reads `package-carla-ue58` and follows it, instead of guessing at CMake targets and cook options.

- [__Connect your agent__](#connect-your-agent)
- [__What is covered__](#what-is-covered)
- [__Using them__](#using-them)
- [__Writing a skill__](#writing-a-skill)

---
## Connect your agent

Most procedures in this documentation can be handed to an AI coding agent: installing CARLA, building it, running a server, spawning traffic and sensors, importing maps and vehicles, writing and running scenarios. Throughout the pages, a box like this one marks those procedures, with a prompt to give your agent and the skills it will follow:

!!! agent "Ask your agent"
    *"Download CARLA 1.0, install its Python client and start a server on Town10."*

    Skills: `download-carla`, `install-python-api`, `run-carla-ue58-server`.

The manual steps stay below each box, for those who prefer to run them by hand or want to know what the agent does.

To give your agent the skills, use one of these:

* __Inside a CARLA checkout__, nothing to do. Agents that read [`AGENTS.md`](https://github.com/carla-simulator/carla/blob/ue58-dev/AGENTS.md) find the library, and the root `.mcp.json` starts the CARLA MCP server for MCP clients.
* __As a plugin__, with no CARLA checkout at all. In Claude Code:

```
/plugin marketplace add carla-simulator/carla-agentic-tools
/plugin install carla@carla-agentic-tools
```

In Codex, run `codex plugin marketplace add carla-simulator/carla-agentic-tools`; in Cursor, add the same marketplace from __Customize__ in the sidebar.

* __As an MCP server registered by path__, from a CARLA checkout, so that agents working in other projects can use it (needs Node 12 or later):

```sh
claude mcp add carla -s user -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
codex mcp add carla -- node /path/to/carla/Util/AgenticTools/bin/carla-agentic-tools.js
```

Starting from nothing, ask for the download first: the `download-carla`, `install-python-api` and `run-carla-ue58-server` skills fetch a release, install the matching client and start the server, and the agent records where CARLA is for the next tasks.

---
## What is covered

| Group | Skills | Covers |
|---|---|---|
| `setup` | 6 | Downloading CARLA, installing the Python API, Scenario Runner, Leaderboard and Scenic, and checking what a given install supports |
| `python-api` | 23 | Driving a running server: spawning, sensors, traffic, weather, maps, recording, debug drawing |
| `ue58` | 8 | Building, packaging and running UE 5.8, importing maps, props and walkers, authoring vehicles, and running Autoware |
| `ue4` | 7 | The same for the UE 4.26 branch |
| `ue5` | 1 | UE 5.5 feature-support checks |
| `scenario-runner` | 5 | Writing and running scenarios, including OpenSCENARIO, and reading the results |
| `leaderboard` | 5 | Writing, packaging and evaluating a Leaderboard agent |
| `scenic` | 2 | Authoring and running Scenic scenarios |
| `ros2` | 3 | The native ROS 2 interface: publishers, message types, RViz |
| `docs` | 2 | Auditing the documentation for gaps and writing the pages that fill them |

---
## Using them

Agents that read [`AGENTS.md`](https://github.com/carla-simulator/carla/blob/ue58-dev/AGENTS.md) pick the library up on their own — that file carries the full catalogue and the repository conventions. No setup is needed beyond working inside the checkout.

The same library is also served over MCP by [`Util/AgenticTools/`](https://github.com/carla-simulator/carla/tree/ue58-dev/Util/AgenticTools), registered for the project by `.mcp.json`, which runs it with Node. The MCP tools add what plain file reading cannot do:

- `list_skills` — the catalogue, with each skill marked available or not depending on whether the checkout or install it needs is configured
- `read_skill` — a skill's `SKILL.md`, prefixed with its absolute directory so the relative `scripts/...` paths in it resolve
- `check_prerequisites` — runs that skill's read-only environment check and reports what is missing
- `get_config` / `set_config` — the configured paths (`CARLA_ROOT` and the engine-specific variables derived from it), persisted between sessions, with the CARLA installs found on this machine offered as candidates

Paths are asked for when a task first needs one, never guessed: several CARLA checkouts on one machine is normal, and the wrong one fails slowly.

---
## Writing a skill

Follow the conventions of the existing ones: a single `SKILL.md` with frontmatter, a `scripts/check_env.sh` named by `metadata.prerequisites` when the skill needs an environment, and a `references/` file for anything too long for the procedure itself. Keep the gotchas in the document — a skill that omits why the naive approach fails saves nobody the day that mistake costs.
